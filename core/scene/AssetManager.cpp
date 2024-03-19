#include "AssetManager.h"
#include "../base/Stream.h"
#include "../base/FileSystem.h"
#include "Mesh.h"
#include "../material/Material.h"
#include "MeshSkin.h"
#include "../base/SerializerJson.h"
#include "../animation/Animation.h"
#include "material/Texture.h"
#include "material/Image.h"
#include "base/StringUtil.h"

using namespace mgp;

AssetManager *AssetManager_instance = NULL;

AssetManager *AssetManager::getInstance() {
  if (AssetManager_instance == NULL) {
    AssetManager_instance = new (AssetManager);
  }
  return AssetManager_instance;
}

void AssetManager::releaseInstance() {
  if (AssetManager_instance) {
    AssetManager_instance->clear();
    delete AssetManager_instance;
    AssetManager_instance = NULL;
  }
}

AssetManager::AssetManager() {
    setPath("res/assets");
}

AssetManager::~AssetManager() {

}

void AssetManager::setPath(const std::string& path) {
   this->path = path;
   FileSystem::mkdirs((path + "/mesh").c_str());
   FileSystem::mkdirs((path + "/skin").c_str());
   FileSystem::mkdirs((path + "/material").c_str());
   FileSystem::mkdirs((path + "/anim").c_str());
   FileSystem::mkdirs((path + "/image").c_str());
   FileSystem::mkdirs((path + "/texture").c_str());
}

const std::string& AssetManager::getPath() {
    return this->path;
}

void AssetManager::clear() {
  std::lock_guard<std::recursive_mutex> lock_guard(_mutex);
  for (int i=0; i<rt_count; ++i) {
    auto &map = resourceMap[i];
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (it->second) {
        it->second->release();
      }
    }
    map.clear();
  }
}

void AssetManager::beginSave() {
    std::lock_guard<std::recursive_mutex> lock_guard(_mutex);
    _saved.clear();
}

void AssetManager::remove(const std::string &name, ResType type) {
  std::lock_guard<std::recursive_mutex> lock_guard(_mutex);

  auto it = resourceMap[type].find(name);
  if (it != resourceMap[type].end()) {
      Refable* res = it->second;
      res->release();
      resourceMap[type].erase(it);
  }
}

UPtr<Resource> AssetManager::load(const std::string &name, ResType type, bool cache) {
    if (name.size() == 0) return UPtr<Resource>(NULL);

    std::lock_guard<std::recursive_mutex> lock_guard(_mutex);

    if (cache) {
        auto itr = this->resourceMap[type].find(name);
        if (itr != this->resourceMap[type].end()) {
            Resource* res = itr->second;
            bool ok = true;
            if (Image* texture = dynamic_cast<Image*>(res)) {
                if (!texture->getData()) {
                    res->release();
                    this->resourceMap->erase(itr);
                    ok = false;
                }
            }
            if (ok) {
                res->addRef();
                return UPtr<Resource>(res);
            }
        }
    }

    Resource* res = NULL;
    switch (type) {
        case rt_mesh: {
            std::string file = path + "/mesh/" + name + ".mesh";
            UPtr<Stream> s = FileSystem::open(file.c_str());
            Mesh *mesh = Mesh::create(VertexFormat(NULL, 0)).take();
            mesh->read(s.get());
            mesh->setId(name);
            s->close();
            //delete s;
            res = mesh;
            break;
        }
        case rt_skin: {
            std::string file = path + "/skin/" + name + ".skin";
            UPtr<Stream> s = FileSystem::open(file.c_str());
            MeshSkin* mesh = new MeshSkin();
            mesh->read(s.get());
            mesh->setId(name);
            s->close();
            //delete s;
            res = mesh;
            break;
        }
        case rt_materail: {
            std::string file = path + "/material/" + name + ".material";
            auto stream = SerializerJson::createReader(file);
            Material* m = dynamic_cast<Material*>(stream->readObject(nullptr).take());
            stream->close();
            m->setId(name);
            res = m;
            break;
        }
        case rt_animation: {
            std::string file = path + "/anim/" + name + ".anim";
            UPtr<Stream> s = FileSystem::open(file.c_str());
            Animation* mesh = new Animation("");
            mesh->read(s.get());
            mesh->setId(name);
            s->close();
            //delete s;
            res = mesh;
            break;
        }
        case rt_texture: {
            std::string file = path + "/texture/" + name + ".texture";
            auto stream = SerializerJson::createReader(file);
            Texture* m = dynamic_cast<Texture*>(stream->readObject(nullptr).take());
            stream->close();
            m->setId(name);
            res = m;
            break;
        }
    }
    if (cache && res) {
        res->addRef();
        this->resourceMap[type][name] = res;
    }
    return UPtr<Resource>(res);
}

void AssetManager::save(Resource* res) {
    if (res == NULL) return;
    std::string name = res->getId();
    if (name.size() == 0) return;

    if (StringUtil::contains(name, "|")) {
        StringUtil::replace(name, "|", "_");
        res->setId(name);
    }

    std::lock_guard<std::recursive_mutex> lock_guard(_mutex);

    if (_saved[name]) return;

    if (Mesh* mesh = dynamic_cast<Mesh*>(res)) {
        std::string file = path + "/mesh/" + name + ".mesh";
        UPtr<Stream> s = FileSystem::open(file.c_str(), FileSystem::WRITE);
        mesh->write(s.get());
        s->close();
        //delete s;
    }
    else if (MeshSkin* mesh = dynamic_cast<MeshSkin*>(res)) {
        std::string file = path + "/skin/" + name + ".skin";
        UPtr<Stream> s = FileSystem::open(file.c_str(), FileSystem::WRITE);
        mesh->write(s.get());
        s->close();
        //delete s;
    }
    else if (Material* m = dynamic_cast<Material*>(res)) {
        std::string file = path + "/material/" + name + ".material";
        auto stream = SerializerJson::createWriter(file);
        stream->writeObject(nullptr, m);
        stream->close();
    }
    else if (Animation* mesh = dynamic_cast<Animation*>(res)) {
        std::string file = path + "/anim/" + name + ".anim";
        UPtr<Stream> s = FileSystem::open(file.c_str(), FileSystem::WRITE);
        mesh->write(s.get());
        s->close();
        //delete s;
    }
    else if (Texture* texture = dynamic_cast<Texture*>(res)) {
        std::string file = path + "/texture/" + name + ".texture";
        auto stream = SerializerJson::createWriter(file);
        stream->writeObject(nullptr, texture);
        stream->close();
    }
    else {
        GP_ERROR("ERROR: unknow resource\n");
    }

    _saved[name] = 1;
}


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
   //FileSystem::mkdirs((path + "/texture").c_str());
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

UPtr<Resource> AssetManager::load(const std::string &name, ResType type) {
    if (name.size() == 0) return UPtr<Resource>(NULL);

    std::lock_guard<std::recursive_mutex> lock_guard(_mutex);
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
        case rt_image: {
            if (StringUtil::endsWith(name, ".image")) {
                std::string file = path + "/image/" + name;
                UPtr<Stream> s = FileSystem::open(file.c_str());
                Image* m = new Image();
                m->read(s.get());
                m->setId(name);
                res = m;
            }
            else {
                std::string file = path + "/image/" + name;
                Image* m = Image::create(file.c_str()).take();
                m->setId(name);
                res = m;
            }
            break;
        }
    }
    if (res) {
        res->addRef();
        this->resourceMap[type][name] = res;
    }
    return UPtr<Resource>(res);
}

void AssetManager::save(Resource* res) {
    if (res == NULL) return;
    std::string name = res->getId();
    if (name.size() == 0) return;

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
    else if (Image* texture = dynamic_cast<Image*>(res)) {
        if (texture->getFilePath().size() > 0) {
            std::string dst = path + "/image/" + name;
            if (!FileSystem::fileExists(dst.c_str())) {
                std::string src = texture->getFilePath();
                FileSystem::copyFile(src.c_str(), dst.c_str());
            }
        }
        else if (StringUtil::endsWith(name, ".png")) {
            std::string file = path + "/image/" + name;
            texture->save(file.c_str(), "png");
        }
        else if (StringUtil::endsWith(name, ".jpg")) {
            std::string file = path + "/image/" + name;
            texture->save(file.c_str(), "jgp");
        }
        else if (StringUtil::endsWith(name, ".image")) {
            std::string file = path + "/image/" + name;
            UPtr<Stream> s = FileSystem::open(file.c_str(), FileSystem::WRITE);
            texture->write(s.get());
            s->close();
        }
        else {
            GP_ASSERT("Unknow image type:%s", name.c_str());
        }
    }
    else {
        GP_ERROR("ERROR: unknow resource\n");
    }

    _saved[name] = 1;
}


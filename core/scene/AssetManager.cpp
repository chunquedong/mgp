#include "AssetManager.h"
#include "../base/Stream.h"
#include "../base/FileSystem.h"
#include "Mesh.h"
#include "../material/Material.h"
#include "MeshSkin.h"
#include "../base/SerializerJson.h"
#include "../animation/Animation.h"

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

AssetManager::AssetManager(): path("res") {

}

AssetManager::~AssetManager() {

}

void AssetManager::clear() {
  std::lock_guard<std::mutex> lock_guard(mutex);
  for (int i=0; i<rt_count; ++i) {
    std::map<std::string, Refable*> &map = resourceMap[i];
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (it->second) {
        it->second->release();
      }
    }
    map.clear();
  }
}

void AssetManager::remove(const std::string &name, ResType type) {
  std::lock_guard<std::mutex> lock_guard(mutex);

  auto it = resourceMap[type].find(name);
  if (it != resourceMap[type].end()) {
      Refable* res = it->second;
      res->release();
      resourceMap[type].erase(it);
  }
}

UPtr<Refable> AssetManager::load(const std::string &name, ResType type) {
    if (name.size() == 0) return UPtr<Refable>(NULL);

    std::lock_guard<std::mutex> lock_guard(mutex);
    auto itr = this->resourceMap[type].find(name);
    if (itr != this->resourceMap[type].end()) {
        Refable* res = itr->second;
        res->addRef();
        return UPtr<Refable>(res);
    }
    Refable* res = NULL;
    switch (type) {
        case rt_mesh: {
            std::string file = path + "/" + name + ".mesh";
            UPtr<Stream> s = FileSystem::open(file.c_str());
            Mesh *mesh = Mesh::create(VertexFormat(NULL, 0)).take();
            mesh->read(s.get());
            mesh->setName(name);
            s->close();
            //delete s;
            res = mesh;
            break;
        }
        case rt_skin: {
            std::string file = path + "/" + name + ".skin";
            UPtr<Stream> s = FileSystem::open(file.c_str());
            MeshSkin* mesh = new MeshSkin();
            mesh->read(s.get());
            mesh->setName(name);
            s->close();
            //delete s;
            res = mesh;
            break;
        }
        case rt_materail: {
            std::string file = path + "/" + name + ".material";
            auto stream = SerializerJson::createReader(file);
            Material *m = dynamic_cast<Material*>(stream->readObject(nullptr));
            stream->close();
            m->setName(name);
            res = m;
            break;
        }
        case rt_animation: {
            std::string file = path + "/" + name + ".anim";
            UPtr<Stream> s = FileSystem::open(file.c_str());
            Animation* mesh = new Animation("");
            mesh->read(s.get());
            mesh->setName(name);
            s->close();
            //delete s;
            res = mesh;
            break;
        }
    }
    if (res) {
        res->addRef();
        this->resourceMap[type][name] = res;
    }
    return UPtr<Refable>(res);
}

void AssetManager::save(const std::string &name, Refable *res) {
     if (res == NULL) return;

     if (Mesh *mesh = dynamic_cast<Mesh*>(res)) {
        std::string file = path + "/" + name + ".mesh";
        UPtr<Stream> s = FileSystem::open(file.c_str(), FileSystem::WRITE);
        mesh->write(s.get());
        s->close();
        //delete s;
      }
      else if (MeshSkin *mesh = dynamic_cast<MeshSkin*>(res)) {
        std::string file = path + "/" + name + ".skin";
        UPtr<Stream> s = FileSystem::open(file.c_str(), FileSystem::WRITE);
        mesh->write(s.get());
        s->close();
        //delete s;
      }
      else if (Material *m = dynamic_cast<Material*>(res)) {
          std::string file = path + "/" + name + ".material";
          auto stream = SerializerJson::createWriter(file);
          stream->writeObject(nullptr, m);
          stream->close();
      }
      else if (Animation *mesh = dynamic_cast<Animation*>(res)) {
       std::string file = path + "/" + name + ".anim";
       UPtr<Stream> s = FileSystem::open(file.c_str(), FileSystem::WRITE);
       mesh->write(s.get());
       s->close();
       //delete s;
     }
}


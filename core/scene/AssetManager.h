#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "base/Base.h"
#include "base/Ref.h"
#include "base/Ptr.h"
#include <mutex>
#include "base/Resource.h"

namespace mgp
{
class AssetManager
{
public:
    enum ResType {
        rt_image,
        rt_materail,
        //rt_shaderProgram,
        rt_mesh,
        rt_animation,
        rt_skin,
        rt_count
      };
private:
    std::map<std::string, Resource*> resourceMap[rt_count];
    std::mutex mutex;
    std::string path;
    std::map<std::string, int> _saved;
public:
    static AssetManager *getInstance();
    static void releaseInstance();
private:
    AssetManager();
    ~AssetManager();

public:
    void setPath(const std::string& path);
    void clear();
    void beginSave();

    template<typename T>
    UPtr<T> load(const std::string &name, ResType type) {
        return load(name, type).dynamicCastTo<T>();
    }

    UPtr<Resource> load(const std::string &name, ResType type);

    void remove(const std::string &name, ResType type);

    void save(Resource*res);
};
}
#endif // ASSETMANAGER_H

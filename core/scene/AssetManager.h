#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "base/Base.h"
#include "base/Ref.h"
#include "base/Ptr.h"
#include <mutex>

namespace mgp
{
class AssetManager
{
public:
    enum ResType {
        rt_none,
        rt_texture,
        rt_materail,
        rt_shaderProgram,
        rt_mesh,
        rt_animation,
        rt_skin,
        rt_count
      };
private:
    std::map<std::string, Refable*> resourceMap[rt_count];
    std::mutex mutex;
    std::string path;
public:
    static AssetManager *getInstance();
    static void releaseInstance();
private:
    AssetManager();
    ~AssetManager();

public:
    void setPath(const std::string& path);
    void clear();

    template<typename T>
    UPtr<T> load(const std::string &name, ResType type) {
        return load(name, type).dynamicCastTo<T>();
    }

    UPtr<Refable> load(const std::string &name, ResType type);

    void remove(const std::string &name, ResType type);

    void save(const std::string &name, Refable *res);
};
}
#endif // ASSETMANAGER_H

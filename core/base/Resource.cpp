#include "Resource.h"
#include "System.h"
#include <time.h>

using namespace mgp;

std::string Resource::genId() {
    static int baseId = 0;
    if (baseId == 0) {
        srand(time(0));
        baseId = rand();
    }
    static int64_t lastTime = 0;
    static int seq = 0;

    int64_t id = System::currentTimeMillis();
    if (id == lastTime) {
        ++seq;
    }
    else {
        lastTime = id;
        seq = 0;
    }

    char buffer[128];
    snprintf(buffer, 128, "%d_%lld_%d", baseId, id, seq);
    return buffer;
}

Resource::Resource() : _id(genId()) {
}

Resource::Resource(const std::string& id) : _id(id) {
    if (_id.size() == 0) {
        _id = genId();
    }
}

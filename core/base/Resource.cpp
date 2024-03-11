#include "Resource.h"

#include <time.h>

using namespace mgp;

std::string Resource::genId() {
    static int baseId = 0;
    if (baseId == 0) {
        srand(time(0));
        baseId = rand();
    }
    int id = time(0);
    char buffer[128];
    snprintf(buffer, 128, "%d_%d", baseId, id);
    return buffer;
}

Resource::Resource() : _id(genId()) {
}

Resource::Resource(const std::string& id) : _id(id) {
    if (_id.size() == 0) {
        _id = genId();
    }
}

#include "Toolkit.h"

using namespace mgp;


Toolkit* Toolkit::g_instance;

Toolkit* Toolkit::cur() { return g_instance; }
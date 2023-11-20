#include "Script.h"
#include "ScriptController.h"

namespace mgp
{

Script::Script() : _scope(GLOBAL), _env(0)
{
}

Script::~Script()
{
    ScriptController::cur()->unloadScript(this);
}

const char* Script::getPath() const
{
    return _path.c_str();
}

Script::Scope Script::getScope() const
{
    return _scope;
}

bool Script::functionExists(const char* name) const
{
    return ScriptController::cur()->functionExists(name, this);
}

bool Script::reload()
{
    ScriptController* sc = ScriptController::cur();

    // First unload our current script
    sc->unloadScript(this);

    // Now attempt to reload the script
    return ScriptController::cur()->loadScript(this);
}

}

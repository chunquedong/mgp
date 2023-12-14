#include "base/Base.h"
#include "scene/Renderer.h"
#include "ShaderProgram.h"
#include "base/FileSystem.h"
#include "platform/Toolkit.h"

namespace mgp
{

// Cache of unique effects.
static std::map<std::string, ShaderProgram*> __effectCache;
//static ShaderProgram* __currentEffect = NULL;

ShaderProgram::ShaderProgram() : _program(0)
{
}

ShaderProgram::~ShaderProgram()
{
    // Remove this effect from the cache.
    __effectCache.erase(_id);

    // Free uniforms.
    for (std::map<std::string, Uniform*>::iterator itr = _uniforms.begin(); itr != _uniforms.end(); ++itr)
    {
        SAFE_DELETE(itr->second);
    }

    Renderer::cur()->deleteProgram(this);
}

ShaderProgram* ShaderProgram::createFromFile(const char* vshPath, const char* fshPath, const char* defines)
{
    GP_ASSERT(vshPath);
    GP_ASSERT(fshPath);

    // Search the effect cache for an identical effect that is already loaded.
    std::string uniqueId = vshPath;
    uniqueId += ';';
    uniqueId += fshPath;
    uniqueId += ';';
    if (defines)
    {
        uniqueId += defines;
    }
    std::map<std::string, ShaderProgram*>::const_iterator itr = __effectCache.find(uniqueId);
    if (itr != __effectCache.end())
    {
        // Found an exiting effect with this id, so increase its ref count and return it.
        GP_ASSERT(itr->second);
        itr->second->addRef();
        return itr->second;
    }

    // Read source from file.
    char* vshSource = FileSystem::readAll(vshPath);
    if (vshSource == NULL)
    {
        GP_ERROR("Failed to read vertex shader from file '%s'.", vshPath);
        return NULL;
    }
    char* fshSource = FileSystem::readAll(fshPath);
    if (fshSource == NULL)
    {
        GP_ERROR("Failed to read fragment shader from file '%s'.", fshPath);
        SAFE_DELETE_ARRAY(vshSource);
        return NULL;
    }

    ShaderProgram* effect = createFromSource(uniqueId.c_str(), vshPath, vshSource, fshPath, fshSource, defines);
    
    SAFE_DELETE_ARRAY(vshSource);
    SAFE_DELETE_ARRAY(fshSource);

    if (effect == NULL)
    {
        GP_ERROR("Failed to create effect from shaders '%s', '%s'.", vshPath, fshPath);
    }
    else
    {
        // Store this effect in the cache.
        effect->_id = uniqueId;
        __effectCache[uniqueId] = effect;
    }

    return effect;
}

ShaderProgram* ShaderProgram::createFromSource(const char* vshSource, const char* fshSource, const char* defines)
{
    return createFromSource("", NULL, vshSource, NULL, fshSource, defines);
}

static void replaceDefines(const char* defines, std::string& out)
{
    Properties* graphicsConfig = Toolkit::cur()->getConfig()->getNamespace("graphics", true);
    const char* globalDefines = graphicsConfig ? graphicsConfig->getString("shaderDefines") : NULL;

    // Build full semicolon delimited list of defines
    if (globalDefines && strlen(globalDefines) > 0)
    {
        if (out.length() > 0)
            out += ';';
        out += globalDefines;
    }
    if (defines && strlen(defines) > 0)
    {
        if (out.length() > 0)
            out += ';';
        out += defines;
    }

    if (out.size() > 0) {
        if (out[0] == ';') {
            out.erase(0, 1);
        }
        if (out.size() > 0 && out[out.size() - 1] == ';') {
            out.erase(out.size()-1, 1);
        }
    }

    // Replace semicolons
    if (out.length() > 0)
    {
        size_t pos;
        out.insert(0, "#define ");
        while ((pos = out.find(';')) != std::string::npos)
        {
            out.replace(pos, 1, "\n#define ");
        }
        out += "\n";
    }
}

static void replaceIncludes(const char* filepath, const char* source, std::string& out)
{
    // Replace the #include "xxxx.xxx" with the sourced file contents of "filepath/xxxx.xxx"
    std::string str = source;
    size_t lastPos = 0;
    size_t headPos = 0;
    size_t fileLen = str.length();
    size_t tailPos = fileLen;
    while (headPos < fileLen)
    {
        lastPos = headPos;
        if (headPos == 0)
        {
            // find the first "#include"
            headPos = str.find("#include");
        }
        else
        {
            // find the next "#include"
            headPos = str.find("#include", headPos + 1);
        }

        // If "#include" is found
        if (headPos != std::string::npos)
        {
            // append from our last position for the legth (head - last position) 
            out.append(str.substr(lastPos,  headPos - lastPos));

            // find the start quote "
            size_t startQuote = str.find("\"", headPos) + 1;
            if (startQuote == std::string::npos)
            {
                // We have started an "#include" but missing the leading quote "
                GP_ERROR("Compile failed for shader '%s' missing leading \".", filepath);
                return;
            }
            // find the end quote "
            size_t endQuote = str.find("\"", startQuote);
            if (endQuote == std::string::npos)
            {
                // We have a start quote but missing the trailing quote "
                GP_ERROR("Compile failed for shader '%s' missing trailing \".", filepath);
                return;
            }

            // jump the head position past the end quote
            headPos = endQuote + 1;
            
            // File path to include and 'stitch' in the value in the quotes to the file path and source it.
            std::string filepathStr = filepath;
            std::string directoryPath = filepathStr.substr(0, filepathStr.rfind('/') + 1);
            size_t len = endQuote - (startQuote);
            std::string includeStr = str.substr(startQuote, len);
            directoryPath.append(includeStr);
            const char* includedSource = FileSystem::readAll(directoryPath.c_str());
            if (includedSource == NULL)
            {
                GP_ERROR("Compile failed for shader '%s' invalid filepath.", filepathStr.c_str());
                return;
            }
            else
            {
                // Valid file so lets attempt to see if we need to append anything to it too (recurse...)
                replaceIncludes(directoryPath.c_str(), includedSource, out);
                SAFE_DELETE_ARRAY(includedSource);
            }
        }
        else
        {
            // Append the remaining
            out.append(str.c_str(), lastPos, tailPos);
        }
    }
}

ShaderProgram* ShaderProgram::createFromSource(const char* id, const char* vshPath, const char* vshSource, 
    const char* fshPath, const char* fshSource, const char* defines)
{
    GP_ASSERT(vshSource);
    GP_ASSERT(fshSource);

    Renderer::ProgramSrc src;

    // Replace all comma separated definitions with #define prefix and \n suffix
    std::string definesStr = "";
    replaceDefines(defines, definesStr);

    src.defines = definesStr.c_str();
    
    std::string vshSourceStr = "";
    if (vshPath)
    {
        // Replace the #include "xxxxx.xxx" with the sources that come from file paths
        replaceIncludes(vshPath, vshSource, vshSourceStr);
        if (vshSource && strlen(vshSource) != 0)
            vshSourceStr += "\n";
    }
    src.vshSource = vshPath ? vshSourceStr.c_str() :  vshSource;

    // Compile the fragment shader.
    std::string fshSourceStr;
    if (fshPath)
    {
        // Replace the #include "xxxxx.xxx" with the sources that come from file paths
        replaceIncludes(fshPath, fshSource, fshSourceStr);
        if (fshSource && strlen(fshSource) != 0)
            fshSourceStr += "\n";
    }
    src.fshSource = fshPath ? fshSourceStr.c_str() : fshSource;
    
    src.id = id;
    src.version = NULL;
    ShaderProgram* effect = Renderer::cur()->createProgram(&src);

    return effect;
}

const char* ShaderProgram::getId() const
{
    return _id.c_str();
}

VertexAttributeLoc ShaderProgram::getVertexAttribute(const char* name) const
{
    std::map<std::string, VertexAttributeLoc>::const_iterator itr = _vertexAttributes.find(name);
    return (itr == _vertexAttributes.end() ? -1 : itr->second);
}

Uniform* ShaderProgram::getUniform(const char* name) const
{
    std::map<std::string, Uniform*>::const_iterator itr = _uniforms.find(name);

	if (itr != _uniforms.end()) {
		// Return cached uniform variable
		return itr->second;
	}

    size_t len = strlen(name);
    if (len>3 && name[len-1] == ']' && name[len - 3] == '[') {
        // Check for array uniforms ("u_directionalLightColor[0]" -> "u_directionalLightColor")
        std::string parentname = name;
        parentname = parentname.substr(0, len-3);
        std::map<std::string, Uniform*>::const_iterator itr = _uniforms.find(parentname);

        if (itr != _uniforms.end()) {
            return itr->second;
        }
    }

    /*
    GLint uniformLocation;
    GL_ASSERT( uniformLocation = glGetUniformLocation(_program, name) );
    if (uniformLocation > -1)
	{
		// Check for array uniforms ("u_directionalLightColor[0]" -> "u_directionalLightColor")
		char* parentname = new char[strlen(name)+1];
		strcpy(parentname, name);
		if (strtok(parentname, "[") != NULL) {
			std::map<std::string, Uniform*>::const_iterator itr = _uniforms.find(parentname);
			if (itr != _uniforms.end()) {
				Uniform* puniform = itr->second;

				Uniform* uniform = new Uniform();
				uniform->_shaderProgram = const_cast<ShaderProgram*>(this);
				uniform->_name = name;
				uniform->_location = uniformLocation;
				uniform->_index = 0;
				uniform->_type = puniform->getType();
				_uniforms[name] = uniform;

				SAFE_DELETE_ARRAY(parentname);
				return uniform;
			}
		}
		SAFE_DELETE_ARRAY(parentname);
    }
    */

	// No uniform variable found - return NULL
	return NULL;
}

Uniform* ShaderProgram::getUniform(unsigned int index) const
{
    unsigned int i = 0;
    for (std::map<std::string, Uniform*>::const_iterator itr = _uniforms.begin(); itr != _uniforms.end(); ++itr, ++i)
    {
        if (i == index)
        {
            return itr->second;
        }
    }
    return NULL;
}

unsigned int ShaderProgram::getUniformCount() const
{
    return (unsigned int)_uniforms.size();
}

void ShaderProgram::bind()
{
    Renderer::cur()->bindProgram(this);
}

/*ShaderProgram* ShaderProgram::getCurrentEffect()
{
    return __currentEffect;
}*/

Uniform::Uniform() :
    _location(-1), _type(0), _index(0), _effect(NULL), _size(1)
{
}

Uniform::~Uniform()
{
    // hidden
}

ShaderProgram* Uniform::getEffect() const
{
    return _effect;
}

const char* Uniform::getName() const
{
    return _name.c_str();
}

const unsigned int Uniform::getType() const
{
    return _type;
}

bool Uniform::isSampler2d() const {
    return getType() == 0x8B5E;
}

}

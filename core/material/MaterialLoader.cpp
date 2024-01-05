#include "base/Base.h"
#include "Material.h"
#include "base/FileSystem.h"
#include "ShaderProgram.h"
#include "base/Properties.h"
#include "scene/Node.h"
#include "MaterialParameter.h"

namespace mgp
{
    static bool isMaterialKeyword(const char* str)
    {
        GP_ASSERT(str);

#define MATERIAL_KEYWORD_COUNT 3
        static const char* reservedKeywords[MATERIAL_KEYWORD_COUNT] =
        {
            "vertexShader",
            "fragmentShader",
            "defines"
        };
        for (unsigned int i = 0; i < MATERIAL_KEYWORD_COUNT; ++i)
        {
            if (strcmp(reservedKeywords[i], str) == 0)
            {
                return true;
            }
        }
        return false;
    }

    static Texture::Filter parseTextureFilterMode(const char* str, Texture::Filter defaultValue)
    {
        if (str == NULL || strlen(str) == 0)
        {
            GP_ERROR("Texture filter mode string must be non-null and non-empty.");
            return defaultValue;
        }
        else if (strcmp(str, "NEAREST") == 0)
        {
            return Texture::NEAREST;
        }
        else if (strcmp(str, "LINEAR") == 0)
        {
            return Texture::LINEAR;
        }
        else if (strcmp(str, "NEAREST_MIPMAP_NEAREST") == 0)
        {
            return Texture::NEAREST_MIPMAP_NEAREST;
        }
        else if (strcmp(str, "LINEAR_MIPMAP_NEAREST") == 0)
        {
            return Texture::LINEAR_MIPMAP_NEAREST;
        }
        else if (strcmp(str, "NEAREST_MIPMAP_LINEAR") == 0)
        {
            return Texture::NEAREST_MIPMAP_LINEAR;
        }
        else if (strcmp(str, "LINEAR_MIPMAP_LINEAR") == 0)
        {
            return Texture::LINEAR_MIPMAP_LINEAR;
        }
        else
        {
            GP_ERROR("Unsupported texture filter mode string ('%s').", str);
            return defaultValue;
        }
    }

    static Texture::Wrap parseTextureWrapMode(const char* str, Texture::Wrap defaultValue)
    {
        if (str == NULL || strlen(str) == 0)
        {
            GP_ERROR("Texture wrap mode string must be non-null and non-empty.");
            return defaultValue;
        }
        else if (strcmp(str, "REPEAT") == 0)
        {
            return Texture::REPEAT;
        }
        else if (strcmp(str, "CLAMP") == 0)
        {
            return Texture::CLAMP;
        }
        else
        {
            GP_ERROR("Unsupported texture wrap mode string ('%s').", str);
            return defaultValue;
        }
    }

    void loadRenderState(Material* renderState, Properties* properties)
    {
        GP_ASSERT(renderState);
        GP_ASSERT(properties);

        // Rewind the properties to start reading from the start.
        properties->rewind();

        const char* name;
        while ((name = properties->getNextProperty()))
        {
            if (isMaterialKeyword(name))
                continue; // keyword - skip

            switch (properties->getType())
            {
            case Properties::NUMBER:
                GP_ASSERT(renderState->getParameter(name));
                renderState->getParameter(name)->setFloat(properties->getFloat());
                break;
            case Properties::VECTOR2:
            {
                Vector2 vector2;
                if (properties->getVector2(NULL, &vector2))
                {
                    GP_ASSERT(renderState->getParameter(name));
                    renderState->getParameter(name)->setVector2(vector2);
                }
            }
            break;
            case Properties::VECTOR3:
            {
                Vector3 vector3;
                if (properties->getVector3(NULL, &vector3))
                {
                    GP_ASSERT(renderState->getParameter(name));
                    renderState->getParameter(name)->setVector3(vector3);
                }
            }
            break;
            case Properties::VECTOR4:
            {
                Vector4 vector4;
                if (properties->getVector4(NULL, &vector4))
                {
                    GP_ASSERT(renderState->getParameter(name));
                    renderState->getParameter(name)->setVector4(vector4);
                }
            }
            break;
            case Properties::MATRIX:
            {
                Matrix matrix;
                if (properties->getMatrix(NULL, &matrix))
                {
                    GP_ASSERT(renderState->getParameter(name));
                    renderState->getParameter(name)->setMatrix(matrix);
                }
            }
            break;
            default:
            {
                // Assume this is a parameter auto-binding.
                //renderState->setParameterAutoBinding(name, properties->getString());
            }
            break;
            }
        }

        // Iterate through all child namespaces searching for samplers and render state blocks.
        Properties* ns;
        while ((ns = properties->getNextNamespace()))
        {
            if (strcmp(ns->getNamespace(), "sampler") == 0)
            {
                // Read the texture uniform name.
                name = ns->getId();
                if (strlen(name) == 0)
                {
                    GP_ERROR("Texture sampler is missing required uniform name.");
                    continue;
                }

                // Get the texture path.
                std::string path;
                if (!ns->getPath("path", &path))
                {
                    GP_ERROR("Texture sampler '%s' is missing required image file path.", name);
                    continue;
                }

                // Read texture state (booleans default to 'false' if not present).
                bool mipmap = ns->getBool("mipmap");
                Texture::Wrap wrapS = parseTextureWrapMode(ns->getString("wrapS"), Texture::REPEAT);
                Texture::Wrap wrapT = parseTextureWrapMode(ns->getString("wrapT"), Texture::REPEAT);
                Texture::Wrap wrapR = Texture::REPEAT;
                if (ns->exists("wrapR"))
                {
                    wrapR = parseTextureWrapMode(ns->getString("wrapR"), Texture::REPEAT);
                }
                Texture::Filter minFilter = parseTextureFilterMode(ns->getString("minFilter"), mipmap ? Texture::NEAREST_MIPMAP_LINEAR : Texture::LINEAR);
                Texture::Filter magFilter = parseTextureFilterMode(ns->getString("magFilter"), Texture::LINEAR);

                // Set the sampler parameter.
                GP_ASSERT(renderState->getParameter(name));
                Texture* sampler = renderState->getParameter(name)->setSampler(path.c_str(), mipmap);
                if (sampler)
                {
                    sampler->setWrapMode(wrapS, wrapT, wrapR);
                    sampler->setFilterMode(minFilter, magFilter);
                }
            }
            else if (strcmp(ns->getNamespace(), "renderState") == 0)
            {
                while ((name = ns->getNextProperty()))
                {
                    GP_ASSERT(renderState->getStateBlock());
                    renderState->getStateBlock()->setState(name, ns->getString());
                }
            }
        }
    }
}
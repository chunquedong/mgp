#include "base/Base.h"
#include "scene/Renderer.h"
#include "VertexAttributeBinding.h"
#include "scene/Mesh.h"
#include "ShaderProgram.h"


// Graphics (GLSL)
#define VERTEX_ATTRIBUTE_POSITION_NAME              "a_position"
#define VERTEX_ATTRIBUTE_NORMAL_NAME                "a_normal"
#define VERTEX_ATTRIBUTE_COLOR_NAME                 "a_color"
#define VERTEX_ATTRIBUTE_TANGENT_NAME               "a_tangent"
#define VERTEX_ATTRIBUTE_BINORMAL_NAME              "a_binormal"
#define VERTEX_ATTRIBUTE_BLENDWEIGHTS_NAME          "a_blendWeights"
#define VERTEX_ATTRIBUTE_BLENDINDICES_NAME          "a_blendIndices"
#define VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME       "a_texCoord"


namespace mgp
{



VertexAttributeBinding::VertexAttributeBinding() {}
VertexAttributeBinding::~VertexAttributeBinding() {
    for (VertexAttributeObject* vao : _vaoList) {
        SAFE_RELEASE(vao);
    }
    _vaoList.clear();
}

UPtr<VertexAttributeBinding> VertexAttributeBinding::create(BufferHandle mesh, const VertexFormat& vertexFormat,
    void* vertexPointer, BufferHandle indexBufferObject)
{
    GP_ASSERT(mesh || vertexPointer);

    // Create a new VertexAttributeObject.
    VertexAttributeBinding* b = new VertexAttributeBinding();
    b->_vertexBufferObject = mesh;
    b->vertexFormat = vertexFormat;
    b->vertexPointer = vertexPointer;
    b->_indexBufferObject = indexBufferObject;
    b->_instanceBufferObject = 0;
    return UPtr<VertexAttributeBinding>(b);
}

VertexAttributeObject* VertexAttributeBinding::getVao(ShaderProgram* effect) {
    GP_ASSERT(effect);
    for (VertexAttributeObject* vao : _vaoList) {
        if (vao->_effect == effect) {
            return vao;
        }
    }
    VertexAttributeObject* vao = new VertexAttributeObject(this, effect);
    _vaoList.push_back(vao);
    return vao;
}

void VertexAttributeBinding::setVertexPointer(void* vertexPointer) {
    if (this->vertexPointer == vertexPointer) return;
    this->vertexPointer = vertexPointer;
    update();
}

void VertexAttributeBinding::update() {
    for (VertexAttributeObject* vao : _vaoList) {
        vao->_isDirty = true;
    }
}



VertexAttributeObject::VertexAttributeObject(VertexAttributeBinding* parent, ShaderProgram* effect) :
    _handle(0), _effect(effect), _vertexAttributeBinding(parent), _isDirty(true)
{
    effect->addRef();
}

VertexAttributeObject::~VertexAttributeObject()
{
    SAFE_RELEASE(_effect);
    Renderer::cur()->deleteVertexAttributeObj(this);
}

void VertexAttributeObject::bind() {
    if (_isDirty) {
        init();
    }
    Renderer::cur()->bindVertexAttributeObj(this);
    _isDirty = false;
}

void VertexAttributeObject::unbind() {
    Renderer::cur()->unbindVertexAttributeObj(this);
}

void VertexAttributeObject::init() {
    ShaderProgram* effect = _effect;
    GP_ASSERT(effect);

    // Create a new VertexAttributeObject.
    VertexAttributeObject* b = this;

    b->_attributes.clear();

    // Call setVertexAttribPointer for each vertex element.
    std::string name;
    for (size_t i = 0, count = _vertexAttributeBinding->vertexFormat.getElementCount(); i < count; ++i)
    {
        const VertexFormat::Element& e = _vertexAttributeBinding->vertexFormat.getElement(i);
        mgp::VertexAttributeLoc attrib = -1;

        if (e.name.size() > 0) {
            attrib = effect->getVertexAttribute(e.name.c_str());
        }

        if (attrib == -1) {

            // Constructor vertex attribute name expected in shader.
            switch (e.usage)
            {
            case VertexFormat::POSITION:
                attrib = effect->getVertexAttribute(VERTEX_ATTRIBUTE_POSITION_NAME);
                break;
            case VertexFormat::NORMAL:
                attrib = effect->getVertexAttribute(VERTEX_ATTRIBUTE_NORMAL_NAME);
                break;
            case VertexFormat::COLOR:
                attrib = effect->getVertexAttribute(VERTEX_ATTRIBUTE_COLOR_NAME);
                break;
            case VertexFormat::TANGENT:
                attrib = effect->getVertexAttribute(VERTEX_ATTRIBUTE_TANGENT_NAME);
                break;
            case VertexFormat::BINORMAL:
                attrib = effect->getVertexAttribute(VERTEX_ATTRIBUTE_BINORMAL_NAME);
                break;
            case VertexFormat::BLENDWEIGHTS:
                attrib = effect->getVertexAttribute(VERTEX_ATTRIBUTE_BLENDWEIGHTS_NAME);
                break;
            case VertexFormat::BLENDINDICES:
                attrib = effect->getVertexAttribute(VERTEX_ATTRIBUTE_BLENDINDICES_NAME);
                break;
            case VertexFormat::TEXCOORD0:
                if ((attrib = effect->getVertexAttribute(VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME)) != -1)
                    break;

            case VertexFormat::TEXCOORD1:
            case VertexFormat::TEXCOORD2:
            case VertexFormat::TEXCOORD3:
            case VertexFormat::TEXCOORD4:
            case VertexFormat::TEXCOORD5:
            case VertexFormat::TEXCOORD6:
            case VertexFormat::TEXCOORD7:
                name = VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME;
                name += '0' + (e.usage - VertexFormat::TEXCOORD0);
                attrib = effect->getVertexAttribute(name.c_str());
                break;
            default:
                // This happens whenever vertex data contains extra information (not an error).
                attrib = -1;
                break;
            }
        }

        if (attrib == -1)
        {
            //GP_WARN("Warning: Vertex element with usage '%s' in mesh '%s' does not correspond to an attribute in effect '%s'.", VertexFormat::toString(e.usage), mesh->getUrl(), effect->getId());
        }
        else
        {
            void* pointer = _vertexAttributeBinding->vertexPointer ? (void*)(((unsigned char*)_vertexAttributeBinding->vertexPointer) + e.offset) : (void*)e.offset;

            VertexAttribute attri;
            attri.enabled = true;
            attri.size = e.size;
            attri.type = e.dataType;//GL_FLOAT
            attri.normalized = 0;
            attri.stride = e.stride;
            attri.location = attrib;
            attri.pointer = pointer;

            b->_attributes.push_back(attri);
        }
    }
}

}

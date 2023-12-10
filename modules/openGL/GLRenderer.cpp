#include "GLRenderer.h"
#include "ogl.h"
#include "material/Material.h"
#include "scene/MeshBatch.h"
#include "GLFrameBuffer.h"
#include "scene/Drawable.h"
#include "base/FileSystem.h"
#include "platform/Toolkit.h"

/** @script{ignore} */
GLenum __gl_error_code = GL_NO_ERROR;

using namespace mgp;

GLRenderer::GLRenderer() {
  GLFrameBuffer::initialize();
}

GLRenderer::~GLRenderer() {
  GLFrameBuffer::finalize();
}

void GLRenderer::clear(ClearFlags flags, const Vector4& color, float clearDepth, int clearStencil) {
    GLbitfield bits = 0;
    if (flags & CLEAR_COLOR)
    {
        glClearColor(color.x, color.y, color.z, color.w);
        bits |= GL_COLOR_BUFFER_BIT;
    }

    if (flags & CLEAR_DEPTH)
    {
        glClearDepth(clearDepth);
        bits |= GL_DEPTH_BUFFER_BIT;

        // We need to explicitly call the static enableDepthWrite() method on StateBlock
        // to ensure depth writing is enabled before clearing the depth buffer (and to
        // update the global StateBlock render state to reflect this).
        enableDepthWrite();
    }

    if (flags & CLEAR_STENCIL)
    {
        glClearStencil(clearStencil);
        bits |= GL_STENCIL_BUFFER_BIT;
    }
    glClear(bits);

    //reset state
    //StateBlock state;
    updateState(&stateBlock, 2);
}

void GLRenderer::setViewport(int x, int y, int w, int h) {
    FrameBuffer *curFrameBuffer = getCurrentFrameBuffer();
    if (curFrameBuffer) {
        if (curFrameBuffer->isDefault()) {
            y = (Toolkit::cur()->getHeight() - y) - h;
        }
        else {
            y = (curFrameBuffer->getHeight() - y) - h;
        }
    }
    glViewport((GLuint)x, (GLuint)y, (GLuint)w, (GLuint)h);
}

static bool drawWireframe(DrawCall* drawCall)
{
    switch (drawCall->_primitiveType)
    {
    case Mesh::TRIANGLES:
    {
        unsigned int vertexCount = drawCall->_vertexCount;
        for (unsigned int i = 0; i < vertexCount; i += 3)
        {
            GL_ASSERT(glDrawArrays(GL_LINE_LOOP, i, 3));
        }
    }
    return true;

    case Mesh::TRIANGLE_STRIP:
    {
        unsigned int vertexCount = drawCall->_vertexCount;
        for (unsigned int i = 2; i < vertexCount; ++i)
        {
            GL_ASSERT(glDrawArrays(GL_LINE_LOOP, i - 2, 3));
        }
    }
    return true;

    default:
        // not supported
        return false;
    }
}

static bool drawWireframeIndexed(DrawCall* drawCall)
{
    unsigned int indexCount = drawCall->_indexCount;
    unsigned int indexSize = 0;
    switch (drawCall->_indexFormat)
    {
    /*case Mesh::INDEX8:
        indexSize = 1;
        break;*/
    case Mesh::INDEX16:
        indexSize = 2;
        break;
    case Mesh::INDEX32:
        indexSize = 4;
        break;
    default:
        GP_ERROR("Unsupported index format (%d).", drawCall->_indexFormat);
        return false;
    }

    switch (drawCall->_primitiveType)
    {
    case Mesh::TRIANGLES:
    {
        for (size_t i = 0; i < indexCount; i += 3)
        {
            GL_ASSERT(glDrawElements(GL_LINE_LOOP, 3, drawCall->_indexFormat, ((const GLvoid*)(i * indexSize))));
        }
    }
    return true;

    case Mesh::TRIANGLE_STRIP:
    {
        for (size_t i = 2; i < indexCount; ++i)
        {
            GL_ASSERT(glDrawElements(GL_LINE_LOOP, 3, drawCall->_indexFormat, ((const GLvoid*)((i - 2) * indexSize))));
        }
    }
    return true;

    default:
        // not supported
        return false;
    }
}

uint64_t GLRenderer::createBuffer(int type) {
    GLuint vbo;
    GL_ASSERT(glGenBuffers(1, &vbo));
    return vbo;
}

void GLRenderer::setBufferData(uint64_t buffer, int type, size_t startOffset, const char* data, size_t len, int usage) {
    GLuint vbo = (GLuint)buffer;
    int gltype = GL_ARRAY_BUFFER;
    if (type == 1) {
        gltype = GL_ELEMENT_ARRAY_BUFFER;
    }

    GL_ASSERT(glBindBuffer(gltype, vbo));

    if (startOffset) {
        GL_ASSERT(glBufferSubData(gltype, startOffset, len, data));
    }
    else {
        GL_ASSERT(glBufferData(gltype, len, data, usage ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
    }
}

void GLRenderer::deleteBuffer(uint64_t buffer) {
    if (!buffer) return;
    GLuint vbo = (GLuint)buffer;
    glDeleteBuffers(1, &vbo);
}

void GLRenderer::draw(DrawCall* drawCall) {
    Material* material = drawCall->_material;
    GL_ASSERT(material);
    material->bind();

    VertexAttributeObject* vao = drawCall->_vertexAttributeArray->getVao(material->getEffect());
    drawCall->_vertexAttributeArray->_instanceBufferObject = drawCall->_instanceVbo;
    vao->bind();

    if (drawCall->_instanceVbo) {
        GP_ASSERT(vao->getEbo());
        if (!drawCall->_wireframe || !drawWireframeIndexed(drawCall)) {
            GL_ASSERT(glDrawElementsInstanced(drawCall->_primitiveType, drawCall->_indexCount, drawCall->_indexFormat, (void*)drawCall->_indexBufferOffset, drawCall->_instanceCount));
        }
    }
    else if (vao->getEbo()) {
        if (!drawCall->_wireframe || !drawWireframeIndexed(drawCall)) {
            GL_ASSERT(glDrawElements(drawCall->_primitiveType, drawCall->_indexCount, drawCall->_indexFormat, (void*)drawCall->_indexBufferOffset));
        }
    }
    else if (drawCall->_indices) {
        GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        GL_ASSERT(glDrawElements(drawCall->_primitiveType, drawCall->_indexCount, drawCall->_indexFormat, (GLvoid*)drawCall->_indices));
    }
    else {
        GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        if (!drawCall->_wireframe || !drawWireframe(drawCall)) {
            GL_ASSERT(glDrawArrays(drawCall->_primitiveType, 0, drawCall->_vertexCount));
        }
    }
    vao->unbind();
    material->unbind();

    ++_drawCallCount;
}

void GLRenderer::enableDepthWrite()
{
    StateBlock* _defaultState = &stateBlock;

    // Internal method used by Game::clear() to restore depth writing before a
    // clear operation. This is necessary if the last code to draw before the
    // next frame leaves depth writing disabled.
    if (!_defaultState->_depthWriteEnabled)
    {
        GL_ASSERT(glDepthMask(GL_TRUE));
        _defaultState->_depthWriteEnabled = true;
    }
}

void GLRenderer::updateState(StateBlock* state, int force) {
    StateBlock* _defaultState = &stateBlock;

    // Update any state that differs from _defaultState and flip _defaultState bits
    if (force == 2 || ((force || (state->_bits & RS_BLEND)) && (state->_blendEnabled != _defaultState->_blendEnabled)))
    {
        if (state->_blendEnabled)
            GL_ASSERT(glEnable(GL_BLEND));
        else
            GL_ASSERT(glDisable(GL_BLEND));
        _defaultState->_blendEnabled = state->_blendEnabled;
    }
    if (force == 2 || ((force || (state->_bits & RS_BLEND_FUNC)) && 
        (state->_blendSrc != _defaultState->_blendSrc || state->_blendDst != _defaultState->_blendDst ||
        state->_blendSrcAlpha != _defaultState->_blendSrcAlpha || state->_blendDstAlpha != _defaultState->_blendDstAlpha)))
    {
        GL_ASSERT(glBlendFuncSeparate((GLenum)state->_blendSrc, (GLenum)state->_blendDst, (GLenum)state->_blendSrcAlpha, (GLenum)state->_blendDstAlpha));
        //GL_ASSERT(glBlendFunc((GLenum)state->_blendSrc, (GLenum)state->_blendDst));
        _defaultState->_blendSrc = state->_blendSrc;
        _defaultState->_blendDst = state->_blendDst;
        _defaultState->_blendSrcAlpha = state->_blendSrcAlpha;
        _defaultState->_blendDstAlpha = state->_blendDstAlpha;
    }
    if (force == 2 || ((force || (state->_bits & RS_CULL_FACE)) && (state->_cullFaceEnabled != _defaultState->_cullFaceEnabled)))
    {
        if (state->_cullFaceEnabled)
            GL_ASSERT(glEnable(GL_CULL_FACE));
        else
            GL_ASSERT(glDisable(GL_CULL_FACE));
        _defaultState->_cullFaceEnabled = state->_cullFaceEnabled;
    }
    if (force == 2 || ((force || (state->_bits & RS_CULL_FACE_SIDE)) && (state->_cullFaceSide != _defaultState->_cullFaceSide)))
    {
        GL_ASSERT(glCullFace((GLenum)state->_cullFaceSide));
        _defaultState->_cullFaceSide = state->_cullFaceSide;
    }
    if (force == 2 || ((force || (state->_bits & RS_FRONT_FACE)) && (state->_frontFace != _defaultState->_frontFace)))
    {
        GL_ASSERT(glFrontFace((GLenum)state->_frontFace));
        _defaultState->_frontFace = state->_frontFace;
    }
    if (force == 2 || ((force || (state->_bits & RS_DEPTH_TEST)) && (state->_depthTestEnabled != _defaultState->_depthTestEnabled)))
    {
        if (state->_depthTestEnabled)
            GL_ASSERT(glEnable(GL_DEPTH_TEST));
        else
            GL_ASSERT(glDisable(GL_DEPTH_TEST));
        _defaultState->_depthTestEnabled = state->_depthTestEnabled;
    }
    if (force == 2 || ((force || (state->_bits & RS_DEPTH_WRITE)) && (state->_depthWriteEnabled != _defaultState->_depthWriteEnabled)))
    {
        GL_ASSERT(glDepthMask(state->_depthWriteEnabled ? GL_TRUE : GL_FALSE));
        _defaultState->_depthWriteEnabled = state->_depthWriteEnabled;
    }
    if (force == 2 || ((force || (state->_bits & RS_DEPTH_FUNC)) && (state->_depthFunction != _defaultState->_depthFunction)))
    {
        GL_ASSERT(glDepthFunc((GLenum)state->_depthFunction));
        _defaultState->_depthFunction = state->_depthFunction;
    }
    if (force == 2 || ((force || (state->_bits & RS_STENCIL_TEST)) && (state->_stencilTestEnabled != _defaultState->_stencilTestEnabled)))
    {
        if (state->_stencilTestEnabled)
            GL_ASSERT(glEnable(GL_STENCIL_TEST));
        else
            GL_ASSERT(glDisable(GL_STENCIL_TEST));
        _defaultState->_stencilTestEnabled = state->_stencilTestEnabled;
    }
    if (force == 2 || ((force || (state->_bits & RS_STENCIL_WRITE)) && (state->_stencilWrite != _defaultState->_stencilWrite)))
    {
        GL_ASSERT(glStencilMask(state->_stencilWrite));
        _defaultState->_stencilWrite = state->_stencilWrite;
    }
    if (force == 2 || ((force || (state->_bits & RS_STENCIL_FUNC)) && (state->_stencilFunction != _defaultState->_stencilFunction ||
        state->_stencilFunctionRef != _defaultState->_stencilFunctionRef ||
        state->_stencilFunctionMask != _defaultState->_stencilFunctionMask)))
    {
        GL_ASSERT(glStencilFunc((GLenum)state->_stencilFunction, state->_stencilFunctionRef, state->_stencilFunctionMask));
        _defaultState->_stencilFunction = state->_stencilFunction;
        _defaultState->_stencilFunctionRef = state->_stencilFunctionRef;
        _defaultState->_stencilFunctionMask = state->_stencilFunctionMask;
    }
    if (force == 2 || ((force || (state->_bits & RS_STENCIL_OP)) && (state->_stencilOpSfail != _defaultState->_stencilOpSfail ||
        state->_stencilOpDpfail != _defaultState->_stencilOpDpfail ||
        state->_stencilOpDppass != _defaultState->_stencilOpDppass)))
    {
        GL_ASSERT(glStencilOp((GLenum)state->_stencilOpSfail, (GLenum)state->_stencilOpDpfail, (GLenum)state->_stencilOpDppass));
        _defaultState->_stencilOpSfail = state->_stencilOpSfail;
        _defaultState->_stencilOpDpfail = state->_stencilOpDpfail;
        _defaultState->_stencilOpDppass = state->_stencilOpDppass;
    }
    if (force == 2 || ((force || (state->_bits & RS_POLYGON_OFFSET)) && (state->_polygonOffset != _defaultState->_polygonOffset ||
        state->_offsetFactor != _defaultState->_offsetFactor ||
        state->_offsetUnits != _defaultState->_offsetUnits)))
    {
        if (state->_polygonOffset) GL_ASSERT(glEnable(GL_POLYGON_OFFSET_FILL));
        else GL_ASSERT(glDisable(GL_POLYGON_OFFSET_FILL));
        GL_ASSERT(glPolygonOffset(state->_offsetFactor, state->_offsetUnits));
        _defaultState->_stencilOpSfail = state->_stencilOpSfail;
        _defaultState->_offsetFactor = state->_offsetFactor;
        _defaultState->_offsetUnits = state->_offsetUnits;
    }
}


GLint getFormatInternal(Texture::Format format)
{
    switch (format)
    {
    case Texture::UNKNOWN:
        return 0;
        //auto size type
    case Texture::RGB:
        return GL_RGB;
    case Texture::RGBA:
        return GL_RGBA;
    case Texture::ALPHA:
        return GL_ALPHA;
    case Texture::RED:
        return GL_R8;
    case Texture::RG:
        return GL_RG;

        //fix size type
    case Texture::RGB888:
        return GL_RGB8;
    case Texture::RGB565:
        return GL_RGB565;
    case Texture::RGBA4444:
        return GL_RGBA4;
    case Texture::RGBA5551:
        return GL_RGB5_A1;
    case Texture::RGBA8888:
        return GL_RGBA8;

        //depth
    case Texture::DEPTH:
        return GL_DEPTH_COMPONENT32F;
    case Texture::DEPTH24_STENCIL8:
        return GL_DEPTH24_STENCIL8;

        //float type
    case Texture::RGB16F:
        return GL_RGB16F;
    case Texture::RGBA16F:
        return GL_RGBA16F;
    case Texture::R16F:
        return GL_R16F;
    case Texture::R11F_G11F_B10F:
        return GL_R11F_G11F_B10F;
    case Texture::RGB9_E5:
        return GL_RGB9_E5;
    case Texture::R32F:
        return GL_R32F;
    case Texture::RGB32F:
        return GL_RGB32F;
    case Texture::RGBA32F:
        return GL_RGBA32F;
    case Texture::RG16F:
        return GL_RG16F;
    default:
        return 0;
    }
}

GLenum getIOFormat(Texture::Format format)
{
    switch (format)
    {
    case Texture::UNKNOWN:
        return 0;
        //auto size type
    case Texture::RGB:
        return GL_RGB;
    case Texture::RGBA:
        return GL_RGBA;
    case Texture::ALPHA:
        return GL_ALPHA;
    case Texture::RED:
        return GL_RED;
    case Texture::RG:
        return GL_RG;

        //fix size type
    case Texture::RGB888:
        return GL_RGB;
    case Texture::RGB565:
        return GL_RGB;
    case Texture::RGBA4444:
        return GL_RGBA;
    case Texture::RGBA5551:
        return GL_RGBA;
    case Texture::RGBA8888:
        return GL_RGBA;

        //depth
    case Texture::DEPTH:
        return GL_DEPTH_COMPONENT;
    case Texture::DEPTH24_STENCIL8:
        return GL_DEPTH_STENCIL;

        //float type
    case Texture::RGB16F:
        return GL_RGB;
    case Texture::RGBA16F:
        return GL_RGBA;
    case Texture::R16F:
        return GL_RED;
    case Texture::R11F_G11F_B10F:
        return GL_RGB;
    case Texture::RGB9_E5:
        return GL_RGB;
    case Texture::R32F:
        return GL_RED;
    case Texture::RGB32F:
        return GL_RGB;
    case Texture::RGBA32F:
        return GL_RGBA;
    case Texture::RG16F:
        return GL_RG;
    default:
        return 0;
    }
}

GLenum getFormatDataType(Texture::Format format)
{
    switch (format)
    {
    case Texture::UNKNOWN:
        return 0;
        //auto size type
    case Texture::RGB:
        return GL_UNSIGNED_BYTE;
    case Texture::RGBA:
        return GL_UNSIGNED_BYTE;
    case Texture::ALPHA:
        return GL_UNSIGNED_BYTE;
    case Texture::RED:
        return GL_UNSIGNED_BYTE;
    case Texture::RG:
        return GL_UNSIGNED_BYTE;

        //fix size type
    case Texture::RGB888:
        return GL_UNSIGNED_BYTE;
    case Texture::RGB565:
        return GL_UNSIGNED_SHORT_5_6_5;
    case Texture::RGBA4444:
        return GL_UNSIGNED_SHORT_4_4_4_4;
    case Texture::RGBA5551:
        return GL_UNSIGNED_SHORT_5_5_5_1;
    case Texture::RGBA8888:
        return GL_UNSIGNED_BYTE;

        //depth
    case Texture::DEPTH:
        return GL_FLOAT;
    case Texture::DEPTH24_STENCIL8:
        return GL_UNSIGNED_INT_24_8;

        //float type
    case Texture::RGB16F:
        return GL_FLOAT;
    case Texture::RGBA16F:
        return GL_FLOAT;
    case Texture::R16F:
        return GL_FLOAT;
    case Texture::R11F_G11F_B10F:
        return GL_FLOAT;
    case Texture::RGB9_E5:
        return GL_FLOAT;
    case Texture::R32F:
        return GL_FLOAT;
    case Texture::RGB32F:
        return GL_FLOAT;
    case Texture::RGBA32F:
        return GL_FLOAT;
    case Texture::RG16F:
        return GL_FLOAT;
    default:
        return 0;
    }
}


void GLRenderer::updateTexture(Texture* texture) {
    Texture::Format format = texture->getFormat();
    Texture::Type type = texture->getType();
    GP_ASSERT(type == Texture::TEXTURE_2D || type == Texture::TEXTURE_CUBE);

    GLenum target = (GLenum)type;

    GLint internalFormat = getFormatInternal(format);
    GP_ASSERT(internalFormat != 0);

    GLenum texelType = getFormatDataType(format);
    GP_ASSERT(texelType != 0);

    GLenum ioFormat = getIOFormat(format);
    GP_ASSERT(ioFormat != 0);

    // Create the texture.
    if (texture->_handle == 0) {
        GLuint textureId;
        GL_ASSERT(glGenTextures(1, &textureId));
        texture->_handle = textureId;
        GL_ASSERT(glBindTexture(target, textureId));
        GL_ASSERT(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
#ifndef OPENGL_ES
        // glGenerateMipmap is new in OpenGL 3.0. For OpenGL 2.0 we must fallback to use glTexParameteri
        // with GL_GENERATE_MIPMAP prior to actual texture creation (glTexImage2D)
        if (texture->isMipmapped() && !std::addressof(glGenerateMipmap))
            GL_ASSERT(glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE));
#endif
    }

    unsigned int width = texture->getWidth();
    unsigned int height = texture->getHeight();

    GLuint textureId = texture->_handle;
    GL_ASSERT(glBindTexture(target, textureId));

    // Load the texture
    size_t bpp = Texture::getFormatBPP(format);
    if (type == Texture::TEXTURE_2D)
    {
        GL_ASSERT(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, ioFormat, texelType, texture->_data));
    }
    else if (type == Texture::TEXTURE_2D_ARRAY) {
        //(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
        GL_ASSERT(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height, 0, 0, ioFormat, texelType, NULL));

        // Get texture size
        unsigned int textureSize = width * height;
        if (bpp == 0)
        {
            glDeleteTextures(1, &textureId);
            GP_ERROR("Failed to determine texture size because format is UNKNOWN.");
            texture->_handle = 0;
            return;
        }
        textureSize *= bpp;
        // Texture Cube
        for (unsigned int i = 0; i < texture->_arrayDepth; i++)
        {
            const unsigned char* texturePtr = (texture->_data == NULL) ? NULL : &texture->_data[i * textureSize];
            //(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
            GL_ASSERT(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, ioFormat, texelType, texturePtr));
        }
    }
    else if (type == Texture::TEXTURE_CUBE)
    {
        // Get texture size
        unsigned int textureSize = width * height;
        if (bpp == 0)
        {
            glDeleteTextures(1, &textureId);
            GP_ERROR("Failed to determine texture size because format is UNKNOWN.");
            texture->_handle = 0;
            return;
        }
        textureSize *= bpp;
        // Texture Cube
        for (unsigned int i = 0; i < 6; i++)
        {
            const unsigned char* texturePtr = (texture->_data == NULL) ? NULL : &texture->_data[i * textureSize];
            GL_ASSERT(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, ioFormat, texelType, texturePtr));
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
    else {
        abort();
    }

    // Set initial minification filter based on whether or not mipmaping was enabled.
    if (texture->_minFilter == Texture::NEAREST)
    {
        GL_ASSERT(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_ASSERT(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_ASSERT(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_ASSERT(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
#if !defined(OPENGL_ES) || defined(GL_ES_VERSION_3_0) && GL_ES_VERSION_3_0
        GL_ASSERT(glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_NONE));
#endif    	
    }
    else
    {
        GL_ASSERT(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, texture->_minFilter));
    }

    if (texture->isMipmapped()) {
        //GL_ASSERT(glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST));
        if (std::addressof(glGenerateMipmap) && glGenerateMipmap)
            GL_ASSERT(glGenerateMipmap(target));
    }
}

void GLRenderer::deleteTexture(Texture* texture) {
    if (texture->_handle)
    {
        GL_ASSERT(glDeleteTextures(1, &texture->_handle));
        texture->_handle = 0;
    }
}

void GLRenderer::bindTextureSampler(Texture* sampler) {
    Texture* _texture = sampler;
    GP_ASSERT(_texture);

    Texture::Type type = _texture->getType();
    GLenum target = (GLenum)type;

    GLuint textureId = _texture->_handle;
    GL_ASSERT(glBindTexture(target, textureId));

    GL_ASSERT(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, (GLenum)sampler->_minFilter));
    GL_ASSERT(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, (GLenum)sampler->_magFilter));
    GL_ASSERT(glTexParameteri(target, GL_TEXTURE_WRAP_S, (GLenum)sampler->_wrapS));
    GL_ASSERT(glTexParameteri(target, GL_TEXTURE_WRAP_T, (GLenum)sampler->_wrapT));
#if defined(GL_TEXTURE_WRAP_R) // OpenGL ES 3.x and up, OpenGL 1.2 and up
    if (target == GL_TEXTURE_CUBE_MAP) // We don't want to run this on something that we know will fail
        GL_ASSERT(glTexParameteri(target, GL_TEXTURE_WRAP_R, (GLenum)sampler->_wrapR));
#endif
}

static void writeToFile(const char* filePath, const char* source)
{
    std::string path = filePath;
    //path += ".err";
    UPtr<Stream> stream(FileSystem::open(path.c_str(), FileSystem::WRITE));
    if (stream.get() != NULL && stream->canWrite())
    {
        stream->write(source, 1, strlen(source));
    }
}

ShaderProgram* GLRenderer::createProgram(ProgramSrc* src) {
    const char* defines = src->defines;
    const char* vshSource = src->vshSource;
    const char* fshSource = src->fshSource;
    const char* version = src->version;

    GP_ASSERT(vshSource);
    GP_ASSERT(fshSource);

    if (!version) {
    #if defined(OPENGL_ES) || defined(__EMSCRIPTEN__)
        version = "#version 300 es";
    #else
        version = "#version 330 core";
    #endif
    }

    const unsigned int SHADER_SOURCE_LENGTH = 5;
    const GLchar* shaderSource[SHADER_SOURCE_LENGTH];
    char* infoLog = NULL;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;
    GLint length;
    GLint success;

    shaderSource[0] = version;
    shaderSource[1] = "\n";
    shaderSource[2] = defines;
    shaderSource[3] = "\n";
    //std::string vshSourceStr = "";
    shaderSource[4] = vshSource;
    GL_ASSERT(vertexShader = glCreateShader(GL_VERTEX_SHADER));
    GL_ASSERT(glShaderSource(vertexShader, SHADER_SOURCE_LENGTH, shaderSource, NULL));
    GL_ASSERT(glCompileShader(vertexShader));
    GL_ASSERT(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
    if (success != GL_TRUE)
    {
        GL_ASSERT(glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &length));
        if (length == 0)
        {
            length = 4096;
        }
        if (length > 0)
        {
            infoLog = new char[length];
            GL_ASSERT(glGetShaderInfoLog(vertexShader, length, NULL, infoLog));
            infoLog[length - 1] = '\0';
        }

        // Write out the expanded shader file.
        writeToFile("shader.err", shaderSource[4]);

        // printf(version);
        // printf(defines);
        // printf(vshSource);

        GP_ERROR("Compile failed for vertex shader '%s' with error '%s'.", src->id, infoLog == NULL ? "" : infoLog);
        SAFE_DELETE_ARRAY(infoLog);

        // Clean up.
        GL_ASSERT(glDeleteShader(vertexShader));

        return NULL;
    }

    shaderSource[4] = fshSource;
    GL_ASSERT(fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL_ASSERT(glShaderSource(fragmentShader, SHADER_SOURCE_LENGTH, shaderSource, NULL));
    GL_ASSERT(glCompileShader(fragmentShader));
    GL_ASSERT(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
    if (success != GL_TRUE)
    {
        GL_ASSERT(glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &length));
        if (length == 0)
        {
            length = 4096;
        }
        if (length > 0)
        {
            infoLog = new char[length];
            GL_ASSERT(glGetShaderInfoLog(fragmentShader, length, NULL, infoLog));
            infoLog[length - 1] = '\0';
        }

        // Write out the expanded shader file.
        writeToFile("shader.err", shaderSource[4]);

        GP_ERROR("Compile failed for fragment shader %s: %s", src->id, infoLog == NULL ? "" : infoLog);
        SAFE_DELETE_ARRAY(infoLog);

        // Clean up.
        GL_ASSERT(glDeleteShader(vertexShader));
        GL_ASSERT(glDeleteShader(fragmentShader));

        return NULL;
    }

    // Link program.
    GL_ASSERT(program = glCreateProgram());
    GL_ASSERT(glAttachShader(program, vertexShader));
    GL_ASSERT(glAttachShader(program, fragmentShader));
    GL_ASSERT(glLinkProgram(program));
    GL_ASSERT(glGetProgramiv(program, GL_LINK_STATUS, &success));

    // Delete shaders after linking.
    GL_ASSERT(glDeleteShader(vertexShader));
    GL_ASSERT(glDeleteShader(fragmentShader));

    // Check link status.
    if (success != GL_TRUE)
    {
        GL_ASSERT(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
        if (length == 0)
        {
            length = 4096;
        }
        if (length > 0)
        {
            infoLog = new char[length];
            GL_ASSERT(glGetProgramInfoLog(program, length, NULL, infoLog));
            infoLog[length - 1] = '\0';
        }
        GP_ERROR("Linking program failed (%s): %s", src->id, infoLog == NULL ? "" : infoLog);
        SAFE_DELETE_ARRAY(infoLog);

        // Clean up.
        GL_ASSERT(glDeleteProgram(program));

        return NULL;
    }

    // Create and return the new ShaderProgram.
    ShaderProgram* effect = new ShaderProgram();
    effect->_program = program;

    // Query and store vertex attribute meta-data from the program.
    // NOTE: Rather than using glBindAttribLocation to explicitly specify our own
    // preferred attribute locations, we're going to query the locations that were
    // automatically bound by the GPU. While it can sometimes be convenient to use
    // glBindAttribLocation, some vendors actually reserve certain attribute indices
    // and therefore using this function can create compatibility issues between
    // different hardware vendors.
    GLint activeAttributes;
    GL_ASSERT(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &activeAttributes));
    if (activeAttributes > 0)
    {
        GL_ASSERT(glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length));
        if (length > 0)
        {
            GLchar* attribName = new GLchar[length + 1];
            GLint attribSize;
            GLenum attribType;
            GLint attribLocation;
            for (int i = 0; i < activeAttributes; ++i)
            {
                // Query attribute info.
                GL_ASSERT(glGetActiveAttrib(program, i, length, NULL, &attribSize, &attribType, attribName));
                attribName[length] = '\0';

                // Query the pre-assigned attribute location.
                GL_ASSERT(attribLocation = glGetAttribLocation(program, attribName));

                // Assign the vertex attribute mapping for the effect.
                effect->_vertexAttributes[attribName] = attribLocation;
            }
            SAFE_DELETE_ARRAY(attribName);
        }
    }

    // Query and store uniforms from the program.
    GLint activeUniforms;
    GL_ASSERT(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms));
    if (activeUniforms > 0)
    {
        GL_ASSERT(glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &length));
        if (length > 0)
        {
            GLchar* uniformName = new GLchar[length + 1];
            GLint uniformSize;
            GLenum uniformType;
            GLint uniformLocation;
            unsigned int samplerIndex = 0;
            for (int i = 0; i < activeUniforms; ++i)
            {
                // Query uniform info.
                GL_ASSERT(glGetActiveUniform(program, i, length, NULL, &uniformSize, &uniformType, uniformName));
                uniformName[length] = '\0';  // null terminate
                if (length > 3)
                {
                    // If this is an array uniform, strip array indexers off it since GL does not
                    // seem to be consistent across different drivers/implementations in how it returns
                    // array uniforms. On some systems it will return "u_matrixArray", while on others
                    // it will return "u_matrixArray[0]".
                    char* c = strrchr(uniformName, '[');
                    if (c)
                    {
                        *c = '\0';
                    }
                }

                // Query the pre-assigned uniform location.
                GL_ASSERT(uniformLocation = glGetUniformLocation(program, uniformName));

                Uniform* uniform = new Uniform();
                uniform->_effect = effect;
                uniform->_name = uniformName;
                uniform->_location = uniformLocation;
                uniform->_type = uniformType;
                uniform->_size = uniformSize;
                if (uniformType == GL_SAMPLER_2D || uniformType == GL_SAMPLER_CUBE)
                {
                    uniform->_index = samplerIndex;
                    samplerIndex += uniformSize;
                }
                else
                {
                    uniform->_index = 0;
                }

                effect->_uniforms[uniformName] = uniform;
            }
            SAFE_DELETE_ARRAY(uniformName);
        }
    }

    return effect;
}

void GLRenderer::deleteProgram(ShaderProgram* effect) {
    if (effect->_program)
    {
        // If our program object is currently bound, unbind it before we're destroyed.
        if (__currentShaderProgram == effect->_program)
        {
            GL_ASSERT(glUseProgram(0));
            __currentShaderProgram = 0;
        }

        GL_ASSERT(glDeleteProgram(effect->_program));
        effect->_program = 0;
    }
}
void GLRenderer::bindProgram(ShaderProgram* effect) {
    if (__currentShaderProgram == effect->_program) {
        return;
    }

    GL_ASSERT(glUseProgram(effect->_program));

    __currentShaderProgram = effect->_program;
}
bool GLRenderer::bindUniform(MaterialParameter* value, Uniform* uniform, ShaderProgram* effect) {
    GP_ASSERT(uniform);
    GP_ASSERT(value);
    GP_ASSERT(effect);

    if (value->_methodBinding) {
        value->_methodBinding->setValue(effect);
    }

    int location = uniform->_location;
    int arrayOffset = 0;
    if (uniform->_size > 1) {
        location = glGetUniformLocation(effect->_program, value->getName());
        if (location < 0) {
            GP_WARN("Material parameter value not set for: '%s' in effect: '%s'.", value->_name.c_str(), effect->getId());
            return false;
        }
        arrayOffset = value->arrrayOffset;
    }

    switch (value->_type)
    {
    case MaterialParameter::FLOAT:
        GL_ASSERT(glUniform1f(location, value->_value.floatValue));
        break;
    case MaterialParameter::FLOAT_ARRAY:
        GP_ASSERT(value->_value.floatPtrValue);
        GL_ASSERT(glUniform1fv(location, value->_count, value->_value.floatPtrValue));
        break;
    case MaterialParameter::INT:
        GL_ASSERT(glUniform1i(location, value->_value.intValue));
        break;
    case MaterialParameter::INT_ARRAY:
        GL_ASSERT(glUniform1iv(location, value->_count, value->_value.intPtrValue));
        break;
    case MaterialParameter::VECTOR2: {
        //Vector2* values2 = reinterpret_cast<Vector2*>(value->_value.floatPtrValue);
        GP_ASSERT(value->_value.floatPtrValue);
        GL_ASSERT(glUniform2fv(location, value->_count, value->_value.floatPtrValue));
        break;
    }
    case MaterialParameter::VECTOR3: {
        //Vector3* values3 = reinterpret_cast<Vector3*>(value->_value.floatPtrValue);
        GP_ASSERT(value->_value.floatPtrValue);
        GL_ASSERT(glUniform3fv(location, value->_count, value->_value.floatPtrValue));
        break;
    }
    case MaterialParameter::VECTOR4: {
        //Vector4* values4 = reinterpret_cast<Vector4*>(value->_value.floatPtrValue);
        GP_ASSERT(value->_value.floatPtrValue);
        GL_ASSERT(glUniform4fv(location, value->_count, value->_value.floatPtrValue));
        break;
    }
    case MaterialParameter::MATRIX: {
        //GL_ASSERT(glUniformMatrix4fv(uniform->_location, 1, GL_FALSE, value.m));
        //Matrix* valuesm = reinterpret_cast<Matrix*>(value->_value.floatPtrValue);
        GP_ASSERT(value->_value.floatPtrValue);
        GL_ASSERT(glUniformMatrix4fv(location, value->_count, GL_FALSE, value->_value.floatPtrValue));
        break;
    }
    case MaterialParameter::SAMPLER: {
        const Texture* sampler = value->_value.samplerValue;
        GP_ASSERT(uniform->_type == GL_SAMPLER_2D || uniform->_type == GL_SAMPLER_CUBE);
        GP_ASSERT(sampler);
        GP_ASSERT((sampler->getType() == Texture::TEXTURE_2D && uniform->_type == GL_SAMPLER_2D) ||
            (sampler->getType() == Texture::TEXTURE_CUBE && uniform->_type == GL_SAMPLER_CUBE));

        GL_ASSERT(glActiveTexture(GL_TEXTURE0 + uniform->_index + arrayOffset));

        // Bind the sampler - this binds the texture and applies sampler state
        const_cast<Texture*>(sampler)->bind();

        GL_ASSERT(glUniform1i(location, uniform->_index + arrayOffset));
        break;
    }
    case MaterialParameter::SAMPLER_ARRAY: {
        const Texture** values = value->_value.samplerArrayValue;
        GP_ASSERT(uniform->_type == GL_SAMPLER_2D || uniform->_type == GL_SAMPLER_CUBE);
        GP_ASSERT(values);

        // Set samplers as active and load texture unit array
        GLint units[32];
        for (unsigned int i = 0; i < value->_count; ++i)
        {
            GP_ASSERT((const_cast<Texture*>(values[i])->getType() == Texture::TEXTURE_2D && uniform->_type == GL_SAMPLER_2D) ||
                (const_cast<Texture*>(values[i])->getType() == Texture::TEXTURE_CUBE && uniform->_type == GL_SAMPLER_CUBE));
            GL_ASSERT(glActiveTexture(GL_TEXTURE0 + uniform->_index + arrayOffset + i));

            // Bind the sampler - this binds the texture and applies sampler state
            const_cast<Texture*>(values[i])->bind();

            units[i] = uniform->_index + arrayOffset + i;
        }

        // Pass texture unit array to GL
        GL_ASSERT(glUniform1iv(location, value->_count, units));
        break;
        //case MaterialParameter::METHOD:
            //if (_value.method)
            //    _value.method->setValue(effect);
            //break;
    }
    default:
    {
        if ((value->_loggerDirtyBits & MaterialParameter::PARAMETER_VALUE_NOT_SET) == 0)
        {
            GP_WARN("Material parameter value not set for: '%s' in effect: '%s'.", value->_name.c_str(), effect->getId());
            value->_loggerDirtyBits |= MaterialParameter::PARAMETER_VALUE_NOT_SET;
        }
        return false;
        break;
    }
    }
    return true;
}


void GLRenderer::bindVertexAttributeObj(VertexAttributeObject* vertextAttribute) {

    // Create a new VertexAttributeBinding.
    VertexAttributeObject* b = vertextAttribute;

    bool needInitVAO = vertextAttribute->_isDirty;

#ifdef GP_USE_VAO
    if (b->_handle == 0 && vertextAttribute->getVbo() && glGenVertexArrays)
    {
        GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        // Use hardware VAOs.
        GLuint handle = 0;
        GL_ASSERT(glGenVertexArrays(1, &handle));
        b->_handle = handle;

        if (b->_handle == 0)
        {
            GP_ERROR("Failed to create VAO handle.");
            //SAFE_DELETE(b);
            return;
        }

        needInitVAO = true;
    }
#endif


    if (b->_handle)
    {
        // Hardware mode
        GL_ASSERT(glBindVertexArray(b->_handle));
    }


    if (b->_handle == 0 || needInitVAO) {
        // Bind the Mesh VBO so our glVertexAttribPointer calls use it.
        // Software mode
        if (b->getVbo())
        {
            GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, b->getVbo()));
        }
        else
        {
            GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }

        // Call setVertexAttribPointer for each vertex element.
        for (size_t i = 0, count = vertextAttribute->_attributes.size(); i < count; ++i)
        {
            VertexAttributeObject::VertexAttribute& attribute = vertextAttribute->_attributes[i];

            if (attribute.location == -1)
            {
                //GP_WARN("Warning: Vertex element with usage '%s' in mesh '%s' does not correspond to an attribute in effect '%s'.", VertexFormat::toString(e.usage), mesh->getUrl(), effect->getId());
            }
            else
            {
                void* pointer = attribute.pointer;
                if (attribute.type != GL_FLOAT) {
                    //(GLuint index, GLint size, GLenum type, GLsizei stride, const void*pointer)
                    GL_ASSERT(glVertexAttribIPointer(attribute.location, (GLint)attribute.size, attribute.type, (GLsizei)attribute.stride, pointer));
                }
                else {
                    //(	GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
                    GL_ASSERT(glVertexAttribPointer(attribute.location, (GLint)attribute.size, attribute.type, GL_FALSE, (GLsizei)attribute.stride, pointer));
                }
                GL_ASSERT(glEnableVertexAttribArray(attribute.location));
            }
        }

        if (b->getInstancedVbo()) {
            GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, b->getInstancedVbo()));
            int loc = b->_effect->getVertexAttribute("a_instanceMatrix");
            if (loc >= 0) {
                int matrixByteSize = 16 * sizeof(float);
                int vector4Size = 4 * sizeof(float);
                GL_ASSERT(glEnableVertexAttribArray(loc));
                GL_ASSERT(glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, matrixByteSize, (void*)0));

                GL_ASSERT(glEnableVertexAttribArray(loc + 1));
                GL_ASSERT(glVertexAttribPointer(loc + 1, 4, GL_FLOAT, GL_FALSE, matrixByteSize, (void*)(vector4Size)));

                GL_ASSERT(glEnableVertexAttribArray(loc + 2));
                GL_ASSERT(glVertexAttribPointer(loc + 2, 4, GL_FLOAT, GL_FALSE, matrixByteSize, (void*)(2 * vector4Size)));

                GL_ASSERT(glEnableVertexAttribArray(loc + 3));
                GL_ASSERT(glVertexAttribPointer(loc + 3, 4, GL_FLOAT, GL_FALSE, matrixByteSize, (void*)(3 * vector4Size)));

                GL_ASSERT(glVertexAttribDivisor(loc, 1));
                GL_ASSERT(glVertexAttribDivisor(loc + 1, 1));
                GL_ASSERT(glVertexAttribDivisor(loc + 2, 1));
                GL_ASSERT(glVertexAttribDivisor(loc + 3, 1));
            }
        }

        if (b->getEbo()) {
            GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->getEbo()));
        }
    }

}

void GLRenderer::unbindVertexAttributeObj(VertexAttributeObject* vertextAttribute) {
    if (vertextAttribute->_handle)
    {
        // Hardware mode
        GL_ASSERT(glBindVertexArray(0));
    }
    else // Software mode
    {
        if (vertextAttribute->getInstancedVbo()) {
            int loc = vertextAttribute->_effect->getVertexAttribute("a_instanceMatrix");
            if (loc >= 0) {
                GL_ASSERT(glVertexAttribDivisor(loc, 0));
                GL_ASSERT(glVertexAttribDivisor(loc + 1, 0));
                GL_ASSERT(glVertexAttribDivisor(loc + 2, 0));
                GL_ASSERT(glVertexAttribDivisor(loc + 3, 0));
                GL_ASSERT(glDisableVertexAttribArray(loc));
                GL_ASSERT(glDisableVertexAttribArray(loc + 1));
                GL_ASSERT(glDisableVertexAttribArray(loc + 2));
                GL_ASSERT(glDisableVertexAttribArray(loc + 3));
            }
        }

        if (vertextAttribute->getVbo())
        {
            GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }

        for (size_t i = 0, count = vertextAttribute->_attributes.size(); i < count; ++i)
        {
            VertexAttributeObject::VertexAttribute& attribute = vertextAttribute->_attributes[i];
            GL_ASSERT(glDisableVertexAttribArray(attribute.location));
        }

        if (vertextAttribute->getEbo()) {
            GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }
    }
}

void GLRenderer::deleteVertexAttributeObj(VertexAttributeObject* vertextAttribute) {
    if (vertextAttribute->_handle)
    {
        GLuint handle = vertextAttribute->_handle;
        GL_ASSERT(glDeleteVertexArrays(1, &handle));
        vertextAttribute->_handle = 0;
    }
}

UPtr<FrameBuffer> GLRenderer::createFrameBuffer(const char* id, unsigned int width, unsigned int height, Texture::Format format) {
    return GLFrameBuffer::create(id, width, height, format);
}

FrameBuffer* GLRenderer::getCurrentFrameBuffer() {
    return GLFrameBuffer::getCurrent();
}

int GLRenderer::drawCallCount() {
    int dr = _drawCallCount;
    _drawCallCount = 0;
    return dr;
}
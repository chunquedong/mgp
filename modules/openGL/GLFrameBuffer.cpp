#include "GLFrameBuffer.h"
#include "base/Base.h"
#include "ogl.h"
#include "DepthStencilTarget.h"
#include "platform/Toolkit.h"
#include "scene/Renderer.h"
#include "GLRenderer.h"

namespace mgp
{

unsigned int GLFrameBuffer::_maxRenderTargets = 8;

GLFrameBuffer::GLFrameBuffer(const char* id, unsigned int width, unsigned int height, FrameBufferHandle handle)
    : _id(id ? id : ""), _handle(handle), _renderTargets(NULL), _renderTargetCount(0), _depthStencilTarget(NULL)
{
}

GLFrameBuffer::~GLFrameBuffer()
{
    if (this == _renderer->_currentFrameBuffer) {
        if (_renderer->_defaultFrameBuffer && glIsFramebuffer(_renderer->_defaultFrameBuffer->_handle)) {
            GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _renderer->_defaultFrameBuffer->_handle));
        }
        _renderer->_currentFrameBuffer = _renderer->_defaultFrameBuffer;
    }

    //do not release default fbo handle
    if (this == _renderer->_defaultFrameBuffer) {
        return;
    }

    if (_renderTargets)
    {
        for (unsigned int i = 0; i < _maxRenderTargets; ++i)
        {
            if (_renderTargets[i])
            {
                SAFE_RELEASE(_renderTargets[i]);
            }
        }
        SAFE_DELETE_ARRAY(_renderTargets);
    }

    if (_depthStencilTarget)
        SAFE_RELEASE(_depthStencilTarget);

    // Release GL resource.
    if (_handle)
        GL_ASSERT( glDeleteFramebuffers(1, &_handle) );
}

void GLFrameBuffer::initialize()
{
    // Query the max supported color attachments. This glGet operation is not supported
    // on GL ES 2.x, so if the define does not exist, assume a value of 1.
#ifdef GL_MAX_COLOR_ATTACHMENTS
        //GLint val;
        //GL_ASSERT( glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &val) );
        //_maxRenderTargets = (unsigned int)std::max(1, val);
    _maxRenderTargets = 8;
#else
    _maxRenderTargets = 8;
#endif
}

UPtr<FrameBuffer> GLFrameBuffer::create(GLRenderer* renderer, const char* id)
{
    return create(renderer, id, 0, 0);
}

UPtr<FrameBuffer> GLFrameBuffer::create(GLRenderer* renderer, const char* id, 
    unsigned int width, unsigned int height, Image::Format format)
{
    UPtr<Texture> renderTarget;
    if (width > 0 && height > 0)
    {
        // Create a default RenderTarget with same ID.
        renderTarget = Texture::create(format, width, height, NULL);
        if (renderTarget.get() == NULL)
        {
            GP_ERROR("Failed to create render target for frame buffer.");
            return UPtr<FrameBuffer>(NULL);
        }
    }

    // Create the frame buffer
    GLuint handle = 0;
    GL_ASSERT( glGenFramebuffers(1, &handle) );
    GLFrameBuffer* frameBuffer = new GLFrameBuffer(id, width, height, handle);
    frameBuffer->_renderer = renderer;

    // Create the render target array for the new frame buffer
    frameBuffer->_renderTargets = new Texture*[_maxRenderTargets];
    memset(frameBuffer->_renderTargets, 0, sizeof(Texture*) * _maxRenderTargets);

    if (renderTarget.get())
    {
        frameBuffer->setRenderTarget(renderTarget.get(), 0);
        //SAFE_RELEASE(renderTarget);
    }

    return UPtr<FrameBuffer>(frameBuffer);
}

const char* GLFrameBuffer::getId() const
{
    return _id.c_str();
}

unsigned int GLFrameBuffer::getWidth() const
{
    if (_renderTargetCount > 0 && _renderTargets != NULL && _renderTargets[0] != NULL)
        return _renderTargets[0]->getWidth();

    return 0;
}

unsigned int GLFrameBuffer::getHeight() const
{
    if (_renderTargetCount > 0 && _renderTargets != NULL && _renderTargets[0] != NULL)
        return _renderTargets[0]->getHeight();

    return 0;
}

unsigned int GLFrameBuffer::getMaxRenderTargets() const
{
    return _maxRenderTargets;
}

void GLFrameBuffer::setRenderTarget(Texture* target, unsigned int index)
{
    GP_ASSERT(!target || (target->getType() == Texture::TEXTURE_2D));

    // No change
    if (_renderTargets[index] == target)
        return;

    setRenderTarget(target, index, GL_TEXTURE_2D, 0);
}

void GLFrameBuffer::setRenderTarget(Texture* target, Texture::CubeFace face, int mipmapLevel, unsigned int index)
{
    GP_ASSERT(face >= Texture::POSITIVE_X && face <= Texture::NEGATIVE_Z);
    GP_ASSERT(!target || (target->getType() == Texture::TEXTURE_CUBE));

    setRenderTarget(target, index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mipmapLevel);
}

void GLFrameBuffer::setRenderTarget(Texture* target, unsigned int index, GLenum textureTarget, int mipmapLevel)
{
    GP_ASSERT(index < _maxRenderTargets);
    GP_ASSERT(_renderTargets);

    // Release our reference to the current RenderTarget at this index.
    if (_renderTargets[index])
    {
        SAFE_RELEASE(_renderTargets[index]);
        --_renderTargetCount;
    }

    _renderTargets[index] = target;

    if (target)
    {
        _renderer->updateTexture(target);

        ++_renderTargetCount;

        // This GLFrameBuffer now references the RenderTarget.
        target->addRef();

        // Now set this target as the color attachment corresponding to index.
        GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _handle) );
        GLenum attachment;
        if (target->getFormat() == Image::DEPTH)
        {
            attachment = GL_DEPTH_ATTACHMENT;
            GL_ASSERT( glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureTarget, _renderTargets[index]->getHandle(), mipmapLevel));

        }
        else if (target->getFormat() == Image::DEPTH24_STENCIL8) {
            attachment = GL_DEPTH_STENCIL_ATTACHMENT;
            GL_ASSERT( glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureTarget, _renderTargets[index]->getHandle(), mipmapLevel));
        }
        else
        {
            attachment = GL_COLOR_ATTACHMENT0 + index;
            GL_ASSERT( glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureTarget, _renderTargets[index]->getHandle(), mipmapLevel) );
        }

        // Restore the FBO binding
        GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _renderer->_currentFrameBuffer->_handle) );
    }
}

bool GLFrameBuffer::check() {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _handle));
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        GP_ERROR("Framebuffer status incomplete: 0x%x", fboStatus);
    }
    // Restore the FBO binding
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _renderer->_currentFrameBuffer->_handle));
    return fboStatus == GL_FRAMEBUFFER_COMPLETE;
}

void GLFrameBuffer::disableDrawBuffer() {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _handle));
#if !defined(OPENGL_ES) && !defined(__EMSCRIPTEN__)
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
#elif defined(GL_ES_VERSION_3_0) && GL_ES_VERSION_3_0
    glDrawBuffers(0, NULL);
#endif
    // Restore the FBO binding
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _renderer->_currentFrameBuffer->_handle));
}

Texture* GLFrameBuffer::getRenderTarget(unsigned int index) const
{
    GP_ASSERT(_renderTargets);
    if (index < _maxRenderTargets)
    {
        return _renderTargets[index];
    }
    return NULL;
}

unsigned int GLFrameBuffer::getRenderTargetCount() const
{
    return _renderTargetCount;
}

void GLFrameBuffer::setDepthStencilTarget(DepthStencilTarget* target)
{
    if (_depthStencilTarget == target)
        return;

    // Release our existing depth stencil target.
    SAFE_RELEASE(_depthStencilTarget);

    _depthStencilTarget = target;

    if (target)
    {
        // The GLFrameBuffer now owns this DepthStencilTarget.
        target->addRef();

        // Now set this target as the color attachment corresponding to index.
        GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _handle) );

        // Attach the render buffer to the framebuffer
        GL_ASSERT( glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthStencilTarget->_depthBuffer) );
        if (target->isPacked())
        {
            GL_ASSERT( glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilTarget->_depthBuffer) );
        }
        else if (target->getFormat() == DepthStencilTarget::DEPTH_STENCIL)
        {
            GL_ASSERT( glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilTarget->_stencilBuffer) );
        }

        // Check the framebuffer is good to go.
        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            GP_ERROR("Framebuffer status incomplete: 0x%x", fboStatus);
        }

        // Restore the FBO binding
        GL_ASSERT( glBindFramebuffer(GL_FRAMEBUFFER, _renderer->_currentFrameBuffer->_handle) );
    }
}

void GLFrameBuffer::createDepthStencilTarget(int format) {
    UPtr<DepthStencilTarget> dst = DepthStencilTarget::create(this->_id.c_str(), format == 0 ? DepthStencilTarget::DEPTH : DepthStencilTarget::DEPTH_STENCIL, getWidth(), getHeight());
    setDepthStencilTarget(dst.get());
    //dst->release();
}

DepthStencilTarget* GLFrameBuffer::getDepthStencilTarget() const
{
    return _depthStencilTarget;
}

bool GLFrameBuffer::isDefault() const
{
    return (this == _renderer->_defaultFrameBuffer);
}

FrameBuffer* GLFrameBuffer::bind(Type type)
{
    GLenum glType = GL_FRAMEBUFFER;
    if (type == Read) {
        glType = GL_READ_FRAMEBUFFER;
    }
    else if (type == Draw) {
        glType = GL_DRAW_FRAMEBUFFER;
    }

    GL_ASSERT( glBindFramebuffer(glType, _handle) );
    GLFrameBuffer* previousGLFrameBuffer = _renderer->_currentFrameBuffer;
    _renderer->_currentFrameBuffer = this;
    return previousGLFrameBuffer;
}

void GLFrameBuffer::getScreenshot(Image* image)
{
    GP_ASSERT( image );

    unsigned int width = getWidth();
    unsigned int height = getHeight();

    if (image->getWidth() == width && image->getHeight() == height) {
        GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _handle));
        GLenum format = image->getFormat() == Image::RGB ? GL_RGB : GL_RGBA;
        GL_ASSERT( glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, image->getData()) );
        GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, _renderer->_currentFrameBuffer->_handle));
    }
}

UPtr<Image> GLFrameBuffer::createScreenshot(Image::Format format)
{
    UPtr<Image> screenshot = Image::create(getWidth(), getHeight(), format, NULL, true, true);
    getScreenshot(screenshot.get());

    return screenshot;
}

// FrameBuffer* GLFrameBuffer::bindDefault(GLenum type)
// {
//     GL_ASSERT( glBindFramebuffer(type, _renderer->_defaultFrameBuffer->_handle) );
//     _renderer->_currentFrameBuffer = _renderer->_defaultFrameBuffer;
//     return _renderer->_defaultFrameBuffer;
// }

// FrameBuffer* GLFrameBuffer::getCurrent()
// {
//     return _renderer->_currentFrameBuffer;
// }

}

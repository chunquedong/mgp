#ifndef GLFRAMEBUFFER_H_
#define GLFRAMEBUFFER_H_

#include "base/Base.h"
#include "material/Texture.h"
#include "DepthStencilTarget.h"
#include "material/Image.h"
#include "render/FrameBuffer.h"

namespace mgp
{
typedef unsigned int FrameBufferHandle;

class GLRenderer;

/**
 * Defines a frame buffer object that may contain one or more render targets and optionally
 * a depth-stencil target.
 *
 * Frame buffers can be created and used for off-screen rendering, which is useful for
 * techniques such as shadow mapping and post-processing. Render targets within a frame
 * buffer can be both written to and read (by calling RenderTarget::getTexture).
 *
 * When binding a custom frame buffer, you should always store the return value of
 * FrameBuffer::bind and restore it when you are finished drawing to your frame buffer.
 *
 * To bind the default frame buffer, call FrameBuffer::bindDefault.
 */
class GLFrameBuffer : public FrameBuffer
{
    friend class GLRenderer;
public:


    /**
     * Creates a new, empty FrameBuffer object.
     *
     * The new FrameBuffer does not have any render targets or a depth/stencil target and these
     * must be added before it can be used. The FrameBuffer is added to the list of available
     * FrameBuffers.
     *
     * @param id The ID of the new FrameBuffer. Uniqueness is recommended but not enforced.
     *
     * @return A newly created FrameBuffer.
     * @script{create}
     */
    static UPtr<FrameBuffer> create(GLRenderer* renderer, const char* id);

    /**
     * Creates a new FrameBuffer with a single RenderTarget of the specified width and height,
     * and adds the FrameBuffer to the list of available FrameBuffers.
     *
     * If width and height are non-zero a default RenderTarget of type RGBA will be created
     * and added to the FrameBuffer, with the same ID. The ID of the render target can be
     * changed later via RenderTarget::setId(const char*).
     *
     * You can additionally add a DepthStencilTarget using FrameBuffer::setDepthStencilTarget.
     *
     * @param id The ID of the new FrameBuffer. Uniqueness is recommended but not enforced.
     * @param width The width of the RenderTarget to be created and attached.
     * @param height The height of the RenderTarget to be created and attached.
     *
     * @return A newly created FrameBuffer.
     * @script{create}
     */
    static UPtr<FrameBuffer> create(GLRenderer* renderer, const char* id, 
        unsigned int width, unsigned int height, Image::Format format = Image::RGBA);

    /**
     * Get the ID of this FrameBuffer.
     *
     * @return The ID of this FrameBuffer.
     */
    const char* getId() const;

    /**
     * Gets the width of the frame buffer.
     *
     * @return The width of the frame buffer.
     */
    unsigned int getWidth() const;

    /**
     * Gets the height of the frame buffer.
     *
     * @return The height of the frame buffer.
     */
    unsigned int getHeight() const;

    /**
     * Get the number of color attachments available on the current hardware.
     *
     * @return The number of color attachments available on the current hardware.
     */
    unsigned int getMaxRenderTargets() const;

    /**
     * Set a RenderTarget on this FrameBuffer's color attachment at the specified index.
     *
     * @param target The 2D RenderTarget to set.
     * @param index The index of the color attachment to set.
     */
    void setRenderTarget(Texture* target, unsigned int index = 0);

    /**
    * Set a RenderTarget on this FrameBuffer's color attachment at the specified index.
    *
    * @param target The Cubemap RenderTarget to set.
    * @param face The face of the cubemap to target.
    * @param index The index of the color attachment to set.
    */
    void setRenderTarget(Texture* target, Texture::CubeFace face, int mipmapLevel, unsigned int index = 0);

    /**
     * Get the RenderTarget attached to the FrameBuffer's color attachment at the specified index.
     *
     * @param index The index of the color attachment to retrieve a RenderTarget from.
     *
     * @return The RenderTarget attached at the specified index.
     */
    Texture* getRenderTarget(unsigned int index = 0) const;

    /**
     * Returns the current number of render targets attached to this frame buffer.
     *
     * @return The number of render targets attached.
     */
    unsigned int getRenderTargetCount() const;

    /**
     * Set this FrameBuffer's DepthStencilTarget.
     *
     * @param target The DepthStencilTarget to set on this FrameBuffer.
     */
    void setDepthStencilTarget(DepthStencilTarget* target);

    void createDepthStencilTarget(int format = 1);

    void disableDrawBuffer();

    bool check();

    /**
     * Get this FrameBuffer's DepthStencilTarget.
     *
     * @return This FrameBuffer's DepthStencilTarget.
     */
    DepthStencilTarget* getDepthStencilTarget() const;

    /**
     * Determines whether this is the default frame buffer.
     *
     * @return true if this is the default frame buffer, false otherwise.
     */
    bool isDefault() const;

    /**
     * Binds this FrameBuffer for off-screen rendering and return you the currently bound one.
     *
     * You should keep the return FrameBuffer and store it and call bind() when you rendering is complete.
     *
     * @ return The currently bound framebuffer.
     */
    FrameBuffer* bind(Type type = ReadWrite);

    /**
     * Records a screenshot of what is stored on the current FrameBuffer.
     *
     * @param format The format the Image should be in.
     * @return A screenshot of the current framebuffer's content.
     */
    UPtr<Image> createScreenshot(Image::Format format = Image::RGBA);

    /**
     * Records a screenshot of what is stored on the current FrameBuffer to an Image.
     *
     * The Image must be the same size as the FrameBuffer, otherwise the operation will fail.
     *
     * @param image The Image to write the current framebuffer's content to.
     */
    void getScreenshot(Image* image);

    // /**
    //  * Binds the default FrameBuffer for rendering to the display.
    //  *
    //  * @ return The default framebuffer.
    //  */
    // static FrameBuffer* bindDefault(unsigned int type = 0x8D40/*GL_FRAMEBUFFER*/);

    // /**
    //  * Gets the currently bound FrameBuffer.
    //  *
    //  * @return The currently bound FrameBuffer.
    //  */
    // static FrameBuffer* getCurrent();

private:
    friend class GLRenderer;
    /**
     * Constructor.
     */
    GLFrameBuffer(const char* id, unsigned int width, unsigned int height, FrameBufferHandle handle);

    /**
     * Destructor.
     */
    ~GLFrameBuffer();

    /**
     * Hidden copy assignment operator.
     */
    FrameBuffer& operator=(const FrameBuffer&);

    void setRenderTarget(Texture* target, unsigned int index, unsigned int textureTarget, int mipmapLevel);

    static void initialize();

    //static bool isPowerOfTwo(unsigned int value);

    std::string _id;
    FrameBufferHandle _handle;
    Texture** _renderTargets;
    unsigned int _renderTargetCount;
    DepthStencilTarget* _depthStencilTarget;

    static unsigned int _maxRenderTargets;

    GLRenderer* _renderer = NULL;
};

}

#endif

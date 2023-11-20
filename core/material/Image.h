#ifndef IMAGE_H__
#define IMAGE_H__

#include "base/Ref.h"

namespace mgp
{

/**
 * Defines an image buffer of RGB or RGBA color data.
 *
 * Currently only supports loading from .png image files.
 */
class Image : public Refable
{
public:

    /**
     * Defines the set of supported color formats.
     */
    enum Format
    {
        RGB,
        RGBA,
        RGBF,
        RGBAF,
    };

    /**
     * Creates an image from the image file at the given path.
     *
     * @param path The path to the image file.
     * @return The newly created image.
     * @script{create}
     */
    static UPtr<Image> create(const char* path, bool flipY = true);
    static UPtr<Image> createHDR(const char* path, bool flipY = true);
    static UPtr<Image> createFromBuf(const char* file_data, size_t file_size, bool flipY = true);

    /**
     * Creates an image from the data provided
     *
     * @param width The width of the image data.
     * @param height The height of the image data.
     * @param format The format of the image data.
     * @param data The image data. If NULL, the data will be allocated.
     * @return The newly created image.
     * @script{create}
     */
    static UPtr<Image> create(unsigned int width, unsigned int height, Format format, unsigned char* data = NULL, bool copy = true);

    /**
     * Gets the image's raw pixel data.
     *
     * @return The image's pixel data.
     * @script{ignore}
     */
    inline unsigned char* getData() const;
    void setData(unsigned char* data);

    /**
     * Gets the image's format.
     *
     * @return The image's format.
     */
    inline Format getFormat() const;

    /**
     * Gets the height of the image.
     *
     * @return The height of the image.
     */
    inline unsigned int getHeight() const;

    /**
     * Gets the width of the image.
     *
     * @return The width of the image.
     */
    inline unsigned int getWidth() const;


    /**
    * write image to file
    * @format image format png jpg
    */
    bool save(const char* file, const char* format = "png");

private:

    /**
     * Constructor.
     */
    Image();

    /**
     * Destructor.
     */
    ~Image();

    /**
     * Hidden copy assignment operator.
     */
    Image& operator=(const Image&);

    unsigned char* _data;
    Format _format;
    unsigned int _width;
    unsigned int _height;
};

}

#include "Image.inl"

#endif

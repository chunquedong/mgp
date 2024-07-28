#ifndef IMAGE_H__
#define IMAGE_H__

#include "base/Ref.h"
#include "base/Resource.h"

namespace mgp
{

/**
 * Defines an image buffer of RGB or RGBA color data.
 *
 * Currently only supports loading from .png image files.
 */
class Image : public Refable
{
    friend class Texture;
public:

    /**
     * Defines the set of supported color formats.
     */
    enum Format
    {
        UNKNOWN = 0,
        //auto size type
        RGB,
        RGBA,
        ALPHA,
        RED,
        RG,

        //fix size type
        RGB888,
        RGB565,
        RGBA4444,
        RGBA5551,
        RGBA8888,

        //depth
        DEPTH,
        DEPTH24_STENCIL8,

        //float type
        RGB16F,
        RGBA16F,
        R16F,
        R11F_G11F_B10F,
        RGB9_E5,
        R32F,
        RGB32F,
        RGBA32F,
        RG16F,
    };

    /**
     * Creates an image from the image file at the given path.
     *
     * @param path The path to the image file.
     * @return The newly created image.
     * @script{create}
     */
    static UPtr<Image> create(const char* path, bool flipY = false);
    static UPtr<Image> createHDR(const char* path, bool flipY = false);
    static UPtr<Image> createFromBuf(const char* file_data, size_t file_size, bool flipY = false);

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
    static UPtr<Image> create(unsigned int width, unsigned int height, Format format, unsigned char* data = NULL, bool copy = true, bool alloc = false);

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
    * byte size per pixcel
    */
    static size_t getFormatBPP(Format format);

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
    bool save(const char* file, const char* format = NULL);


    std::string& getFilePath() { return _filePath; }

    /**
     * Constructor.
     */
    Image();

    /**
    * set default formt to save
    */
    void setDefaultFileFormat(const char* format);

    void flipY();
private:
    /**
     * Destructor.
     */
    ~Image();

    /**
     * Hidden copy assignment operator.
     */
    Image& operator=(const Image&) = delete;

    void write(Stream* file);
    bool read(Stream* file);

    unsigned char* _data;
    Format _format;
    unsigned int _width;
    unsigned int _height;
    std::string _filePath;
    std::string _defaultFileFormat;
};

}

#include "Image.inl"

#endif

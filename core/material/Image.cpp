#include "base/Base.h"
#include "base/FileSystem.h"
#include "Image.h"
#include "base/StringUtil.h"

using namespace mgp;

#if USE_PNG
// Image
#include <png.h>

// Callback for reading a png image using Stream
static void readStream(png_structp png, png_bytep data, png_size_t length)
{
    Stream* stream = reinterpret_cast<Stream*>(png_get_io_ptr(png));
    if (stream == NULL || stream->read(data, 1, length) != length)
    {
        png_error(png, "Error reading PNG.");
    }
}

Image* Image::create(const char* path)
{
    GP_ASSERT(path);

    // Open the file.
    std::unique_ptr<Stream> stream(FileSystem::open(path));
    if (stream.get() == NULL || !stream->canRead())
    {
        GP_ERROR("Failed to open image file '%s'.", path);
        return NULL;
    }

    // Verify PNG signature.
    unsigned char sig[8];
    if (stream->read(sig, 1, 8) != 8 || png_sig_cmp(sig, 0, 8) != 0)
    {
        GP_ERROR("Failed to load file '%s'; not a valid PNG.", path);
        return NULL;
    }

    // Initialize png read struct (last three parameters use stderr+longjump if NULL).
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL)
    {
        GP_ERROR("Failed to create PNG structure for reading PNG file '%s'.", path);
        return NULL;
    }

    // Initialize info struct.
    png_infop info = png_create_info_struct(png);
    if (info == NULL)
    {
        GP_ERROR("Failed to create PNG info structure for PNG file '%s'.", path);
        png_destroy_read_struct(&png, NULL, NULL);
        return NULL;
    }

    // Set up error handling (required without using custom error handlers above).
    if (setjmp(png_jmpbuf(png)))
    {
        GP_ERROR("Failed to set up error handling for reading PNG file '%s'.", path);
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    // Initialize file io.
    png_set_read_fn(png, stream.get(), readStream);

    // Indicate that we already read the first 8 bytes (signature).
    png_set_sig_bytes(png, 8);

    // Read the entire image into memory.
    png_read_png(png, info, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_GRAY_TO_RGB, NULL);

    Image* image = new Image();
    image->_width = png_get_image_width(png, info);
    image->_height = png_get_image_height(png, info);

    png_byte colorType = png_get_color_type(png, info);
    switch (colorType)
    {
    case PNG_COLOR_TYPE_RGBA:
        image->_format = Image::RGBA;
        break;

    case PNG_COLOR_TYPE_RGB:
        image->_format = Image::RGB;
        break;

    default:
        GP_ERROR("Unsupported PNG color type (%d) for image file '%s'.", (int)colorType, path);
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    size_t stride = png_get_rowbytes(png, info);

    // Allocate image data.
    image->_data = new unsigned char[stride * image->_height];

    // Read rows into image data.
    png_bytepp rows = png_get_rows(png, info);
    for (unsigned int i = 0; i < image->_height; ++i)
    {
        memcpy(image->_data+(stride * (image->_height-1-i)), rows[i], stride);
    }

    // Clean up.
    png_destroy_read_struct(&png, &info, NULL);

    return image;
}

#else

#define STB_IMAGE_IMPLEMENTATION
#include "../3rd/stb_image.h"
namespace mgp {
    void getFullPath(const char* path, std::string& fullPath);
}

UPtr<Image> Image::createFromBuf(const char* file_data, size_t file_size, bool flipY) {
    stbi_set_flip_vertically_on_load_thread(flipY);

    int iw, ih, n;
    unsigned char* data = stbi_load_from_memory((const stbi_uc*)file_data, file_size , &iw, &ih, &n, 0);
    if (!data) return UPtr<Image>(NULL);

    Image* image = new Image();
    image->_width = iw;
    image->_height = ih;

    switch (n)
    {
    case 4:
        image->_format = Image::RGBA;
        break;

    case 3:
        image->_format = Image::RGB;
        break;

    case 1:
        image->_format = Image::RED;
        break;

    case 2:
        image->_format = Image::RG;
        break;

    default:
        GP_ERROR("Unsupported PNG color type (%d) for image buffer.", (int)n);
        stbi_image_free(data);
        delete image;
        return UPtr<Image>(NULL);
    }
    image->_data = data;
    //stbi_image_free(data);
    return UPtr<Image>(image);
}

UPtr<Image> Image::createHDR(const char* path, bool flipY) {
    GP_ASSERT(path);

    std::string fullPath;
    mgp::getFullPath(path, fullPath);

    stbi_set_flip_vertically_on_load(flipY);

    int iw, ih, n;
    float* data = stbi_loadf(fullPath.c_str(), &iw, &ih, &n, 0);
    if (!data) return UPtr<Image>(NULL);

    Image* image = new Image();
    image->_width = iw;
    image->_height = ih;

    switch (n)
    {
    case 4:
        image->_format = Image::RGBA16F;
        break;

    case 3:
        image->_format = Image::RGB16F;
        break;

    default:
        GP_ERROR("Unsupported PNG color type (%d) for image file '%s'.", (int)n, path);
        stbi_image_free(data);
        delete image;
        return UPtr<Image>(NULL);
    }
    image->_data = (unsigned char*)data;
    //stbi_image_free(data);
    return UPtr<Image>(image);
}

UPtr<Image> Image::create(const char* path, bool flipY)
{
    GP_ASSERT(path);

    if (StringUtil::endsWith(path, ".hdr")) {
        return createHDR(path, flipY);
    }

    std::string fullPath;
    mgp::getFullPath(path, fullPath);

    stbi_set_flip_vertically_on_load(flipY);

    int iw, ih, n;
    unsigned char* data = stbi_load(fullPath.c_str(), &iw, &ih, &n, 0);
    if (!data) return UPtr<Image>(NULL);

    Image* image = new Image();
    image->_width = iw;
    image->_height = ih;

    switch (n)
    {
    case 4:
        image->_format = Image::RGBA;
        break;

    case 3:
        image->_format = Image::RGB;
        break;

    default:
        GP_ERROR("Unsupported PNG color type (%d) for image file '%s'.", (int)n, path);
        stbi_image_free(data);
        delete image;
        return UPtr<Image>(NULL);
    }
    image->_data = data;

    std::string name = FileSystem::getBaseName(path)+ FileSystem::getExtension(path, false);
    image->_id = name;
    image->_filePath = path;
    //stbi_image_free(data);
    return UPtr<Image>(image);
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "3rd/stb_image_write.h"
bool Image::save(const char* file, const char* format) {
    unsigned int pixelSize = 0;
    switch (_format)
    {
    case Image::RGB:
        pixelSize = 3;
        break;
    case Image::RGBA:
        pixelSize = 4;
        break;
    default:
        GP_WARN("unsupport save image: %d", _format);
        return false;
    }

    if (strcmp(format, "png") == 0) {
        return stbi_write_png(file, _width, _height, pixelSize, _data, _width * pixelSize);
    }
    else if (strcmp(format, "jpg") == 0) {
        return stbi_write_jpg(file, _width, _height, pixelSize, _data, _width * pixelSize);
    }
    return false;
}

#endif


UPtr<Image> Image::create(unsigned int width, unsigned int height, Image::Format format, unsigned char* data, bool copy)
{
    GP_ASSERT(width > 0 && height > 0);

    unsigned int pixelSize = getFormatBPP(format);

    Image* image = new Image();

    unsigned int dataSize = width * height * pixelSize;

    image->_width = width;
    image->_height = height;
    image->_format = format;

    if (copy && data) {
        image->_data = (unsigned char*)malloc(dataSize);
        if (data)
            memcpy(image->_data, data, dataSize);
        else
            memset(image->_data, 0, dataSize);
    }
    else {
        image->_data = data;
    }
    return UPtr<Image>(image);
}

Image::Image() : _data(NULL), _format(RGB), _width(0), _height(0), Resource(genId()+".image")
{
}

Image::~Image()
{
   free(_data);
}

size_t Image::getFormatBPP(Format format)
{
    switch (format)
    {
    case Image::UNKNOWN:
        return 1;
        //auto size type
    case Image::RGB:
        return 3;
    case Image::RGBA:
        return 4;
    case Image::ALPHA:
        return 1;
    case Image::RED:
        return 1;
    case Image::RG:
        return 2;

        //fix size type
    case Image::RGB888:
        return 3;
    case Image::RGB565:
        return 2;
    case Image::RGBA4444:
        return 2;
    case Image::RGBA5551:
        return 2;
    case Image::RGBA8888:
        return 4;

        //depth
    case Image::DEPTH:
        return 4;
    case Image::DEPTH24_STENCIL8:
        return 4;

        //float type
    case Image::RGB16F:
        return 6;
    case Image::RGBA16F:
        return 8;
    case Image::R16F:
        return 2;
    case Image::R11F_G11F_B10F:
        return 4;
    case Image::RGB9_E5:
        return 4;
    case Image::RGB32F:
        return 4 * 3;
    case Image::RGBA32F:
        return 4 * 4;
    case Image::RG16F:
        return 4 * 2;
    default:
        return 0;
    }
}

void Image::write(Stream* file) {
    if (!_data) return;
    int bpp = getFormatBPP(_format);
    int size = bpp * _width * _height;
    file->writeUInt16(_format);
    file->writeUInt16(_width);
    file->writeUInt16(_height);
    file->write((const char*)_data, size);
}

bool Image::read(Stream* file) {
    _format = (Format)file->readUInt16();
    _width = file->readUInt16();
    _height = file->readUInt16();

    GP_ASSERT(_data == NULL);
    int bpp = getFormatBPP(_format);
    int size = bpp * _width * _height;
    _data = (unsigned char*)malloc(size);
    file->read((char*)_data, size);
    return true;
}
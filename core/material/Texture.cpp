#include "base/Base.h"
#include "scene/Renderer.h"
#include "material/Image.h"
#include "Texture.h"
#include "base/FileSystem.h"
#include <algorithm>

namespace mgp
{

class CompressedTexture {
public:
    static UPtr<Texture> createCompressedDdsKtx(const char* path);

    static UPtr<Texture> createCompressedPVRTC(const char* path);

    static UPtr<Texture> createCompressedDDS(const char* path);

};

static std::vector<Texture*> __textureCache;

Texture::Texture() : _handle(0), _format(UNKNOWN), _type((Texture::Type)0), _width(0), _height(0), _arrayDepth(0), _mipmapped(false), _cached(false), _compressed(false),
    _wrapS(Texture::REPEAT), _wrapT(Texture::REPEAT), _wrapR(Texture::REPEAT), _minFilter(Texture::NEAREST), _magFilter(Texture::LINEAR),
    _keepMemory(false), _dataDirty(false), _data(NULL)
{
}

Texture::~Texture()
{
    if (_data) {
        free((void*)_data);
        _data = NULL;
    }
    Renderer::cur()->deleteTexture(this);

    // Remove ourself from the texture cache.
    if (_cached)
    {
        std::vector<Texture*>::iterator itr = std::find(__textureCache.begin(), __textureCache.end(), this);
        if (itr != __textureCache.end())
        {
            __textureCache.erase(itr);
        }
    }
}

static Texture::Format imageFormatToTexture(Image::Format imgFormat) {
    Texture::Format _format;
    switch (imgFormat)
    {
    case Image::RGB:
        _format = Texture::RGB;
        break;
    case Image::RGBA:
        _format = Texture::RGBA;
        break;
    case Image::RGBF:
        _format = Texture::RGB16F;
        break;
    case Image::RGBAF:
        _format = Texture::RGBA16F;
        break;
    default:
        GP_ERROR("Unsupported image format (%d).", imgFormat);
        _format = Texture::UNKNOWN;
    }
    return _format;
}

bool Texture::load(const char* path) {
    UPtr<Image> image = Image::create(path);
    if (!image.get()) {
        return false;
    }
    _format = imageFormatToTexture(image->getFormat());
    _width = image->getWidth();
    _height = image->getHeight();
    _data = image->getData();
    image->setData(NULL);
    _dataDirty = true;
    
    //image->release();
    return true;
}

UPtr<Texture> Texture::create(const char* path, bool generateMipmaps)
{
    GP_ASSERT( path );

    // Search texture cache first.
    for (size_t i = 0, count = __textureCache.size(); i < count; ++i)
    {
        Texture* t = __textureCache[i];
        GP_ASSERT( t );
        if (t->_path == path)
        {
            // If 'generateMipmaps' is true, call Texture::generateMipamps() to force the
            // texture to generate its mipmap chain if it hasn't already done so.
            if (generateMipmaps)
            {
                t->_mipmapped = true;
            }

            // Found a match.
            t->addRef();

            return UPtr<Texture>(t);
        }
    }

    UPtr<Texture> texture;

    // Filter loading based on file extension.
    const char* ext = strrchr(FileSystem::resolvePath(path), '.');
    if (ext)
    {
        switch (strlen(ext))
        {
        case 4:
            if ((tolower(ext[1]) == 'p' && tolower(ext[2]) == 'n' && tolower(ext[3]) == 'g') ||
                (tolower(ext[1]) == 'j' && tolower(ext[2]) == 'p' && tolower(ext[3]) == 'g') ||
                (tolower(ext[1]) == 'h' && tolower(ext[2]) == 'd' && tolower(ext[3]) == 'r')
                )
            {
                bool flipY = false;
                UPtr<Image> image = Image::create(path, flipY);
                if (image.get())
                    texture = create(image.get(), generateMipmaps);
                //SAFE_RELEASE(image);
            }
            else if (tolower(ext[1]) == 'p' && tolower(ext[2]) == 'v' && tolower(ext[3]) == 'r')
            {
                // PowerVR Compressed Texture RGBA.
                texture = CompressedTexture::createCompressedPVRTC(path);
            }
            else if (tolower(ext[1]) == 'd' && tolower(ext[2]) == 'd' && tolower(ext[3]) == 's')
            {
                // DDS file format (DXT/S3TC) compressed textures
                texture = CompressedTexture::createCompressedDDS(path);
            }
            else if (tolower(ext[1]) == 'k' && tolower(ext[2]) == 't' && tolower(ext[3]) == 'x')
            {
                // KTX file format compressed textures
                texture = CompressedTexture::createCompressedDdsKtx(path);
            }
            break;
        }
    }

    if (texture.get())
    {
        texture->_path = path;
        texture->_cached = true;

        // Add to texture cache.
        __textureCache.push_back(texture.get());

        return texture;
    }

    GP_ERROR("Failed to load texture from file '%s'.", path);
    return UPtr<Texture>(NULL);
}

UPtr<Texture> Texture::create(Image* image, bool generateMipmaps, bool copyData)
{
    GP_ASSERT( image );
    const unsigned char* data = image->getData();

    Texture::Format _format = imageFormatToTexture(image->getFormat());
    image->setData(NULL);
    return create(_format, image->getWidth(), image->getHeight(), data, generateMipmaps, TEXTURE_2D, false);
}

UPtr<Texture> Texture::create(Format format, unsigned int width, unsigned int height, const unsigned char* data,
    bool generateMipmaps, Texture::Type type, bool copyData, unsigned int arrayDepth)
{
    Texture* texture = new Texture();
    // Set initial minification filter based on whether or not mipmaping was enabled.
    Filter minFilter;
    if (format == Texture::DEPTH)
    {
    	minFilter = NEAREST;
    }
    else
    {
    	minFilter = generateMipmaps ? NEAREST_MIPMAP_LINEAR : LINEAR;
    }

    if (type == Texture::TEXTURE_CUBE || format == Texture::DEPTH) {
        texture->_wrapR = CLAMP;
        texture->_wrapS = CLAMP;
        texture->_wrapT = CLAMP;
    }

    texture->_handle = 0;
    texture->_format = format;
    texture->_type = type;
    texture->_width = width;
    texture->_height = height;
    texture->_arrayDepth = arrayDepth;
    texture->_minFilter = minFilter;
    texture->_mipmapped = generateMipmaps;
    if (copyData) {
        texture->_data = data;
        Renderer::cur()->updateTexture(texture);
        texture->_data = NULL;
    }
    else {
        texture->_data = data;
        texture->_dataDirty = true;
    }
    return UPtr<Texture>(texture);
}

size_t Texture::getFormatBPP(Texture::Format format)
{
    switch (format)
    {
    case Texture::UNKNOWN:
        return 0;
        //auto size type
    case Texture::RGB:
        return 3;
    case Texture::RGBA:
        return 4;
    case Texture::ALPHA:
        return 1;
    case Texture::RED:
        return 1;
    case Texture::RG:
        return 2;

        //fix size type
    case Texture::RGB888:
        return 3;
    case Texture::RGB565:
        return 2;
    case Texture::RGBA4444:
        return 2;
    case Texture::RGBA5551:
        return 2;
    case Texture::RGBA8888:
        return 4;

        //depth
    case Texture::DEPTH:
        return 4;
    case Texture::DEPTH24_STENCIL8:
        return 4;

        //float type
    case Texture::RGB16F:
        return 6;
    case Texture::RGBA16F:
        return 8;
    case Texture::R16F:
        return 2;
    case Texture::R11F_G11F_B10F:
        return 4;
    case Texture::RGB9_E5:
        return 4;
    case Texture::RGB32F:
        return 4*3;
    case Texture::RGBA32F:
        return 4 * 4;
    case Texture::RG16F:
        return 4 * 2;
    default:
        return 0;
    }
}

UPtr<Texture> Texture::loadCubeMap(const char* faces[]) {
    unsigned char* data = NULL;
    int width;
    int height;
    Format format;
    for (int i=0; i<6; ++i) {
        const char *url = faces[i];
        UPtr<Image> image = Image::create(url, false);
        if (!image.get()) {
            GP_ERROR("image load fail: %s\n", url);
            return UPtr<Texture>(nullptr);
        }
        int bpp = 4;
        switch (image->getFormat())
        {
        case Image::RGB:
            format = Texture::RGB;
            bpp = 3;
            break;
        case Image::RGBA:
            format = Texture::RGBA;
            bpp = 4;
            break;
        case Image::RGBF:
            format = Texture::RGB16F;
            bpp = 3*4;
            break;
        case Image::RGBAF:
            format = Texture::RGBA16F;
            bpp = 4*4;
            break;
        default:
            GP_ERROR("Unsupported image format (%d).", image->getFormat());
            return UPtr<Texture>(NULL);
        }

        width = image->getWidth();
        height = image->getHeight();
        int imageDataSize = image->getWidth() * image->getHeight() * bpp;
        if (!data) {
            data = (unsigned char*)malloc(imageDataSize * 6);
        }

        memcpy(data+(imageDataSize*i), image->getData(), imageDataSize);

        //SAFE_RELEASE(image);
    }

    Texture *texture = new Texture();
    texture->_type = TEXTURE_CUBE;
    texture->_format = format;
    texture->_width = width;
    texture->_height = height;
    texture->_data = data;
    texture->_minFilter = NEAREST;
    texture->_wrapR = CLAMP;
    texture->_wrapS = CLAMP;
    texture->_wrapT = CLAMP;
    Renderer::cur()->updateTexture(texture);
    texture->_data = NULL;
    free(data);
    return UPtr<Texture>(texture);
}

//void Texture::generateMipmaps() {
//    _generateMipmaps = true;
//}

void Texture::setData(const unsigned char* data, bool copyMem)
{
    // Don't work with any compressed or cached textures
    GP_ASSERT(data);

    if (_data == data) {
        _dataDirty = true;
        return;
    }

    free((void*)_data);
    if (copyMem) {
        if (!_keepMemory) {
            _data = data;
            Renderer::cur()->updateTexture(this);
            _data = NULL;
            _dataDirty = false;
        }
        else {
            int bpp = getFormatBPP(_format);
            int size = bpp * _width * _height;
            unsigned char* t = (unsigned char*)malloc(size);
            memcpy(t, data, size);
            this->_data = t;
            _dataDirty = true;
        }
    }
    else {
        this->_data = data;
        _dataDirty = true;
    }
}

void Texture::setKeepMemory(bool b) {
    _keepMemory = b;
}

Texture::Format Texture::getFormat() const
{
    return _format;
}

Texture::Type Texture::getType() const
{
    return _type;
}

const char* Texture::getPath() const
{
    return _path.c_str();
}

unsigned int Texture::getWidth() const
{
    return _width;
}

unsigned int Texture::getHeight() const
{
    return _height;
}

unsigned int Texture::getArrayDepth() const {
    return _arrayDepth;
}

TextureHandle Texture::getHandle() const
{
    return _handle;
}

bool Texture::isMipmapped() const
{
    return _mipmapped;
}

bool Texture::isCompressed() const
{
    return _compressed;
}

Serializable* Texture::createObject() {
    return new Texture();
}


std::string Texture::enumToString(const std::string& enumName, int value)
{
    if (enumName.compare("mgp::Texture::Format") == 0)
    {
        switch (value)
        {
            case static_cast<int>(Format::UNKNOWN) :
                return "UNKNOWN";
            //case static_cast<int>(Format::RGB) :
            //    return "RGB";
            case static_cast<int>(Format::RGB888) :
                return "RGB888";
            case static_cast<int>(Format::RGB565) :
                return "RGB565";
            //case static_cast<int>(Format::RGBA) :
            //    return "RGBA";
            case static_cast<int>(Format::RGBA8888) :
                return "RGBA8888";
            case static_cast<int>(Format::RGBA4444) :
                return "RGBA4444";
            case static_cast<int>(Format::RGBA5551) :
                return "RGBA5551";
            case static_cast<int>(Format::ALPHA) :
                return "ALPHA";
            case static_cast<int>(Format::DEPTH) :
                return "DEPTH";
            default:
                return "RGBA";
        }
    }
    else if (enumName.compare("mgp::Texture::Filter") == 0)
    {
        switch (value)
        {
            case static_cast<int>(Filter::NEAREST) :
                return "NEAREST";
            case static_cast<int>(Filter::LINEAR) :
                return "LINEAR";
            case static_cast<int>(Filter::NEAREST_MIPMAP_NEAREST) :
                return "NEAREST_MIPMAP_NEAREST";
            case static_cast<int>(Filter::LINEAR_MIPMAP_NEAREST) :
                return "LINEAR_MIPMAP_NEAREST";
            case static_cast<int>(Filter::NEAREST_MIPMAP_LINEAR) :
                return "NEAREST_MIPMAP_LINEAR";
            case static_cast<int>(Filter::LINEAR_MIPMAP_LINEAR) :
                return "LINEAR_MIPMAP_LINEAR";
            default:
                return "NEAREST";
        }
    }
    else if (enumName.compare("mgp::Texture::Wrap") == 0)
    {
        switch (value)
        {
            case static_cast<int>(Wrap::REPEAT) :
                return "REPEAT";
            case static_cast<int>(Wrap::CLAMP) :
                return "CLAMP";
            default:
                return "REPEAT";
        }
    }
    else if (enumName.compare("mgp::Texture::Type") == 0)
    {
        switch (value)
        {
            case static_cast<int>(Type::TEXTURE_2D) :
                return "TEXTURE_2D";
            case static_cast<int>(Type::TEXTURE_CUBE) :
                return "TEXTURE_CUBE";
            default:
                return "TEXTURE_2D";
        }
    }
    return "";
}

int Texture::enumParse(const std::string& enumName, const std::string& str)
{
    if (enumName.compare("mgp::Texture::Format") == 0)
    {
        if (str.compare("UNKNOWN") == 0)
            return static_cast<int>(Format::UNKNOWN);
        else if (str.compare("RGB888") == 0)
            return static_cast<int>(Format::RGB888);
        else if (str.compare("RGB565") == 0)
            return static_cast<int>(Format::RGB565);
        else if (str.compare("RGB") == 0)
            return static_cast<int>(Format::RGB);
        else if (str.compare("RGBA") == 0)
            return static_cast<int>(Format::RGBA);
        else if (str.compare("RGB565") == 0)
            return static_cast<int>(Format::RGBA8888);
        else if (str.compare("RGBA8888") == 0)
            return static_cast<int>(Format::RGBA4444);
        else if (str.compare("RGBA4444") == 0)
            return static_cast<int>(Format::RGBA5551);
        else if (str.compare("RGBA5551") == 0)
            return static_cast<int>(Format::ALPHA);
        else if (str.compare("ALPHA") == 0)
            return static_cast<int>(Format::RGB565);
        else if (str.compare("DEPTH") == 0)
            return static_cast<int>(Format::DEPTH);
    }
    else if (enumName.compare("mgp::Texture::Filter") == 0)
    {
        if (str.compare("NEAREST") == 0)
            return static_cast<int>(Filter::NEAREST);
        else if (str.compare("LINEAR") == 0)
            return static_cast<int>(Filter::LINEAR);
        else if (str.compare("NEAREST_MIPMAP_NEAREST") == 0)
            return static_cast<int>(Filter::NEAREST_MIPMAP_NEAREST);
        else if (str.compare("LINEAR_MIPMAP_NEAREST") == 0)
            return static_cast<int>(Filter::LINEAR_MIPMAP_NEAREST);
        else if (str.compare("NEAREST_MIPMAP_LINEAR") == 0)
            return static_cast<int>(Filter::NEAREST_MIPMAP_LINEAR);
        else if (str.compare("LINEAR_MIPMAP_LINEAR") == 0)
            return static_cast<int>(Filter::LINEAR_MIPMAP_LINEAR);
    }
    else if (enumName.compare("mgp::Texture::Wrap") == 0)
    {
        if (str.compare("REPEAT") == 0)
            return static_cast<int>(Wrap::REPEAT);
        else if (str.compare("CLAMP") == 0)
            return static_cast<int>(Wrap::CLAMP);
    }
    else if (enumName.compare("mgp::Texture::Type") == 0)
    {
        if (str.compare("TEXTURE_2D") == 0)
            return static_cast<int>(Type::TEXTURE_2D);
        else if (str.compare("TEXTURE_CUBE") == 0)
            return static_cast<int>(Type::TEXTURE_CUBE);
    }
    return static_cast<int>(0);
}

/**
 * @see Serializable::getClassName
 */
std::string Texture::getClassName() {
    return "mgp::Texture";
}

/**
 * @see Serializable::onSerialize
 */
void Texture::onSerialize(Serializer* serializer) {
    serializer->writeString("path", getPath(), "");
    serializer->writeEnum("minFilter", "mgp::Texture::Filter", static_cast<int>(_minFilter), -1);
    serializer->writeEnum("magFilter", "mgp::Texture::Filter", static_cast<int>(_magFilter), -1);

    serializer->writeEnum("wrapS", "mgp::Texture::Wrap", static_cast<int>(_wrapS), REPEAT);
    serializer->writeEnum("wrapT", "mgp::Texture::Wrap", static_cast<int>(_wrapT), REPEAT);
    serializer->writeEnum("wrapR", "mgp::Texture::Wrap", static_cast<int>(_wrapR), REPEAT);

    serializer->writeEnum("format", "mgp::Texture::Format", static_cast<int>(_format), RGBA8888);
    serializer->writeEnum("type", "mgp::Texture::Type", static_cast<int>(_type), TEXTURE_2D);
    serializer->writeBool("mipmap", _mipmapped, false);
}

/**
 * @see Serializable::onDeserialize
 */
void Texture::onDeserialize(Serializer* serializer) {
    std::string path;
    serializer->readString("path", path, "");

    this->load(path.c_str());
    _minFilter = static_cast<Texture::Filter>(serializer->readEnum("minFilter", "mgp::Texture::Filter", -1));
    _magFilter = static_cast<Texture::Filter>(serializer->readEnum("magFilter", "mgp::Texture::Filter", -1));

    _wrapS = static_cast<Texture::Wrap>(serializer->readEnum("wrapS", "mgp::Texture::Wrap", REPEAT));
    _wrapT = static_cast<Texture::Wrap>(serializer->readEnum("wrapT", "mgp::Texture::Wrap", REPEAT));
    _wrapR = static_cast<Texture::Wrap>(serializer->readEnum("wrapR", "mgp::Texture::Wrap", REPEAT));

    _format = static_cast<Texture::Format>(serializer->readEnum("format", "mgp::Texture::Format", RGBA8888));
    _type = static_cast<Texture::Type>(serializer->readEnum("type", "mgp::Texture::Type", TEXTURE_2D));
    _mipmapped = serializer->readBool("mipmap", false);
}

void Texture::setWrapMode(Wrap wrapS, Wrap wrapT, Wrap wrapR)
{
    _wrapS = wrapS;
    _wrapT = wrapT;
    _wrapR = wrapR;
}

void Texture::setFilterMode(Filter minificationFilter, Filter magnificationFilter)
{
    _minFilter = minificationFilter;
    _magFilter = magnificationFilter;
}

void Texture::bind()
{
    if (_dataDirty && _data) {
        _dataDirty = false;
        Renderer::cur()->updateTexture(this);
        if (!_keepMemory) {
            free((void*)_data);
            _data = NULL;
        }
    }

    if (!this->isMipmapped()) {
        if (_minFilter >= NEAREST_MIPMAP_NEAREST && _minFilter <= LINEAR_MIPMAP_LINEAR) {
            GP_ERROR("Unsupported minFilter (%d).", _minFilter);
        }
    }
    Renderer::cur()->bindTextureSampler(this);
}

void Texture::setSize(unsigned int width, unsigned int height) {
    _width = width;
    _height = height;
}

void* Texture::lock() {
    return (void*)_data;
    //TODO;
}
void Texture::unlock() {
    //TODO;
}

}

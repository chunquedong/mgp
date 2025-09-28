#include "base/Base.h"
#include "scene/Renderer.h"
#include "material/Image.h"
#include "Texture.h"
#include "base/FileSystem.h"
#include <algorithm>
#include "base/SerializerJson.h"
#include "scene/AssetManager.h"
#include "base/StringUtil.h"

mgp::CompressedTexture* g_compressedTexture = NULL;

namespace mgp
{

static std::vector<Texture*> __textureCache;
static std::mutex __textureCacheMutex;

Texture::Texture() : _handle(0), _format(Image::UNKNOWN), _type((Texture::Type)0), _width(0), _height(0), _arrayDepth(0), _mipmapped(false), _cached(false), _compressed(false),
    _wrapS(Texture::REPEAT), _wrapT(Texture::REPEAT), _wrapR(Texture::REPEAT), _minFilter(Texture::NEAREST), _magFilter(Texture::LINEAR),
    _keepMemory(false), _dataDirty(true), _anisotropy(false)
{
}

Texture::~Texture()
{
    _datas.clear();
    if (_handle) {
        Renderer::cur()->deleteTexture(this);
    }
    // Remove ourself from the texture cache.
    if (_cached)
    {
        std::lock_guard<std::mutex> guard(__textureCacheMutex);

        std::vector<Texture*>::iterator itr = std::find(__textureCache.begin(), __textureCache.end(), this);
        if (itr != __textureCache.end())
        {
            __textureCache.erase(itr);
        }
    }
}


bool Texture::load(const char* path) {
    UPtr<Image> image = Image::create(path);
    if (!image.get()) {
        return false;
    }
    _format = image->getFormat();
    _width = image->getWidth();
    _height = image->getHeight();

    _datas.push_back(SPtr<Image>(image.take()));
    
    //image->release();
    return true;
}

UPtr<Texture> Texture::create(const char* path, bool generateMipmaps)
{
    GP_ASSERT( path );

    if (true) {
        std::lock_guard<std::mutex> guard(__textureCacheMutex);
        // Search texture cache first.
        for (size_t i = 0, count = __textureCache.size(); i < count; ++i)
        {
            Texture* t = __textureCache[i];
            GP_ASSERT(t);
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
    }

    UPtr<Texture> texture;

    // Filter loading based on file extension.
    std::string ext = FileSystem::getExtension(FileSystem::resolvePath(path));
    if (ext.size() > 0)
    {
        if (ext == ".PNG" || ext == ".JPG" || ext == ".HDR" || ext == ".JPEG" || ext == ".TGA")
        {
            bool flipY = false;
            UPtr<Image> image = Image::create(path, flipY);
            if (image.get())
                texture = create(std::move(image), generateMipmaps);
            //SAFE_RELEASE(image);
        }
        else if (ext == ".PVR")
        {
            // PowerVR Compressed Texture RGBA.
            if (g_compressedTexture)
                texture = g_compressedTexture->createCompressedPVRTC(path);
        }
        else if (ext == ".DDS")
        {
            // DDS file format (DXT/S3TC) compressed textures
            if (g_compressedTexture)
                texture = g_compressedTexture->createCompressedDDS(path);
        }
        else if (ext == ".KTX")
        {
            // KTX file format compressed textures
            if (g_compressedTexture)
                texture = g_compressedTexture->createCompressedDdsKtx(path);
        }
    }

    if (texture.get())
    {
        texture->_path = path;
        texture->_cached = true;

        std::lock_guard<std::mutex> guard(__textureCacheMutex);
        // Add to texture cache.
        __textureCache.push_back(texture.get());

        return texture;
    }

    GP_ERROR("Failed to load texture from file '%s'.", path);
    return UPtr<Texture>(NULL);
}

UPtr<Texture> Texture::create(UPtr<Image> image, bool generateMipmaps)
{
    Texture* texture = new Texture();
    // Set initial minification filter based on whether or not mipmaping was enabled.
    Filter minFilter;
    if (image->getFormat() == Image::DEPTH)
    {
        minFilter = NEAREST;
    }
    else
    {
        minFilter = generateMipmaps ? NEAREST_MIPMAP_LINEAR : LINEAR;
    }

    texture->_format = image->getFormat();
    texture->_type = TEXTURE_2D;
    texture->_width = image->getWidth();
    texture->_height = image->getHeight();
    texture->_minFilter = minFilter;
    texture->_mipmapped = generateMipmaps;
    texture->_path = image->getFilePath().c_str();
    texture->_datas.push_back(SPtr<Image>(image.take()));

    return UPtr<Texture>(texture);
}

UPtr<Texture> Texture::create(Image::Format format, unsigned int width, unsigned int height, const unsigned char* data,
    bool generateMipmaps, Texture::Type type, bool copyData, unsigned int arrayDepth)
{
    Texture* texture = new Texture();
    // Set initial minification filter based on whether or not mipmaping was enabled.
    Filter minFilter;
    if (format == Image::DEPTH)
    {
    	minFilter = NEAREST;
    }
    else
    {
    	minFilter = generateMipmaps ? NEAREST_MIPMAP_LINEAR : LINEAR;
    }

    if (type == Texture::TEXTURE_CUBE || format == Image::DEPTH) {
        texture->_wrapR = CLAMP;
        texture->_wrapS = CLAMP;
        texture->_wrapT = CLAMP;
    }

    texture->_format = format;
    texture->_type = type;
    texture->_width = width;
    texture->_height = height;
    texture->_arrayDepth = arrayDepth;
    texture->_minFilter = minFilter;
    texture->_mipmapped = generateMipmaps;

    UPtr<Image> image = Image::create(width, height, format, (unsigned char*)data, copyData);
    /*if (copyData) {
        texture->_data = data;
        Renderer::cur()->updateTexture(texture);
        texture->_data = NULL;
    }
    else {
        texture->_data = data;
        texture->_dataDirty = true;
    }*/
    texture->_datas.push_back(SPtr<Image>(image.take()));

    return UPtr<Texture>(texture);
}

UPtr<Texture> Texture::loadCubeMap(const char* faces[]) {
    Texture* texture = new Texture();

    unsigned char* data = NULL;
    int width;
    int height;
    Image::Format format;
    for (int i=0; i<6; ++i) {
        const char *url = faces[i];
        UPtr<Image> image = Image::create(url, false);
        if (!image.get()) {
            GP_ERROR("image load fail: %s\n", url);
            texture->release();
            return UPtr<Texture>(nullptr);
        }

        format = image->_format;
        width = image->_width;
        height = image->_height;
        
        texture->_datas.push_back(SPtr<Image>(image.take()));
    }

    texture->_type = TEXTURE_CUBE;
    texture->_format = format;
    texture->_width = width;
    texture->_height = height;

    texture->_minFilter = NEAREST;
    texture->_wrapR = CLAMP;
    texture->_wrapS = CLAMP;
    texture->_wrapT = CLAMP;

    return UPtr<Texture>(texture);
}

//void Texture::generateMipmaps() {
//    _generateMipmaps = true;
//}

void Texture::setData(const unsigned char* data, bool copyMem)
{
    // Don't work with any compressed or cached textures
    GP_ASSERT(data);
    Image* image = _datas.at(0).get();
    if (image->getData() == data) {
        _dataDirty = true;
        return;
    }

    if (copyMem) {
        if (!_keepMemory) {
            image->setData((unsigned char*)data);
            Renderer::cur()->updateTexture(this);
            image->_data = NULL;
            _dataDirty = false;
        }
        else {
            int bpp = Image::getFormatBPP(_format);
            int size = bpp * _width * _height;
            unsigned char* t = (unsigned char*)malloc(size);
            memcpy(t, data, size);
            image->setData(t);
            _dataDirty = true;
        }
    }
    else {
        image->setData((unsigned char*)data);
        _dataDirty = true;
    }
}

void Texture::setKeepMemory(bool b) {
    _keepMemory = b;
}

Texture::Type Texture::getType() const
{
    return _type;
}

const char* Texture::getPath() const
{
    return _path.c_str();
}

Image::Format Texture::getFormat() const {
    return _format;
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

void Texture::setAnisotropy(bool anisotropy) {
    _anisotropy = anisotropy;
}

std::string Texture::enumToString(const std::string& enumName, int value)
{
    if (enumName.compare("mgp::Image::Format") == 0)
    {
        switch (value)
        {
            case static_cast<int>(Image::UNKNOWN) :
                return "UNKNOWN";
            case static_cast<int>(Image::RGB) :
                return "RGB";
            case static_cast<int>(Image::RGB888) :
                return "RGB888";
            case static_cast<int>(Image::RGB565) :
                return "RGB565";
            case static_cast<int>(Image::RGBA) :
                return "RGBA";
            case static_cast<int>(Image::RGBA8888) :
                return "RGBA8888";
            case static_cast<int>(Image::RGBA4444) :
                return "RGBA4444";
            case static_cast<int>(Image::RGBA5551) :
                return "RGBA5551";
            case static_cast<int>(Image::ALPHA) :
                return "ALPHA";
            case static_cast<int>(Image::DEPTH) :
                return "DEPTH";
            default:
                return "UNKNOWN";
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
    if (enumName.compare("mgp::Image::Format") == 0)
    {
        if (str.compare("UNKNOWN") == 0)
            return static_cast<int>(Image::UNKNOWN);
        else if (str.compare("RGB888") == 0)
            return static_cast<int>(Image::RGB888);
        else if (str.compare("RGB565") == 0)
            return static_cast<int>(Image::RGB565);
        else if (str.compare("RGB") == 0)
            return static_cast<int>(Image::RGB);
        else if (str.compare("RGBA") == 0)
            return static_cast<int>(Image::RGBA);
        else if (str.compare("RGB565") == 0)
            return static_cast<int>(Image::RGBA8888);
        else if (str.compare("RGBA8888") == 0)
            return static_cast<int>(Image::RGBA4444);
        else if (str.compare("RGBA4444") == 0)
            return static_cast<int>(Image::RGBA5551);
        else if (str.compare("RGBA5551") == 0)
            return static_cast<int>(Image::ALPHA);
        else if (str.compare("ALPHA") == 0)
            return static_cast<int>(Image::RGB565);
        else if (str.compare("DEPTH") == 0)
            return static_cast<int>(Image::DEPTH);
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

    serializer->writeList("images", _datas.size());
    for (auto& image : _datas) {
        std::string imageFile = image->getFilePath();
        if (imageFile.size() > 0) {
            std::string name = FileSystem::getBaseName(imageFile.c_str())+FileSystem::getExtension(imageFile.c_str());
            std::string dst = AssetManager::getInstance()->getPath() + "/image/" + name;
            if (!FileSystem::fileExists(dst.c_str())) {
                FileSystem::copyFile(imageFile.c_str(), dst.c_str());
            }
            imageFile = "image/" + name;
        }
        else {
            imageFile = "image/" + Resource::genId()+"."+image->_defaultFileFormat;
            std::string fullName = AssetManager::getInstance()->getPath() + "/" + imageFile;
            image->save(fullName.c_str(), image->_defaultFileFormat.c_str());
        }
        serializer->writeString(NULL, imageFile.c_str(), "");
    }
    serializer->finishColloction();

    //serializer->writeString("path", _path.c_str(), "");
    serializer->writeEnum("minFilter", "mgp::Texture::Filter", static_cast<int>(_minFilter), -1);
    serializer->writeEnum("magFilter", "mgp::Texture::Filter", static_cast<int>(_magFilter), -1);

    serializer->writeEnum("wrapS", "mgp::Texture::Wrap", static_cast<int>(_wrapS), REPEAT);
    serializer->writeEnum("wrapT", "mgp::Texture::Wrap", static_cast<int>(_wrapT), REPEAT);
    serializer->writeEnum("wrapR", "mgp::Texture::Wrap", static_cast<int>(_wrapR), REPEAT);

    serializer->writeEnum("format", "mgp::Image::Format", static_cast<int>(_format), Image::RGBA);
    serializer->writeEnum("type", "mgp::Texture::Type", static_cast<int>(_type), TEXTURE_2D);
    serializer->writeBool("mipmap", _mipmapped, false);
    serializer->writeInt("arrayDepth", _arrayDepth, 0);
    serializer->writeBool("keepMemory", _keepMemory, false);
}

/**
 * @see Serializable::onDeserialize
 */
void Texture::onDeserialize(Serializer* serializer) {
    int imagesSize = serializer->readList("images");
    for (int i = 0; i < imagesSize; ++i) {
        std::string imageFile;
        serializer->readString(NULL, imageFile, "");
        if (StringUtil::startsWith(imageFile, "image/")) {
            imageFile = AssetManager::getInstance()->getPath() + "/" + imageFile;
        }
        UPtr<Image> image = Image::create(imageFile.c_str());
        this->_datas.push_back(SPtr<Image>(image.take()));
    }
    serializer->finishColloction();

    _minFilter = static_cast<Texture::Filter>(serializer->readEnum("minFilter", "mgp::Texture::Filter", -1));
    _magFilter = static_cast<Texture::Filter>(serializer->readEnum("magFilter", "mgp::Texture::Filter", -1));

    _wrapS = static_cast<Texture::Wrap>(serializer->readEnum("wrapS", "mgp::Texture::Wrap", REPEAT));
    _wrapT = static_cast<Texture::Wrap>(serializer->readEnum("wrapT", "mgp::Texture::Wrap", REPEAT));
    _wrapR = static_cast<Texture::Wrap>(serializer->readEnum("wrapR", "mgp::Texture::Wrap", REPEAT));

    _format = static_cast<Image::Format>(serializer->readEnum("format", "mgp::Image::Format", Image::RGBA));
    _type = static_cast<Texture::Type>(serializer->readEnum("type", "mgp::Texture::Type", TEXTURE_2D));
    _mipmapped = serializer->readBool("mipmap", false);
    _arrayDepth = serializer->readInt("arrayDepth", 0);
    _keepMemory = serializer->readBool("keepMemory", false);

    //overwrite format
    if (_datas.size() > 0) {
        if (_format != _datas[0]->_format) {
            _format = _datas[0]->_format;
        }
        _width = _datas[0]->_width;
        _height = _datas[0]->_height;
    }
}

void Texture::write(Stream* file) {
    auto stream = SerializerJson::create(file);
    stream->writeObject(nullptr, this);
    stream->flush();
}

bool Texture::read(Stream* file) {
    auto stream = SerializerJson::create(file);
    UPtr<Texture> m = stream->readObject(nullptr).dynamicCastTo<Texture>();
    this->copyFrom(m.get());
    return true;
}

void Texture::copyFrom(Texture* that) {
    _path = that->_path;
    _minFilter = that->_minFilter;
    _magFilter = that->_magFilter;

    _wrapS = that->_wrapS;
    _wrapT = that->_wrapT;
    _wrapR = that->_wrapR;

    _format = that->_format;
    _type = that->_type;
    _mipmapped = that->_mipmapped;
    _arrayDepth = that->_arrayDepth;
    _cached = that->_cached;
    _compressed = that->_compressed;
    _keepMemory = that->_keepMemory;
    _dataDirty = that->_dataDirty;

    _handle = that->_handle;
    _datas = that->_datas;
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
    if (_dataDirty && _datas.size() > 0) {
        _dataDirty = false;
        Renderer::cur()->updateTexture(this);
        if (!_keepMemory) {
            for (auto& image : _datas) {
                image->setData(NULL);
            }
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
    return (void*)_datas.at(0)->getData();
    //TODO;
}
void Texture::unlock() {
    //TODO;
}

}

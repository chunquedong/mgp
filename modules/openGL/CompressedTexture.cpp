
#include "CompressedTexture.h"
#include "ogl.h"
#include "base/FileSystem.h"

using namespace mgp;

// PVRTC (GL_IMG_texture_compression_pvrtc) : Imagination based gpus
#ifndef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#endif
#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#endif

// S3TC/DXT (GL_EXT_texture_compression_s3tc) : Most desktop/console gpus
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

// ATC (GL_AMD_compressed_ATC_texture) : Qualcomm/Adreno based gpus
#ifndef ATC_RGB_AMD
#define ATC_RGB_AMD 0x8C92
#endif
#ifndef ATC_RGBA_EXPLICIT_ALPHA_AMD
#define ATC_RGBA_EXPLICIT_ALPHA_AMD 0x8C93
#endif
#ifndef ATC_RGBA_INTERPOLATED_ALPHA_AMD
#define ATC_RGBA_INTERPOLATED_ALPHA_AMD 0x87EE
#endif

// ETC1 (OES_compressed_ETC1_RGB8_texture) : All OpenGL ES chipsets
#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES 0x8d64
#endif
#ifndef GL_ETC2_RGB8_OES
#define GL_ETC2_RGB8_OES 0x9274
#endif
#ifndef GL_ETC2_RGBA8_OES
#define GL_ETC2_RGBA8_OES 0x9278
#endif

//#define GP_DDSKTX
//#define GP_PVR
//#define GP_DDS


#ifdef GP_DDSKTX

#define DDSKTX_IMPLEMENT
#include "3rd/dds-ktx.h"

void* load_file(const char*file, int *outSize) {
    FILE* f = fopen(file, "rb");
    if (!f) {
        printf("Error: could not open file: %s\n", file);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    int size = (int)ftell(f);
    if (size == 0) {
        printf("Error: file '%s' is empty\n", file);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);

    void* data = malloc(size);
    if (!data) {
        printf("out of memory: requested size: %d\n", (int)size);
        return NULL;
    }

    if (fread(data, 1, size, f) != size) {
        printf("could not read file data : %s\n", file);
        return NULL;
    }

    fclose(f);

    *outSize = size;
    return data;
}

UPtr<Texture> CompressedTexture::createCompressedDdsKtx(const char* path) {
    int size;
    void* dds_data = load_file(path, &size);
    if (!dds_data) return UPtr<Texture>(NULL);
    ddsktx_texture_info tc = { 0 };
    GLuint RGBtex = 0;
    if (ddsktx_parse(&tc, dds_data, size, NULL)) {
        assert(tc.depth == 1);
        assert(tc.num_layers == 1);

        //Create GPU texture from tc data
        glGenTextures(1, &tex);
        glActiveTexture(GL_TEXTURE0);
        
        int glTexImageTarget = GL_TEXTURE_2D;
        if (tc.flags & DDSKTX_TEXTURE_FLAG_CUBEMAP) {
            glTexImageTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        }

        glBindTexture(glTexImageTarget, tex);

        int format = GL_RGBA;
        if (tc.bpp == 24) {
            format = GL_RGB;
        }
        if (ddsktx_format_compressed(tc.format)) {
            switch (tc.format) {
            case DDSKTX_FORMAT_BC1:     format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
            case DDSKTX_FORMAT_BC3:     format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
            case DDSKTX_FORMAT_BC5:     format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
            case DDSKTX_FORMAT_ETC1:   format = GL_ETC1_RGB8_OES;  break;
            case DDSKTX_FORMAT_ETC2:   format = GL_ETC2_RGB8_OES;  break;
            case DDSKTX_FORMAT_ETC2A:   format = GL_ETC2_RGBA8_OES;  break;
            default:    assert(0); exit(-1);
            }
        }

        for (int mip = 0; mip < tc.num_mips; mip++) {
            ddsktx_sub_data sub_data;
            ddsktx_get_sub(&tc, &sub_data, dds_data, size, 0, 0, mip);
            // Fill/Set texture sub resource data (mips in this case)
            if (ddsktx_format_compressed(tc.format)) {
                GL_ASSERT(glCompressedTexImage2D(glTexImageTarget, mip, format, sub_data.width, sub_data.height, 0, sub_data.size_bytes, sub_data.buff));
            }
            else {
                GL_ASSERT(glTexImage2D(glTexImageTarget, mip, format, sub_data.width, sub_data.height, 0, format, GL_UNSIGNED_BYTE, sub_data.buff));
            }
        }

        // Now we can delete file data
        free(dds_data);

        Texture::Filter minFilter = tc.num_mips > 1 ? Texture::NEAREST_MIPMAP_LINEAR : Texture::LINEAR;
        GL_ASSERT(glTexParameteri(glTexImageTarget, GL_TEXTURE_MIN_FILTER, minFilter));

        UPtr<Texture> texture = UPtr<Texture>(new Texture());
        texture->_handle = tex;
        texture->_type = (Texture::Type)glTexImageTarget;
        texture->_width = tc.width;
        texture->_height = tc.height;
        texture->_mipmapped = tc.num_mips > 1;
        texture->_compressed = ddsktx_format_compressed(tc.format);
        texture->_minFilter = minFilter;
    }
    return UPtr<Texture>(NULL);
}

#else
UPtr<Texture> CompressedTexture::createCompressedDdsKtx(const char* path) {
    return UPtr<Texture>();
}
#endif

#ifdef GP_PVR

// Computes the size of a PVRTC data chunk for a mipmap level of the given size.
static unsigned int computePVRTCDataSize(int width, int height, int bpp)
{
    int blockSize;
    int widthBlocks;
    int heightBlocks;

    if (bpp == 4)
    {
        blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
        widthBlocks = std::max(width >> 2, 2);
        heightBlocks = std::max(height >> 2, 2);
    }
    else
    {
        blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
        widthBlocks = std::max(width >> 3, 2);
        heightBlocks = std::max(height >> 2, 2);
    }

    return widthBlocks * heightBlocks * ((blockSize * bpp) >> 3);
}

GLubyte* readCompressedPVRTC(const char* path, Stream* stream, GLsizei* width, GLsizei* height, GLenum* format, unsigned int* mipMapCount, unsigned int* faceCount, GLenum faces[6]);

GLubyte* readCompressedPVRTCLegacy(const char* path, Stream* stream, GLsizei* width, GLsizei* height, GLenum* format, unsigned int* mipMapCount, unsigned int* faceCount, GLenum faces[6]);

UPtr<Texture> CompressedTexture::createCompressedPVRTC(const char* path)
{
    UPtr<Texture> texture;

    UPtr<Stream> stream(FileSystem::open(path));
    if (stream.get() == NULL || !stream->canRead())
    {
        GP_ERROR("Failed to load file '%s'.", path);
        return texture;
    }

    // Read first 4 bytes to determine PVRTC format.
    size_t read;
    unsigned int version;
    read = stream->read(&version, sizeof(unsigned int), 1);
    if (read != 1)
    {
        GP_ERROR("Failed to read PVR version.");
        return texture;
    }

    // Rewind to start of header.
    if (stream->seek(0, SEEK_SET) == false)
    {
        GP_ERROR("Failed to seek backwards to beginning of file after reading PVR version.");
        return texture;
    }

    // Read texture data.
    GLsizei width, height;
    GLenum format;
    GLubyte* data = NULL;
    unsigned int mipMapCount;
    unsigned int faceCount;
    GLenum faces[6] = { GL_TEXTURE_2D };

    if (version == 0x03525650)
    {
        // Modern PVR file format.
        data = readCompressedPVRTC(path, stream.get(), &width, &height, &format, &mipMapCount, &faceCount, faces);
    }
    else
    {
        // Legacy PVR file format.
        data = readCompressedPVRTCLegacy(path, stream.get(), &width, &height, &format, &mipMapCount, &faceCount, faces);
    }
    if (data == NULL)
    {
        GP_ERROR("Failed to read texture data from PVR file '%s'.", path);
        return texture;
    }
    stream->close();

    int bpp = (format == GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG || format == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG) ? 2 : 4;

    // Generate our texture.
    GLenum target = faceCount > 1 ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    GLuint textureId;
    GL_ASSERT(glGenTextures(1, &textureId));
    GL_ASSERT(glBindTexture(target, textureId));

    Texture::Filter minFilter = mipMapCount > 1 ? Texture::NEAREST_MIPMAP_LINEAR : Texture::LINEAR;
    GL_ASSERT(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter));

    texture = UPtr<Texture>(new Texture());
    texture->_handle = textureId;
    texture->_type = faceCount > 1 ? Texture::TEXTURE_CUBE : Texture::TEXTURE_2D;
    texture->_width = width;
    texture->_height = height;
    texture->_mipmapped = mipMapCount > 1;
    texture->_compressed = true;
    texture->_minFilter = minFilter;

    // Load the data for each level.
    GLubyte* ptr = data;
    for (unsigned int level = 0; level < mipMapCount; ++level)
    {
        unsigned int dataSize = computePVRTCDataSize(width, height, bpp);

        for (unsigned int face = 0; face < faceCount; ++face)
        {
            // Upload data to GL.
            GL_ASSERT(glCompressedTexImage2D(faces[face], level, format, width, height, 0, dataSize, &ptr[face * dataSize]));
        }

        width = std::max(width >> 1, 1);
        height = std::max(height >> 1, 1);
        ptr += dataSize * faceCount;
    }

    // Free data.
    SAFE_DELETE_ARRAY(data);

    // Restore the texture id
    //GL_ASSERT(glBindTexture((GLenum)__currentTextureType, __currentTextureId));

    return texture;
}

GLubyte* readCompressedPVRTC(const char* path, Stream* stream, GLsizei* width, GLsizei* height, GLenum* format, unsigned int* mipMapCount, unsigned int* faceCount, GLenum* faces)
{
    GP_ASSERT(stream);
    GP_ASSERT(path);
    GP_ASSERT(width);
    GP_ASSERT(height);
    GP_ASSERT(format);
    GP_ASSERT(mipMapCount);
    GP_ASSERT(faceCount);
    GP_ASSERT(faces);

    struct pvrtc_file_header
    {
        unsigned int version;
        unsigned int flags;
        unsigned int pixelFormat[2];
        unsigned int colorSpace;
        unsigned int channelType;
        unsigned int height;
        unsigned int width;
        unsigned int depth;
        unsigned int surfaceCount;
        unsigned int faceCount;
        unsigned int mipMapCount;
        unsigned int metaDataSize;
    };

    struct pvrtc_metadata
    {
        char fourCC[4];
        unsigned int key;
        unsigned int dataSize;
    };

    size_t read;

    // Read header data.
    pvrtc_file_header header;
    read = stream->read(&header, sizeof(pvrtc_file_header), 1);
    if (read != 1)
    {
        GP_ERROR("Failed to read PVR header data for file '%s'.", path);
        return NULL;
    }

    if (header.pixelFormat[1] != 0)
    {
        // Unsupported pixel format.
        GP_ERROR("Unsupported pixel format in PVR file '%s'. (MSB == %d != 0)", path, header.pixelFormat[1]);
        return NULL;
    }

    int bpp;
    switch (header.pixelFormat[0])
    {
    case 0:
        *format = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        bpp = 2;
        break;
    case 1:
        *format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        bpp = 2;
        break;
    case 2:
        *format = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        bpp = 4;
        break;
    case 3:
        *format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        bpp = 4;
        break;
    default:
        // Unsupported format.
        GP_ERROR("Unsupported pixel format value (%d) in PVR file '%s'.", header.pixelFormat[0], path);
        return NULL;
    }

    *width = (GLsizei)header.width;
    *height = (GLsizei)header.height;
    *mipMapCount = header.mipMapCount;
    *faceCount = std::min(header.faceCount, 6u);

    if ((*faceCount) > 1)
    {
        // Look for cubemap metadata and setup faces
        unsigned int remainingMetadata = header.metaDataSize;
        pvrtc_metadata mdHeader;
        bool foundTextureCubeMeta = false;
        while (remainingMetadata > 0)
        {
            read = stream->read(&mdHeader, sizeof(pvrtc_metadata), 1);
            if (read != 1)
            {
                GP_ERROR("Failed to read PVR metadata header data for file '%s'.", path);
                return NULL;
            }
            remainingMetadata -= sizeof(pvrtc_metadata) + mdHeader.dataSize;

            // Check that it's a known metadata type (specifically, cubemap order), otherwise skip to next metadata
            if ((mdHeader.fourCC[0] != 'P') ||
                (mdHeader.fourCC[1] != 'V') ||
                (mdHeader.fourCC[2] != 'R') ||
                (mdHeader.fourCC[3] != 3) ||
                (mdHeader.key != 2) || // Everything except cubemap order (cubemap order key is 2)
                (mdHeader.dataSize != 6)) // Cubemap order datasize should be 6
            {
                if (stream->seek(mdHeader.dataSize, SEEK_CUR) == false)
                {
                    GP_ERROR("Failed to seek to next meta data header in PVR file '%s'.", path);
                    return NULL;
                }
                continue;
            }

            // Get cubemap order
            foundTextureCubeMeta = true;
            char faceOrder[6];
            read = stream->read(faceOrder, 1, sizeof(faceOrder));
            if (read != sizeof(faceOrder))
            {
                GP_ERROR("Failed to read cubemap face order meta data for file '%s'.", path);
                return NULL;
            }
            for (unsigned int face = 0; face < (*faceCount); ++face)
            {
                faces[face] = GL_TEXTURE_CUBE_MAP_POSITIVE_X + (faceOrder[face] <= 'Z' ?
                    ((faceOrder[face] - 'X') * 2) :
                    (((faceOrder[face] - 'x') * 2) + 1));
                if (faces[face] < GL_TEXTURE_CUBE_MAP_POSITIVE_X)
                {
                    // Just overwrite this face
                    faces[face] = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                }
            }
        }
        if (!foundTextureCubeMeta)
        {
            // Didn't find cubemap metadata. Just assume it's "in order"
            for (unsigned int face = 0; face < (*faceCount); ++face)
            {
                faces[face] = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
            }
        }
    }
    else
    {
        // Skip meta-data.
        if (stream->seek(header.metaDataSize, SEEK_CUR) == false)
        {
            GP_ERROR("Failed to seek past header meta data in PVR file '%s'.", path);
            return NULL;
        }
    }

    // Compute total size of data to be read.
    int w = *width;
    int h = *height;
    size_t dataSize = 0;
    for (unsigned int level = 0; level < header.mipMapCount; ++level)
    {
        dataSize += computePVRTCDataSize(w, h, bpp) * (*faceCount);
        w = std::max(w >> 1, 1);
        h = std::max(h >> 1, 1);
    }

    // Read data.
    GLubyte* data = new GLubyte[dataSize];
    read = stream->read(data, 1, dataSize);
    if (read != dataSize)
    {
        SAFE_DELETE_ARRAY(data);
        GP_ERROR("Failed to read texture data from PVR file '%s'.", path);
        return NULL;
    }

    return data;
}

GLubyte* readCompressedPVRTCLegacy(const char* path, Stream* stream, GLsizei* width, GLsizei* height, GLenum* format, unsigned int* mipMapCount, unsigned int* faceCount, GLenum* faces)
{
    char PVRTCIdentifier[] = "PVR!";

    struct pvrtc_file_header_legacy
    {
        unsigned int size;                  // size of the structure
        unsigned int height;                // height of surface to be created
        unsigned int width;                 // width of input surface
        unsigned int mipmapCount;           // number of mip-map levels requested
        unsigned int formatflags;           // pixel format flags
        unsigned int dataSize;              // total size in bytes
        unsigned int bpp;                   // number of bits per pixel
        unsigned int redBitMask;            // mask for red bit
        unsigned int greenBitMask;          // mask for green bits
        unsigned int blueBitMask;           // mask for blue bits
        unsigned int alphaBitMask;          // mask for alpha channel
        unsigned int pvrtcTag;              // magic number identifying pvrtc file
        unsigned int surfaceCount;          // number of surfaces present in the pvrtc
    };

    // Read the file header.
    unsigned int size = sizeof(pvrtc_file_header_legacy);
    pvrtc_file_header_legacy header;
    unsigned int read = (int)stream->read(&header, 1, size);
    if (read != size)
    {
        GP_ERROR("Failed to read file header for pvrtc file '%s'.", path);
        return NULL;
    }

    // Proper file header identifier.
    if (PVRTCIdentifier[0] != (char)((header.pvrtcTag >> 0) & 0xff) ||
        PVRTCIdentifier[1] != (char)((header.pvrtcTag >> 8) & 0xff) ||
        PVRTCIdentifier[2] != (char)((header.pvrtcTag >> 16) & 0xff) ||
        PVRTCIdentifier[3] != (char)((header.pvrtcTag >> 24) & 0xff))
    {
        GP_ERROR("Failed to load pvrtc file '%s': invalid header.", path);
        return NULL;
    }

    // Format flags for GLenum format.
    if (header.bpp == 4)
    {
        *format = header.alphaBitMask ? GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
    }
    else if (header.bpp == 2)
    {
        *format = header.alphaBitMask ? GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
    }
    else
    {
        GP_ERROR("Failed to load pvrtc file '%s': invalid pvrtc compressed texture format flags.", path);
        return NULL;
    }

    *width = (GLsizei)header.width;
    *height = (GLsizei)header.height;
    *mipMapCount = header.mipmapCount + 1; // +1 because mipmapCount does not include the base level
    *faceCount = 1;

    // Flags (needed legacy documentation on format, pre-PVR Format 3.0)
    if ((header.formatflags & 0x1000) != 0)
    {
        // Texture cube
        *faceCount = std::min(header.surfaceCount, 6u);
        for (unsigned int face = 0; face < (*faceCount); ++face)
        {
            faces[face] = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
        }
    }
    else if ((header.formatflags & 0x4000) != 0)
    {
        // Volume texture
        GP_ERROR("Failed to load pvrtc file '%s': volume texture is not supported.", path);
        return NULL;
    }

    unsigned int totalSize = header.dataSize; // Docs say dataSize is the size of the whole surface, or one face of a texture cube. But this does not appear to be the case with the latest PVRTexTool
    GLubyte* data = new GLubyte[totalSize];
    read = (int)stream->read(data, 1, totalSize);
    if (read != totalSize)
    {
        SAFE_DELETE_ARRAY(data);
        GP_ERROR("Failed to load texture data for pvrtc file '%s'.", path);
        return NULL;
    }

    return data;
}
#else
UPtr<Texture> CompressedTexture::createCompressedPVRTC(const char* path) {
    return UPtr<Texture>();
}
#endif

#ifdef GP_DDS

int getMaskByteIndex(unsigned int mask)
{
    switch (mask)
    {
    case 0xff000000:
        return 3;
    case 0x00ff0000:
        return 2;
    case 0x0000ff00:
        return 1;
    case 0x000000ff:
        return 0;
    default:
        return -1; // no or invalid mask
    }
}


UPtr<Texture> CompressedTexture::createCompressedDDS(const char* path)
{
    GP_ASSERT(path);

    // DDS file structures.
    struct dds_pixel_format
    {
        unsigned int dwSize;
        unsigned int dwFlags;
        unsigned int dwFourCC;
        unsigned int dwRGBBitCount;
        unsigned int dwRBitMask;
        unsigned int dwGBitMask;
        unsigned int dwBBitMask;
        unsigned int dwABitMask;
    };

    struct dds_header
    {
        unsigned int     dwSize;
        unsigned int     dwFlags;
        unsigned int     dwHeight;
        unsigned int     dwWidth;
        unsigned int     dwPitchOrLinearSize;
        unsigned int     dwDepth;
        unsigned int     dwMipMapCount;
        unsigned int     dwReserved1[11];
        dds_pixel_format ddspf;
        unsigned int     dwCaps;
        unsigned int     dwCaps2;
        unsigned int     dwCaps3;
        unsigned int     dwCaps4;
        unsigned int     dwReserved2;
    };

    struct dds_mip_level
    {
        GLubyte* data;
        GLsizei width;
        GLsizei height;
        GLsizei size;
    };

    UPtr<Texture> texture;

    // Read DDS file.
    UPtr<Stream> stream(FileSystem::open(path));
    if (stream.get() == NULL || !stream->canRead())
    {
        GP_ERROR("Failed to open file '%s'.", path);
        return texture;
    }

    // Validate DDS magic number.
    char code[4];
    if (stream->read(code, 1, 4) != 4 || strncmp(code, "DDS ", 4) != 0)
    {
        GP_ERROR("Failed to read DDS file '%s': invalid DDS magic number.", path);
        return texture;
    }

    // Read DDS header.
    dds_header header;
    if (stream->read(&header, sizeof(dds_header), 1) != 1)
    {
        GP_ERROR("Failed to read header for DDS file '%s'.", path);
        return texture;
    }

    if ((header.dwFlags & 0x20000/*DDSD_MIPMAPCOUNT*/) == 0)
    {
        // Mipmap count not specified (non-mipmapped texture).
        header.dwMipMapCount = 1;
    }

    // Check type of images. Default is a regular texture
    unsigned int facecount = 1;
    GLenum faces[6] = { GL_TEXTURE_2D };
    GLenum target = GL_TEXTURE_2D;
    if ((header.dwCaps2 & 0x200/*DDSCAPS2_CUBEMAP*/) != 0)
    {
        facecount = 0;
        for (unsigned int off = 0, flag = 0x400/*DDSCAPS2_CUBEMAP_POSITIVEX*/; off < 6; ++off, flag <<= 1)
        {
            if ((header.dwCaps2 & flag) != 0)
            {
                faces[facecount++] = GL_TEXTURE_CUBE_MAP_POSITIVE_X + off;
            }
        }
        target = GL_TEXTURE_CUBE_MAP;
    }
    else if ((header.dwCaps2 & 0x200000/*DDSCAPS2_VOLUME*/) != 0)
    {
        // Volume textures unsupported.
        GP_ERROR("Failed to create texture from DDS file '%s': volume textures are unsupported.", path);
        return texture;
    }

    // Allocate mip level structures.
    dds_mip_level* mipLevels = new dds_mip_level[header.dwMipMapCount * facecount];
    memset(mipLevels, 0, sizeof(dds_mip_level) * header.dwMipMapCount * facecount);

    GLenum format = 0;
    GLenum internalFormat = 0;
    bool compressed = false;
    GLsizei width = header.dwWidth;
    GLsizei height = header.dwHeight;
    Image::Format textureFormat = Texture::UNKNOWN;

    if (header.ddspf.dwFlags & 0x4/*DDPF_FOURCC*/)
    {
        compressed = true;
        int bytesPerBlock;

        // Compressed.
        switch (header.ddspf.dwFourCC)
        {
        case ('D' | ('X' << 8) | ('T' << 16) | ('1' << 24)):
            format = internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            bytesPerBlock = 8;
            break;
        case ('D' | ('X' << 8) | ('T' << 16) | ('3' << 24)):
            format = internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            bytesPerBlock = 16;
            break;
        case ('D' | ('X' << 8) | ('T' << 16) | ('5' << 24)):
            format = internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            bytesPerBlock = 16;
            break;
        case ('A' | ('T' << 8) | ('C' << 16) | (' ' << 24)):
            format = internalFormat = ATC_RGB_AMD;
            bytesPerBlock = 8;
            break;
        case ('A' | ('T' << 8) | ('C' << 16) | ('A' << 24)):
            format = internalFormat = ATC_RGBA_EXPLICIT_ALPHA_AMD;
            bytesPerBlock = 16;
            break;
        case ('A' | ('T' << 8) | ('C' << 16) | ('I' << 24)):
            format = internalFormat = ATC_RGBA_INTERPOLATED_ALPHA_AMD;
            bytesPerBlock = 16;
            break;
        case ('E' | ('T' << 8) | ('C' << 16) | ('1' << 24)):
            format = internalFormat = GL_ETC1_RGB8_OES;
            bytesPerBlock = 8;
            break;
        default:
            GP_ERROR("Unsupported compressed texture format (%d) for DDS file '%s'.", header.ddspf.dwFourCC, path);
            SAFE_DELETE_ARRAY(mipLevels);
            return texture;
        }

        for (unsigned int face = 0; face < facecount; ++face)
        {
            for (unsigned int i = 0; i < header.dwMipMapCount; ++i)
            {
                dds_mip_level& level = mipLevels[i + face * header.dwMipMapCount];

                level.width = width;
                level.height = height;
                level.size = std::max(1, (width + 3) >> 2) * std::max(1, (height + 3) >> 2) * bytesPerBlock;
                level.data = new GLubyte[level.size];

                if (stream->read(level.data, 1, level.size) != (unsigned int)level.size)
                {
                    GP_ERROR("Failed to load dds compressed texture bytes for texture: %s", path);

                    // Cleanup mip data.
                    for (unsigned int face = 0; face < facecount; ++face)
                        for (unsigned int i = 0; i < header.dwMipMapCount; ++i)
                            SAFE_DELETE_ARRAY(mipLevels[i + face * header.dwMipMapCount].data);
                    SAFE_DELETE_ARRAY(mipLevels);
                    return texture;
                }

                width = std::max(1, width >> 1);
                height = std::max(1, height >> 1);
            }
            width = header.dwWidth;
            height = header.dwHeight;
        }
    }
    else if (header.ddspf.dwFlags & 0x40/*DDPF_RGB*/)
    {
        // RGB/RGBA (uncompressed)
        bool colorConvert = false;
        unsigned int rmask = header.ddspf.dwRBitMask;
        unsigned int gmask = header.ddspf.dwGBitMask;
        unsigned int bmask = header.ddspf.dwBBitMask;
        unsigned int amask = header.ddspf.dwABitMask;
        int ridx = getMaskByteIndex(rmask);
        int gidx = getMaskByteIndex(gmask);
        int bidx = getMaskByteIndex(bmask);
        int aidx = getMaskByteIndex(amask);

        if (header.ddspf.dwRGBBitCount == 24)
        {
            format = internalFormat = GL_RGB;
            textureFormat = Image::RGB;
            colorConvert = (ridx != 0) || (gidx != 1) || (bidx != 2);
        }
        else if (header.ddspf.dwRGBBitCount == 32)
        {
            format = internalFormat = GL_RGBA;
            textureFormat = Image::RGBA;
            if (ridx == 0 && gidx == 1 && bidx == 2)
            {
                aidx = 3; // XBGR or ABGR
                colorConvert = false;
            }
            else if (ridx == 2 && gidx == 1 && bidx == 0)
            {
                aidx = 3; // XRGB or ARGB
                colorConvert = true;
            }
            else
            {
                format = 0; // invalid format
            }
        }

        if (format == 0)
        {
            GP_ERROR("Failed to create texture from uncompressed DDS file '%s': Unsupported color format (must be one of R8G8B8, A8R8G8B8, A8B8G8R8, X8R8G8B8, X8B8G8R8.", path);
            SAFE_DELETE_ARRAY(mipLevels);
            return texture;
        }

        // Read data.
        for (unsigned int face = 0; face < facecount; ++face)
        {
            for (unsigned int i = 0; i < header.dwMipMapCount; ++i)
            {
                dds_mip_level& level = mipLevels[i + face * header.dwMipMapCount];

                level.width = width;
                level.height = height;
                level.size = width * height * (header.ddspf.dwRGBBitCount >> 3);
                level.data = new GLubyte[level.size];

                if (stream->read(level.data, 1, level.size) != (unsigned int)level.size)
                {
                    GP_ERROR("Failed to load bytes for RGB dds texture: %s", path);

                    // Cleanup mip data.
                    for (unsigned int face = 0; face < facecount; ++face)
                        for (unsigned int i = 0; i < header.dwMipMapCount; ++i)
                            SAFE_DELETE_ARRAY(mipLevels[i + face * header.dwMipMapCount].data);
                    SAFE_DELETE_ARRAY(mipLevels);
                    return texture;
                }

                width = std::max(1, width >> 1);
                height = std::max(1, height >> 1);
            }
            width = header.dwWidth;
            height = header.dwHeight;
        }

        // Perform color conversion.
        if (colorConvert)
        {
            // Note: While it's possible to use BGRA_EXT texture formats here and avoid CPU color conversion below,
            // there seems to be different flavors of the BGRA extension, with some vendors requiring an internal
            // format of RGBA and others requiring an internal format of BGRA.
            // We could be smarter here later and skip color conversion in favor of GL_BGRA_EXT (for format
            // and/or internal format) based on which GL extensions are available.
            // Tip: Using A8B8G8R8 and X8B8G8R8 DDS format maps directly to GL RGBA and requires on no color conversion.
            GLubyte* pixel, r, g, b, a;
            if (format == GL_RGB)
            {
                for (unsigned int face = 0; face < facecount; ++face)
                {
                    for (unsigned int i = 0; i < header.dwMipMapCount; ++i)
                    {
                        dds_mip_level& level = mipLevels[i + face * header.dwMipMapCount];
                        for (int j = 0; j < level.size; j += 3)
                        {
                            pixel = &level.data[j];
                            r = pixel[ridx]; g = pixel[gidx]; b = pixel[bidx];
                            pixel[0] = r; pixel[1] = g; pixel[2] = b;
                        }
                    }
                }
            }
            else if (format == GL_RGBA)
            {
                for (unsigned int face = 0; face < facecount; ++face)
                {
                    for (unsigned int i = 0; i < header.dwMipMapCount; ++i)
                    {
                        dds_mip_level& level = mipLevels[i + face * header.dwMipMapCount];
                        for (int j = 0; j < level.size; j += 4)
                        {
                            pixel = &level.data[j];
                            r = pixel[ridx]; g = pixel[gidx]; b = pixel[bidx]; a = pixel[aidx];
                            pixel[0] = r; pixel[1] = g; pixel[2] = b; pixel[3] = a;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Unsupported.
        GP_ERROR("Failed to create texture from DDS file '%s': unsupported flags (%d).", path, header.ddspf.dwFlags);
        SAFE_DELETE_ARRAY(mipLevels);
        return texture;
    }

    // Close file.
    stream->close();

    // Generate GL texture.
    GLuint textureId;
    GL_ASSERT(glGenTextures(1, &textureId));
    GL_ASSERT(glBindTexture(target, textureId));

    Texture::Filter minFilter = header.dwMipMapCount > 1 ? Texture::NEAREST_MIPMAP_LINEAR : Texture::LINEAR;
    GL_ASSERT(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter));

    // Create gameplay texture.
    texture = UPtr<Texture>(new Texture());
    texture->_handle = textureId;
    texture->_type = (Texture::Type)target;
    texture->_width = header.dwWidth;
    texture->_height = header.dwHeight;
    texture->_compressed = compressed;
    texture->_mipmapped = header.dwMipMapCount > 1;
    texture->_minFilter = minFilter;

    // Load texture data.
    for (unsigned int face = 0; face < facecount; ++face)
    {
        GLenum texImageTarget = faces[face];
        for (unsigned int i = 0; i < header.dwMipMapCount; ++i)
        {
            dds_mip_level& level = mipLevels[i + face * header.dwMipMapCount];
            if (compressed)
            {
                GL_ASSERT(glCompressedTexImage2D(texImageTarget, i, format, level.width, level.height, 0, level.size, level.data));
            }
            else
            {
                GL_ASSERT(glTexImage2D(texImageTarget, i, internalFormat, level.width, level.height, 0, format, GL_UNSIGNED_BYTE, level.data));
            }

            // Clean up the texture data.
            SAFE_DELETE_ARRAY(level.data);
        }
    }

    // Clean up mip levels structure.
    SAFE_DELETE_ARRAY(mipLevels);

    // Restore the texture id
    //GL_ASSERT(glBindTexture((GLenum)__currentTextureType, __currentTextureId));

    return texture;
}
#else
UPtr<Texture> CompressedTexture::createCompressedDDS(const char* path) {
    return UPtr<Texture>();
}
#endif
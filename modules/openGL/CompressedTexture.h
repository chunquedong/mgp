#ifndef COMPRESSEDTEXTURE_H_
#define COMPRESSEDTEXTURE_H_

#include "material/Texture.h"

namespace mgp
{
    class GLCompressedTexture : public CompressedTexture {
    public:
        UPtr<Texture> createCompressedDdsKtx(const char* path);

        UPtr<Texture> createCompressedPVRTC(const char* path);

        UPtr<Texture> createCompressedDDS(const char* path);
    };
}

#endif
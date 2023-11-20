#ifndef COMPRESSEDTEXTURE_H_
#define COMPRESSEDTEXTURE_H_

#include "material/Texture.h"

namespace mgp
{
    class CompressedTexture {
    public:
        static UPtr<Texture> createCompressedDdsKtx(const char* path);

        static UPtr<Texture> createCompressedPVRTC(const char* path);

        static UPtr<Texture> createCompressedDDS(const char* path);

    };
}

#endif
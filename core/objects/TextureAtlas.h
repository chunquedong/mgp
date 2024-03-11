/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef TEXTUREATLAS_H_
#define TEXTUREATLAS_H_

#include "material/Texture.h"
#include "material/ShaderProgram.h"
#include "scene/Mesh.h"
#include "math/Rectangle.h"
#include "math/Matrix.h"
#include "material/Material.h"
#include "scene/MeshBatch.h"

namespace mgp
{
class SpriteBatch;

class TextureAtlas : public Refable {
    Texture* texture;

    int lastX;
    int curY;
    int maxY;
    bool isFull;

    std::vector<Rectangle> rects;
    unsigned char* data;

public:
    TextureAtlas(Image::Format format, int w, int h);
    ~TextureAtlas();

    bool addImageData(int imgW, int imgH, const unsigned char* imgData, Rectangle& rect);
    bool addImage(Image* image, Rectangle& rect);
    bool addImageUri(const std::string& file, Rectangle& rect);

    Texture* getTexture() { return texture; }
private:
    template<int pixelSize>
    void copyImageData(unsigned char* dst, int dstW, int dstH, int x, int y, const unsigned char* src, int imgW, int imgH) {
        for (int i = 0; i < imgH; ++i) {
            int dstRow = (y + i) * dstW * pixelSize;
            int srcRow = i * imgW * pixelSize;
            for (int j = 0; j < imgW; ++j) {
                int dstX = (x + j) * pixelSize;
                int srcX = j * pixelSize;

                for (int k = 0; k < pixelSize; ++k) {
                    dst[dstRow + dstX + k] = src[srcRow + srcX + k];
                }
            }
        }
    }
};

}

#endif
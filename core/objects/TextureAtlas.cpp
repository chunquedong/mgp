/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "TextureAtlas.h"
#include "material/Image.h"
#include "SpriteBatch.h"

#include "3rd/stb_image_write.h"

using namespace mgp;

TextureAtlas::TextureAtlas(Image::Format format, int w, int h) : texture(NULL), lastX(0), curY(0), maxY(0), isFull(false), data(NULL) {
    
    texture = Texture::create(format, w, h, data).take();

    int pixelSize = Image::getFormatBPP(texture->getFormat());
    unsigned char* data = (unsigned char*)calloc(1, w * h * pixelSize);
    this->data = data;
}

TextureAtlas::~TextureAtlas() {
    SAFE_RELEASE(texture);
    data = NULL;
}


bool TextureAtlas::addImage(Image* image, Rectangle& rect) {
    if (texture->getFormat() == Image::RGBA) {
        GP_ASSERT(image->getFormat() == Image::RGBA);
    }
    return addImageData(image->getWidth(), image->getHeight(), image->getData(), rect);
}

bool TextureAtlas::addImageData(int imgW, int imgH, const unsigned char* imgData, Rectangle& rect) {
    TextureAtlas* ft = this;
    if (ft->isFull) {
        return false;
    }
    int textureWidth = texture->getWidth();
    int textureHeight = texture->getHeight();

    if (ft->lastX + imgW + 1 >= textureWidth) {
        ft->lastX = 0;
        ft->curY = ft->maxY;
    }
    if (ft->curY + imgH + 1 >= textureHeight) {
        ft->isFull = true;
        return false;
    }

    int pixelSize = Image::getFormatBPP(texture->getFormat());

    //copy sub image
    int x = this->lastX + 1;
    int y = this->curY + 1;
    if (pixelSize == 1) {
        copyImageData<1>(this->data, textureWidth, textureHeight, x, y, imgData, imgW, imgH);
    }
    else if (pixelSize == 3) {
        copyImageData<3>(this->data, textureWidth, textureHeight, x, y, imgData, imgW, imgH);
    }
    else if (pixelSize == 4) {
        copyImageData<4>(this->data, textureWidth, textureHeight, x, y, imgData, imgW, imgH);
    }
    else {
        return false;
    }

    if (this->maxY < y + imgH) {
        this->maxY = y + imgH;
    }
    this->lastX = x + imgW;

    rect.x = x;
    rect.y = y;
    rect.width = imgW;
    rect.height = imgH;
    this->rects.push_back(rect);

    //upload image data
    this->texture->setData(this->data, true);

    //if (pixelSize == 1) {
    //    char buffer[128];
    //    snprintf(buffer, 128, "TextureAtlas_%p.png", this);
    //    stbi_write_png(buffer, textureWidth, textureHeight, pixelSize, this->data, textureWidth * pixelSize);
    //}
    return true;
}

bool TextureAtlas::addImageUri(const std::string& file, Rectangle& rect) {
    UPtr<Image> img = Image::create(file.c_str(), false);
    if (!img.get()) return false;
    bool rc = addImage(img.get(), rect);
    //img->release();
    return rc;
}

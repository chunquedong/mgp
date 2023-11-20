/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "FontEngine.h"
#include "platform/Toolkit.h"

extern "C" {
#include "3rd/edtaa3func.h"
}

using namespace mgp;

unsigned char* createDistanceFields(unsigned char* _img, unsigned int _width, unsigned int _height)
{
    int16_t* xdist = (int16_t*)malloc(_width * _height * sizeof(int16_t));
    int16_t* ydist = (int16_t*)malloc(_width * _height * sizeof(int16_t));
    double* gx = (double*)calloc(_width * _height, sizeof(double));
    double* gy = (double*)calloc(_width * _height, sizeof(double));
    double* data = (double*)calloc(_width * _height, sizeof(double));
    double* outside = (double*)calloc(_width * _height, sizeof(double));
    double* inside = (double*)calloc(_width * _height, sizeof(double));
    uint32_t ii;

    // Convert img into double (data)
    double img_min = 255, img_max = -255;
    for (ii = 0; ii < _width * _height; ++ii)
    {
        double v = _img[ii];
        data[ii] = v;
        if (v > img_max)
        {
            img_max = v;
        }

        if (v < img_min)
        {
            img_min = v;
        }
    }

    // Rescale image levels between 0 and 1
    for (ii = 0; ii < _width * _height; ++ii)
    {
        data[ii] = (_img[ii] - img_min) / (img_max - img_min);
    }

    // Compute outside = edtaa3(bitmap); % Transform background (0's)
    computegradient(data, _width, _height, gx, gy);
    edtaa3(data, gx, gy, _width, _height, xdist, ydist, outside);
    for (ii = 0; ii < _width * _height; ++ii)
    {
        if (outside[ii] < 0)
        {
            outside[ii] = 0.0;
        }
    }

    // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
    memset(gx, 0, sizeof(double) * _width * _height);
    memset(gy, 0, sizeof(double) * _width * _height);
    for (ii = 0; ii < _width * _height; ++ii)
    {
        data[ii] = 1.0 - data[ii];
    }

    computegradient(data, _width, _height, gx, gy);
    edtaa3(data, gx, gy, _width, _height, xdist, ydist, inside);
    for (ii = 0; ii < _width * _height; ++ii)
    {
        if (inside[ii] < 0)
        {
            inside[ii] = 0.0;
        }
    }

    // distmap = outside - inside; % Bipolar distance field
    uint8_t* out = (uint8_t*)malloc(_width * _height * sizeof(uint8_t));
    for (ii = 0; ii < _width * _height; ++ii)
    {
        outside[ii] -= inside[ii];
        outside[ii] = 128 + outside[ii] * 16;

        if (outside[ii] < 0)
        {
            outside[ii] = 0;
        }

        if (outside[ii] > 255)
        {
            outside[ii] = 255;
        }

        out[ii] = 255 - (uint8_t)outside[ii];
    }

    free(xdist);
    free(ydist);
    free(gx);
    free(gy);
    free(data);
    free(outside);
    free(inside);
    return out;
}

static unsigned char* paddingImage(unsigned char* _img, unsigned int _width, unsigned int _height, int padding) {
    int w = (_width + padding + padding);
    int h = (_height + padding + padding);
    unsigned char* data = (unsigned char*)malloc(w * h);

    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            int pos = j * w + i;
            if (j < padding || j >= _height + padding) {
                data[pos] = 0;
            }
            else if (i < padding || i >= _width + padding) {
                data[pos] = 0;
            }
            else {
                int opos = (j - padding) * _width + (i - padding);
                data[pos] = _img[opos];
            }
        }
    }
    return data;
}

void GlyphMetrics::scaleMetrics(float scale) {
    GlyphMetrics* metrics = this;
    metrics->width *= scale;
    metrics->height *= scale;

    metrics->horiBearingX *= scale;
    metrics->horiBearingY *= scale;
    metrics->horiAdvance *= scale;

    metrics->vertBearingX *= scale;
    metrics->vertBearingY *= scale;
    metrics->vertAdvance *= scale;
}


#ifndef __EMSCRIPTEN__

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TYPES_H
#include FT_OUTLINE_H
#include FT_RENDER_H
#include FT_STROKER_H
}
//#define PF_FONT_DPI 300


FontFace::FontFace() : library(NULL), face(NULL) {}
FontFace::~FontFace() {
    FT_Done_Face((FT_Face)face);
    FT_Done_FreeType((FT_Library)library);
    face = NULL;
    library = NULL;
}

bool FontFace::load(const char* fileName) {
    FT_Library  library;   /* handle to library     */
    FT_Face     face;      /* handle to face object */
    FT_Error error;


    error = FT_Init_FreeType(&library);
    if (error) {
        printf("init font error\n");
        return false;
    }

    error = FT_New_Face(library,
        fileName,
        0,
        &face);
    if (error != FT_Err_Ok) {
        printf("open file error:%s\n", fileName);
        return false;
    }

    this->library = library;
    this->face = face;

    if (face->num_faces > 1) {
        for (int i = 1, n = face->num_faces; i < n; ++i) {
            error = FT_New_Face(library,
                fileName,
                0,
                &face);
            if (error != FT_Err_Ok) {
                break;
            }
            size_t len = strlen(face->family_name);
            //SC��ʾ��������
            if (len > 2 && face->family_name[len - 2] == 'S' && (face->family_name[len - 1] == 'C')) {

                FT_Done_Face((FT_Face)this->face);
                this->face = face;
                break;
            }
        }
    }

    return true;
}

bool FontFace::merics(Char uniChar, FontInfo& font, GlyphMetrics& m) {
    FT_Face face = (FT_Face)this->face;      /* handle to face object */
    FT_Error error;
    FT_UInt glyph_index;

    if (!face) return false;

    /* set character size */
    float scale = Toolkit::cur()->getScreenScale();
    error = FT_Set_Pixel_Sizes(face, 0, font.size * scale);

    if (uniChar == 0) {
        m.horiAdvance = face->size->metrics.max_advance / 64.0;
        m.horiBearingY = face->size->metrics.ascender / 64.0;
        m.horiBearingX = 0;
        m.vertAdvance = face->size->metrics.height / 64.0;
        m.vertBearingX = 0;
        m.vertBearingY = 0;
        m.height = face->size->metrics.height / 64.0;
        m.width = face->size->metrics.max_advance / 64.0;
        m.scaleMetrics(1.0 / scale);
        return true;
    }

    /* retrieve glyph index from character code */
    glyph_index = FT_Get_Char_Index(face, uniChar);
    if (glyph_index == 0) return false;

    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    if (error) {
        return false;
    }

    m.height = face->glyph->metrics.height / 64.0;
    m.width = face->glyph->metrics.width / 64.0;
    m.horiBearingX = face->glyph->metrics.horiBearingX / 64.0;
    m.horiBearingY = face->glyph->metrics.horiBearingY / 64.0;
    m.horiAdvance = face->glyph->metrics.horiAdvance / 64.0;
    m.vertBearingX = face->glyph->metrics.vertBearingX / 64.0;
    m.vertBearingY = face->glyph->metrics.vertBearingY / 64.0;
    m.vertAdvance = face->glyph->metrics.vertAdvance / 64.0;
    m.scaleMetrics(1.0 / scale);
    return true;
}

float FontFace::getKerning(FontInfo& font, Char previous, Char current) {
    FT_Face     face = (FT_Face)this->face;      /* handle to face object */
    FT_Bool use_kerning = FT_HAS_KERNING(face);

    if (use_kerning && previous && current) {
        FT_Set_Pixel_Sizes(face, 0, font.size);

        FT_UInt index1 = FT_Get_Char_Index(face, previous);
        FT_UInt index2 = FT_Get_Char_Index(face, current);

        FT_Vector  delta;
        FT_Error error = FT_Get_Kerning(face, index1, index2,
            FT_KERNING_DEFAULT, &delta);
        if (error) {
            return 0;
        }
        return delta.x / 64.0;
    }
    return 0;
}

static void bitmapToImage(FT_Glyph glyph, Glyph& image) {
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    FT_Bitmap bitmap = bitmap_glyph->bitmap;
    image.imgX = 0;
    image.imgY = 0;
    image.texture = -1;

    image.imgW = bitmap.width;
    image.imgH = bitmap.rows;
    int padding = image.imgPadding;
    unsigned char* data = paddingImage(bitmap.buffer, bitmap.width, bitmap.rows, padding);
    image.imgW += padding + padding;
    image.imgH += padding + padding;
    image.imgData = createDistanceFields(data, image.imgW, image.imgH);
    free(data);
}

bool FontFace::renderChar(Char uniChar, FontInfo& font, int fontSize, Glyph& g) {
    FT_Face face = (FT_Face)this->face;      /* handle to face object */
    FT_Error error;
    FT_UInt glyph_index;
    FT_Glyph glyph;

    if (!face) return false;

    /* use 50pt at 100dpi */
    /* set character size */
    float scale = Toolkit::cur()->getScreenScale();
    error = FT_Set_Pixel_Sizes(face, 0, fontSize *scale);

    /* retrieve glyph index from character code */
    glyph_index = FT_Get_Char_Index(face, uniChar);
    if (glyph_index == 0) return false;

    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    if (error) return false;

    if (font.bold) {
        FT_Outline_Embolden(&face->glyph->outline, font.bold << 6);
    }

    error = FT_Get_Glyph(face->glyph, &glyph);
    if (error) return false;

    FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    if (error) {
        FT_Done_Glyph(glyph);
        return false;
    }

    g.code = uniChar;
    g.metrics.horiAdvance = face->glyph->advance.x / 64.0f;
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    g.metrics.width = bitmap_glyph->bitmap.width;
    g.metrics.height = bitmap_glyph->bitmap.rows;
    g.metrics.horiBearingX = bitmap_glyph->left;
    g.metrics.horiBearingY = bitmap_glyph->top;
    g.metrics.scaleMetrics(1.0 / scale);
    g.imgScale = scale;
    g.imgPadding = 2*scale;
    bitmapToImage(glyph, g);
    

    FT_Done_Glyph(glyph);

    /////////////////////////////////////////////////////////////
    /*
    FT_Stroker stroker;
    error = FT_Get_Glyph(face->glyph, &glyph);
    if (error) {
        return false;
    }
    error = FT_Stroker_New((FT_Library)library, &stroker);
    if (error) {
        FT_Done_Glyph(glyph);
        return false;
    }

    FT_Stroker_Set(stroker,
        (int)(font.outline * 64),
        FT_STROKER_LINECAP_ROUND,
        FT_STROKER_LINEJOIN_ROUND,
        0);

    error = FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);
    if (error) {
        FT_Done_Glyph(glyph);
        FT_Stroker_Done(stroker);
        return false;
    }
    error = FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    if (error) {
        FT_Done_Glyph(glyph);
        FT_Stroker_Done(stroker);
        return false;
    }

    FT_BitmapGlyph bitmap_glyph2 = (FT_BitmapGlyph)glyph;
    bitmapToImage(glyph, g.outlineImage);

    FT_Done_Glyph(glyph);
    FT_Stroker_Done(stroker);
    */
    return true;
}

#else

extern "C" {
  extern void* fontRender(int uniChar, const char* fontName, int bold, int fontSize, float* out);
  extern int fontMerics(int uniChar, const char* fontName, int bold, int fontSize, float* out);
  extern float fontKerning(int uniChar, const char* fontName, int bold, int fontSize, int previous);
}

FontFace::FontFace() : library(NULL), face(NULL) {}
FontFace::~FontFace() {
    face = NULL;
    library = NULL;
}

bool FontFace::load(const char* fileName) {
    return true;
}

bool FontFace::merics(Char uniChar, FontInfo& font, GlyphMetrics& m) {
    uint64_t key = ((uint64_t)uniChar << 32) | (((uint64_t)font.size) << 8) | (font.bold);
    auto it = metricsCache.find(key);
    if (it != metricsCache.end()) {
        m = it->second;
        return true;
    }

    float out[5];
    float scale = Toolkit::cur()->getScreenScale();
    fontMerics(uniChar, "", font.bold, font.size*scale, out);

    int i = 0;
    m.horiAdvance = out[i++];
    m.width = out[i++];
    m.height = out[i++];
    m.horiBearingX = out[i++];
    m.horiBearingY = out[i++];

    m.vertBearingX = 0;
    m.vertBearingY = 0;
    m.vertAdvance = 0;

    m.scaleMetrics(1.0 / scale);
    //printf("fontMerics: %d,%d,%d: w:%f, h:%f\n", out[0], out[1], out[2], m.width, m.height);
    metricsCache[key] = m;

    return true;
}

float FontFace::getKerning(FontInfo& font, Char previous, Char current) {
    if (previous >= 128 || current >= 128) {
        return 0;
    }
    uint64_t key = ((uint64_t)previous << 32) | (((uint64_t)font.size) << 16) | ((uint64_t)current);
    auto it = kerningCache.find(key);
    if (it != kerningCache.end()) {
        return it->second;
    }

    float size = fontKerning(current, "", font.bold, font.size, previous);
    kerningCache[key] = size;
    return size;
}

static void bitmapToImage(int width, int rows, void* buffer, Glyph& image) {
    //printf("renderFont:w:%d, h:%d, %p\n", width, rows, buffer);

    image.imgX = 0;
    image.imgY = 0;
    image.texture = -1;
    image.imgW = width;
    image.imgH = rows;
    //image.pixelFormat = PixelFormat::GRAY8;
    int size = image.imgW * image.imgH;
    unsigned char* grayData = (unsigned char*)malloc(size);
    for (int i=0; i<rows; ++i) {
        for (int j=0; j<width; ++j) {
            unsigned char value = ((unsigned char*)buffer)[i*width*4+j*4+3];
            grayData[i*width+j] = value;
        }
    }

    int padding = image.imgPadding;
    unsigned char* paddingData = paddingImage(grayData, image.imgW, image.imgH, padding);
    image.imgW += padding + padding;
    image.imgH += padding + padding;
    image.imgData = createDistanceFields(paddingData, image.imgW, image.imgH);
    free(paddingData);
    free(grayData);
    free(buffer);
}

bool FontFace::renderChar(Char uniChar, FontInfo& font, int fontSize, Glyph& g) {
    float out[5];
    float scale = Toolkit::cur()->getScreenScale();
    void* buffer = fontRender(uniChar, "", font.bold, fontSize *scale, out);

    g.code = uniChar;
    int i = 0;
    g.metrics.horiAdvance = out[i++];
    g.metrics.width = out[i++];
    g.metrics.height = out[i++];
    g.metrics.horiBearingX = out[i++];
    g.metrics.horiBearingY = out[i++];
    g.metrics.vertBearingX = 0;
    g.metrics.vertBearingY = 0;
    g.metrics.vertAdvance = 0;
    g.metrics.scaleMetrics(1.0 / scale);
    g.imgScale = scale;
    g.imgPadding = 2 * scale;
    bitmapToImage(ceil(out[1]), ceil(out[2]), buffer, g);
    return true;
}
#endif
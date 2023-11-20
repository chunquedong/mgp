/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef FONTFACE_H_
#define FONTFACE_H_

#include "objects/SpriteBatch.h"


namespace mgp {

struct FontInfo {
	char name[128];
	int size;
	int bold;
	int outline;

	FontInfo() : size(30), bold(1), outline(1) { name[0] = 0; }
};

struct GlyphMetrics {
	float  width;
	float  height;

	float  horiBearingX;
	float  horiBearingY;
	float  horiAdvance;

	float  vertBearingX;
	float  vertBearingY;
	float  vertAdvance;

	void scaleMetrics(float scale);
};

struct Glyph {
	GlyphMetrics metrics;
	int code;
	int imgX;
	int imgY;
	int imgW;
	int imgH;
	float imgScale;
	unsigned char* imgData;
	int texture;
	int imgPadding;
};


class FontFace {
	void* library;
	void* face;

	std::map<uint64_t, float> kerningCache;
	std::map<uint64_t, GlyphMetrics> metricsCache;
public:
	typedef uint32_t Char;
	FontFace();
	~FontFace();
	bool load(const char* file);
	bool renderChar(Char ch, FontInfo& font, int fontSize, Glyph& glyph);

	float getKerning(FontInfo& font, Char previous, Char current);
	bool merics(Char ch, FontInfo& font, GlyphMetrics& m);
};


}


#endif
/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef FONT_H_
#define FONT_H_

#include "objects/SpriteBatch.h"
#include "TextureAtlas.h"
#include "FontEngine.h"

namespace mgp
{

class FontCache : public Refable
{
    friend class Font;
public:

    /**
    * Defines the set of allowable font styles.
    */
    enum Style
    {
        PLAIN = 0,
        BOLD = 1,
        ITALIC = 2,
        BOLD_ITALIC = 4
    };

    static SPtr<FontCache> create(const char* path, int fontSize = 30);

    bool getGlyph(FontInfo& fontInfo, wchar_t ch, Glyph& glyph);
private:
    FontCache();
    ~FontCache();

    std::string _path;
    Style _style;
    unsigned int _size;

    int textureWidth;
    int textureHeight;

    std::vector<TextureAtlas*> fontTextures;

    std::vector<FontFace*> fontFaces;

    std::map<uint64_t, Glyph> glyphCache;
};

/**
 * Defines a font for text rendering.
 */
class Font : public Refable, public BatchableLayer
{
    //friend class Bundle;
    //friend class Text;
    //friend class TextBox;
public:

    /**
     * Defines the set of allowable font styles.
     */
    enum Style
    {
        PLAIN = 0,
        BOLD = 1,
        ITALIC = 2,
        BOLD_ITALIC = 4
    };


    /**
     * Creates a font from the given bundle.
     *
     * If the 'id' parameter is NULL, it is assumed that the Bundle at 'path'
     * contains exactly one Font resource. If the Bundle does not meet this criteria,
     * NULL is returned.
     *
     * If a font for the given path has already been loaded, the existing font will be
     * returned with its reference count increased.
     *
     * @param path The path to a bundle file containing a font resource.
     * @param id An optional ID of the font resource within the bundle (NULL for the first/only resource).
     *
     * @return The specified Font or NULL if there was an error.
     * @script{create}
     */
    static UPtr<Font> create(const char* path, int outline = 0, int fontSize = 30);

    /**
     * Determines if this font supports the specified character code.
     *
     * @param character The character code to check.
     * @return True if this Font supports (can draw) the specified character, false otherwise.
     */
    bool isCharacterSupported(int character) const;

    /**
     * Starts text drawing for this font.
     */
    void start();

    /**
     * Draws the specified text in a solid color, with a scaling factor.
     *
     * @param text The text to draw.
     * @param x The viewport x position to draw text at.
     * @param y The viewport y position to draw text at.
     * @param color The color of text.
     * @param size The size to draw text (0 for default size).
     * @param rightToLeft Whether to draw text from right to left.
     * @return drawed height
     */
    int drawText(const char* text, float x, float y, const Vector4& color, float fontSize = 0, int textLen = -1, const Rectangle* clip = NULL);
    int drawText(const wchar_t* text, float x, float y, const Vector4& color, float fontSize, int textLen, const Rectangle* clip = NULL);


    /**
     * Finishes text batching for this font and renders all drawn text.
     */
    void finish(RenderInfo* view);
    void finalDraw(RenderInfo* view);

    virtual void setProjectionMatrix(const Matrix& matrix);
    virtual bool isStarted() const;

    /**
     * Measures a string's width and height without alignment, wrapping or clipping.
     *
     * @param text The text to measure.
     * @param size The font height to scale to.
     * @param widthOut Destination for the text's width.
     * @param heightOut Destination for the text's height.
     */
    void measureText(const char* text, float fontSize, unsigned int* widthOut, unsigned int* heightOut, int textLen = -1);
    void measureText(const wchar_t* text, float fontSize, unsigned int* widthOut, unsigned int* heightOut, int textLen);


    /**
     * Returns current character spacing for this font in percentage of fonts size.
     *
     * @see setCharacterSpacing(float)
     */
    float getCharacterSpacing() const;

    /**
     * Sets the additional character spacing for this font.
     *
     * Character spacing is the additional amount of space that is inserted between characters. Character spacing is defined
     * as a floating point value that is interpreted as a percentage of size used to draw the font. For example,
     * a value of 0.1 would cause a spacing of 10% of the font size to be inserted between adjacent characters.
     * For a font size of 20, this would equate to 2 pixels of extra space between characters.
     *
     * The default additional character spacing for fonts is 0.0.
     *
     * @param spacing New fixed character spacing, expressed as a percentage of font size.
     */
    void setCharacterSpacing(float spacing);


    int getSize() { return _fontCache->_size; }

    int getOutline() { return _outline; }

    unsigned int getLineHeight(unsigned int fontSize);

    int indexAtCoord(const wchar_t *text, unsigned int fontSize, bool clipToFloor, int textLen, int x);

    bool isImmediatelyDraw() { return _immediatelyDraw; }
    void setImmediatelyDraw(bool g) { _immediatelyDraw = g; }
    void set3D(bool s) { _is3D = s; }
private:
    bool _immediatelyDraw = false;

    /**
     * Constructor.
     */
    Font();

    /**
     * Constructor.
     */
    Font(const Font& copy);

    /**
     * Destructor.
     */
    ~Font();

    /**
     * Hidden copy assignment operator.
     */
    Font& operator=(const Font&);

    bool drawChar(int c, FontInfo &fontInfo, Glyph &glyph, float x, float y, const Vector4& color, int previous, const Rectangle* clip);


    void lazyStart();

    bool _isStarted;
    float _spacing;
    int _outline;
    bool _hasProjectionMatrix;
    bool _is3D;

    std::vector<SpriteBatch*> fontDrawers;
    SPtr<FontCache> _fontCache;

    ShaderProgram* shaderProgram;
};


class FontLayout {

public:
    /**
    * Defines the set of allowable alignments when drawing text.
    */
    enum Justify
    {
        ALIGN_LEFT = 0x01,
        ALIGN_HCENTER = 0x02,
        ALIGN_RIGHT = 0x04,
        ALIGN_TOP = 0x10,
        ALIGN_VCENTER = 0x20,
        ALIGN_BOTTOM = 0x40,
        ALIGN_TOP_LEFT = ALIGN_TOP | ALIGN_LEFT,
        ALIGN_VCENTER_LEFT = ALIGN_VCENTER | ALIGN_LEFT,
        ALIGN_BOTTOM_LEFT = ALIGN_BOTTOM | ALIGN_LEFT,
        ALIGN_TOP_HCENTER = ALIGN_TOP | ALIGN_HCENTER,
        ALIGN_VCENTER_HCENTER = ALIGN_VCENTER | ALIGN_HCENTER,
        ALIGN_BOTTOM_HCENTER = ALIGN_BOTTOM | ALIGN_HCENTER,
        ALIGN_TOP_RIGHT = ALIGN_TOP | ALIGN_RIGHT,
        ALIGN_VCENTER_RIGHT = ALIGN_VCENTER | ALIGN_RIGHT,
        ALIGN_BOTTOM_RIGHT = ALIGN_BOTTOM | ALIGN_RIGHT
    };

private:
    std::wstring unicode;

    struct Line {
        int pos;
        int len;
    };

    std::vector<Line> lines;

    int wrapWidth = -1;

    Font* font = NULL;
    unsigned int fontSize = 0;
    unsigned int lineHeight = 0;

public:
    static std::string enumToString(const std::string& enumName, int value);
    static int enumParse(const std::string& enumName, const std::string& str);
    static Justify getJustify(const char* justify);

    void update(Font* font, unsigned int fontSize, const char* text, int textLen = -1, int wrapWidth = -1);


    void drawText(const Rectangle& area, const Vector4& color, Justify align, const Rectangle* clip = NULL);
    void measureText(unsigned int* widthOut, unsigned int* heightOut);

    Vector2 positionAtIndex(int index);
    int indexAtPosition(Vector2& pos);

private:
    bool nextLine(Line& line);
    void doWrap();
    int lenAtWrap(Line line);
};

}

#endif

/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "base/Base.h"
#include "Font.h"
//#include "Text.h"
#include "platform/Toolkit.h"
#include "base/FileSystem.h"
#include "material/Material.h"
#include "material/MaterialParameter.h"
#include <algorithm>
#include "base/StringUtil.h"

extern "C" {
#include "3rd/utf8.h"
}
// Default font shaders
#define FONT_VSH "res/shaders/font.vert"
#define FONT_FSH "res/shaders/font.frag"

#include "3rd/stb_image_write.h"

size_t utf8decode(char const* str, int len, std::wstring& out, int* illegal) {

    char* s = (char*)str;
    size_t i = 0;
    wchar_t uc = 0;
    int r_illegal_all = 0, r_illegal;

    while ((uc = getu8c(&s, &r_illegal)))
    {
        if (len != -1 && s > str + len) break;

        out.push_back(uc);
        r_illegal_all += r_illegal;
        ++i;
    }

    //out.push_back(0);
    if (illegal)
    {
        *illegal = r_illegal_all + r_illegal;
    }

    return i;
}


namespace mgp
{

static std::vector<FontCache*> __fontCache;

//static ShaderProgram* __fontEffect = NULL;

SPtr<FontCache> FontCache::create(const char* path, int fontSize)
{
    GP_ASSERT(path);

    // Search the font cache for a font with the given path and ID.
    for (size_t i = 0, count = __fontCache.size(); i < count; ++i)
    {
        FontCache* f = __fontCache[i];
        GP_ASSERT(f);
        if (f->_path == path && f->_size == fontSize)
        {
            // Found a match.
            f->addRef();
            return SPtr<FontCache>(f);
        }
    }

    FontCache* font = new FontCache();
    FontFace* face = new FontFace();
    face->load(path);
    font->fontFaces.push_back(face);
    font->_path = path;
    font->_size = fontSize;

    __fontCache.push_back(font);
    return SPtr<FontCache>(font);
}

FontCache::~FontCache() {
    std::vector<FontCache*>::iterator itr = std::find(__fontCache.begin(), __fontCache.end(), this);
    if (itr != __fontCache.end())
    {
        __fontCache.erase(itr);
    }

    for (size_t i = 0, count = fontTextures.size(); i < count; ++i)
    {
        fontTextures[i]->release();
    }
    fontTextures.clear();

    for (size_t i = 0, count = fontFaces.size(); i < count; ++i)
    {
        delete fontFaces[i];
    }
    fontFaces.clear();
}

FontCache::FontCache() :
    _style(PLAIN), _size(25), textureWidth(512), textureHeight(512)
{
}

bool FontCache::getGlyph(FontInfo& fontInfo, wchar_t c, Glyph& glyph) {
    uint64_t key = ((uint64_t)c << 8) | (fontInfo.bold);
    double fontSizeScale = fontInfo.size / (double)_size;

    TextureAtlas* fontTexture = NULL;
    int textureIndex = 0;
    auto itr = glyphCache.find(key);
    if (itr != glyphCache.end()) {
        glyph = itr->second;
        fontTexture = fontTextures[glyph.texture];
        glyph.metrics.scaleMetrics(fontSizeScale);
    }
    else {
        //render char to image
        if (!fontFaces.at(0)->renderChar(c, fontInfo, _size, glyph)) {
            return false;
        }

        //find free space
        Rectangle rect;
        int i = 0;
        //if (c > 128) i = 1;
        for (; i < fontTextures.size(); ++i) {
            TextureAtlas* ft = fontTextures[i];
            if (ft->addImageData(glyph.imgW, glyph.imgH, glyph.imgData, rect)) {
                fontTexture = ft;
                textureIndex = i;
                break;
            }
        }

        //new texture
        if (fontTexture == NULL) {
            fontTexture = new TextureAtlas(Image::Format::RED, textureWidth, textureHeight);
            fontTexture->getTexture()->setFilterMode(Texture::LINEAR, Texture::LINEAR);
            //fontTexture->getTexture()->setFilterMode(Texture::NEAREST, Texture::NEAREST);
            textureIndex = fontTextures.size();
            fontTextures.push_back(fontTexture);

            bool ok = fontTexture->addImageData(glyph.imgW, glyph.imgH, glyph.imgData, rect);
            GP_ASSERT(ok);
        }

        free(glyph.imgData);
        glyph.imgData = NULL;

        glyph.texture = textureIndex;
        glyph.imgX = rect.x;
        glyph.imgY = rect.y;

        glyphCache[key] = glyph;

        glyph.metrics.scaleMetrics(fontSizeScale);
        //char name[256];
        //snprintf(name, 256, "fontTexture_%p.png", this);
        //stbi_write_png(name, textureWidth, textureHeight, 1, fontTexture->data, textureWidth * 1);
    }
    return true;
}

Font::Font() : _spacing(0.0f), _isStarted(false), shaderProgram(NULL), _outline(0)
{
    //shaderProgram = ShaderProgram::createFromFile(FONT_VSH, FONT_FSH);
}

Font::~Font()
{
    for (size_t i = 0, count = fontDrawers.size(); i < count; ++i) {
        SpriteBatch* _batch = fontDrawers[i];
        if (!_batch) continue;
        SAFE_DELETE(_batch);
    }
    fontDrawers.clear();

    if (shaderProgram) {
        shaderProgram->release();
        shaderProgram = NULL;
    }
}

UPtr<Font> Font::create(const char* path, int outline, int fontSize)
{
    GP_ASSERT(path);

    Font* font = new Font();
    font->_fontCache = FontCache::create(path, fontSize);
    font->_outline = outline;

    return UPtr<Font>(font);
}

bool Font::isCharacterSupported(int character) const
{
    //TODO
    return true;
}

void Font::start()
{
    // no-op : fonts now are lazily started on the first draw call
    _isStarted = true;
    if (!shaderProgram) {
        std::string define = "DISTANCE_FIELD";
        if (_outline) {
            define += ";FONT_OUTLINE";
        }
        shaderProgram = ShaderProgram::createFromFile(FONT_VSH, FONT_FSH, define.c_str());
    }
}

void Font::lazyStart()
{
    for (size_t i = 0, count = fontDrawers.size(); i < count; ++i) {
        SpriteBatch* _batch = fontDrawers[i];
        if (!_batch) continue;

        if (_batch->isStarted())
            return; // already started

        _batch->start();
    }
}

void Font::finish(RenderInfo* view)
{
    for (size_t i = 0, count = fontDrawers.size(); i < count; ++i) {
        SpriteBatch* _batch = fontDrawers[i];
        if (!_batch) continue;

        if (view) {
            int w = view->viewport.width / Toolkit::cur()->getScreenScale();
            int h = view->viewport.height / Toolkit::cur()->getScreenScale();
            Matrix projectionMatrix;
            Matrix::createOrthographicOffCenter(0, w, h, 0, 0, 1, &projectionMatrix);
            _batch->setProjectionMatrix(projectionMatrix);
        }
        else {
            int w = Toolkit::cur()->getDpWidth();
            int h = Toolkit::cur()->getDpHeight();
            if (w && h)
            {
                Matrix projectionMatrix;
                Matrix::createOrthographicOffCenter(0, w, h, 0, 0, 1, &projectionMatrix);
                _batch->setProjectionMatrix(projectionMatrix);
            }
        }

        // Finish any font batches that have been started
        if (_batch->isStarted()) {
            _batch->finish(_immediatelyDraw ? NULL : view);
        }
    }
    _isStarted = false;
}

void Font::setProjectionMatrix(const Matrix& matrix) {
    for (size_t i = 0, count = fontDrawers.size(); i < count; ++i) {
        SpriteBatch* _batch = fontDrawers[i];
        if (!_batch) continue;
        _batch->setProjectionMatrix(matrix);
    }
}

bool Font::isStarted() const {
    return _isStarted;
}


bool Font::drawChar(int c, FontInfo& fontInfo, Glyph& glyph, float x, float y, const Vector4& color, int previous, const Rectangle* clip) {

    if (!_fontCache->getGlyph(fontInfo, c, glyph)) {
        return false;
    }

    double fontSizeScale = fontInfo.size / (double)_fontCache->_size;

    if (fontDrawers.size() != _fontCache->fontTextures.size()) {
        fontDrawers.resize(_fontCache->fontTextures.size(), NULL);
    }
    SpriteBatch* _batch = fontDrawers[glyph.texture];
    if (!_batch) {
        TextureAtlas* fontTexture = _fontCache->fontTextures[glyph.texture];
        _batch = SpriteBatch::create(fontTexture->getTexture(), shaderProgram).take();
        _batch->getBatch()->setRenderLayer(Drawable::Overlay);
        auto _cutoffParam = _batch->getMaterial()->getParameter("u_cutoff");
        _cutoffParam->setVector2(Vector2(0.50, 0.1));
        if (_outline) {
            auto u_outline = _batch->getMaterial()->getParameter("u_outline");
            u_outline->setVector2(Vector2(0.45, 0.1));
        }
        fontDrawers[glyph.texture] = _batch;
        _batch->start();
    }

    if (previous > 0 && previous < 128 && c < 128) {
        float kerning = _fontCache->fontFaces.at(0)->getKerning(fontInfo, previous, c);
        x += kerning;
    }

    _batch->draw(
            x + (glyph.metrics.horiBearingX) - glyph.imgPadding / glyph.imgScale,
            y - (glyph.metrics.horiBearingY - fontInfo.size) - glyph.imgPadding / glyph.imgScale,
            glyph.imgW / glyph.imgScale * fontSizeScale,
            glyph.imgH / glyph.imgScale * fontSizeScale,

            (glyph.imgX) / (float)_fontCache->textureWidth,
            (glyph.imgY) / (float)_fontCache->textureHeight,
            (glyph.imgX + glyph.imgW) / (float)_fontCache->textureWidth,
            (glyph.imgY + glyph.imgH) / (float)_fontCache->textureHeight,

            color, clip);

    return true;
}


int Font::drawText(const char* text, float x, float y, const Vector4& color, unsigned int fontSize, int textLen, const Rectangle* clip) {
    std::wstring utext;
    int utextSize = utf8decode(text, textLen, utext, NULL);

    return drawText(utext.data(), x, y, color, fontSize, utextSize, clip);
}

int Font::drawText(const wchar_t* utext, float x, float y, const Vector4& color, unsigned int fontSize, int utextSize, const Rectangle* clip)
{
    GP_ASSERT(utext);

    if (fontSize == 0)
    {
        fontSize = _fontCache->_size;
    }
    
    lazyStart();

    float spacing = (fontSize * _spacing);


    float xPos = x, yPos = y;

    FontInfo fontInfo;
    fontInfo.bold = 0;
    fontInfo.size = fontSize;
    GlyphMetrics metrics;
    _fontCache->fontFaces.at(0)->merics(0, fontInfo, metrics);

    wchar_t previous = 0;
    for (size_t i = 0; i < utextSize; i++)
    {
        uint32_t c = utext[i];

        // Draw this character.
        switch (c)
        {
        case ' ':
            xPos += fontSize/3.0;
            break;
        case '\r':
            break;
        case '\n':
            yPos += metrics.height;
            xPos = x;
            break;
        case '\t':
            xPos += fontSize * 2;
            break;
        default: {
            Glyph glyph;
            if (drawChar(c, fontInfo, glyph, xPos, yPos, color, previous, clip)) {
                xPos += glyph.metrics.horiAdvance + spacing;
            }
            else {
                xPos += fontSize;
            }
        }
        }

        previous = c;
    }

    return yPos + metrics.height - y;
}

void Font::measureText(const char* text, unsigned int fontSize, unsigned int* width, unsigned int* height, int textLen) {
    std::wstring utext;
    int utextSize = utf8decode(text, textLen, utext, NULL);

    measureText(utext.data(), fontSize, width, height, utextSize);
}

void Font::measureText(const wchar_t* utext, unsigned int fontSize, unsigned int* width, unsigned int* height, int textLen)
{
    //GP_ASSERT(_size);
    GP_ASSERT(utext);
    GP_ASSERT(width);
    GP_ASSERT(height);

    if (fontSize == 0)
    {
        fontSize = _fontCache->_size;
    }

    if (textLen == 0)
    {
        *width = 0;
        *height = 0;
        return;
    }

    float spacing = (fontSize * _spacing);

    float xPos = 0, yPos = 0;

    FontInfo fontInfo;
    fontInfo.bold = 0;
    fontInfo.size = fontSize;
    GlyphMetrics metrics;
    _fontCache->fontFaces.at(0)->merics(0, fontInfo, metrics);

    float maxW = 0;
    for (size_t i = 0; i < textLen; i++)
    {
        uint32_t c = utext[i];

        // Draw this character.
        switch (c)
        {
        case ' ':
            xPos += fontSize/3;
            break;
        case '\r':
            break;
        case '\n':
            yPos += metrics.height;
            xPos = 0;
            break;
        case '\t':
            xPos += fontSize * 2;
            break;
        default: {
            GlyphMetrics m;
            if (_fontCache->fontFaces.at(0)->merics(c, fontInfo, m)) {
                xPos += (m.horiAdvance + spacing);
            }
            else {
                xPos += fontSize;
            }
        }
        }

        if (maxW < xPos) {
            maxW = xPos;
        }
    }
    *width = ceil(maxW);
    *height = ceil(metrics.height + yPos);
}

unsigned int Font::getLineHeight(unsigned int fontSize) {
    if (fontSize == 0)
    {
        fontSize = _fontCache->_size;
    }

    FontInfo fontInfo;
    fontInfo.bold = 0;
    fontInfo.size = fontSize;
    GlyphMetrics metrics;
    _fontCache->fontFaces.at(0)->merics(0, fontInfo, metrics);
    return metrics.height;
}

float Font::getCharacterSpacing() const
{
    return _spacing;
}

void Font::setCharacterSpacing(float spacing)
{
    _spacing = spacing;
}


int Font::indexAtCoord(const wchar_t* utext, unsigned int fontSize, bool clipToFloor, int textLen, int x)
{

    if (fontSize == 0)
    {
        fontSize = _fontCache->_size;
    }

    if (textLen == 0)
    {
        return 0;
    }

    float spacing = (fontSize * _spacing);

    float xPos = 0, yPos = 0;

    FontInfo fontInfo;
    fontInfo.bold = 0;
    fontInfo.size = fontSize;
    GlyphMetrics metrics;
    _fontCache->fontFaces.at(0)->merics(0, fontInfo, metrics);

    for (size_t i = 0; i < textLen; i++)
    {
        if (xPos == x) {
            return i;
        }
        else if (xPos > x) {
            if (clipToFloor) return i-1;
            else return i;
        }

        uint32_t c = utext[i];

        // Draw this character.
        switch (c)
        {
        case ' ':
            xPos += fontSize / 3;
            break;
        case '\r':
            break;
        case '\n':
            yPos += metrics.height;
            xPos = 0;
            break;
        case '\t':
            xPos += fontSize * 2;
            break;
        default: {
            GlyphMetrics m;
            if (_fontCache->fontFaces.at(0)->merics(c, fontInfo, m)) {
                xPos += (m.horiAdvance + spacing);
            }
            else {
                xPos += fontSize;
            }
        }
        }

    }
    return textLen;
}



/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

std::string FontLayout::enumToString(const std::string& enumName, int value)
{
    if (enumName.compare("mgp::FontLayout::Justify") == 0)
    {
        std::string h;
        switch (value & 0x0F)
        {
        case static_cast<int>(ALIGN_LEFT):
            h = "Left";
        case static_cast<int>(ALIGN_HCENTER):
            h = "HCenter";
        case static_cast<int>(ALIGN_RIGHT):
            h = "Right";
        default:
            h = "Left";
        }

        std::string v;
        switch (value & 0xF0)
        {
        case static_cast<int>(ALIGN_TOP):
            h = "Top";
        case static_cast<int>(ALIGN_VCENTER):
            h = "VCenter";
        case static_cast<int>(ALIGN_BOTTOM):
            h = "Bottom";
        default:
            h = "Left";
        }

        return v + "_" + h;
    }
    return "";
}

int FontLayout::enumParse(const std::string& enumName, const std::string& str)
{
    if (enumName.compare("mgp::FontLayout::Justify") == 0)
    {
        std::vector<std::string> fs = StringUtil::split(str, "_");
        if (fs.size() == 2) {
            std::string& v = fs[0];
            std::string& h = fs[1];
            int iv = 0;
            int ih = 0;
            if (v.compare("Left") == 0)
                iv = static_cast<int>(ALIGN_LEFT);
            else if (v.compare("HCenter") == 0)
                iv = static_cast<int>(ALIGN_HCENTER);
            else if (v.compare("Right") == 0)
                iv = static_cast<int>(ALIGN_RIGHT);

            if (h.compare("Top") == 0)
                ih = static_cast<int>(ALIGN_TOP);
            else if (v.compare("VCenter") == 0)
                ih = static_cast<int>(ALIGN_VCENTER);
            else if (v.compare("Bottom") == 0)
                ih = static_cast<int>(ALIGN_BOTTOM);

            return iv << 8 | ih;
        }
    }
    return 0;
}

FontLayout::Justify FontLayout::getJustify(const char* justify)
{
    if (!justify)
    {
        return Justify::ALIGN_TOP_LEFT;
    }

    if (strcmpnocase(justify, "ALIGN_LEFT") == 0)
    {
        return Justify::ALIGN_LEFT;
    }
    else if (strcmpnocase(justify, "ALIGN_HCENTER") == 0)
    {
        return Justify::ALIGN_HCENTER;
    }
    else if (strcmpnocase(justify, "ALIGN_RIGHT") == 0)
    {
        return Justify::ALIGN_RIGHT;
    }
    else if (strcmpnocase(justify, "ALIGN_TOP") == 0)
    {
        return Justify::ALIGN_TOP;
    }
    else if (strcmpnocase(justify, "ALIGN_VCENTER") == 0)
    {
        return Justify::ALIGN_VCENTER;
    }
    else if (strcmpnocase(justify, "ALIGN_BOTTOM") == 0)
    {
        return Justify::ALIGN_BOTTOM;
    }
    else if (strcmpnocase(justify, "ALIGN_TOP_LEFT") == 0)
    {
        return Justify::ALIGN_TOP_LEFT;
    }
    else if (strcmpnocase(justify, "ALIGN_VCENTER_LEFT") == 0)
    {
        return Justify::ALIGN_VCENTER_LEFT;
    }
    else if (strcmpnocase(justify, "ALIGN_BOTTOM_LEFT") == 0)
    {
        return Justify::ALIGN_BOTTOM_LEFT;
    }
    else if (strcmpnocase(justify, "ALIGN_TOP_HCENTER") == 0)
    {
        return Justify::ALIGN_TOP_HCENTER;
    }
    else if (strcmpnocase(justify, "ALIGN_VCENTER_HCENTER") == 0)
    {
        return Justify::ALIGN_VCENTER_HCENTER;
    }
    else if (strcmpnocase(justify, "ALIGN_BOTTOM_HCENTER") == 0)
    {
        return Justify::ALIGN_BOTTOM_HCENTER;
    }
    else if (strcmpnocase(justify, "ALIGN_TOP_RIGHT") == 0)
    {
        return Justify::ALIGN_TOP_RIGHT;
    }
    else if (strcmpnocase(justify, "ALIGN_VCENTER_RIGHT") == 0)
    {
        return Justify::ALIGN_VCENTER_RIGHT;
    }
    else if (strcmpnocase(justify, "ALIGN_BOTTOM_RIGHT") == 0)
    {
        return Justify::ALIGN_BOTTOM_RIGHT;
    }
    else
    {
        GP_WARN("Invalid alignment string: '%s'. Defaulting to ALIGN_TOP_LEFT.", justify);
    }

    // Default.
    return Justify::ALIGN_TOP_LEFT;
}

void FontLayout::update(Font* font, unsigned int fontSize, const char* text, int textLen, int wrapWidth)
{
    unicode.clear();
    int utextSize = utf8decode(text, textLen, unicode, NULL);

    this->font = font;
    this->fontSize = fontSize;
    this->wrapWidth = wrapWidth;
    this->lineHeight = font->getLineHeight(fontSize);
    
    if (wrapWidth == -1) {
        lines.clear();
        Line line = { 0,0 };
        while (nextLine(line)) {
            lines.push_back(line);
        }
    }
    else {
        doWrap();
    }
}

void FontLayout::drawText(const Rectangle& area, const Vector4& color, Justify align, const Rectangle* clip)
{
    int yPos = area.y;
    int xPos = area.x;

    int textHeight = lineHeight * lines.size();
    if (align & FontLayout::ALIGN_VCENTER) {
        yPos = area.y + ((area.height - textHeight) / 2.0);
    }
    else if (align & FontLayout::ALIGN_BOTTOM) {
        yPos = area.y + (area.height - textHeight);
    }

    for (Line& line : lines) {

        if (align & FontLayout::ALIGN_HCENTER) {
            unsigned int h;
            unsigned int w;
            font->measureText(unicode.data() + line.pos, fontSize, &w, &h, line.len);
            xPos = area.x + ((area.width - w) / 2.0);
        }
        else if (align & FontLayout::ALIGN_RIGHT) {
            unsigned int h;
            unsigned int w;
            font->measureText(unicode.data() + line.pos, fontSize, &w, &h, line.len);
            xPos = area.x + (area.width - w);
        }

        yPos += font->drawText(unicode.data() + line.pos, xPos, yPos, color, fontSize, line.len, clip);
    }
}

void FontLayout::measureText(unsigned int* widthOut, unsigned int* heightOut)
{
    unsigned maxWidth = 0;
    unsigned int h = 0;
    for (Line& line : lines) {
        unsigned int w;
        font->measureText(unicode.data() + line.pos, fontSize, &w, &h, line.len);
        if (w > maxWidth) maxWidth = w;
    }
    *widthOut = maxWidth;
    *heightOut = h * lines.size();
}

Vector2 FontLayout::positionAtIndex(int index)
{
    int row = 0;
    int x = 0;
    for (int i = 0; i < lines.size(); ++i) {
        Line& line = lines[i];
        if (line.pos == index) {
            row = i;
            x = 0;
        }
        else if (line.pos+line.len > index || i == lines.size()-1) {
            row = i;
            unsigned int w;
            unsigned int h;
            font->measureText(unicode.data() + line.pos, fontSize, &w, &h, index - line.pos);
            x = w;
        }
    }
    return Vector2(x, row * lineHeight);
}

int FontLayout::indexAtPosition(Vector2& pos)
{
    int row = pos.y / lineHeight;
    if (row < 0) row = 0;
    if (row >= lines.size()) {
        return unicode.size();
    }

    Line line = lines[row];
    int len = font->indexAtCoord(unicode.data() + line.pos, fontSize, true, line.len, pos.x);
    return line.pos + len;
}

bool FontLayout::nextLine(Line& line)
{
    if (line.pos + line.len >= unicode.size()) return false;
    line.pos += line.len;
    line.len = 0;

    if (unicode[line.pos] == '\n') {
        line.pos += 1;
    }

    for (int i = line.pos; i < unicode.size(); ++i) {
        if (unicode[i] == '\n') {
            line.len = i-line.pos;
        }
    }
    if (line.len == 0) {
        line.len = unicode.size() - line.pos;
    }
    return true;
}

void FontLayout::doWrap()
{
    lines.clear();
    Line line = { 0,0 };
    while (nextLine(line)) {
        while (true) {
            int len = lenAtWrap(line);
            if (len == line.len) {
                lines.push_back(line);
                break;
            }

            Line subLine = { line.pos, len };
            lines.push_back(subLine);
            line.pos = line.pos + len;
            line.len = line.len - len;
        }
    }
}

int FontLayout::lenAtWrap(Line line)
{
    int len = font->indexAtCoord(unicode.data()+line.pos, fontSize, true, line.len, wrapWidth);
    if (len == line.len) return len;
    for (int i = len; i > 0; --i) {
        if (unicode[line.pos + i] == L' ') {
            return i;
        }
    }
    return len;
}



}

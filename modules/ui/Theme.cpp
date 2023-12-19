#include "base/Base.h"
#include "Theme.h"
#include "ThemeStyle.h"
#include "platform/Toolkit.h"
#include "base/FileSystem.h"

namespace mgp
{

static std::vector<Theme*> __themeCache;
static Theme* __defaultTheme = NULL;

Theme::Theme() : _texture(NULL), _spriteBatch(NULL)
{
}

Theme::~Theme()
{
    clear();
    SAFE_RELEASE(_texture);
    SAFE_DELETE(_spriteBatch);

    // Remove ourself from the theme cache.
    std::vector<Theme*>::iterator itr = std::find(__themeCache.begin(), __themeCache.end(), this);
    if (itr != __themeCache.end())
    {
        __themeCache.erase(itr);
    }


	if (__defaultTheme == this)
		__defaultTheme = NULL;
}

void Theme::clear() {
    // Destroy all the cursors, styles and , fonts.
    for (auto it = _styles.begin(); it != _styles.end(); ++it)
    {
        Style* style = it->second;
        SAFE_RELEASE(style);
    }
    _styles.clear();

    for (auto it = _images.begin(); it != _images.end(); ++it)
    {
        ThemeImage* image = it->second;
        SAFE_RELEASE(image);
    }
    _images.clear();
}

Theme* Theme::getDefault()
{
	if (!__defaultTheme)
	{
		// Check game.config for a default theme setting
		Properties* config = Toolkit::cur()->getConfig()->getNamespace("ui", true);
		if (config)
		{
			const char* defaultTheme = config->getString("theme");
			if (defaultTheme && FileSystem::fileExists(defaultTheme))
				__defaultTheme = Theme::create(defaultTheme).take();
		}

        if (!__defaultTheme)
        {
            __defaultTheme = Theme::create("res/ui/default.theme").take();
        }

        if (!__defaultTheme)
        {
            // Create an empty theme so that UI's with no theme don't just crash
            GP_WARN("Creating empty UI Theme.");
            __defaultTheme = new Theme();
            //unsigned int color = 0x00000000;
            //__defaultTheme->_texture = Texture::create(Texture::RGBA, 1, 1, (unsigned char*)&color, false);
            //__defaultTheme->_emptyImage = new ThemeImage(1.0f, 1.0f, Rectangle::empty(), Vector4::zero());
            //__defaultTheme->_spriteBatch = SpriteBatch::create(__defaultTheme->_texture);
            //__defaultTheme->_spriteBatch->getSampler()->setFilterMode(Texture::LINEAR, Texture::LINEAR);
            //__defaultTheme->_spriteBatch->getSampler()->setWrapMode(Texture::CLAMP, Texture::CLAMP);
        }

		// TODO: Use a built-in (compiled-in) default theme resource as the final fallback so that
		// UI still works even when no theme files are present.
	}

	return __defaultTheme;
}

void Theme::setDefault(Theme* t) {
    SAFE_RELEASE(__defaultTheme);
    __defaultTheme = t;
    t->addRef();
}

void Theme::finalize()
{
    if (__defaultTheme) {
        __defaultTheme->clear();
        SAFE_RELEASE(__defaultTheme);
    }
}

SPtr<Theme> Theme::create(const char* url)
{
    GP_ASSERT(url);

    // Search theme cache first.
    for (size_t i = 0, count = __themeCache.size(); i < count; ++i)
    {
        Theme* t = __themeCache[i];
        if (t->_url == url)
        {
            // Found a match.
            t->addRef();

            return SPtr<Theme>(t);
        }
    }

    // Load theme properties from file path.
    UPtr<Properties> properties = Properties::create(url);
    GP_ASSERT(properties.get());
    if (properties.get() == NULL)
    {
        return SPtr<Theme>(NULL);
    }

    // Check if the Properties is valid and has a valid namespace.
    Properties* themeProperties = (strlen(properties->getNamespace()) > 0) ? properties.get() : properties->getNextNamespace();
    GP_ASSERT(themeProperties);
    if (!themeProperties || !(strcmpnocase(themeProperties->getNamespace(), "theme") == 0))
    {
        //SAFE_DELETE(properties);
        return SPtr<Theme>(NULL);
    }

    // Create a new theme.
    SPtr<Theme> theme(new Theme());
    theme->_url = url;

    // Parse the Properties object and set up the theme.
    //std::string textureFile;
    //themeProperties->getPath("texture", &textureFile);
    theme->_texture = new TextureAtlas(Texture::RGBA, 1024, 1024);//Texture::create(textureFile.c_str(), false);
    GP_ASSERT(theme->_texture);
    Texture* texture = theme->_texture->getTexture();
    theme->_spriteBatch = SpriteBatch::create(texture).take();
    GP_ASSERT(theme->_spriteBatch);
    texture->setFilterMode(Texture::LINEAR, Texture::LINEAR);
    texture->setWrapMode(Texture::CLAMP, Texture::CLAMP);

    float tw = 1.0f / texture->getWidth();
    float th = 1.0f / texture->getHeight();

    //theme->_emptyImage = new ThemeImage(Rectangle::empty());
    
    //themeProperties->rewind();
    Properties* space = themeProperties->getNextNamespace();
    while (space != NULL)
    {
        const char* spacename = space->getNamespace();
        if (strcmpnocase(spacename, "style") == 0)
        {
            Style* style = new Style(theme, space->getId());

            Properties* innerSpace = space;
            
            //Font
            if (true) {
                if (innerSpace->exists("textColor"))
                {
                    Vector4 textColor(0, 0, 0, 1);
                    innerSpace->getColor("textColor", &textColor);
                    style->setTextColor(textColor);
                }

                std::string fontPath;
                innerSpace->getPath("font", &fontPath);
                if (fontPath.size() > 0)
                {
#if WIN32
                    fontPath = "C:/Windows/Fonts/msyh.ttc";
#endif
                    UPtr<Font> font = Font::create(fontPath.c_str());
                    if (font.get())
                    {
                        style->setFont(font.get());
                        //font->release();
                    }
                }

                if (innerSpace->exists("fontSize")) {
                    unsigned int fontSize = innerSpace->getInt("fontSize");
                    style->setFontSize(fontSize);
                }

                const char* textAlignmentString = innerSpace->getString("textAlignment");
                FontLayout::Justify textAlignment = FontLayout::ALIGN_TOP_LEFT;
                if (textAlignmentString)
                {
                    textAlignment = FontLayout::getJustify(textAlignmentString);
                }
                style->setTextAlignment(textAlignment);

                bool rightToLeft = innerSpace->getBool("rightToLeft");
                style->setTextRightToLeft(rightToLeft);
            }
            
            if (innerSpace->exists("opacity"))
            {
                float opacity = 1.0f;
                opacity = innerSpace->getFloat("opacity");
                style->setOpacity(opacity);
            }

            if (innerSpace->exists("color"))
            {
                Vector4 color;
                innerSpace->getColor("color", &color);
                style->setColor(color, Style::OVERLAY_MAX);
            }

            if (innerSpace->exists("normalColor"))
            {
                Vector4 normalColor;
                innerSpace->getColor("normalColor", &normalColor);
                style->setColor(normalColor, Style::OVERLAY_NORMAL);
            }

            if (innerSpace->exists("hoverColor"))
            {
                Vector4 hoverColor;
                innerSpace->getColor("hoverColor", &hoverColor);
                style->setColor(hoverColor, Style::OVERLAY_HOVER);
            }

            if (innerSpace->exists("activeColor"))
            {
                Vector4 activeColor;
                innerSpace->getColor("activeColor", &activeColor);
                style->setColor(activeColor, Style::OVERLAY_ACTIVE);
            }

            if (innerSpace->exists("focusColor"))
            {
                Vector4 focusColor;
                innerSpace->getColor("focusColor", &focusColor);
                style->setColor(focusColor, Style::OVERLAY_FOCUS);
            }

            if (innerSpace->exists("disabledColor"))
            {
                Vector4 disabledColor;
                innerSpace->getColor("disabledColor", &disabledColor);
                style->setColor(disabledColor, Style::OVERLAY_DISABLED);
            }


            // background
            innerSpace = space->getNextNamespace();
            while (innerSpace != NULL)
            {
                const char* innerSpacename = innerSpace->getNamespace();
                if (strcmpnocase(innerSpacename, "background") == 0)
                {
                    std::string imagePath;
                    innerSpace->getPath("image", &imagePath);
                    if (imagePath.size() == 0) {
                        imagePath = "empty.png";
                    }
                    ThemeImage* image = theme->getImageFullName(imagePath.c_str());
                    if (!image) {
                        GP_ERROR("image not found:%d", imagePath.c_str());
                        break;
                    }

                    Border border;
                    Properties* borderSpace = innerSpace->getNextNamespace();
                    while (borderSpace != NULL) {
                        const char* innerSpacename = borderSpace->getNamespace();
                        if (strcmpnocase(innerSpacename, "border") == 0)
                        {
                            border.top = borderSpace->getFloat("top");
                            border.bottom = borderSpace->getFloat("bottom");
                            border.left = borderSpace->getFloat("left");
                            border.right = borderSpace->getFloat("right");
                            break;
                        }
                        borderSpace = innerSpace->getNextNamespace();
                    }

                    BorderImage* bg = new BorderImage(image->getRegion(), border);
                    style->setBgImage(bg);
                    bg->release();

                    // Done with this pass.
                    break;
                }

                innerSpace = space->getNextNamespace();
            }

            theme->setStyle(style->getId(), style);
            style->release();

            if (strcmpnocase(style->getId(), "Default") == 0) {
                theme->setStyle("EMPTY_STYLE", style);
            }
        }

        space = themeProperties->getNextNamespace();
    }
    

    // Add this theme to the cache.
    __themeCache.push_back(theme.get());

    //SAFE_DELETE(properties);

    return SPtr<Theme>(theme);
}

Style* Theme::getStyle(const char* name) const
{
    auto it = _styles.find(name);
    if (it == _styles.end()) return NULL;
    return it->second;
}

void Theme::setStyle(const char* id, Style* style) {
    _styles[id] = style;
    style->addRef();
}

Style* Theme::getEmptyStyle()
{
    Style* emptyStyle = getStyle("EMPTY_STYLE");

    if (!emptyStyle)
    {
        SPtr<Theme> theme; theme = this;
        emptyStyle = new Style(theme, "EMPTY_STYLE");
        //ThemeImage* image = getImage("empty");
        //BorderImage* bg = new BorderImage(image->getRegion(), Border(0,0,0,0));
        //emptyStyle->setBgImage(bg);
        _styles["EMPTY_STYLE"] = emptyStyle;
    }

    return emptyStyle;
}

// void Theme::setProjectionMatrix(const Matrix& matrix)
// {
//     GP_ASSERT(_spriteBatch);
//     _spriteBatch->setProjectionMatrix(matrix);
// }
SpriteBatch* Theme::getSpriteBatch() const
{
    return _spriteBatch;
}

ThemeImage* Theme::getImageFullName(const char* file) {
    auto it = _images.find(file);
    if (it != _images.end()) {
        return it->second;
    }

    Rectangle rect;
    bool rc = _texture->addImageUri(file, rect);
    GP_ASSERT(rc);
    ThemeImage* image = new ThemeImage(rect);
    _images[file] = image;
    return image;
}

ThemeImage* Theme::getImage(const char* id) {
    std::string file = "res/ui/";
    file += id;
    file += ".png";
    return getImageFullName(file.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


BorderImage::BorderImage(const Rectangle& region, const Border& border) : 
    _region(region), _border(border) {
    setRegion(region);
}

const Vector4 BorderImage::getUVs(SkinArea area, float tw, float th) const
{
    const Vector4& r = _uvs[area];
    return Vector4(r.x*tw, r.y*th, (r.z)*tw, (r.w)*th);
}

void BorderImage::setRegion(const Rectangle& region)
{
    // Can calculate all measurements in advance.
    float leftEdge = region.x;
    float rightEdge = (region.x + region.width);
    float leftBorder = (region.x + _border.left);
    float rightBorder = (region.x + region.width - _border.right);

    float topEdge = (region.y);
    float bottomEdge = ((region.y + region.height));
    float topBorder = ((region.y + _border.top));
    float bottomBorder = ((region.y + region.height - _border.bottom));

    // There are 9 sets of UVs to set.
    _uvs[TOP_LEFT].x = leftEdge;
    _uvs[TOP_LEFT].y = topEdge;
    _uvs[TOP_LEFT].z = leftBorder;
    _uvs[TOP_LEFT].w = topBorder;

    _uvs[TOP].x = leftBorder;
    _uvs[TOP].y = topEdge;
    _uvs[TOP].z = rightBorder;
    _uvs[TOP].w = topBorder;

    _uvs[TOP_RIGHT].x = rightBorder;
    _uvs[TOP_RIGHT].y = topEdge;
    _uvs[TOP_RIGHT].z = rightEdge;
    _uvs[TOP_RIGHT].w = topBorder;

    _uvs[LEFT].x = leftEdge;
    _uvs[LEFT].y = topBorder;
    _uvs[LEFT].z = leftBorder;
    _uvs[LEFT].w = bottomBorder;

    _uvs[CENTER].x = leftBorder;
    _uvs[CENTER].y = topBorder;
    _uvs[CENTER].z = rightBorder;
    _uvs[CENTER].w = bottomBorder;

    _uvs[RIGHT].x = rightBorder;
    _uvs[RIGHT].y = topBorder;
    _uvs[RIGHT].z = rightEdge;
    _uvs[RIGHT].w = bottomBorder;

    _uvs[BOTTOM_LEFT].x = leftEdge;
    _uvs[BOTTOM_LEFT].y = bottomBorder;
    _uvs[BOTTOM_LEFT].z = leftBorder;
    _uvs[BOTTOM_LEFT].w = bottomEdge;

    _uvs[BOTTOM].x = leftBorder;
    _uvs[BOTTOM].y = bottomBorder;
    _uvs[BOTTOM].z = rightBorder;
    _uvs[BOTTOM].w = bottomEdge;

    _uvs[BOTTOM_RIGHT].x = rightBorder;
    _uvs[BOTTOM_RIGHT].y = bottomBorder;
    _uvs[BOTTOM_RIGHT].z = rightEdge;
    _uvs[BOTTOM_RIGHT].w = bottomEdge;
}

unsigned int BorderImage::draw(SpriteBatch* batch, const Rectangle& _absoluteBounds, const Vector4& skinColor, const Rectangle& clip, const SideRegions& padding) {
    unsigned int drawCalls = 0;
    BorderImage* _skin = this;

    float tw = 1.0 / batch->getSampler()->getWidth();
    float th = 1.0 / batch->getSampler()->getHeight();

    // Get the border and background images for this control's current state.
    const Vector4& topLeft = _skin->getUVs(BorderImage::TOP_LEFT, tw, th);
    const Vector4& top = _skin->getUVs(BorderImage::TOP, tw, th);
    const Vector4& topRight = _skin->getUVs(BorderImage::TOP_RIGHT, tw, th);
    const Vector4& left = _skin->getUVs(BorderImage::LEFT, tw, th);
    const Vector4& center = _skin->getUVs(BorderImage::CENTER, tw, th);
    const Vector4& right = _skin->getUVs(BorderImage::RIGHT, tw, th);
    const Vector4& bottomLeft = _skin->getUVs(BorderImage::BOTTOM_LEFT, tw, th);
    const Vector4& bottom = _skin->getUVs(BorderImage::BOTTOM, tw, th);
    const Vector4& bottomRight = _skin->getUVs(BorderImage::BOTTOM_RIGHT, tw, th);

    // Calculate screen-space positions.
    Border border = _skin->getBorder();
    float scale = 1 / Toolkit::cur()->getScreenScale();
    border.bottom *= scale;
    border.top *= scale;
    border.left *= scale;
    border.right *= scale;
    //Vector4 skinColor = getStyle()->getColor((Style::OverlayType)getState());
    //skinColor.w *= getStyle()->getOpacity();

    float midWidth = _absoluteBounds.width - border.left - border.right;
    float midHeight = _absoluteBounds.height - border.top - border.bottom;
    float midX = _absoluteBounds.x + border.left;
    float midY = _absoluteBounds.y + border.top;
    float rightX = _absoluteBounds.x + _absoluteBounds.width - border.right;
    float bottomY = _absoluteBounds.y + _absoluteBounds.height - border.bottom;

    // Draw themed border sprites.
    if (!border.left && !border.right && !border.top && !border.bottom)
    {
        // No border, just draw the image.
        batch->draw(_absoluteBounds.x, _absoluteBounds.y, _absoluteBounds.width, _absoluteBounds.height, center.x, center.y, center.z, center.w, skinColor, &clip);
        ++drawCalls;
    }
    else
    {
        if (border.left && border.top)
        {
            batch->draw(_absoluteBounds.x, _absoluteBounds.y, border.left, border.top, topLeft.x, topLeft.y, topLeft.z, topLeft.w, skinColor, &clip);
            ++drawCalls;
        }
        if (border.top)
        {
            batch->draw(_absoluteBounds.x + border.left, _absoluteBounds.y, midWidth, border.top, top.x, top.y, top.z, top.w, skinColor, &clip);
            ++drawCalls;
        }
        if (border.right && border.top)
        {
            batch->draw(rightX, _absoluteBounds.y, border.right, border.top, topRight.x, topRight.y, topRight.z, topRight.w, skinColor, &clip);
            ++drawCalls;
        }
        if (border.left)
        {
            batch->draw(_absoluteBounds.x, midY, border.left, midHeight, left.x, left.y, left.z, left.w, skinColor, &clip);
            ++drawCalls;
        }

        // Always draw the background.
        batch->draw(_absoluteBounds.x + border.left, _absoluteBounds.y + border.top, _absoluteBounds.width - border.left - border.right, _absoluteBounds.height - border.top - border.bottom,
            center.x, center.y, center.z, center.w, skinColor, &clip);
        ++drawCalls;

        if (border.right)
        {
            batch->draw(rightX, midY, border.right, midHeight, right.x, right.y, right.z, right.w, skinColor, &clip);
            ++drawCalls;
        }
        if (border.bottom && border.left)
        {
            batch->draw(_absoluteBounds.x, bottomY, border.left, border.bottom, bottomLeft.x, bottomLeft.y, bottomLeft.z, bottomLeft.w, skinColor, &clip);
            ++drawCalls;
        }
        if (border.bottom)
        {
            batch->draw(midX, bottomY, midWidth, border.bottom, bottom.x, bottom.y, bottom.z, bottom.w, skinColor, &clip);
            ++drawCalls;
        }
        if (border.bottom && border.right)
        {
            batch->draw(rightX, bottomY, border.right, border.bottom, bottomRight.x, bottomRight.y, bottomRight.z, bottomRight.w, skinColor, &clip);
            ++drawCalls;
        }
    }

    return drawCalls;
}


}

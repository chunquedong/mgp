#include "ThemeStyle.h"

namespace mgp
{

Style::Style(SPtr<Theme> theme, const char* id)
    : _theme(theme), _id(id), _background(NULL), _font(NULL),
    _fontSize(16), _alignment(FontLayout::ALIGN_TOP_LEFT), _textRightToLeft(false), _textColor(Vector4::one()), _opacity(1.0f)
{
    for (int i=0; i<OVERLAY_MAX; ++i) {
        _bgColors[i].x = 1;
        _bgColors[i].y = 1;
        _bgColors[i].z = 1;
        _bgColors[i].w = 1;
    }
}

Style::~Style()
{
    SAFE_RELEASE(_background);
    SAFE_RELEASE(_font);
}

Theme* Style::getTheme() const
{
    return _theme.get();
}

const char* Style::getId() const
{
    return _id.c_str();
}

Style::Style(const Style& copy) : _background(NULL), _font(NULL), _theme(NULL)
{
    if (copy._background)
    {
        _background = new BorderImage(*copy._background);
    }

    _theme = copy._theme;

    _font = copy._font;
    _fontSize = copy._fontSize;
    _alignment = copy._alignment;
    _textRightToLeft = copy._textRightToLeft;
    _textColor = Vector4(copy._textColor);
    _opacity = copy._opacity;

    for (int i = 0; i < OVERLAY_MAX; ++i) {
        _bgColors[i] = copy._bgColors[i];
    }

    if (_font)
    {
        _font->addRef();
    }
    if (_background) {
        _background->addRef();
    }
}

float Style::getOpacity() const
{
    return _opacity;
}

void Style::setOpacity(float opacity)
{
    _opacity = opacity;
}

//void Style::setBorder(float top, float bottom, float left, float right)
//{
//    if (_background)
//    {
//        _background->_border.top = top;
//        _background->_border.bottom = bottom;
//        _background->_border.left = left;
//        _background->_border.right = right;
//    }
//}
//
//const Border& Style::getBorder() const
//{
//    if (_background)
//    {
//        return _background->getBorder();
//    }
//    else
//    {
//        return Border::empty();
//    }
//}

void Style::setBgColor(const Vector4& color, OverlayType state)
{
    if (state == OVERLAY_MAX) {
        for (int i = 0; i < OVERLAY_MAX; ++i) {
            _bgColors[i] = color;
        }
    }
    else {
        _bgColors[state] = color;
    }
}

const Vector4& Style::getBgColor(OverlayType state) const
{
    return _bgColors[state];
}

Font* Style::getFont() const
{
    return _font;
}

void Style::setFont(Font* font)
{
    if (_font != font)
    {
        SAFE_RELEASE(_font);

        _font = font;

        if (_font)
        {
            _font->addRef();
        }
    }
}

unsigned int Style::getFontSize() const
{
    return _fontSize;
}

void Style::setFontSize(unsigned int fontSize)
{
    _fontSize = fontSize;
}

FontLayout::Justify Style::getTextAlignment() const
{
    return _alignment;
}

void Style::setTextAlignment(FontLayout::Justify alignment)
{
    _alignment = alignment;
}

bool Style::getTextRightToLeft() const
{
    return _textRightToLeft;
}

void Style::setTextRightToLeft(bool rightToLeft)
{
    _textRightToLeft = rightToLeft;
}

const Vector4& Style::getTextColor() const
{
    return _textColor;
}

void Style::setTextColor(const Vector4& color)
{
    _textColor = color;
}

void Style::setBgImage(BorderImage* skin)
{
    if (_background != skin)
    {
        SAFE_RELEASE(_background);
        _background = skin;

        if (_background)
        {
            _background->addRef();
        }
    }
}

BorderImage* Style::getBgImage() const
{
    return _background;
}

// Implementation of AnimationHandler
unsigned int Style::getAnimationPropertyComponentCount(int propertyId) const
{
    switch(propertyId)
    {
    case Style::ANIMATE_OPACITY:
        return 1;
    default:
        return -1;
    }
}

void Style::getAnimationPropertyValue(int propertyId, AnimationValue* value)
{
    GP_ASSERT(value);

    switch(propertyId)
    {
    case ANIMATE_OPACITY:
        value->setFloat(0, _opacity);
        break;
    default:
        break;
    }
}

void Style::setAnimationPropertyValue(int propertyId, AnimationValue* value, float blendWeight)
{
    GP_ASSERT(value);

    switch(propertyId)
    {
        case ANIMATE_OPACITY:
        {
            _opacity = Curve::lerp(blendWeight, _opacity, value->getFloat(0));
            break;
        }
        default:
            break;
    }
}


}
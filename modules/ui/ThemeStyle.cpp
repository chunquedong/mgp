#include "ThemeStyle.h"

namespace mgp
{

Style::Style(SPtr<Theme> theme, const char* id)
    : _theme(theme), _id(id), _background(NULL), _font(NULL), _image(nullptr), _color(Vector4::one()), _bgColor(Vector4::one()),
    _fontSize(16), _alignment(FontLayout::ALIGN_TOP_LEFT), _textRightToLeft(false), _textColor(Vector4::one()), _opacity(1.0f)
{
}

Style::~Style()
{
    _stateStyles.clear();
    SAFE_RELEASE(_background);
    SAFE_RELEASE(_image);
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

void Style::setId(const char* id)
{
    _id = id;
}

Style::Style(const Style& copy) : _background(NULL), _font(NULL), _theme(NULL), _image(nullptr)
{
    if (copy._background)
    {
        _background = copy._background->clone();
    }

    if (copy._image)
    {
        _image = new ThemeImage(copy._image->getRegion());
    }

    _theme = copy._theme;

    _font = copy._font;
    _fontSize = copy._fontSize;
    _alignment = copy._alignment;
    _textRightToLeft = copy._textRightToLeft;
    _textColor = Vector4(copy._textColor);
    _opacity = copy._opacity;

    _color = copy._color;
    _bgColor = copy._bgColor;

    if (_font)
    {
        _font->addRef();
    }
}

Style* Style::getStateStyle(OverlayType state)
{
    if (_stateStyles.size() == 0) {
        return this;
    }
    Style* s = _stateStyles[state].get();
    if (!s) {
        return this;
    }
    return s;
}

void Style::setStateStyle(UPtr<Style> style, OverlayType state)
{
    if (_stateStyles.size() == 0) {
        _stateStyles.resize(OVERLAY_MAX);
    }
    _stateStyles[state] = std::move(style);
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

void Style::setColor(const Vector4& color)
{
    _color = color;
}

const Vector4& Style::getColor() const
{
    return _color;
}

void Style::setBgColor(const Vector4& color) {
    _bgColor = color;
}
const Vector4& Style::getBgColor() const {
    return _bgColor;
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

void Style::setImage(ThemeImage* image) {
    if (_image != image)
    {
        SAFE_RELEASE(_image);
        _image = image;

        if (_image)
        {
            _image->addRef();
        }
    }
}
ThemeImage* Style::getImage() const {
    return _image;
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
#ifndef THEMESTYLE_H_
#define THEMESTYLE_H_

#include "base/Base.h"
#include "base/Ref.h"
//#include "objects/Text.h"
#include "math/Rectangle.h"
#include "material/Texture.h"
#include "base/Properties.h"
#include "Theme.h"

namespace mgp
{



/**
 * Defines the style of a control.  
 *
 * A style can have padding and margin values,
 * as well as overlays for each of the control's states.  
 * Each overlay in turn can reference other theme classes to determine
 * the border, background, cursor, and image settings to use for
 * a particular state, as well as color and font settings, etcetera.
 */
class Style : public Refable, public AnimationTarget
{
    friend class Theme;
    friend class Control;
    friend class Container;
    friend class Form;

public:

    /**
     * Get the theme this style belongs to.
     *
     * @return The theme this style belongs to.
     */
    Theme* getTheme() const;

    /**
     * A style has one overlay for each possible control state.
     */
    enum OverlayType
    {
        OVERLAY_NORMAL,
        OVERLAY_FOCUS,
        OVERLAY_ACTIVE,
        OVERLAY_DISABLED,
        OVERLAY_HOVER,
        OVERLAY_MAX
    };

    static const int ANIMATE_OPACITY = 1;

    /**
     * Get the opacity of this control for a given state. 
     *
     * @param state The state to get this property from.
     *
     * @return The opacity of this control for a given state.
     */
    float getOpacity() const;

    /**
     * Set the opacity of this control.
     *
     * @param opacity The new opacity.
     * @param states The states to set this property on. One or more members of the Control::State enum, OR'ed together.
     */
    void setOpacity(float opacity);

    /**
     * Set the blend color of this control's skin.
     *
     * @param color The new blend color.
     * @param states The states to set this property on.
     *               One or more members of the Control::State enum, ORed together.
     */
    void setBgColor(const Vector4& color, OverlayType state=OverlayType::OVERLAY_NORMAL);

    /**
     * Get the blend color of this control's skin for a given state.
     *
     * @param state The state to get this property from.
     *
     * @return The blend color of this control's skin.
     */
    const Vector4& getBgColor(OverlayType state=OverlayType::OVERLAY_NORMAL) const;

    /**
     * Get the font used by this control for a given state.
     *
     * @param state The state to get this property from.
     *
     * @return the font used by this control.
     */
    Font* getFont() const;
    
    /**
     * Set the font used by this control.
     *
     * @param font The new font to use.
     * @param states The states to set this property on.
     *               One or more members of the Control::State enum, ORed together.
     */
    void setFont(Font* font);

    /**
     * Get this control's font size for a given state.
     *
     * @param state The state to get this property from.
     *
     * @return This control's font size.
     */
    unsigned int getFontSize() const;

    /**
     * Set this control's font size.
     *
     * @param size The new font size.
     * @param states The states to set this property on.
     *               One or more members of the Control::State enum, ORed together.
     */
    void setFontSize(unsigned int fontSize);

    /**
     * Get this control's text alignment for a given state.
     *
     * @param state The state to get this property from.
     *
     * @return This control's text alignment for the given state.
     */
    FontLayout::Justify getTextAlignment() const;

    /**
     * Set this control's text alignment.
     *
     * @param alignment The new text alignment.
     * @param states The states to set this property on.
     *               One or more members of the Control::State enum, ORed together.
     */
    void setTextAlignment(FontLayout::Justify alignment);
        
    bool getTextRightToLeft() const;

    void setTextRightToLeft(bool rightToLeft);

    /**
     * Get this control's text color for a given state.
     *
     * @param state The state to get this property from.
     *
     * @return This control's text color.
     */
    const Vector4& getTextColor() const;

    /**
     * Set this control's text color.
     *
     * @param color The new text color.
     * @param states The states to set this property on.
     *               One or more members of the Control::State enum, ORed together.
     */
    void setTextColor(const Vector4& color);

    /**
    * @see AnimationTarget::getAnimationPropertyComponentCount
    */
    unsigned int getAnimationPropertyComponentCount(int propertyId) const;

    /**
    * @see AnimationTarget::getAnimationProperty
    */
    void getAnimationPropertyValue(int propertyId, AnimationValue* value);

    /**
    * @see AnimationTarget::setAnimationProperty
    */
    void setAnimationPropertyValue(int propertyId, AnimationValue* value, float blendWeight = 1.0f);
    
    void setBgImage(BorderImage* BorderImage);
    BorderImage* getBgImage() const;

    /**
     * Constructor.
     */
    Style(Theme* theme, const char* id);

    /**
     * Constructor.
     */
    Style(const Style& style);

    /**
     * Destructor.
     */
    ~Style();

    /**
     * Hidden copy assignment operator.
     */
    Style& operator=(const Style&);

    /**
     * Returns the Id of this Style.
     */
    const char* getId() const;
    
private:
    Theme* _theme;
    std::string _id;
    BorderImage* _background;
    Vector4 _bgColors[OVERLAY_MAX];

    //font
    Font* _font;
    unsigned int _fontSize;
    FontLayout::Justify _alignment;
    bool _textRightToLeft;
    Vector4 _textColor;
    float _opacity;
};

}

#endif
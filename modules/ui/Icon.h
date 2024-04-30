#ifndef ICON_H_
#define ICON_H_

#include "Control.h"
#include "Theme.h"
#include "material/Image.h"
#include "objects/SpriteBatch.h"
#include "math/Rectangle.h"

namespace mgp
{

/**
 * Defines an image control.
 *
 * This allows forms to display seperate images from arbitrary files not specified in the theme.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class Icon : public Control
{
    friend class Control;

    bool _checked = false;
    bool _checkable = false;
public:

    /**
     * Set the path of the image for this ImageControl to display.
     *
     * @param path The path to the image.
     */
    void setImagePath(const char* path);

    const char* getImagePath();

    bool isCheckable() { return _checkable; }
    void setCheckable(bool c) { _checkable = c; }

    /**
     * Gets whether this checkbox is checked.
     *
     * @return Whether this checkbox is checked.
     */
    bool isChecked();

    /**
     * Sets whether the checkbox is checked.
     *
     * @param checked TRUE if the checkbox is checked; FALSE if the checkbox is not checked.
     */
    void setChecked(bool checked);

protected:

    Icon();

    virtual ~Icon();

    virtual void onSerialize(Serializer* serializer);

    virtual void onDeserialize(Serializer* serializer);

    virtual unsigned int drawImages(Form* form, const Rectangle& clip, RenderInfo* view);

    void measureSize();

    void controlEvent(Control::Listener::EventType evt);

    virtual unsigned int drawBorder(Form* form, const Rectangle& clip, RenderInfo* view);
protected:
    std::string _imagePath;
    ThemeImage* _image = NULL;
};

class LoadingView : public Icon {
    friend class Control;
    float _progress = 0;
protected:
    LoadingView();
    void updateState(State state);
    unsigned int drawImages(Form* form, const Rectangle& clip, RenderInfo* view) override;
};

}

#endif
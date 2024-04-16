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

public:

    /**
     * Set the path of the image for this ImageControl to display.
     *
     * @param path The path to the image.
     */
    void setImagePath(const char* path);

    const char* getImagePath();

protected:

    Icon();

    virtual ~Icon();

    virtual void onSerialize(Serializer* serializer);

    virtual void onDeserialize(Serializer* serializer);

    virtual unsigned int drawImages(Form* form, const Rectangle& clip, RenderInfo* view);

    void measureSize();
protected:
    std::string _imagePath;
    ThemeImage* _image = NULL;
};

class LoadingView : public Icon {
    friend class Control;
    float _progress = 0;
protected:
    void updateState(State state);
    unsigned int drawImages(Form* form, const Rectangle& clip, RenderInfo* view) override;
};

}

#endif
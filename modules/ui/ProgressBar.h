#ifndef PROGRESS_BAR_H_
#define PROGRESS_BAR_H_

#include "base/Base.h"
#include "Theme.h"
#include "base/Properties.h"
#include "Button.h"

namespace mgp
{


class ProgressBar : public Control
{
    friend class Control;

public:

    /**
     * Set this slider's value.  The new value will be clamped to fit within
     * the slider's minimum and maximum values.
     *
     * @param value The new value.
     */
    void setValue(float value, bool fireEvent = true);

    /**
     * Get this slider's current value.
     *
     * @return This slider's current value.
     */
    float getValue() const;

protected:

    /**
     * Constructor.
     */
    ProgressBar();

    /**
     * Destructor.
     */
    ~ProgressBar();


    virtual void onSerialize(Serializer* serializer);

    virtual void onDeserialize(Serializer* serializer);

    /**
     * @see Control::drawImages
     */
    unsigned int drawImages(Form* form, const Rectangle& clip, RenderInfo* view);

    /**
     * @see Control::updateState
     */
    void updateState(State state);

    /**
     * @see Control::updateBounds
     */
    void measureSize();
    
    /**
     * The Slider's current value.
     */
    float _value;
    
    /**
     * The image for the slider track.
     */
    ThemeImage* _trackImage;

private:

    /**
     * Constructor.
     */
    ProgressBar(const ProgressBar& copy) = delete;


    float _trackHeight;
};

}

#endif

#ifndef FORM_H_
#define FORM_H_

#include "base/Ref.h"
#include "Container.h"
#include "scene/Mesh.h"
#include "scene/Node.h"
#include "platform/Keyboard.h"
#include "platform/Mouse.h"
//#include "platform/Gamepad.h"
#include "render/FrameBuffer.h"
#include "scene/Drawable.h"

namespace mgp
{

class Theme;
class FormManager;

/**
 * Defines a form that is a root container that contains zero or more controls.
 *
 * This can also be attached on a scene Node to support 3D forms.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class Form : public Drawable
{
    friend class Control;
    friend class Container;
    friend class FormManager;

public:

    /**
     * Creates a form from a .form properties file.
     * 
     * @param url The URL pointing to the Properties object defining the form. 
     * 
     * @return The new form or NULL if there was an error.
     * @script{create}
     */
    static UPtr<Form> create(const char* url);

    /**
     * Create a new Form.
	 *
	 * The specified style defines the visual style for the form. If NULL is passed
	 * for the style, the default UI theme is used. All controls attached to this
	 * form will inherit the theme that contains the form's style.
     *
     * @param id The Form's ID.
     * @param style The Form's custom style (optional - may be NULL).
	 * @param layoutType The form's layout type (optional).
     *
     * @return The new Form.
     * @script{create}
     */
    static UPtr<Form> create();


    /**
     * Returns the currently active control within the UI system.
     *
     * An active control is a control that is currently pressed or hovered over. On a multi-touch
     * system, it is possible for several controls to be active at once (one for each touch point).
     * However, only a single control can have focus at once.
     *
     * @param touchIndex Optional touch point index to retrieve the active control for.
     *
     * @return The currently active control, or NULL if no controls are currently active.
     */
    Control* getActiveControl(unsigned int touchIndex = 0);

    /**
     * Returns the current control that is in focus.
     *
     * @return The current control in focus, or NULL if no controls are in focus.
     */
    Control* getFocusControl();

    /**
     * Removes focus from any currently focused UI control.
     */
    void clearFocus();


public:
    /**
     * @see Control::update
     */
    void update(float elapsedTime);

    /**
     * Draws this form.
     *
     * @return The nubmer of draw calls issued to draw the form.
     */
    unsigned int draw(RenderInfo* view);

    /**
     * Determines whether batching is enabled for this form.
     *
     * @return True if batching is enabled for this form, false otherwise.
     */
    bool isBatchingEnabled() const;

    /**
     * Turns batching on or off for this form.
     *
     * By default, forms enable batching as a way to optimize performance. However, on certain
     * complex forms that contain multiple layers of overlapping text and transparent controls,
     * batching may cause some visual artifacts due alpha blending issues. In these cases,
     * turning batching off usually fixes the issue at a slight performance cost.
     *
     * @param enabled True to enable batching (default), false otherwise.
     */
    void setBatchingEnabled(bool enabled);


    Container* getRoot();

    bool screenToForm(int* x, int* y);

private:
    
    /**
     * Constructor.
     */
    Form();

    /**
     * Constructor.
     */
    Form(const Form& copy);

    /**
     * Destructor.
     */
    virtual ~Form();

    /**
     * @see Drawable::clone
     */
    UPtr<Drawable> clone(NodeCloneContext &context);

    /**
     * @see Control::initialize
     */
    void initialize();

    /**
     * Called during drawing to prepare a sprite batch for being drawn into for this form.
     */
    void startBatch(BatchableLayer* batch);

    /**
     * Called during drawing to signal completion of drawing into a batch.
     */
    void finishBatch(BatchableLayer* batch, RenderInfo* view);

    int flushBatch(RenderInfo* view);

    /**
     * Unproject a point (from a mouse or touch event) into the scene and then project it onto the form.
     *
     * @param x The x coordinate of the mouse/touch point.
     * @param y The y coordinate of the mouse/touch point.
     * @param point A destination vector to populate with the projected point, in the form's plane.
     *
     * @return True if the projected point lies within the form's plane, false otherwise.
     */
    bool projectPoint(int x, int y, Vector3* point);

    const Matrix& getProjectionMatrix() const;



    void verifyRemovedControlState(Control* control);

    void controlDisabled(Control* control);

    void setFocusControl(Control* control);

    Control* findInputControl(int* x, int* y, bool focus, unsigned int contactIndex, bool* consumed);
    Control* handlePointerPressRelease(int* x, int* y, bool pressed, unsigned int contactIndex, bool* consumed);
    bool pointerEventInternal(bool mouse, int evt, int x, int y, int wheelDelta, unsigned int contactIndex, int button);
    Control* handlePointerMove(int* x, int* y, unsigned int contactIndex, bool* consumed);
    bool keyEventInternal(Keyboard::KeyEvent evt, int key);
private:
    Matrix _projectionMatrix;           // Projection matrix to be set on SpriteBatch objects when rendering the form
    std::vector<BatchableLayer*> _batches;
    UPtr<Container> _root;
    bool _batched;

    Control* __focusControl = NULL;
    Control* __activeControl[MotionEvent::MAX_TOUCH_POINTS];
    bool __shiftKeyDown = false;
};


class FormManager {
    std::vector<Form*> __forms;
    Form* _focusForm = NULL;
    friend class Control;
    friend class Form;
private:
    /**
     * Get a form from its ID.
     *
     * @param id The ID of the form to search for.
     *
     * @return A form with the given ID, or null if one was not found.
     */
    //static Form* getForm(const char* id);
    
public:
    FormManager();
    ~FormManager();
    static FormManager* cur();

    std::vector<Form*>& getForms() { return __forms; }

    void add(UPtr<Form> f);

    void finalize();

    /**
     * Updates all visible, enabled forms.
     */
    void updateInternal(float elapsedTime);

    /**
     * Propagate key events to enabled forms.
     *
     * @return Whether the key event was consumed by a form.
     */
    bool keyEventInternal(Keyboard::KeyEvent evt, int key);

    /**
     * Propagate mouse events to enabled forms.
     *
     * @return True if the mouse event is consumed or false if it is not consumed.
     *
     * @see MotionEvent::MotionType
     */
    bool mouseEventInternal(MotionEvent& evt);

    /**
     * Fired by the platform when the game window resizes.
     *
     * @param width The new window width.
     * @param height The new window height.
     */
    void resizeEventInternal(unsigned int width, unsigned int height);


    void verifyRemovedControlState(Control* control);
};

}

#endif

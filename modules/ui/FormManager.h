#ifndef FORM_MANAGER_H_
#define FORM_MANAGER_H_

#include "Form.h"

namespace mgp
{
class SerializerManager;

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

    void remove(Form* form);

    void finalize();

    /**
     * Draws this form.
     *
     * @return The nubmer of draw calls issued to draw the form.
     */
    unsigned int draw(RenderInfo* view);

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

    static void regiseterSerializer(SerializerManager *mgr);
};

}

#endif

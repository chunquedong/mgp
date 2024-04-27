#ifndef CONTAINER_H_
#define CONTAINER_H_

#include "Control.h"
#include "Layout.h"
#include "platform/Toolkit.h"

namespace mgp
{
/**
 * Defines a container that contains zero or more controls.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class Container : public Control
{
    friend class Form;
    friend class Control;
    //friend class ControlFactory;

public:

    /**
     * Get this container's layout.
     *
     * @return This container's layout object.
     */
    Layout* getLayout();

	/**
	 * Sets the layout type for this container.
	 *
	 * @param type The new layout type for the container.
	 */
	void setLayout(Layout::Type type);

    /**
     * Adds a new control to this container.
     *
	 * @param control The control to add.
     *
     * @return The index assigned to the new Control.
     */
    unsigned int addControl(UPtr<Control> control);

    /**
     * Inserts a control at a specific index.
     *
     * @param control The control to insert.
     * @param index The index at which to insert the control.
     */
    void insertControl(UPtr<Control> control, unsigned int index);

    /**
     * Remove a control at a specific index.
     *
     * @param index The index from which to remove the control.
     */
    void removeControl(unsigned int index);

    /**
     * Remove a control with the given ID.
     *
     * @param id The ID of the control to remove.
     */
    void removeControl(const char* id);

    /**
     * Remove a specific control.
     *
     * @param control The control to remove.
     */
    void removeControl(Control* control);


    void removeSelf();

    /**
    * Remove all contrls
    */
    void clear();

    /**
     * Get the Control at a specific index.
     *
     * @param index The index at which to retrieve the Control.
     *
     * @return The Control at the given index.
     */
    Control* getControl(unsigned int index) const;

    /**
     * Get a Control with a specific ID that belongs to this Layout.
     *
     * @param id The ID of the Control to search for.
     */
    Control* findControl(const char* id);

    /**
     * Returns the number of child controls for this container.
     *
     * @return The number of child controls.
     */
    unsigned int getControlCount() const;

    /**
     * Get the vector of controls within this container.
     *
     * @return The vector of the controls within this container.
     * @script{ignore}
     */
    const std::vector<Control*>& getControls() const;

    /**
     * Determines if this container is a top level form.
     *
     * @return True if the container is a top level form, false otherwise.
     */
    virtual bool isRoot() const;

    /**
     * @see AnimationTarget::getAnimation
     */
    Animation* getAnimation(const char* id = NULL) const;

    /**
     * @see Control::setFocus
     */
    bool setFocus();

    /**
     * Attempts to switch focus to a child of this container in the specified direction.
     *
     * @param direction The direction for focus change.
     *
     * @return True on success, false if there are no controls to focus on.
     */
    bool moveFocus(Direction direction);

    /**
     * Returns the currently active control for this container.
     *
     * @return This container's active control.
     */
    Control* getActiveControl() const;
    
    /**
     * Sets the active control for this container.
     *
     * A container's active control is the control that will receive focus
     * when the container receives focus.
     *
     * @param control The container's new active control (must be a child of this container).
     */
    void setActiveControl(Control* control);

    // /**
    //  * @see AnimationTarget::getAnimationPropertyComponentCount
    //  */
    // virtual unsigned int getAnimationPropertyComponentCount(int propertyId) const;

    // /**
    //  * @see AnimationTarget::getAnimationProperty
    //  */
    // virtual void getAnimationPropertyValue(int propertyId, AnimationValue* value);

    // /**
    //  * @see AnimationTarget::setAnimationProperty
    //  */
    // virtual void setAnimationPropertyValue(int propertyId, AnimationValue* value, float blendWeight = 1.0f);


    Control* findInputControl(int x, int y, bool focus, unsigned int contactIndex) override;

    
    bool canReceiveFocus() const override;

    static std::string enumToString(const std::string& enumName, int value);
    static int enumParse(const std::string& enumName, const std::string& str);
protected:
    /**
     * Sets the specified dirty bits for all children within this container.
     *
     * @param bits The bits to set.
     * @param recursive If true, set the bits recursively on all children and their children.
     */
    void setDirty(int bits, bool recursive = false) override;

    /**
     * Constructor.
     */
    Container();

    /**
     * Destructor.
     */
    virtual ~Container();


    virtual void onSerialize(Serializer* serializer);

    virtual void onDeserialize(Serializer* serializer);

    /**
     * @see Control::update
     */
    void update(float elapsedTime) override;

    /**
     * @see Control::updateState
     */
    //void updateState(State state) override;

    /**
     * Returns this control's top level form, or NULL if this control does not belong to a form.
     *
     * @return this control's form.
     */
    Form* getTopLevelForm() const override;

    /**
     * @see Control::updateBounds
     */
    //void updateBounds() override;

    /**
     * @see Control::updateAbsoluteBounds
     */
    //void updateAbsoluteBounds(const Vector2& offset) override;

    //void onBoundsUpdate() override;

    /**
     * Updates the bounds for this container's child controls.
     */
    virtual void updateChildBounds();
    void layoutChildren(bool dirtyBounds) override;

    void measureSize() override;

    /**
     * Gets a Layout::Type enum from a matching string.
     *
     * @param layoutString The layout string to parse
     * @return The parsed layout type.
     */
    static Layout::Type getLayoutType(const char* layoutString);

    /**
     * Creates a layout for the specified layout type.
     *
     * @param type The type of layout to create.
     * @return The new Layout.
     */
    static UPtr<Layout> createLayout(Layout::Type type);


    /**
     * @see Control::draw
     */
    virtual unsigned int draw(Form* form, const Rectangle& clip, RenderInfo* view) override;

    /**
     * Sorts controls by Z-Order (for absolute layouts only).
     * This method is used by controls to notify their parent container when
     * their Z-Index changes.
     */
    void sortControls();

    /**
     * The container's layout.
     */
    UPtr<Layout> _layout;
    /**
     * List of controls within the container.
     */
    std::vector<Control*> _controls;
    /**
     * The active control for the container.
     */
    Control* _activeControl;

    float _leftWidth;
    float _leftHeight;
    
private:

    /**
     * Constructor.
     */
    Container(const Container& copy);

    static const int MAX_CONTACT_INDICES = 10;

	bool moveFocusNextPrevious(Direction direction);
	bool moveFocusDirectional(Direction direction);

    void clearContacts();
    bool inContact();

    int _zIndexDefault;
    bool _contactIndices[MAX_CONTACT_INDICES];
    Form* _form;
};

}

#endif

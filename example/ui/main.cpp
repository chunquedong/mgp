
#include <iostream>
#include "mgp.h"

using namespace mgp;

class MainApp : public Game, Control::Listener {
    Label* _label = NULL;
    //Button* button;

    void initialize() {
        UPtr<Theme> theme = Theme::create("res/ui/default.theme");
        Theme::setDefault(theme.get());
        
        UPtr<Form> form = Form::create();
        form->getRoot()->setSize(600, 600);
        form->getRoot()->setPadding(20, 20, 20, 20);
        form->getRoot()->setLayout(Layout::LAYOUT_VERTICAL);
        
        UPtr<Label> label = Label::create("testLabel");
        //label->setPosition(50, 50);
        //label->setSize(200, 50);
        label->setText("Label");
        _label = label.get();
        _label->addRef();
        form->getRoot()->addControl(label.dynamicCastTo<Control>());

        UPtr<Button> button = Button::create("testButton");
        //button->setPosition(45, 100);
        //button->setSize(200, 100);
        button->setText("Button");
        button->addListener(this, Control::Listener::CLICK);
        form->getRoot()->addControl(button.dynamicCastTo<Control>());

        UPtr<CheckBox> checkbox = CheckBox::create("checkbox");
        //checkbox->setPosition(45, 200);
        checkbox->setText("CheckBox");
        form->getRoot()->addControl(checkbox.dynamicCastTo<Control>());
        //SAFE_RELEASE(checkbox);

        UPtr<RadioButton> radio = RadioButton::create("radio");
        radio->setGroupId("radioGroup");
        radio->setText("RadioButton");
        form->getRoot()->addControl(radio.dynamicCastTo<Control>());
        //SAFE_RELEASE(radio);

        UPtr<RadioButton> radio2 = RadioButton::create("radio");
        radio2->setGroupId("radioGroup");
        radio2->setText("RadioButton");
        form->getRoot()->addControl(radio2.dynamicCastTo<Control>());
        //SAFE_RELEASE(radio2);

        UPtr<Slider> slider = Slider::create("slider");
        slider->setText("Slider");
        slider->setWidth(1.0, true);
        form->getRoot()->addControl(slider.dynamicCastTo<Control>());
        //SAFE_RELEASE(slider);

        UPtr<TextBox> text = TextBox::create("text");
        text->setText("input");
        text->setWidth(300);
        form->getRoot()->addControl(text.dynamicCastTo<Control>());
        //SAFE_RELEASE(text);


        //test scroll
        UPtr<ScrollContainer> container = ScrollContainer::create("container");
        container->setSize(200, 100);
        container->setScroll(ScrollContainer::SCROLL_BOTH);
        UPtr<Button> button2 = Button::create("testButton2");
        button2->setText("test\nButton");
        button2->setSize(200, 100);
        container->addControl(button2.dynamicCastTo<Control>());
        form->getRoot()->addControl(container.dynamicCastTo<Control>());
        //SAFE_RELEASE(button2);
        //SAFE_RELEASE(container);


        UPtr<ImageControl> image = ImageControl::create("image");
        image->setImage("res/image/logo.png");
        image->setSize(50, 50);
        form->getRoot()->addControl(image.dynamicCastTo<Control>());
        //SAFE_RELEASE(image);


        UPtr<JoystickControl> joystick = JoystickControl::create("joystick");
        joystick->setSize(100, 100);
        form->getRoot()->addControl(joystick.dynamicCastTo<Control>());
        //SAFE_RELEASE(joystick);
        
#if 0
        UPtr<Font> font = Font::create("res/ui/sans.ttf");
        set button style
        button->overrideStyle();
        button->getStyle()->setFont(font);
        
        ThemeImage* image = theme->getImage("button");
        BorderImage* border = new BorderImage(image->getRegion(), Border(20,20,20,20));
        button->getStyle()->setBgImage(border);
        border->release();
        button->getStyle()->setBgColor(Vector4(1.0, 0, 0, 1.0), Style::OVERLAY_HOVER);

        font->release();
#endif

        getFormManager()->add(std::move(form));
        //SAFE_RELEASE(theme);
    }

    void render(float elapsedTime) override {
        Renderer::cur()->clear(Renderer::CLEAR_COLOR_DEPTH_STENCIL);
        Game::render(elapsedTime);
    }

    void finalize() {
        SAFE_RELEASE(_label);
        //SAFE_RELEASE(button);
    }

    void controlEvent(Control* control, EventType evt)
    {
        if (evt == CLICK)
        {
            if (strcmp("testButton", control->getId()) == 0)
            {
                _label->setText("clicked");
            }
        }
    }

};

int main() {
    printf("main start\n");
    
    #if __EMSCRIPTEN__
        MainApp* instance = new MainApp();
    #else
        MainApp instance;
    #endif
    return Platform::run();
}
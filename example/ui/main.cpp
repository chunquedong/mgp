
#include <iostream>
#include "mgp.h"

using namespace mgp;

class MainApp : public Game, Control::Listener {
    Label* _label = NULL;
    //Button* button;

    void initialize() {
        auto theme = Theme::create("res/ui/default.theme");
        Theme::setDefault(theme.get());

        UPtr<Form> form = Form::create();
        form->getContent()->setSize(600, 700);
        form->getContent()->setPadding(20, 20, 20, 20);
        form->getContent()->setLayout(Layout::LAYOUT_FLOW);
#if 1
        UPtr<TreeView> tree = Control::create<TreeView>("treeview");
        //tree->setCheckbox(false);
        tree->setWidth(120);
        tree->setHeight(300);
        tree->root->children = {
            TreeView::TreeItem::create(0, "item1", {
                TreeView::TreeItem::create(0, "item11", {}),
                TreeView::TreeItem::create(0, "item12", {}),
                TreeView::TreeItem::create(0, "item13", {}),
            }),
            TreeView::TreeItem::create(0, "item2", {
                TreeView::TreeItem::create(0, "item21", {}),
                TreeView::TreeItem::create(0, "item22", {}),
                TreeView::TreeItem::create(0, "item23", {}),
            }),
            TreeView::TreeItem::create(0, "item3", {
                TreeView::TreeItem::create(0, "item31", {}),
                TreeView::TreeItem::create(0, "item32", {}),
                TreeView::TreeItem::create(0, "item33", {}),
            }),
            TreeView::TreeItem::create(0, "item4", {}),
        };
        form->getContent()->addControl(std::move(tree));
#endif
#if 1
        UPtr<ComboBox> combobox = Control::create<ComboBox>("combobox");
        combobox->setWidth(100);
        combobox->setText("Combobox");
        for (int i = 0; i < 40; ++i) {
            combobox->getItems().push_back("Item:" + std::to_string(i));
        }
        form->getContent()->addControl(std::move(combobox));


        UPtr<Label> label = Control::create<Label>("testLabel");
        //label->setPosition(50, 50);
        //label->setSize(200, 50);
        label->setText("Label");
        _label = label.get();
        _label->addRef();
        form->getContent()->addControl(std::move(label));

        UPtr<Button> button = Control::create<Button>("testButton");
        //button->setPosition(45, 100);
        //button->setSize(200, 100);
        button->setText("Button");
        button->addListener(this, Control::Listener::CLICK);
        form->getContent()->addControl(std::move(button));


        UPtr<CheckBox> checkbox = Control::create<CheckBox>("checkbox");
        //checkbox->setPosition(45, 200);
        checkbox->setText("CheckBox");
        form->getContent()->addControl(std::move(checkbox));
        //SAFE_RELEASE(checkbox);

        UPtr<RadioButton> radio = Control::create<RadioButton>("radio");
        radio->setGroupId("radioGroup");
        radio->setText("RadioButton");
        form->getContent()->addControl(std::move(radio));
        //SAFE_RELEASE(radio);

        UPtr<RadioButton> radio2 = Control::create<RadioButton>("radio");
        radio2->setGroupId("radioGroup");
        radio2->setText("RadioButton");
        form->getContent()->addControl(std::move(radio2));
        //SAFE_RELEASE(radio2);

        UPtr<Slider> slider = Control::create<Slider>("slider");
        slider->setText("Slider");
        slider->setWidth(1.0, Control::AUTO_PERCENT_PARENT);
        form->getContent()->addControl(std::move(slider));
        //SAFE_RELEASE(slider);

        UPtr<TextBox> text = Control::create<TextBox>("text");
        text->setText("input");
        text->setWidth(300);
        form->getContent()->addControl(std::move(text));
        //SAFE_RELEASE(text);


        //test scroll
        UPtr<ScrollContainer> container = Control::create<ScrollContainer>("container");
        container->setSize(200, 100);
        container->setScroll(ScrollContainer::SCROLL_BOTH);
        UPtr<Button> button2 = Control::create<Button>("testButton2");
        button2->setText("test\nButton");
        button2->setSize(200, 100);
        container->addControl(std::move(button2));
        form->getContent()->addControl(std::move(container));
        //SAFE_RELEASE(button2);
        //SAFE_RELEASE(container);


        UPtr<ImageView> image = Control::create<ImageView>("image");
        image->setImage("res/image/logo.png");
        image->setSize(50, 50);
        form->getContent()->addControl(std::move(image));
        //SAFE_RELEASE(image);


        UPtr<JoystickControl> joystick = Control::create<JoystickControl>("joystick");
        joystick->setSize(100, 100);
        form->getContent()->addControl(std::move(joystick));
        //SAFE_RELEASE(joystick);
#endif
        UPtr<ComboBox> combobox2 = Control::create<ComboBox>("combobox2");
        combobox2->setWidth(100);
        combobox2->setText("Combobox2");
        for (int i = 0; i < 10; ++i) {
            combobox2->getItems().push_back("Item:"+std::to_string(i));
        }
        form->getContent()->addControl(std::move(combobox2));
#if 0
        //test scroll
        UPtr<ScrollContainer> containerS = Control::create<ScrollContainer>("container2");
        containerS->setSize(200, 100);
        containerS->setScroll(ScrollContainer::SCROLL_BOTH);
        UPtr<Button> buttonS = Control::create<Button>("testButton2");
        buttonS->setText("test\nButton");
        buttonS->setSize(200, 100);
        containerS->addControl(std::move(buttonS));
        form->getContent()->addControl(std::move(containerS));

        //test scroll
        UPtr<ScrollContainer> containerS3 = Control::create<ScrollContainer>("container3");
        containerS3->setSize(200, 100);
        containerS3->setScroll(ScrollContainer::SCROLL_BOTH);
        UPtr<Button> buttonS3 = Control::create<Button>("testButton3");
        buttonS3->setText("test\nButton");
        buttonS3->setSize(200, 100);
        containerS3->addControl(std::move(buttonS3));
        form->getContent()->addControl(std::move(containerS3));
#endif
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
                if (_label) _label->setText("clicked");
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
﻿
#include <iostream>
#include "mgp.h"

using namespace mgp;

class MainApp : public Application, Control::Listener {
    Label* _label = NULL;
    //Button* button;

    void initialize() {
        //auto theme = Theme::create("res/ui/default.theme");
        //Theme::setDefault(theme.get());

        UPtr<Form> form = Form::create();
        form->getContent()->setSize(600, 700);
        form->getContent()->setPadding(20, 20, 20, 20);
        form->getContent()->setLayout(Layout::LAYOUT_FLOW);
#if 0
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

        UPtr<ProgressBar> progressBar = Control::create<ProgressBar>("ProgressBar");
        progressBar->setWidth(1.0, Control::AUTO_PERCENT_PARENT);
        progressBar->setValue(0.3);
        form->getContent()->addControl(std::move(progressBar));

        UPtr<LoadingView> loading = Control::create<LoadingView>("LoadingView");
        loading->setWidth(1.0, Control::AUTO_PERCENT_PARENT);
        form->getContent()->addControl(std::move(loading));


        UPtr<TextBox> text = Control::create<TextBox>("text");
        text->setText("input");
        text->setWidth(300);
        form->getContent()->addControl(std::move(text));
        //SAFE_RELEASE(text);

#endif
#if 0
        //test scroll
        UPtr<ScrollContainer> container = Control::create<ScrollContainer>("container");
        container->setSize(200, 100);
        container->setScroll(ScrollContainer::SCROLL_BOTH); {
            UPtr<Button> button2 = Control::create<Button>("testButton2");
            button2->setText("test\nButton");
            button2->setSize(200, 100);
            container->addControl(std::move(button2));
        }
        form->getContent()->addControl(std::move(container));

#endif
#if 0
        UPtr<ImageView> image = Control::create<ImageView>("image");
        image->setImage("res/image/logo.png");
        image->setSize(50, 50);
        form->getContent()->addControl(std::move(image));
        //SAFE_RELEASE(image);


        UPtr<JoystickControl> joystick = Control::create<JoystickControl>("joystick");
        joystick->setSize(100, 100);
        form->getContent()->addControl(std::move(joystick));
        //SAFE_RELEASE(joystick);

        UPtr<ComboBox> combobox2 = Control::create<ComboBox>("combobox2");
        combobox2->setWidth(100);
        combobox2->setText("Combobox2");
        for (int i = 0; i < 10; ++i) {
            combobox2->getItems().push_back("Item:"+std::to_string(i));
        }
        form->getContent()->addControl(std::move(combobox2));
#endif
#if 0
        UPtr<Button> button = Control::create<Button>("testButton");
        button->setText("Button");
        button->addListener(this, Control::Listener::CLICK);
        form->getContent()->addControl(std::move(button));

        UPtr<Icon> icon = Control::create<Icon>("Icon");
        icon->setImagePath("res/image/point.png");
        icon->setToolTip("Hello World");
        icon->setPadding(4);
        form->getContent()->addControl(std::move(icon));
#endif
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
        UPtr<Button> button = Control::create<Button>("testButton");
        //button->setPosition(45, 100);
        //button->setSize(200, 100);
        button->setText("Button");
        button->addListener(this, Control::Listener::CLICK);
        
        UPtr<Font> font = Font::create("res/ui/sans.ttf");
        //set button style
        button->overrideStyle();
        button->getStyle()->setFont(font.get());
        
        ThemeImage* image = Theme::getDefault()->getImage("button");
        UPtr<BorderImage> border(new BorderImage(image->getRegion(), Border(20,20,20,20)));
        button->getStyle()->setBgImage(border.get());
        form->getContent()->addControl(std::move(button));

#endif
#if 0
        form->getContent()->setLayout(Layout::LAYOUT_VERTICAL);
        auto accordion1 = Control::create<Accordion>("accord1");
        accordion1->getButton()->setText("Accord1");
        accordion1->getContent()->setStyleName("Rect");
        accordion1->getContent()->setHeight(100);
        form->getContent()->addControl(std::move(accordion1));

        auto accordion2 = Control::create<Accordion>("accord2");
        accordion2->getContent()->setHeight(100);
        accordion2->getContent()->setStyleName("Panel");
        form->getContent()->addControl(std::move(accordion2));
#endif
        /*auto sceneWriter = mgp::SerializerJson::createWriter("ui.hml", true);
        sceneWriter->writeObject(nullptr, form->getContent());
        sceneWriter->close();

        auto reader = mgp::Serializer::createReader("ui.hml", true);
        auto content = reader->readObject(nullptr).dynamicCastTo<mgp::Container>();
        form->setContent(std::move(content));*/

        getFormManager()->add(std::move(form));
        //SAFE_RELEASE(theme);
    }

    void render(float elapsedTime) override {
        Renderer::cur()->clear(Renderer::CLEAR_COLOR_DEPTH_STENCIL, Vector4::fromColor(0x888888ff));
        Application::render(elapsedTime);
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
                Toast::showToast(control, "Message");
            }
        }
    }

};

int main() {
    printf("main start\n");
    
    #if __EMSCRIPTEN__
        MainApp* instance = new MainApp();
        return Platform::run(instance);
    #else
        MainApp instance;
        return Platform::run(&instance);
    #endif
}
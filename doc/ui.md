

## GUI

mgp自带UI库，支持常用的控件。
新版本使用wase库，自带的GUi库以后将不再维护。

```
    UPtr<Form> form = Form::create();
    form->getContent()->setSize(600, 700);
    form->getContent()->setLayout(Layout::LAYOUT_FLOW);

    UPtr<Button> button = Control::create<Button>("testButton");
    button->setText("Button");
    
    //事件监听可以用addListener观察者模式，或者lambda闭包来实现
    //button->addListener(this, Control::Listener::CLICK);
    button->setListener([&](Control* control, Control::Listener::EventType evt){
        //
    });

    form->getContent()->addControl(std::move(button));
    getFormManager()->add(std::move(form));
```

- 所有控件统一用Control::create来创建。
- Form类似于窗口。同时可以有多个Form存在。
- 每个控件有一个ID，可以通过findControl来由ID查找。

## 控件自动大小

通过setAutoSize*设置控件的相对大小和位置。

- AUTO_SIZE_NONE 绝对大小，使用DP逻辑单位
- AUTO_WRAP_CONTENT 包裹内部内容的最小大小
- AUTO_PERCENT_LEFT 父容器的剩余空间的百分比
- AUTO_PERCENT_PARENT 百分比单位（以父容器大小为基准）

例如设置为填充父容器：
```
    setWidth(1.0, Control::AUTO_PERCENT_PARENT);
```

## 边框大小

- setMargin设置控件的边距
- setPadding设置控件的内部填充

## 控件布局
通过setLayout设置控件的布局方式。

- LAYOUT_FLOW 流式布局，从左到右，放不下的时候自动换行。
- LAYOUT_VERTICAL 垂直布局
- LAYOUT_ABSOLUTE 绝对布局（默认）
- LAYOUT_HORIZONTAL 水平布局

在非绝对布局情况下，setX/setY设置的位置也是有效的，效果相当于标准位置的偏移量。

## 控件对齐

使用setAlignment来设置控件在父控件中的对齐方式。
在父容器为LAYOUT_ABSOLUTE时所有Alignment选项都有效，其他布局下仅部分Alignment选项有效。

## 样式

通过setStyleName来改变控件的样式，样式名称对应配置文件res/ui/default.theme里面的某一项。
除了默认样式文件，也可以手动加载自定义的样式：
```
    auto theme = Theme::create(...);
    Theme::setDefault(theme.get());
```

## 界面描述语言
UI使用HiML格式配置。详见[jsonc](https://github.com/chunquedong/jsonc) readme文件。

```
mgp::ScrollContainer {
  version = 4.0
  style = Form
  id = _form_content
  width = 600
  height = 700
  padding = {
    20,
    20,
    20,
    20
  }
  scrollBarsAutoHide = true
  mgp::Label {
    id = testLabel
    autoSizeW = WrapContent
    autoSizeH = WrapContent
    text = Label
  }
  mgp::Button {
    id = testButton
    autoSizeW = WrapContent
    autoSizeH = WrapContent
    padding = {
      4,
      12,
      4,
      12
    }
    x = 100
    y = 100
    text = Button
  }
}
```
加载UI文件
```
auto reader = mgp::Serializer::createReader("ui.hml", true);
auto content = reader->readObject(nullptr).dynamicCastTo<mgp::Container>();
```

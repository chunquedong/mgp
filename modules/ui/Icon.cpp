#include "base/Base.h"
#include "Icon.h"

namespace mgp
{

Icon::Icon()
{
    _canFocus = true;
    _className = "Icon";
}

Icon::~Icon()
{
}

bool Icon::isChecked() {
    return _checked;
}

void Icon::setChecked(bool checked) {
    if (_checked != checked)
    {
        _checked = checked;
        setDirty(DIRTY_STATE);
        notifyListeners(Control::Listener::VALUE_CHANGED);
    }
}

void Icon::controlEvent(Control::Listener::EventType evt)
{
    Control::controlEvent(evt);
    if (_checkable) {
        switch (evt)
        {
        case Control::Listener::CLICK:

            setChecked(!_checked);
            break;
        }
    }
}

unsigned int Icon::drawBorder(Form* form, const Rectangle& clip, RenderInfo* view) {
    if (!_checked) return 0;
    return Control::drawBorder(form, clip, view);
}

void Icon::onSerialize(Serializer* serializer) {
    Control::onSerialize(serializer);
    serializer->writeString("path", _imagePath.c_str(), "");
    serializer->writeBool("checkable", _checkable, false);
    serializer->writeBool("checked", _checked, false);
}

void Icon::onDeserialize(Serializer* serializer) {
    Control::onDeserialize(serializer);
    std::string path;
    serializer->readString("path", path, "");
    if (path.size() > 0)
    {
        setImagePath(path.c_str());
    }
    _checkable = serializer->readBool("checkable", false);
    _checked = serializer->readBool("checked", false);
}

void Icon::setImagePath(const char* path)
{
    _image = getTheme()->getImageFullName(path);
    GP_ASSERT(_image);
    _imagePath = path;
}

const char* Icon::getImagePath() {
    return _imagePath.c_str();
}

void Icon::measureSize()
{
    Control::measureSize();
    if (isWrapContentSize() && _image)
    {
        unsigned int w = _image->getRegion().width;
        unsigned int h = _image->getRegion().height;
        if (_autoSizeW == AUTO_WRAP_CONTENT)
        {
            setMeasureContentWidth(w);
        }
        if (_autoSizeH == AUTO_WRAP_CONTENT)
        {
            setMeasureContentHeight(h);
        }
    }
}

unsigned int Icon::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!_image) return 0;

    const Rectangle& region = _image->getRegion();
    Vector4 color = getStyle()->getColor((Style::OverlayType)getState());
    color.w *= _opacity;

    SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
    startBatch(form, batch);
    batch->drawImage(Rectangle(_viewportBounds.x, _viewportBounds.y, _viewportBounds.height, _viewportBounds.height), 
        region, color, &_viewportClipBounds);
    finishBatch(form, batch, view);
    return 1;
}

LoadingView::LoadingView() {
    _canFocus = false;
    _className = "LoadingView";
}

void LoadingView::updateState(State state)
{
    Control::updateState(state);
    _image = getTheme()->getImage("loading");
}

unsigned int LoadingView::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!_image) return 0;

    const Rectangle& region = _image->getRegion();
    Vector4 color = getStyle()->getColor((Style::OverlayType)getState());
    color.w *= _opacity;

    SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
    startBatch(form, batch);

    //batch->drawImage(Rectangle(_viewportBounds.x, _viewportBounds.y, _viewportBounds.height, _viewportBounds.height),
    //    region, color, &_viewportClipBounds);
    batch->drawImageRotated(Vector3(_absoluteBounds.x, _absoluteBounds.y, 0),
        region, Vector2(_viewportBounds.height, _viewportBounds.height), color, Vector2(0.5, 0.5), _progress, false);
    _progress += 0.04;

    finishBatch(form, batch, view);
    return 1;
}

}

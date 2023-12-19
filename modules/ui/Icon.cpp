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

void Icon::onSerialize(Serializer* serializer) {
    Control::onSerialize(serializer);
}

void Icon::onDeserialize(Serializer* serializer) {
    Control::onDeserialize(serializer);
    std::string path;
    serializer->readString("path", path, "");
    if (path.size() > 0)
    {
        setImagePath(path.c_str());
    }
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

void Icon::updateBounds()
{
    Control::updateBounds();
    if (isAutoSize() && _image)
    {
        unsigned int w = _image->getRegion().width;
        unsigned int h = _image->getRegion().height;
        if (_autoSizeW == AUTO_WRAP_CONTENT)
        {
            setWidthInternal(w + getPadding().left + getPadding().right);
        }
        if (_autoSizeH == AUTO_WRAP_CONTENT)
        {
            setHeightInternal(h + getPadding().top + getPadding().bottom);
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


}

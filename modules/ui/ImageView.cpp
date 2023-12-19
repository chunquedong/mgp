#include "base/Base.h"
#include "ImageView.h"

namespace mgp
{

ImageView::ImageView() :
    _srcRegion(Rectangle::empty()), _dstRegion(Rectangle::empty()), _batch(NULL),
    _tw(0.0f), _th(0.0f), _uvs(Vector4(0,0,1,1))
{
    _className = "ImageView";
}

ImageView::~ImageView()
{
    SAFE_DELETE(_batch);
}

void ImageView::onSerialize(Serializer* serializer) {
    Control::onSerialize(serializer);
}

void ImageView::onDeserialize(Serializer* serializer) {
    Control::onDeserialize(serializer);
    std::string path;
    serializer->readString("path", path, "");
    if (path.size() > 0)
    {
        setImage(path.c_str());
    }

    Vector4 region = serializer->readVector("srcRegion", region);
    setRegionSrc(region.x, region.y, region.z, region.w);

    Vector4 regionDst = serializer->readVector("dstRegion", region);
    setRegionDst(regionDst.x, regionDst.y, regionDst.z, regionDst.w);
}

void ImageView::setImage(const char* path)
{
    SAFE_DELETE(_batch);
    UPtr<Texture> texture = Texture::create(path, false);
    _batch = SpriteBatch::create(texture.get()).take();
    _tw = 1.0f / texture->getWidth();
    _th = 1.0f / texture->getHeight();
    //texture->release();

    if (isAutoSize())
        setDirty(DIRTY_BOUNDS);
}

void ImageView::setRegionSrc(float x, float y, float width, float height)
{
    _srcRegion.set(x, y, width, height);

    _uvs.x = x * _tw;
    _uvs.z = (x + width) * _tw;
    _uvs.y = (y * _th);
    _uvs.w = ((y + height) * _th);
}

void ImageView::setRegionSrc(const Rectangle& region)
{
    setRegionSrc(region.x, region.y, region.width, region.height);
}

const Rectangle& ImageView::getRegionSrc() const
{
    return _srcRegion;
}

void ImageView::setRegionDst(float x, float y, float width, float height)
{
    _dstRegion.set(x, y, width, height);
}

void ImageView::setRegionDst(const Rectangle& region)
{
    setRegionDst(region.x, region.y, region.width, region.height);
}

const Rectangle& ImageView::getRegionDst() const
{
    return _dstRegion;
}

unsigned int ImageView::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!_batch)
        return 0;

    startBatch(form, _batch);

    Vector4 color = Vector4::one();
    color.w *= _opacity;

    if (_dstRegion.isEmpty())
    {
        _batch->draw(_viewportBounds.x, _viewportBounds.y, _viewportBounds.width, _viewportBounds.height,
            _uvs.x, _uvs.y, _uvs.z, _uvs.w, color, &_viewportClipBounds);
    }
    else
    {
        _batch->draw(_viewportBounds.x + _dstRegion.x, _viewportBounds.y + _dstRegion.y,
            _dstRegion.width, _dstRegion.height,
            _uvs.x, _uvs.y, _uvs.z, _uvs.w, color, &_viewportClipBounds);
    }

    finishBatch(form, _batch, view);

    return 1;
}

void ImageView::updateBounds()
{
    if (_batch)
    {
        if (_autoSizeW == AUTO_WRAP_CONTENT)
        {
            setWidthInternal(_batch->getSampler()->getWidth());
        }

        if (_autoSizeH == AUTO_WRAP_CONTENT)
        {
            setHeightInternal(_batch->getSampler()->getWidth());
        }
    }

    Control::updateBounds();
}

}

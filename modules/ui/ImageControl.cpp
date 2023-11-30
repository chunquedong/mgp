#include "base/Base.h"
#include "ImageControl.h"

namespace mgp
{

ImageControl::ImageControl() :
    _srcRegion(Rectangle::empty()), _dstRegion(Rectangle::empty()), _batch(NULL),
    _tw(0.0f), _th(0.0f), _uvs(Vector4(0,0,1,1))
{
}

ImageControl::~ImageControl()
{
    SAFE_DELETE(_batch);
}

void ImageControl::initialize(const char* typeName, Style* style, Properties* properties)
{
	Control::initialize(typeName, style, properties);

	if (properties)
	{
		std::string path;
		if (properties->getPath("path", &path))
		{
			setImage(path.c_str());
		}

		if (properties->exists("srcRegion"))
		{
			Vector4 region;
			properties->getVector4("srcRegion", &region);
			setRegionSrc(region.x, region.y, region.z, region.w);
		}

		if (properties->exists("dstRegion"))
		{
			Vector4 region;
			properties->getVector4("dstRegion", &region);
			setRegionDst(region.x, region.y, region.z, region.w);
		}
	}
}

const char* ImageControl::getTypeName() const
{
    return "ImageControl";
}

void ImageControl::setImage(const char* path)
{
    SAFE_DELETE(_batch);
    UPtr<Texture> texture = Texture::create(path, false);
    _batch = SpriteBatch::create(texture.get()).take();
    _tw = 1.0f / texture->getWidth();
    _th = 1.0f / texture->getHeight();
    //texture->release();

    if (_autoSize != AUTO_SIZE_NONE)
        setDirty(DIRTY_BOUNDS);
}

void ImageControl::setRegionSrc(float x, float y, float width, float height)
{
    _srcRegion.set(x, y, width, height);

    _uvs.x = x * _tw;
    _uvs.z = (x + width) * _tw;
    _uvs.y = (y * _th);
    _uvs.w = ((y + height) * _th);
}

void ImageControl::setRegionSrc(const Rectangle& region)
{
    setRegionSrc(region.x, region.y, region.width, region.height);
}

const Rectangle& ImageControl::getRegionSrc() const
{
    return _srcRegion;
}

void ImageControl::setRegionDst(float x, float y, float width, float height)
{
    _dstRegion.set(x, y, width, height);
}

void ImageControl::setRegionDst(const Rectangle& region)
{
    setRegionDst(region.x, region.y, region.width, region.height);
}

const Rectangle& ImageControl::getRegionDst() const
{
    return _dstRegion;
}

unsigned int ImageControl::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
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

void ImageControl::updateBounds()
{
    if (_batch)
    {
        if (_autoSize & AUTO_SIZE_WIDTH)
        {
            setWidthInternal(_batch->getSampler()->getWidth());
        }

        if (_autoSize & AUTO_SIZE_HEIGHT)
        {
            setHeightInternal(_batch->getSampler()->getWidth());
        }
    }

    Control::updateBounds();
}

}

#include "Image.h"

namespace mgp
{

inline unsigned char* Image::getData() const
{
    return _data;
}

inline void Image::setData(unsigned char* data) {
    _data = data;
}

inline Image::Format Image::getFormat() const
{
    return _format;
}

inline unsigned int Image::getHeight() const
{
    return _height;
}
        
inline unsigned int Image::getWidth() const
{
    return _width;
}

}

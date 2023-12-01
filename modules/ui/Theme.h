#ifndef THEME_H_
#define THEME_H_

#include "base/Base.h"
#include "base/Ref.h"
#include "objects/Font.h"
#include "math/Rectangle.h"
#include "material/Texture.h"
#include "base/Properties.h"

namespace mgp
{
class Style;
/**
* Struct representing margin, border, and padding areas by
* the width or height of each side.
*/
struct SideRegions
{
    /** 
    * Constructor.
    */
    SideRegions() : top(0), bottom(0), left(0), right(0) {}

    SideRegions(float top, float right, float bottom, float left) : top(top), bottom(bottom), left(left), right(right) {}

    /**
    * Gets an empty SideRegion.
    */
    static const SideRegions& empty() {
        static SideRegions empty;
        return empty;
    }

    /**
    * The top of the SideRegion.
    */
    float top;
    
    /**
    * The bottom of the SideRegion.
    */
    float bottom;
    
    /**
    * The left side of the SideRegion.
    */
    float left;
    
    /**
    * The right side of the SideRegion.
    */
    float right;
};

/** 
* Struct representing margin areas by the width or height of each side.
*/
typedef SideRegions Margin;

/** 
* Struct representing border areas by the width or height of each side.
*/
typedef SideRegions Border;

/** 
* Struct representing padding areas by the width or height of each side.
*/
typedef SideRegions Padding;

/**
* Class representing an image within the theme's texture atlas.
* An image has a region and a blend color in addition to an ID.
* UV coordinates are calculated from the region and can be retrieved.
*/
class ThemeImage : public Refable
{
    //friend class Theme;
    //friend class Style;
public:

    /** 
    * Gets the Rectangle region of the ThemeImage.
    */
    const Rectangle& getRegion() const { return _region; }

    ThemeImage(const Rectangle& region) : _region(region) {}

    ~ThemeImage() {}

private:
    Rectangle _region;
};

/**
* A skin defines the border and background of a control.
*/
class BorderImage : public Refable
{
    //friend class Style;
public:

    enum SkinArea
    {
        TOP_LEFT, TOP, TOP_RIGHT,
        LEFT, CENTER, RIGHT,
        BOTTOM_LEFT, BOTTOM, BOTTOM_RIGHT
    };

    /**
    * Gets this skin's ID.
    *
    * @return This skin's ID.
    */
    //const char* getId() const;

    /**
    * Gets this skin's border.
    *
    * @return This skin's border.
    */
    const Border& getBorder() const { return _border; }

    /**
    * Gets the skin region within the theme texture.
    *
    * @return The skin region.
    */
    const Rectangle& getRegion() const { return _region; }

    /**
    * Gets this skin's UVs.
    *
    * @return This skin's UVs. x:u1, y:v1, z:u2, w:v2
    */
    const Vector4 getUVs(SkinArea area, float tw, float th) const;

    BorderImage(const Rectangle& region, const Border& border);
    
    //~BorderImage();

    /**
    * Hidden copy assignment operator.
    */
    BorderImage& operator=(const BorderImage&);

    //static BorderImage* create(const char* id, const Rectangle& region, const Border& border, const Vector4& color);

    void setRegion(const Rectangle& region);


    unsigned int draw(SpriteBatch* batch, const Rectangle& _absoluteBounds, const Vector4& skinColor, const Rectangle& clip);

private:
    //std::string _id;
    Border _border;
    Vector4 _uvs[9];
    Rectangle _region;
};

/**
 * Defines a theme used to represent the look or appearance of controls.
 *
 * Once loaded, the appearance properties can be retrieved from their style IDs and set on other
 * UI controls.  A Theme has one property, 'texture', which points to a texture atlas containing
 * all the images used by the theme.  Cursor images, skins, and lists of images used by controls
 * are defined in their own namespaces.  The rest of the Theme consists of Style namespaces.
 * A Style describes the border, margin, and padding of a Control, what images, skins, and cursors
 * are associated with a Control, and Font properties to apply to a Control's text.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Theme
 */
class Theme: public Refable
{
    //friend class Control;
    //friend class Form;
    //friend class BorderImage;
    //friend class Game;

public:

    /**
     * Creates a theme using the data from the Properties object defined at the specified URL, 
     * where the URL is of the format "<file-path>.<extension>#<namespace-id>/<namespace-id>/.../<namespace-id>"
     * (and "#<namespace-id>/<namespace-id>/.../<namespace-id>" is optional). 
     * 
     * @param url The URL pointing to the Properties object defining the theme. 
     * @script{create}
     */
    static SPtr<Theme> create(const char* url);

    /**
     * Returns the default theme.
     *
     * @return The default theme.
     */
    static Theme* getDefault();
    static void setDefault(Theme *t);

    /**
     * Get a style by its ID.
     *
     * @param id The style ID.
     *
     * @return The style with the specified ID, or NULL if it does not exist.
     */
    Style* getStyle(const char* id) const;

    void setStyle(const char* id, Style* style);


    ThemeImage* getImage(const char* id);
    ThemeImage* getImageFullName(const char* file);

    /**
     * Get the empty style.  Used when a control does not specify a style.
     * This is especially useful for containers that are being used only for
     * layout and positioning, and have no background or border themselves.
     * The empty style has no border, background, margin, padding, images, etc..
     * Any needed properties can be set on the control directly.
     *
     * @return The empty style.
     */
    Style* getEmptyStyle();

    /**
     * Returns the sprite batch for this theme.
     *
     * @return The theme's sprite batch.
     */
    SpriteBatch* getSpriteBatch() const;

    /**
     * Cleans up any theme related resources when the game shuts down.
     */
    static void finalize();
private:

    /**
     * Constructor.
     */
    Theme();

    /**
     * Constructor.
     */
    Theme(const Theme& theme);

    /**
     * Destructor.
     */
    ~Theme();

    void clear();

    /**
     * Hidden copy assignment operator.
     */
    Theme& operator=(const Theme&);

    std::string _url;
    //ThemeImage* _emptyImage;

    TextureAtlas* _texture;
    SpriteBatch* _spriteBatch;

    std::map<std::string, Style*> _styles;
    std::map<std::string, ThemeImage*> _images;
};

}

#endif

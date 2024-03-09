#ifndef LIGHT_H_
#define LIGHT_H_

#include "base/Ref.h"
#include "math/Vector3.h"
#include "base/Properties.h"
#include "base/Serializable.h"
#include "scene/Component.h"

namespace mgp
{

class Node;
class NodeCloneContext;

/**
 * Defines a light. 
 *
 * There are 3 types of lights that can be created
 * directional, point and spot lights.
 */
class Light : public Serializable, public Component, public Refable
{
    friend class Node;

public:

    /**
     * Defines the supported light types.
     */
    enum Type
    {
        DIRECTIONAL = 1,
        POINT = 2,
        SPOT = 3
    };

    /**
     * Defines the lighting.
     */
    enum class Lighting
    {
        eRealtime,
        eBaked
    };

    /**
     * Defines the shadows to be applied.
     */
    enum class Shadows
    {
        eNone,
        eHard,
        eSoft
    };

    /**
     * Creates a directional light.
     * 
     * @param color Color of the light.
     * 
     * @return The new directional light.
     * @script{create}
     */
    static UPtr<Light> createDirectional(const Vector3& color);

    /**
     * Creates a directional light.
     * 
     * @param red The red channel.
     * @param green The green channel.
     * @param blue The blue channel.
     * 
     * @return The new directional light.
     * @script{create}
     */
    static UPtr<Light> createDirectional(float red, float green, float blue);

    /**
     * Creates a point light.
     * 
     * @param color The light's color.
     * @param range The light's range.
     * 
     * @return The new point light.
     * @script{create}
     */
    static UPtr<Light> createPoint(const Vector3& color, float range);

    /**
     * Creates a point light.
     * 
     * @param red The red channel.
     * @param green The green channel.
     * @param blue The blue channel.
     * @param range The light's range.
     * 
     * @return The new point light.
     * @script{create}
     */
    static UPtr<Light> createPoint(float red, float green, float blue, float range);

    /**
     * Creates a spot light.
     * 
     * @param color The light's color.
     * @param range The light's range.
     * @param innerAngle The light's inner angle (in radians).
     * @param outerAngle The light's outer angle (in radians).
     * 
     * @return The new spot light.
     * @script{create}
     */
    static UPtr<Light> createSpot(const Vector3& color, float range, float innerAngle, float outerAngle);

    /**
     * Creates a spot light.
     * 
     * @param red The red channel.
     * @param green The green channel.
     * @param blue The blue channel.
     * @param range The light's range.
     * @param innerAngle The light's inner angle (in radians).
     * @param outerAngle The light's outer angle (in radians).
     * 
     * @return The new spot light.
     * @script{create}
     */
    static UPtr<Light> createSpot(float red, float green, float blue, float range, float innerAngle, float outerAngle);

    /**
     * Creates a light from a properties definition.
     *
     * The properties object must contain a "type" parameter, specifying one of the
     * supported Light::Type values. In addition, values must be supplied for all
     * parameters of the corresponding light-specific creation method.
     *
     * @param properties The properties definition of the Light.
     *
     * @return The new Light.
     */
    static UPtr<Light> create(Properties* properties);

    /**
     * Destructor.
     */
    virtual ~Light();

    /**
     * Hidden copy assignment operator.
     */
    Light& operator=(const Light&);

    /**
     * Returns the light type.
     * 
     * @return The light type.
     */
    Type getLightType() const;

    /**
     * Gets the light color.
     * 
     * @return The light color.
     */
    const Vector3& getColor() const;

    /**
     * Sets the light color.
     * 
     * @param color The light color to set.
     */
    void setColor(const Vector3& color);

    /**
     * Sets the light color.
     * 
     * @param red The red channel.
     * @param green The green channel.
     * @param blue The blue channel.
     */
    void setColor(float red, float green, float blue);

    /**
     * Returns the node associated with this light.
     * 
     * @return The node associated with this light.
     */
    Node* getNode() const;

    /**
     * Returns the Range of the point or spot light. 
     *
     * @return The range of the point or spot light.
     */
    float getRange() const;

    /**
     * Sets the range of point or spot light.
     *
     * @param range The range of point or spot light.
     */
    void setRange(float range);

    /**
     * Returns the inverse of the range of point or spot light.
     *
     * @return The range of the point or spot light.
     */
    float getRangeInverse() const;

    /**
     * Returns the inner angle the spot light (in radians).
     *
     * @return The inner angle of the spot light (in radians).
     */
    float getInnerAngle() const;

    /**
     * Sets the inner angle of a spot light (in radians).
     *
     * @param innerAngle The angle of spot light (in radians).
     */
    void setInnerAngle(float innerAngle);

    /**
     * Returns the outer angle of the spot light (in radians).
     *
     * @return The outer angle of the spot light (in radians).
     */
    float getOuterAngle() const;

    /**
     * Sets the outer angle of a spot light (in radians).
     *
     * @param outerAngle The angle of spot light (in radians).
     */
    void setOuterAngle(float outerAngle);

    /**
     * Returns the cosine of the inner angle of spot light.
     *
     * @return The cosine of the inner angle of spot light.
     */
    float getInnerAngleCos() const;

    /**
     * Returns the cosine of the outer angle of spot light.
     *
     * @return The cosine of the outer angle of spot light.
     */
    float getOuterAngleCos() const;


    int getLightMask() const { return _lightMask; }
    void setLightMask(int mask) { _lightMask = mask; }

    Shadows getShadows() const { return _shadows; }
    void setShadows(Shadows s) { _shadows = s; }
public:
    /**
     * @see Activator::createObject
     */
    static Serializable *createObject();

    /**
     * @see Activator::enumToString
     */
    static std::string enumToString(const std::string& enumName, int value);

    /**
     * @see Activator::enumParse
     */
    static int enumParse(const std::string& enumName, const std::string& str);
protected:
    /**
     * @see Serializable::getClassName
     */
    std::string getClassName();

    /**
     * @see Serializable::onSerialize
     */
    void onSerialize(Serializer* serializer);

    /**
     * @see Serializable::onDeserialize
     */
    void onDeserialize(Serializer* serializer);

private:

    /**
     * Directional light data.
     */
    class Directional
    {
    public:
        Vector3 color;

        Directional(const Vector3& color);
    };

    /**
     * Point light data.
     */
    class Point
    {
    public:
        Vector3 color;
        float range;
        float rangeInverse;

        Point(const Vector3& color, float range);
    };

    /**
     * Spot light data.
     */
    class Spot
    {
    public:
        Vector3 color;
        float range;
        float rangeInverse;
        float innerAngle;
        float innerAngleCos;
        float outerAngle;
        float outerAngleCos;

        Spot(const Vector3& color, float range, float innerAngle, float outerAngle);
    };

    Light();

    /**
     * Constructor for the directional light.
     */
    Light(Light::Type type, const Vector3& color);

    /**
     * Constructor for point light.
     */
    Light(Light::Type type, const Vector3& color, float range);

    /**
     * Constructor for spot light.
     */
    Light(Light::Type type, const Vector3& color, float range, float innerAngle, float outerAngle);

    /**
     * Sets the node associated with this light.
     * 
     * @param node The node to be associated with this light.
     */
    void setNode(Node* node);

    /**
     * Clones the light and returns a new light.
     * 
     * @param context The clone context.
     * @return The newly created light.
     */
    UPtr<Light> clone(NodeCloneContext& context);

    Light::Type _type;

    Light::Lighting _lighting;
    Light::Shadows _shadows;

    int _lightMask;

    Vector3 _color;
    float _intensity;
    
    union
    {
        /** @script{ignore} */
        Directional* _directional;
        /** @script{ignore} */
        Point* _point;
        /** @script{ignore} */
        Spot* _spot;
    };
    //Node* _node;
};

}

#endif

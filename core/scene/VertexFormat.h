#ifndef VERTEXFORMAT_H_
#define VERTEXFORMAT_H_

#include <string>
#include <vector>
#include "base/Stream.h"

namespace mgp
{

/**
 * Defines the format of a vertex layout used by a mesh.
 *
 * A VertexFormat is immutable and cannot be changed once created.
 */
class VertexFormat
{
public:

    /**
     * Defines a set of usages for vertex elements.
     */
    enum Usage
    {
        POSITION = 1,
        NORMAL = 2,
        COLOR = 3,
        TANGENT = 4,
        BINORMAL = 5,
        BLENDWEIGHTS = 6,
        BLENDINDICES = 7,
        TEXCOORD0 = 8,
        TEXCOORD1 = 9,
        TEXCOORD2 = 10,
        TEXCOORD3 = 11,
        TEXCOORD4 = 12,
        TEXCOORD5 = 13,
        TEXCOORD6 = 14,
        TEXCOORD7 = 15,
        CUSTEM = 16,
        MORPHTARGET0,
        MORPHTARGET1,
        MORPHTARGET2,
        MORPHTARGET3,
        MORPHTARGET4,
        MORPHTARGET5,
        MORPHTARGET6,
        MORPHTARGET7,
        MORPHNORMAL0,
        MORPHNORMAL1,
        MORPHNORMAL2,
        MORPHNORMAL3,
        MORPHTANGENT0,
        MORPHTANGENT1,
    };

    static const int MAX_MORPH_TARGET = 8;

    enum DataType {
        INT8 = 0x1400,
        INT16 = 0x1402,
        INT32 = 0x1404,
        FLOAT32 = 0x1406,
    };

    /**
     * Defines a single element within a vertex format.
     *
     * All vertex elements are assumed to be of type float, but can
     * have a varying number of float values (1-4), which is represented
     * by the size attribute. Additionally, vertex elements are assumed
     * to be tightly packed.
     */
    class Element
    {
    public:
        /**
         * The vertex element usage semantic.
         */
        Usage usage;

        /**
         * The number of values in the vertex element.
         */
        unsigned int size;

        /**
        * shader name to binding
        */
        std::string name;

        /**
        * offset from vertex buffer
        */
        int offset = -1;

        /**
        * advance next offset
        */
        int stride = -1;

        /**
        * float or int
        */
        DataType dataType = FLOAT32;

        /**
         * Constructor.
         */
        Element();

        /**
         * Constructor.
         *
         * @param usage The vertex element usage semantic.
         * @param size The number of float values in the vertex element.
         */
        Element(Usage usage, unsigned int size);

        Element(const std::string &name, unsigned int size);

        /**
         * Compares two vertex elements for equality.
         *
         * @param e The vertex element to compare.
         *
         * @return true if this element matches the specified one, false otherwise.
         */
        bool operator == (const Element& e) const;

        /**
         * Compares to vertex elements for inequality.
         *
         * @param e The vertex element to compare.
         *
         * @return true if this element does not match the specified one, false otherwise.
         */
        bool operator != (const Element& e) const;

        void write(Stream* file) const;
        bool read(Stream* file);
    };

    /**
     * Constructor.
     *
     * The passed in element array is copied into the new VertexFormat.
     *
     * @param elements The list of vertex elements defining the vertex format.
     * @param elementCount The number of items in the elements array.
     */
    VertexFormat(const Element* elements, unsigned int elementCount);

    VertexFormat();

    void update();

    void addElement(const Element& element);

    /**
     * Destructor.
     */
    ~VertexFormat();

    const Element* getPositionElement() const;

    /**
     * Gets the vertex element at the specified index.
     *
     * @param index The index of the element to retrieve.
     */
    const Element& getElement(unsigned int index) const;
    Element& getElement(unsigned int index);

    /**
     * Gets the number of elements in this VertexFormat.
     *
     * @return The number of items in the elements array.
     */
    unsigned int getElementCount() const;

    /**
     * Gets the size (in bytes) of a single vertex using this format.
     */
    unsigned int getVertexSize() const;

    /**
     * Compares two vertex formats for equality.
     *
     * @param f The vertex format to compare.
     *
     * @return true if the elements in this VertexFormat matches the specified one, false otherwise.
     */
    bool operator == (const VertexFormat& f) const;

    /**
     * Compares to vertex formats for inequality.
     *
     * @param f The vertex format to compare.
     *
     * @return true if the elements in this VertexFormat are not equal to the specified one, false otherwise.
     */
    bool operator != (const VertexFormat& f) const;

    /**
     * Returns a string representation of a Usage enumeration value.
     */
    static const char* toString(Usage usage);

private:

    std::vector<Element> _elements;
    unsigned int _vertexSize;
};

}

#endif

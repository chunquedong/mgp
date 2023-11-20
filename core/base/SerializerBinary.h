#pragma once

#include "Serializer.h"

namespace mgp
{

/**
 * Defines a binary serializer.
 *
 * @see Serializer
 */
class SerializerBinary : public Serializer
{
    friend class Serializer;

public:

    /**
     * Creates a binary serializer for writing(serializing)
     *
     * @param path The path to open create and open the serializer from.
     * @return The new binary serializer.
     */
    static UPtr<Serializer> createWriter(const std::string& path);
    
    /**
     * @see Serializer::close
     */
    void close();
    
    /**
     * @see Serializer::getFormat
     */
    Format getFormat() const;

    /**
     * @see Serializer::writeEnum
     */
    void writeEnum(const char* propertyName, const char* enumName, int value, int defaultValue);

    /**
     * @see Serializer::writeBool
     */
    void writeBool(const char* propertyName, bool value, bool defaultValue);

    /**
     * @see Serializer::writeInt
     */
    void writeInt(const char* propertyName, int value, int defaultValue);

    /**
     * @see Serializer::writeFloat
     */
    void writeFloat(const char* propertyName, float value, float defaultValue);

    /**
     * @see Serializer::writeVector
     */
    void writeVector(const char* propertyName, const Vector2& value, const Vector2& defaultValue);
    
    /**
     * @see Serializer::writeVector
     */
    void writeVector(const char* propertyName, const Vector3& value, const Vector3& defaultValue);
    
    /**
     * @see Serializer::writeVector
     */
    void writeVector(const char* propertyName, const Vector4& value, const Vector4& defaultValue);

    /**
     * @see Serializer::writeColor
     */
    void writeColor(const char* propertyName, const Vector3& value, const Vector3& defaultValue);

    /**
     * @see Serializer::writeColor
     */
    void writeColor(const char* propertyName, const Vector4& value, const Vector4& defaultValue);
    
    /**
     * @see Serializer::writeMatrix
     */
    void writeMatrix(const char* propertyName, const Matrix4& value, const Matrix4& defaultValue);

    /**
     * @see Serializer::writeString
     */
    void writeString(const char* propertyName, const char* value, const char* defaultValue);

    /**
     * @see Serializer::writeMap
     */
    void writeMap(const char* propertyName, std::vector<std::string> &keys);

    /**
     * @see Serializer::writeObject
     */
    void writeObject(const char* propertyName, Serializable* value);

    /**
     * @see Serializer::writeList
     */
    void writeList(const char* propertyName, size_t count);

    void finishColloction();

    /**
     * @see Serializer::writeIntArray
     */
    void writeIntArray(const char* propertyName, const int* data, size_t count);

    /**
     * @see Serializer::writeFloatArray
     */
    void writeFloatArray(const char* propertyName, const float* data, size_t count);
    void writeDFloatArray(const char* propertyName, const double* data, size_t count);
    
    /**
     * @see Serializer::writeByteArray
     */
    void writeByteArray(const char* propertyName, const unsigned char* data, size_t count);

    /**
     * @see Serializer::readEnum
     */
    int readEnum(const char* propertyName, const char* enumName, int defaultValue);

    /**
     * @see Serializer::readBool
     */
    bool readBool(const char* propertyName, bool defaultValue);

    /**
     * @see Serializer::readInt
     */
    int readInt(const char* propertyName, int defaultValue);

    /**
     * @see Serializer::readFloat
     */
    float readFloat(const char* propertyName, float defaultValue);
    
    /**
     * @see Serializer::readVector
     */
    Vector2 readVector(const char* propertyName, const Vector2& defaultValue);
    
    /**
     * @see Serializer::readVector
     */
    Vector3 readVector(const char* propertyName, const Vector3& defaultValue);
    
    /**
     * @see Serializer::readVector
     */
    Vector4 readVector(const char* propertyName, const Vector4& defaultValue);
    
    /**
     * @see Serializer::readColor
     */
    Vector3 readColor(const char* propertyName, const Vector3& defaultValue);
    
    /**
     * @see Serializer::readColor
     */
    Vector4 readColor(const char* propertyName, const Vector4& defaultValue);

    /**
     * @see Serializer::readMatrix
     */
    Matrix4 readMatrix(const char* propertyName, const Matrix4& defaultValue);

    /**
     * @see Serializer::readString
     */
    void readString(const char* propertyName, std::string& value, const char* defaultValue);

    /**
     * @see Serializer::readMap
     */
    void readMap(const char* propertyName, std::vector<std::string> &keys);

    /**
     * @see Serializer::readObject
     */
    Serializable* readObject(const char* propertyName);

    /**
     * @see Serializer::readList
     */
    size_t readList(const char* propertyName);

    /**
     * @see Serializer::readIntArray
     */
    size_t readIntArray(const char* propertyName, int** data);

    /**
     * @see Serializer::readFloatArray
     */
    size_t readFloatArray(const char* propertyName, float** data);
    size_t readDFloatArray(const char* propertyName, double** data);

    /**
     * @see Serializer::readByteArray
     */
    size_t readByteArray(const char* propertyName, unsigned char** data);
    
    /**
     * Destructor
     */
    ~SerializerBinary();
    
protected:
    
    SerializerBinary(Type type, const std::string& path, Stream* stream, uint32_t versionMajor, uint32_t versionMinor);
    static UPtr<Serializer> create(const std::string& path, Stream* stream);
    void writeLengthPrefixedString(const char* str);
    void readLengthPrefixedString(std::string& str);
    
private:
    
    static unsigned char BIT_NULL;
    static unsigned char BIT_VALUE;
    static unsigned char BIT_XREF;
    static unsigned char BIT_DEFAULT;

    std::map< unsigned long, Serializable*> _xrefs;
};

}

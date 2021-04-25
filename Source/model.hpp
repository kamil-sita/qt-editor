#pragma once

#include <string>
#include <vector>
#include <memory>
#include <jsonxx.h>

class ModelException
{
public:
    virtual ~ModelException() { }
    ModelException() = default;
    explicit ModelException(const std::string& what) : mWhat(what) { }
    const std::string& what() const
    {
        return mWhat;
    }
protected:
    std::string mWhat;
};

// Error opening json file on disk
class IOReadException final : public ModelException { public: using ModelException::ModelException; };

// Json data failed to parse
class InvalidJsonException final : public ModelException {};

// Game name in the json isn't AO or AE
class InvalidGameException final : public ModelException { public: using ModelException::ModelException; };

// In the json schema the "typeName" couldn't be found in basic or enum types for "structureName"
class ObjectPropertyTypeNotFoundException final : public ModelException
{
public:
    explicit ObjectPropertyTypeNotFoundException(const std::string& structureName, const std::string& typeName)
        : ModelException(structureName + ":" + typeName), mStructName(structureName), mTypeName(typeName)
    {

    }

    const std::string& StructName() const { return mStructName; }
    const std::string& TypeName() const { return mTypeName; }

private:
    std::string mStructName;
    std::string mTypeName;
};

// Expected to read "key" in the json but it either didn't exist or was the wrong type
class JsonKeyNotFoundException final : public ModelException
{
public:
    explicit JsonKeyNotFoundException(const std::string& key)
        : ModelException(key), mKey(key)
    {

    }

    const std::string& Key() const { return mKey; }

private:
    std::string mKey;
};

struct ObjectProperty final
{
    std::string mName;
    std::string mTypeName;
    int mBasicTypeValue = 0;
    std::string mEnumValue;
    bool mVisible = true;
};
using UP_ObjectProperty = std::unique_ptr<ObjectProperty>;

inline const ObjectProperty* PropertyByName(const std::string& name, const std::vector<UP_ObjectProperty>& props)
{
    for (const auto& prop : props)
    {
        if (prop->mName == name)
        {
            return prop.get();
        }
    }
    return nullptr;
}

struct MapObject final
{
    std::string mName;
    std::string mObjectStructureType;
    std::vector<UP_ObjectProperty> mProperties;

    int XPos() const 
    {
        return PropertyByName("xpos", mProperties)->mBasicTypeValue;
    }

    int YPos() const
    {
        return PropertyByName("ypos", mProperties)->mBasicTypeValue;
    }

    int Width() const
    {
        return PropertyByName("width", mProperties)->mBasicTypeValue;
    }

    int Height() const
    {
        return PropertyByName("height", mProperties)->mBasicTypeValue;
    }
};
using UP_MapObject = std::unique_ptr<MapObject>;

struct Camera final
{
    std::string mName;
    int mId = 0;
    int mX = 0;
    int mY = 0;
    std::vector<UP_MapObject> mMapObjects;
};
using UP_Camera = std::unique_ptr<Camera>;

class CollisionObject final
{
public:
    std::vector<UP_ObjectProperty> mProperties;

    int X1() const
    {
        return PropertyByName("x1", mProperties)->mBasicTypeValue;
    }

    int Y1() const
    {
        return PropertyByName("y1", mProperties)->mBasicTypeValue;
    }

    int X2() const
    {
        return PropertyByName("x2", mProperties)->mBasicTypeValue;
    }

    int Y2() const
    {
        return PropertyByName("y2", mProperties)->mBasicTypeValue;
    }
};
using UP_CollisionObject = std::unique_ptr<CollisionObject>;

struct Enum final
{
    std::string mName;
    std::vector<std::string> mValues;
};
using UP_Enum = std::unique_ptr<Enum>;

struct EnumOrBasicTypeProperty final
{
    std::string mName;
    std::string mType;
    bool mVisible = true;
};

struct ObjectStructure final
{
    std::string mName;
    std::vector<EnumOrBasicTypeProperty> mEnumAndBasicTypeProperties;
};
using UP_ObjectStructure = std::unique_ptr<ObjectStructure>;

struct BasicType final
{
    std::string mName;
    int mMinValue = 0;
    int mMaxValue = 0;
};
using UP_BasicType = std::unique_ptr<BasicType>;

struct MapInfo final
{
    int mApiVersion = 0;
    std::string mGame;
    std::string mPathBnd;
    int mPathId = 0;
    int mXGridSize = 0;
    int mXSize = 0;
    int mYGridSize = 0;
    int mYSize = 0;
};

class Model final
{
public:
    void LoadJson(const std::string& jsonFile);
    const MapInfo& GetMapInfo() const { return mMapInfo; }

    Camera* CameraAt(int x, int y) const
    {
        for (auto& cam : mCameras)
        {
            if (cam->mX == x && cam->mY == y)
            {
                return cam.get();
            }
        }
        return nullptr;
    }

    std::vector<UP_CollisionObject>& CollisionItems()
    {
        return mCollisions;
    }

    struct FoundType
    {
        Enum* mEnum = nullptr;
        BasicType* mBasicType = nullptr;
    };

    FoundType FindType(const std::string& toFind)
    {
        for (const auto& enumType : mEnums)
        {
            if (enumType->mName == toFind)
            {
                return { enumType.get(), nullptr };
            }
        }

        for (const auto& basicType : mBasicTypes)
        {
            if (basicType->mName == toFind)
            {
               return { nullptr, basicType.get() };
            }
        }

        return { nullptr, nullptr };
    }

private:
    std::vector<UP_ObjectProperty> ReadProperties(const ObjectStructure* pObjStructure, jsonxx::Object& properties);

    MapInfo mMapInfo;
    std::vector<UP_Camera> mCameras;
    std::vector<UP_CollisionObject> mCollisions;
    UP_ObjectStructure mCollisionStructure;

    std::vector<UP_Enum> mEnums;
    std::vector<UP_ObjectStructure> mObjectStructures;
    std::vector<UP_BasicType> mBasicTypes;
};
using UP_Model = std::unique_ptr<Model>;

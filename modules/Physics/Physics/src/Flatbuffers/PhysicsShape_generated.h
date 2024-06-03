// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PHYSICSSHAPE_DEEPSEAPHYSICS_H_
#define FLATBUFFERS_GENERATED_PHYSICSSHAPE_DEEPSEAPHYSICS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "PhysicsBox_generated.h"
#include "PhysicsCapsule_generated.h"
#include "PhysicsCommon_generated.h"
#include "PhysicsCone_generated.h"
#include "PhysicsConvexHull_generated.h"
#include "PhysicsCylinder_generated.h"
#include "PhysicsMesh_generated.h"
#include "PhysicsSphere_generated.h"

namespace DeepSeaPhysics {

struct ShapeRef;
struct ShapeRefBuilder;

struct ShapeInstance;
struct ShapeInstanceBuilder;

struct Shape;
struct ShapeBuilder;

enum class ShapeUnion : uint8_t {
  NONE = 0,
  Box = 1,
  Capsule = 2,
  Cone = 3,
  ConvexHull = 4,
  Cylinder = 5,
  Mesh = 6,
  Sphere = 7,
  ShapeRef = 8,
  MIN = NONE,
  MAX = ShapeRef
};

inline const ShapeUnion (&EnumValuesShapeUnion())[9] {
  static const ShapeUnion values[] = {
    ShapeUnion::NONE,
    ShapeUnion::Box,
    ShapeUnion::Capsule,
    ShapeUnion::Cone,
    ShapeUnion::ConvexHull,
    ShapeUnion::Cylinder,
    ShapeUnion::Mesh,
    ShapeUnion::Sphere,
    ShapeUnion::ShapeRef
  };
  return values;
}

inline const char * const *EnumNamesShapeUnion() {
  static const char * const names[10] = {
    "NONE",
    "Box",
    "Capsule",
    "Cone",
    "ConvexHull",
    "Cylinder",
    "Mesh",
    "Sphere",
    "ShapeRef",
    nullptr
  };
  return names;
}

inline const char *EnumNameShapeUnion(ShapeUnion e) {
  if (::flatbuffers::IsOutRange(e, ShapeUnion::NONE, ShapeUnion::ShapeRef)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesShapeUnion()[index];
}

template<typename T> struct ShapeUnionTraits {
  static const ShapeUnion enum_value = ShapeUnion::NONE;
};

template<> struct ShapeUnionTraits<DeepSeaPhysics::Box> {
  static const ShapeUnion enum_value = ShapeUnion::Box;
};

template<> struct ShapeUnionTraits<DeepSeaPhysics::Capsule> {
  static const ShapeUnion enum_value = ShapeUnion::Capsule;
};

template<> struct ShapeUnionTraits<DeepSeaPhysics::Cone> {
  static const ShapeUnion enum_value = ShapeUnion::Cone;
};

template<> struct ShapeUnionTraits<DeepSeaPhysics::ConvexHull> {
  static const ShapeUnion enum_value = ShapeUnion::ConvexHull;
};

template<> struct ShapeUnionTraits<DeepSeaPhysics::Cylinder> {
  static const ShapeUnion enum_value = ShapeUnion::Cylinder;
};

template<> struct ShapeUnionTraits<DeepSeaPhysics::Mesh> {
  static const ShapeUnion enum_value = ShapeUnion::Mesh;
};

template<> struct ShapeUnionTraits<DeepSeaPhysics::Sphere> {
  static const ShapeUnion enum_value = ShapeUnion::Sphere;
};

template<> struct ShapeUnionTraits<DeepSeaPhysics::ShapeRef> {
  static const ShapeUnion enum_value = ShapeUnion::ShapeRef;
};

bool VerifyShapeUnion(::flatbuffers::Verifier &verifier, const void *obj, ShapeUnion type);
bool VerifyShapeUnionVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<ShapeUnion> *types);

struct ShapeRef FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ShapeRefBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           verifier.EndTable();
  }
};

struct ShapeRefBuilder {
  typedef ShapeRef Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(ShapeRef::VT_NAME, name);
  }
  explicit ShapeRefBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ShapeRef> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ShapeRef>(end);
    fbb_.Required(o, ShapeRef::VT_NAME);
    return o;
  }
};

inline ::flatbuffers::Offset<ShapeRef> CreateShapeRef(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0) {
  ShapeRefBuilder builder_(_fbb);
  builder_.add_name(name);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ShapeRef> CreateShapeRefDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaPhysics::CreateShapeRef(
      _fbb,
      name__);
}

struct ShapeInstance FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ShapeInstanceBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SHAPE_TYPE = 4,
    VT_SHAPE = 6,
    VT_DENSITY = 8,
    VT_TRANSLATE = 10,
    VT_ROTATE = 12,
    VT_SCALE = 14,
    VT_MATERIAL = 16
  };
  DeepSeaPhysics::ShapeUnion shape_type() const {
    return static_cast<DeepSeaPhysics::ShapeUnion>(GetField<uint8_t>(VT_SHAPE_TYPE, 0));
  }
  const void *shape() const {
    return GetPointer<const void *>(VT_SHAPE);
  }
  template<typename T> const T *shape_as() const;
  const DeepSeaPhysics::Box *shape_as_Box() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Box ? static_cast<const DeepSeaPhysics::Box *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Capsule *shape_as_Capsule() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Capsule ? static_cast<const DeepSeaPhysics::Capsule *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Cone *shape_as_Cone() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Cone ? static_cast<const DeepSeaPhysics::Cone *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::ConvexHull *shape_as_ConvexHull() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::ConvexHull ? static_cast<const DeepSeaPhysics::ConvexHull *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Cylinder *shape_as_Cylinder() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Cylinder ? static_cast<const DeepSeaPhysics::Cylinder *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Mesh *shape_as_Mesh() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Mesh ? static_cast<const DeepSeaPhysics::Mesh *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Sphere *shape_as_Sphere() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Sphere ? static_cast<const DeepSeaPhysics::Sphere *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::ShapeRef *shape_as_ShapeRef() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::ShapeRef ? static_cast<const DeepSeaPhysics::ShapeRef *>(shape()) : nullptr;
  }
  float density() const {
    return GetField<float>(VT_DENSITY, 0.0f);
  }
  const DeepSeaPhysics::Vector3f *translate() const {
    return GetStruct<const DeepSeaPhysics::Vector3f *>(VT_TRANSLATE);
  }
  const DeepSeaPhysics::Quaternion4f *rotate() const {
    return GetStruct<const DeepSeaPhysics::Quaternion4f *>(VT_ROTATE);
  }
  const DeepSeaPhysics::Vector3f *scale() const {
    return GetStruct<const DeepSeaPhysics::Vector3f *>(VT_SCALE);
  }
  const DeepSeaPhysics::ShapePartMaterial *material() const {
    return GetStruct<const DeepSeaPhysics::ShapePartMaterial *>(VT_MATERIAL);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_SHAPE_TYPE, 1) &&
           VerifyOffsetRequired(verifier, VT_SHAPE) &&
           VerifyShapeUnion(verifier, shape(), shape_type()) &&
           VerifyField<float>(verifier, VT_DENSITY, 4) &&
           VerifyField<DeepSeaPhysics::Vector3f>(verifier, VT_TRANSLATE, 4) &&
           VerifyField<DeepSeaPhysics::Quaternion4f>(verifier, VT_ROTATE, 4) &&
           VerifyField<DeepSeaPhysics::Vector3f>(verifier, VT_SCALE, 4) &&
           VerifyField<DeepSeaPhysics::ShapePartMaterial>(verifier, VT_MATERIAL, 4) &&
           verifier.EndTable();
  }
};

template<> inline const DeepSeaPhysics::Box *ShapeInstance::shape_as<DeepSeaPhysics::Box>() const {
  return shape_as_Box();
}

template<> inline const DeepSeaPhysics::Capsule *ShapeInstance::shape_as<DeepSeaPhysics::Capsule>() const {
  return shape_as_Capsule();
}

template<> inline const DeepSeaPhysics::Cone *ShapeInstance::shape_as<DeepSeaPhysics::Cone>() const {
  return shape_as_Cone();
}

template<> inline const DeepSeaPhysics::ConvexHull *ShapeInstance::shape_as<DeepSeaPhysics::ConvexHull>() const {
  return shape_as_ConvexHull();
}

template<> inline const DeepSeaPhysics::Cylinder *ShapeInstance::shape_as<DeepSeaPhysics::Cylinder>() const {
  return shape_as_Cylinder();
}

template<> inline const DeepSeaPhysics::Mesh *ShapeInstance::shape_as<DeepSeaPhysics::Mesh>() const {
  return shape_as_Mesh();
}

template<> inline const DeepSeaPhysics::Sphere *ShapeInstance::shape_as<DeepSeaPhysics::Sphere>() const {
  return shape_as_Sphere();
}

template<> inline const DeepSeaPhysics::ShapeRef *ShapeInstance::shape_as<DeepSeaPhysics::ShapeRef>() const {
  return shape_as_ShapeRef();
}

struct ShapeInstanceBuilder {
  typedef ShapeInstance Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_shape_type(DeepSeaPhysics::ShapeUnion shape_type) {
    fbb_.AddElement<uint8_t>(ShapeInstance::VT_SHAPE_TYPE, static_cast<uint8_t>(shape_type), 0);
  }
  void add_shape(::flatbuffers::Offset<void> shape) {
    fbb_.AddOffset(ShapeInstance::VT_SHAPE, shape);
  }
  void add_density(float density) {
    fbb_.AddElement<float>(ShapeInstance::VT_DENSITY, density, 0.0f);
  }
  void add_translate(const DeepSeaPhysics::Vector3f *translate) {
    fbb_.AddStruct(ShapeInstance::VT_TRANSLATE, translate);
  }
  void add_rotate(const DeepSeaPhysics::Quaternion4f *rotate) {
    fbb_.AddStruct(ShapeInstance::VT_ROTATE, rotate);
  }
  void add_scale(const DeepSeaPhysics::Vector3f *scale) {
    fbb_.AddStruct(ShapeInstance::VT_SCALE, scale);
  }
  void add_material(const DeepSeaPhysics::ShapePartMaterial *material) {
    fbb_.AddStruct(ShapeInstance::VT_MATERIAL, material);
  }
  explicit ShapeInstanceBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ShapeInstance> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ShapeInstance>(end);
    fbb_.Required(o, ShapeInstance::VT_SHAPE);
    return o;
  }
};

inline ::flatbuffers::Offset<ShapeInstance> CreateShapeInstance(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaPhysics::ShapeUnion shape_type = DeepSeaPhysics::ShapeUnion::NONE,
    ::flatbuffers::Offset<void> shape = 0,
    float density = 0.0f,
    const DeepSeaPhysics::Vector3f *translate = nullptr,
    const DeepSeaPhysics::Quaternion4f *rotate = nullptr,
    const DeepSeaPhysics::Vector3f *scale = nullptr,
    const DeepSeaPhysics::ShapePartMaterial *material = nullptr) {
  ShapeInstanceBuilder builder_(_fbb);
  builder_.add_material(material);
  builder_.add_scale(scale);
  builder_.add_rotate(rotate);
  builder_.add_translate(translate);
  builder_.add_density(density);
  builder_.add_shape(shape);
  builder_.add_shape_type(shape_type);
  return builder_.Finish();
}

struct Shape FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ShapeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SHAPE_TYPE = 4,
    VT_SHAPE = 6
  };
  DeepSeaPhysics::ShapeUnion shape_type() const {
    return static_cast<DeepSeaPhysics::ShapeUnion>(GetField<uint8_t>(VT_SHAPE_TYPE, 0));
  }
  const void *shape() const {
    return GetPointer<const void *>(VT_SHAPE);
  }
  template<typename T> const T *shape_as() const;
  const DeepSeaPhysics::Box *shape_as_Box() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Box ? static_cast<const DeepSeaPhysics::Box *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Capsule *shape_as_Capsule() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Capsule ? static_cast<const DeepSeaPhysics::Capsule *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Cone *shape_as_Cone() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Cone ? static_cast<const DeepSeaPhysics::Cone *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::ConvexHull *shape_as_ConvexHull() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::ConvexHull ? static_cast<const DeepSeaPhysics::ConvexHull *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Cylinder *shape_as_Cylinder() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Cylinder ? static_cast<const DeepSeaPhysics::Cylinder *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Mesh *shape_as_Mesh() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Mesh ? static_cast<const DeepSeaPhysics::Mesh *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::Sphere *shape_as_Sphere() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::Sphere ? static_cast<const DeepSeaPhysics::Sphere *>(shape()) : nullptr;
  }
  const DeepSeaPhysics::ShapeRef *shape_as_ShapeRef() const {
    return shape_type() == DeepSeaPhysics::ShapeUnion::ShapeRef ? static_cast<const DeepSeaPhysics::ShapeRef *>(shape()) : nullptr;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_SHAPE_TYPE, 1) &&
           VerifyOffset(verifier, VT_SHAPE) &&
           VerifyShapeUnion(verifier, shape(), shape_type()) &&
           verifier.EndTable();
  }
};

template<> inline const DeepSeaPhysics::Box *Shape::shape_as<DeepSeaPhysics::Box>() const {
  return shape_as_Box();
}

template<> inline const DeepSeaPhysics::Capsule *Shape::shape_as<DeepSeaPhysics::Capsule>() const {
  return shape_as_Capsule();
}

template<> inline const DeepSeaPhysics::Cone *Shape::shape_as<DeepSeaPhysics::Cone>() const {
  return shape_as_Cone();
}

template<> inline const DeepSeaPhysics::ConvexHull *Shape::shape_as<DeepSeaPhysics::ConvexHull>() const {
  return shape_as_ConvexHull();
}

template<> inline const DeepSeaPhysics::Cylinder *Shape::shape_as<DeepSeaPhysics::Cylinder>() const {
  return shape_as_Cylinder();
}

template<> inline const DeepSeaPhysics::Mesh *Shape::shape_as<DeepSeaPhysics::Mesh>() const {
  return shape_as_Mesh();
}

template<> inline const DeepSeaPhysics::Sphere *Shape::shape_as<DeepSeaPhysics::Sphere>() const {
  return shape_as_Sphere();
}

template<> inline const DeepSeaPhysics::ShapeRef *Shape::shape_as<DeepSeaPhysics::ShapeRef>() const {
  return shape_as_ShapeRef();
}

struct ShapeBuilder {
  typedef Shape Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_shape_type(DeepSeaPhysics::ShapeUnion shape_type) {
    fbb_.AddElement<uint8_t>(Shape::VT_SHAPE_TYPE, static_cast<uint8_t>(shape_type), 0);
  }
  void add_shape(::flatbuffers::Offset<void> shape) {
    fbb_.AddOffset(Shape::VT_SHAPE, shape);
  }
  explicit ShapeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Shape> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Shape>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Shape> CreateShape(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaPhysics::ShapeUnion shape_type = DeepSeaPhysics::ShapeUnion::NONE,
    ::flatbuffers::Offset<void> shape = 0) {
  ShapeBuilder builder_(_fbb);
  builder_.add_shape(shape);
  builder_.add_shape_type(shape_type);
  return builder_.Finish();
}

inline bool VerifyShapeUnion(::flatbuffers::Verifier &verifier, const void *obj, ShapeUnion type) {
  switch (type) {
    case ShapeUnion::NONE: {
      return true;
    }
    case ShapeUnion::Box: {
      auto ptr = reinterpret_cast<const DeepSeaPhysics::Box *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ShapeUnion::Capsule: {
      auto ptr = reinterpret_cast<const DeepSeaPhysics::Capsule *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ShapeUnion::Cone: {
      auto ptr = reinterpret_cast<const DeepSeaPhysics::Cone *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ShapeUnion::ConvexHull: {
      auto ptr = reinterpret_cast<const DeepSeaPhysics::ConvexHull *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ShapeUnion::Cylinder: {
      auto ptr = reinterpret_cast<const DeepSeaPhysics::Cylinder *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ShapeUnion::Mesh: {
      auto ptr = reinterpret_cast<const DeepSeaPhysics::Mesh *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ShapeUnion::Sphere: {
      auto ptr = reinterpret_cast<const DeepSeaPhysics::Sphere *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ShapeUnion::ShapeRef: {
      auto ptr = reinterpret_cast<const DeepSeaPhysics::ShapeRef *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyShapeUnionVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<ShapeUnion> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (::flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyShapeUnion(
        verifier,  values->Get(i), types->GetEnum<ShapeUnion>(i))) {
      return false;
    }
  }
  return true;
}

inline const DeepSeaPhysics::Shape *GetShape(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaPhysics::Shape>(buf);
}

inline const DeepSeaPhysics::Shape *GetSizePrefixedShape(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaPhysics::Shape>(buf);
}

inline bool VerifyShapeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaPhysics::Shape>(nullptr);
}

inline bool VerifySizePrefixedShapeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaPhysics::Shape>(nullptr);
}

inline void FinishShapeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaPhysics::Shape> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedShapeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaPhysics::Shape> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaPhysics

#endif  // FLATBUFFERS_GENERATED_PHYSICSSHAPE_DEEPSEAPHYSICS_H_

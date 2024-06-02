// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PHYSICSCOMMON_DEEPSEAPHYSICS_H_
#define FLATBUFFERS_GENERATED_PHYSICSCOMMON_DEEPSEAPHYSICS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace DeepSeaPhysics {

struct Vector3f;

struct Quaternion4f;

struct ShapePartMaterial;

enum class Axis : uint8_t {
  X = 0,
  Y = 1,
  Z = 2,
  MIN = X,
  MAX = Z
};

inline const Axis (&EnumValuesAxis())[3] {
  static const Axis values[] = {
    Axis::X,
    Axis::Y,
    Axis::Z
  };
  return values;
}

inline const char * const *EnumNamesAxis() {
  static const char * const names[4] = {
    "X",
    "Y",
    "Z",
    nullptr
  };
  return names;
}

inline const char *EnumNameAxis(Axis e) {
  if (::flatbuffers::IsOutRange(e, Axis::X, Axis::Z)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesAxis()[index];
}

enum class MotionType : uint8_t {
  Static = 0,
  Kinematic = 1,
  Dynamic = 2,
  MIN = Static,
  MAX = Dynamic
};

inline const MotionType (&EnumValuesMotionType())[3] {
  static const MotionType values[] = {
    MotionType::Static,
    MotionType::Kinematic,
    MotionType::Dynamic
  };
  return values;
}

inline const char * const *EnumNamesMotionType() {
  static const char * const names[4] = {
    "Static",
    "Kinematic",
    "Dynamic",
    nullptr
  };
  return names;
}

inline const char *EnumNameMotionType(MotionType e) {
  if (::flatbuffers::IsOutRange(e, MotionType::Static, MotionType::Dynamic)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMotionType()[index];
}

enum class DOFMask : uint8_t {
  TransX = 0,
  TransY = 1,
  TransZ = 2,
  RotX = 3,
  RotY = 4,
  RotZ = 5,
  MIN = TransX,
  MAX = RotZ
};

inline const DOFMask (&EnumValuesDOFMask())[6] {
  static const DOFMask values[] = {
    DOFMask::TransX,
    DOFMask::TransY,
    DOFMask::TransZ,
    DOFMask::RotX,
    DOFMask::RotY,
    DOFMask::RotZ
  };
  return values;
}

inline const char * const *EnumNamesDOFMask() {
  static const char * const names[7] = {
    "TransX",
    "TransY",
    "TransZ",
    "RotX",
    "RotY",
    "RotZ",
    nullptr
  };
  return names;
}

inline const char *EnumNameDOFMask(DOFMask e) {
  if (::flatbuffers::IsOutRange(e, DOFMask::TransX, DOFMask::RotZ)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesDOFMask()[index];
}

enum class PhysicsLayer : uint8_t {
  StaticWorld = 0,
  Objects = 1,
  Projectiles = 2,
  MIN = StaticWorld,
  MAX = Projectiles
};

inline const PhysicsLayer (&EnumValuesPhysicsLayer())[3] {
  static const PhysicsLayer values[] = {
    PhysicsLayer::StaticWorld,
    PhysicsLayer::Objects,
    PhysicsLayer::Projectiles
  };
  return values;
}

inline const char * const *EnumNamesPhysicsLayer() {
  static const char * const names[4] = {
    "StaticWorld",
    "Objects",
    "Projectiles",
    nullptr
  };
  return names;
}

inline const char *EnumNamePhysicsLayer(PhysicsLayer e) {
  if (::flatbuffers::IsOutRange(e, PhysicsLayer::StaticWorld, PhysicsLayer::Projectiles)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesPhysicsLayer()[index];
}

enum class RigidBodyFlags : uint32_t {
  MutableMotionType = 1,
  MutableShape = 2,
  Scalable = 4,
  LinearCollision = 8,
  Sensor = 16,
  SensorDetectStatic = 32,
  AlwaysActive = 64,
  DisableGravity = 128,
  GyroscopicForces = 256,
  AllContacts = 512,
  CustomContactProperties = 1024,
  ContactCallbacks = 2048,
  NONE = 0,
  ANY = 4095
};
FLATBUFFERS_DEFINE_BITMASK_OPERATORS(RigidBodyFlags, uint32_t)

inline const RigidBodyFlags (&EnumValuesRigidBodyFlags())[12] {
  static const RigidBodyFlags values[] = {
    RigidBodyFlags::MutableMotionType,
    RigidBodyFlags::MutableShape,
    RigidBodyFlags::Scalable,
    RigidBodyFlags::LinearCollision,
    RigidBodyFlags::Sensor,
    RigidBodyFlags::SensorDetectStatic,
    RigidBodyFlags::AlwaysActive,
    RigidBodyFlags::DisableGravity,
    RigidBodyFlags::GyroscopicForces,
    RigidBodyFlags::AllContacts,
    RigidBodyFlags::CustomContactProperties,
    RigidBodyFlags::ContactCallbacks
  };
  return values;
}

inline const char *EnumNameRigidBodyFlags(RigidBodyFlags e) {
  switch (e) {
    case RigidBodyFlags::MutableMotionType: return "MutableMotionType";
    case RigidBodyFlags::MutableShape: return "MutableShape";
    case RigidBodyFlags::Scalable: return "Scalable";
    case RigidBodyFlags::LinearCollision: return "LinearCollision";
    case RigidBodyFlags::Sensor: return "Sensor";
    case RigidBodyFlags::SensorDetectStatic: return "SensorDetectStatic";
    case RigidBodyFlags::AlwaysActive: return "AlwaysActive";
    case RigidBodyFlags::DisableGravity: return "DisableGravity";
    case RigidBodyFlags::GyroscopicForces: return "GyroscopicForces";
    case RigidBodyFlags::AllContacts: return "AllContacts";
    case RigidBodyFlags::CustomContactProperties: return "CustomContactProperties";
    case RigidBodyFlags::ContactCallbacks: return "ContactCallbacks";
    default: return "";
  }
}

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Vector3f FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;
  float z_;

 public:
  Vector3f()
      : x_(0),
        y_(0),
        z_(0) {
  }
  Vector3f(float _x, float _y, float _z)
      : x_(::flatbuffers::EndianScalar(_x)),
        y_(::flatbuffers::EndianScalar(_y)),
        z_(::flatbuffers::EndianScalar(_z)) {
  }
  float x() const {
    return ::flatbuffers::EndianScalar(x_);
  }
  float y() const {
    return ::flatbuffers::EndianScalar(y_);
  }
  float z() const {
    return ::flatbuffers::EndianScalar(z_);
  }
};
FLATBUFFERS_STRUCT_END(Vector3f, 12);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Quaternion4f FLATBUFFERS_FINAL_CLASS {
 private:
  float i_;
  float j_;
  float k_;
  float r_;

 public:
  Quaternion4f()
      : i_(0),
        j_(0),
        k_(0),
        r_(0) {
  }
  Quaternion4f(float _i, float _j, float _k, float _r)
      : i_(::flatbuffers::EndianScalar(_i)),
        j_(::flatbuffers::EndianScalar(_j)),
        k_(::flatbuffers::EndianScalar(_k)),
        r_(::flatbuffers::EndianScalar(_r)) {
  }
  float i() const {
    return ::flatbuffers::EndianScalar(i_);
  }
  float j() const {
    return ::flatbuffers::EndianScalar(j_);
  }
  float k() const {
    return ::flatbuffers::EndianScalar(k_);
  }
  float r() const {
    return ::flatbuffers::EndianScalar(r_);
  }
};
FLATBUFFERS_STRUCT_END(Quaternion4f, 16);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) ShapePartMaterial FLATBUFFERS_FINAL_CLASS {
 private:
  float friction_;
  float restitution_;
  float hardness_;

 public:
  ShapePartMaterial()
      : friction_(0),
        restitution_(0),
        hardness_(0) {
  }
  ShapePartMaterial(float _friction, float _restitution, float _hardness)
      : friction_(::flatbuffers::EndianScalar(_friction)),
        restitution_(::flatbuffers::EndianScalar(_restitution)),
        hardness_(::flatbuffers::EndianScalar(_hardness)) {
  }
  float friction() const {
    return ::flatbuffers::EndianScalar(friction_);
  }
  float restitution() const {
    return ::flatbuffers::EndianScalar(restitution_);
  }
  float hardness() const {
    return ::flatbuffers::EndianScalar(hardness_);
  }
};
FLATBUFFERS_STRUCT_END(ShapePartMaterial, 12);

}  // namespace DeepSeaPhysics

#endif  // FLATBUFFERS_GENERATED_PHYSICSCOMMON_DEEPSEAPHYSICS_H_

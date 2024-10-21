// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PHYSICSCYLINDER_DEEPSEAPHYSICS_H_
#define FLATBUFFERS_GENERATED_PHYSICSCYLINDER_DEEPSEAPHYSICS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Physics/Flatbuffers/PhysicsCommon_generated.h"

namespace DeepSeaPhysics {

struct Cylinder;
struct CylinderBuilder;

struct Cylinder FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef CylinderBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_HALFHEIGHT = 4,
    VT_RADIUS = 6,
    VT_AXIS = 8,
    VT_CONVEXRADIUS = 10
  };
  float halfHeight() const {
    return GetField<float>(VT_HALFHEIGHT, 0.0f);
  }
  float radius() const {
    return GetField<float>(VT_RADIUS, 0.0f);
  }
  DeepSeaPhysics::Axis axis() const {
    return static_cast<DeepSeaPhysics::Axis>(GetField<uint8_t>(VT_AXIS, 0));
  }
  float convexRadius() const {
    return GetField<float>(VT_CONVEXRADIUS, -1.0f);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_HALFHEIGHT, 4) &&
           VerifyField<float>(verifier, VT_RADIUS, 4) &&
           VerifyField<uint8_t>(verifier, VT_AXIS, 1) &&
           VerifyField<float>(verifier, VT_CONVEXRADIUS, 4) &&
           verifier.EndTable();
  }
};

struct CylinderBuilder {
  typedef Cylinder Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_halfHeight(float halfHeight) {
    fbb_.AddElement<float>(Cylinder::VT_HALFHEIGHT, halfHeight, 0.0f);
  }
  void add_radius(float radius) {
    fbb_.AddElement<float>(Cylinder::VT_RADIUS, radius, 0.0f);
  }
  void add_axis(DeepSeaPhysics::Axis axis) {
    fbb_.AddElement<uint8_t>(Cylinder::VT_AXIS, static_cast<uint8_t>(axis), 0);
  }
  void add_convexRadius(float convexRadius) {
    fbb_.AddElement<float>(Cylinder::VT_CONVEXRADIUS, convexRadius, -1.0f);
  }
  explicit CylinderBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Cylinder> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Cylinder>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<Cylinder> CreateCylinder(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    float halfHeight = 0.0f,
    float radius = 0.0f,
    DeepSeaPhysics::Axis axis = DeepSeaPhysics::Axis::X,
    float convexRadius = -1.0f) {
  CylinderBuilder builder_(_fbb);
  builder_.add_convexRadius(convexRadius);
  builder_.add_radius(radius);
  builder_.add_halfHeight(halfHeight);
  builder_.add_axis(axis);
  return builder_.Finish();
}

}  // namespace DeepSeaPhysics

#endif  // FLATBUFFERS_GENERATED_PHYSICSCYLINDER_DEEPSEAPHYSICS_H_

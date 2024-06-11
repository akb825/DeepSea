// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SLIDERPHYSICSCONSTRAINT_DEEPSEAPHYSICS_H_
#define FLATBUFFERS_GENERATED_SLIDERPHYSICSCONSTRAINT_DEEPSEAPHYSICS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "PhysicsCommon_generated.h"

namespace DeepSeaPhysics {

struct SliderConstraint;
struct SliderConstraintBuilder;

struct SliderConstraint FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef SliderConstraintBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_FIRSTACTOR = 4,
    VT_FIRSTPOSITION = 6,
    VT_FIRSTROTATION = 8,
    VT_SECONDACTOR = 10,
    VT_SECONDPOSITION = 12,
    VT_SECONDROTATION = 14,
    VT_LIMITENABLED = 16,
    VT_MINDISTANCE = 18,
    VT_MAXDISTANCE = 20,
    VT_LIMITSTIFFNESS = 22,
    VT_LIMITDAMPING = 24,
    VT_MOTORTYPE = 26,
    VT_MOTORTARGET = 28,
    VT_MAXMOTORFORCE = 30
  };
  const ::flatbuffers::String *firstActor() const {
    return GetPointer<const ::flatbuffers::String *>(VT_FIRSTACTOR);
  }
  const DeepSeaPhysics::Vector3f *firstPosition() const {
    return GetStruct<const DeepSeaPhysics::Vector3f *>(VT_FIRSTPOSITION);
  }
  const DeepSeaPhysics::Quaternion4f *firstRotation() const {
    return GetStruct<const DeepSeaPhysics::Quaternion4f *>(VT_FIRSTROTATION);
  }
  const ::flatbuffers::String *secondActor() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SECONDACTOR);
  }
  const DeepSeaPhysics::Vector3f *secondPosition() const {
    return GetStruct<const DeepSeaPhysics::Vector3f *>(VT_SECONDPOSITION);
  }
  const DeepSeaPhysics::Quaternion4f *secondRotation() const {
    return GetStruct<const DeepSeaPhysics::Quaternion4f *>(VT_SECONDROTATION);
  }
  bool limitEnabled() const {
    return GetField<uint8_t>(VT_LIMITENABLED, 0) != 0;
  }
  float minDistance() const {
    return GetField<float>(VT_MINDISTANCE, 0.0f);
  }
  float maxDistance() const {
    return GetField<float>(VT_MAXDISTANCE, 0.0f);
  }
  float limitStiffness() const {
    return GetField<float>(VT_LIMITSTIFFNESS, 0.0f);
  }
  float limitDamping() const {
    return GetField<float>(VT_LIMITDAMPING, 0.0f);
  }
  DeepSeaPhysics::ConstraintMotorType motorType() const {
    return static_cast<DeepSeaPhysics::ConstraintMotorType>(GetField<uint8_t>(VT_MOTORTYPE, 0));
  }
  float motorTarget() const {
    return GetField<float>(VT_MOTORTARGET, 0.0f);
  }
  float maxMotorForce() const {
    return GetField<float>(VT_MAXMOTORFORCE, 0.0f);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_FIRSTACTOR) &&
           verifier.VerifyString(firstActor()) &&
           VerifyFieldRequired<DeepSeaPhysics::Vector3f>(verifier, VT_FIRSTPOSITION, 4) &&
           VerifyFieldRequired<DeepSeaPhysics::Quaternion4f>(verifier, VT_FIRSTROTATION, 4) &&
           VerifyOffsetRequired(verifier, VT_SECONDACTOR) &&
           verifier.VerifyString(secondActor()) &&
           VerifyFieldRequired<DeepSeaPhysics::Vector3f>(verifier, VT_SECONDPOSITION, 4) &&
           VerifyFieldRequired<DeepSeaPhysics::Quaternion4f>(verifier, VT_SECONDROTATION, 4) &&
           VerifyField<uint8_t>(verifier, VT_LIMITENABLED, 1) &&
           VerifyField<float>(verifier, VT_MINDISTANCE, 4) &&
           VerifyField<float>(verifier, VT_MAXDISTANCE, 4) &&
           VerifyField<float>(verifier, VT_LIMITSTIFFNESS, 4) &&
           VerifyField<float>(verifier, VT_LIMITDAMPING, 4) &&
           VerifyField<uint8_t>(verifier, VT_MOTORTYPE, 1) &&
           VerifyField<float>(verifier, VT_MOTORTARGET, 4) &&
           VerifyField<float>(verifier, VT_MAXMOTORFORCE, 4) &&
           verifier.EndTable();
  }
};

struct SliderConstraintBuilder {
  typedef SliderConstraint Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_firstActor(::flatbuffers::Offset<::flatbuffers::String> firstActor) {
    fbb_.AddOffset(SliderConstraint::VT_FIRSTACTOR, firstActor);
  }
  void add_firstPosition(const DeepSeaPhysics::Vector3f *firstPosition) {
    fbb_.AddStruct(SliderConstraint::VT_FIRSTPOSITION, firstPosition);
  }
  void add_firstRotation(const DeepSeaPhysics::Quaternion4f *firstRotation) {
    fbb_.AddStruct(SliderConstraint::VT_FIRSTROTATION, firstRotation);
  }
  void add_secondActor(::flatbuffers::Offset<::flatbuffers::String> secondActor) {
    fbb_.AddOffset(SliderConstraint::VT_SECONDACTOR, secondActor);
  }
  void add_secondPosition(const DeepSeaPhysics::Vector3f *secondPosition) {
    fbb_.AddStruct(SliderConstraint::VT_SECONDPOSITION, secondPosition);
  }
  void add_secondRotation(const DeepSeaPhysics::Quaternion4f *secondRotation) {
    fbb_.AddStruct(SliderConstraint::VT_SECONDROTATION, secondRotation);
  }
  void add_limitEnabled(bool limitEnabled) {
    fbb_.AddElement<uint8_t>(SliderConstraint::VT_LIMITENABLED, static_cast<uint8_t>(limitEnabled), 0);
  }
  void add_minDistance(float minDistance) {
    fbb_.AddElement<float>(SliderConstraint::VT_MINDISTANCE, minDistance, 0.0f);
  }
  void add_maxDistance(float maxDistance) {
    fbb_.AddElement<float>(SliderConstraint::VT_MAXDISTANCE, maxDistance, 0.0f);
  }
  void add_limitStiffness(float limitStiffness) {
    fbb_.AddElement<float>(SliderConstraint::VT_LIMITSTIFFNESS, limitStiffness, 0.0f);
  }
  void add_limitDamping(float limitDamping) {
    fbb_.AddElement<float>(SliderConstraint::VT_LIMITDAMPING, limitDamping, 0.0f);
  }
  void add_motorType(DeepSeaPhysics::ConstraintMotorType motorType) {
    fbb_.AddElement<uint8_t>(SliderConstraint::VT_MOTORTYPE, static_cast<uint8_t>(motorType), 0);
  }
  void add_motorTarget(float motorTarget) {
    fbb_.AddElement<float>(SliderConstraint::VT_MOTORTARGET, motorTarget, 0.0f);
  }
  void add_maxMotorForce(float maxMotorForce) {
    fbb_.AddElement<float>(SliderConstraint::VT_MAXMOTORFORCE, maxMotorForce, 0.0f);
  }
  explicit SliderConstraintBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<SliderConstraint> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<SliderConstraint>(end);
    fbb_.Required(o, SliderConstraint::VT_FIRSTACTOR);
    fbb_.Required(o, SliderConstraint::VT_FIRSTPOSITION);
    fbb_.Required(o, SliderConstraint::VT_FIRSTROTATION);
    fbb_.Required(o, SliderConstraint::VT_SECONDACTOR);
    fbb_.Required(o, SliderConstraint::VT_SECONDPOSITION);
    fbb_.Required(o, SliderConstraint::VT_SECONDROTATION);
    return o;
  }
};

inline ::flatbuffers::Offset<SliderConstraint> CreateSliderConstraint(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> firstActor = 0,
    const DeepSeaPhysics::Vector3f *firstPosition = nullptr,
    const DeepSeaPhysics::Quaternion4f *firstRotation = nullptr,
    ::flatbuffers::Offset<::flatbuffers::String> secondActor = 0,
    const DeepSeaPhysics::Vector3f *secondPosition = nullptr,
    const DeepSeaPhysics::Quaternion4f *secondRotation = nullptr,
    bool limitEnabled = false,
    float minDistance = 0.0f,
    float maxDistance = 0.0f,
    float limitStiffness = 0.0f,
    float limitDamping = 0.0f,
    DeepSeaPhysics::ConstraintMotorType motorType = DeepSeaPhysics::ConstraintMotorType::Disabled,
    float motorTarget = 0.0f,
    float maxMotorForce = 0.0f) {
  SliderConstraintBuilder builder_(_fbb);
  builder_.add_maxMotorForce(maxMotorForce);
  builder_.add_motorTarget(motorTarget);
  builder_.add_limitDamping(limitDamping);
  builder_.add_limitStiffness(limitStiffness);
  builder_.add_maxDistance(maxDistance);
  builder_.add_minDistance(minDistance);
  builder_.add_secondRotation(secondRotation);
  builder_.add_secondPosition(secondPosition);
  builder_.add_secondActor(secondActor);
  builder_.add_firstRotation(firstRotation);
  builder_.add_firstPosition(firstPosition);
  builder_.add_firstActor(firstActor);
  builder_.add_motorType(motorType);
  builder_.add_limitEnabled(limitEnabled);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<SliderConstraint> CreateSliderConstraintDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *firstActor = nullptr,
    const DeepSeaPhysics::Vector3f *firstPosition = nullptr,
    const DeepSeaPhysics::Quaternion4f *firstRotation = nullptr,
    const char *secondActor = nullptr,
    const DeepSeaPhysics::Vector3f *secondPosition = nullptr,
    const DeepSeaPhysics::Quaternion4f *secondRotation = nullptr,
    bool limitEnabled = false,
    float minDistance = 0.0f,
    float maxDistance = 0.0f,
    float limitStiffness = 0.0f,
    float limitDamping = 0.0f,
    DeepSeaPhysics::ConstraintMotorType motorType = DeepSeaPhysics::ConstraintMotorType::Disabled,
    float motorTarget = 0.0f,
    float maxMotorForce = 0.0f) {
  auto firstActor__ = firstActor ? _fbb.CreateString(firstActor) : 0;
  auto secondActor__ = secondActor ? _fbb.CreateString(secondActor) : 0;
  return DeepSeaPhysics::CreateSliderConstraint(
      _fbb,
      firstActor__,
      firstPosition,
      firstRotation,
      secondActor__,
      secondPosition,
      secondRotation,
      limitEnabled,
      minDistance,
      maxDistance,
      limitStiffness,
      limitDamping,
      motorType,
      motorTarget,
      maxMotorForce);
}

}  // namespace DeepSeaPhysics

#endif  // FLATBUFFERS_GENERATED_SLIDERPHYSICSCONSTRAINT_DEEPSEAPHYSICS_H_

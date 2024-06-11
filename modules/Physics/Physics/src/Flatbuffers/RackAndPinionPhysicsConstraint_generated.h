// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RACKANDPINIONPHYSICSCONSTRAINT_DEEPSEAPHYSICS_H_
#define FLATBUFFERS_GENERATED_RACKANDPINIONPHYSICSCONSTRAINT_DEEPSEAPHYSICS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "PhysicsCommon_generated.h"

namespace DeepSeaPhysics {

struct RackAndPinionConstraint;
struct RackAndPinionConstraintBuilder;

struct RackAndPinionConstraint FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RackAndPinionConstraintBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_RACKACTOR = 4,
    VT_RACKAXIS = 6,
    VT_RACKCONSTRAINT = 8,
    VT_PINIONACTOR = 10,
    VT_PINIONAXIS = 12,
    VT_PINIONCONSTRAINT = 14,
    VT_RATIO = 16
  };
  const ::flatbuffers::String *rackActor() const {
    return GetPointer<const ::flatbuffers::String *>(VT_RACKACTOR);
  }
  const DeepSeaPhysics::Vector3f *rackAxis() const {
    return GetStruct<const DeepSeaPhysics::Vector3f *>(VT_RACKAXIS);
  }
  const ::flatbuffers::String *rackConstraint() const {
    return GetPointer<const ::flatbuffers::String *>(VT_RACKCONSTRAINT);
  }
  const ::flatbuffers::String *pinionActor() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PINIONACTOR);
  }
  const DeepSeaPhysics::Vector3f *pinionAxis() const {
    return GetStruct<const DeepSeaPhysics::Vector3f *>(VT_PINIONAXIS);
  }
  const ::flatbuffers::String *pinionConstraint() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PINIONCONSTRAINT);
  }
  float ratio() const {
    return GetField<float>(VT_RATIO, 0.0f);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_RACKACTOR) &&
           verifier.VerifyString(rackActor()) &&
           VerifyFieldRequired<DeepSeaPhysics::Vector3f>(verifier, VT_RACKAXIS, 4) &&
           VerifyOffset(verifier, VT_RACKCONSTRAINT) &&
           verifier.VerifyString(rackConstraint()) &&
           VerifyOffsetRequired(verifier, VT_PINIONACTOR) &&
           verifier.VerifyString(pinionActor()) &&
           VerifyFieldRequired<DeepSeaPhysics::Vector3f>(verifier, VT_PINIONAXIS, 4) &&
           VerifyOffset(verifier, VT_PINIONCONSTRAINT) &&
           verifier.VerifyString(pinionConstraint()) &&
           VerifyField<float>(verifier, VT_RATIO, 4) &&
           verifier.EndTable();
  }
};

struct RackAndPinionConstraintBuilder {
  typedef RackAndPinionConstraint Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_rackActor(::flatbuffers::Offset<::flatbuffers::String> rackActor) {
    fbb_.AddOffset(RackAndPinionConstraint::VT_RACKACTOR, rackActor);
  }
  void add_rackAxis(const DeepSeaPhysics::Vector3f *rackAxis) {
    fbb_.AddStruct(RackAndPinionConstraint::VT_RACKAXIS, rackAxis);
  }
  void add_rackConstraint(::flatbuffers::Offset<::flatbuffers::String> rackConstraint) {
    fbb_.AddOffset(RackAndPinionConstraint::VT_RACKCONSTRAINT, rackConstraint);
  }
  void add_pinionActor(::flatbuffers::Offset<::flatbuffers::String> pinionActor) {
    fbb_.AddOffset(RackAndPinionConstraint::VT_PINIONACTOR, pinionActor);
  }
  void add_pinionAxis(const DeepSeaPhysics::Vector3f *pinionAxis) {
    fbb_.AddStruct(RackAndPinionConstraint::VT_PINIONAXIS, pinionAxis);
  }
  void add_pinionConstraint(::flatbuffers::Offset<::flatbuffers::String> pinionConstraint) {
    fbb_.AddOffset(RackAndPinionConstraint::VT_PINIONCONSTRAINT, pinionConstraint);
  }
  void add_ratio(float ratio) {
    fbb_.AddElement<float>(RackAndPinionConstraint::VT_RATIO, ratio, 0.0f);
  }
  explicit RackAndPinionConstraintBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<RackAndPinionConstraint> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<RackAndPinionConstraint>(end);
    fbb_.Required(o, RackAndPinionConstraint::VT_RACKACTOR);
    fbb_.Required(o, RackAndPinionConstraint::VT_RACKAXIS);
    fbb_.Required(o, RackAndPinionConstraint::VT_PINIONACTOR);
    fbb_.Required(o, RackAndPinionConstraint::VT_PINIONAXIS);
    return o;
  }
};

inline ::flatbuffers::Offset<RackAndPinionConstraint> CreateRackAndPinionConstraint(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> rackActor = 0,
    const DeepSeaPhysics::Vector3f *rackAxis = nullptr,
    ::flatbuffers::Offset<::flatbuffers::String> rackConstraint = 0,
    ::flatbuffers::Offset<::flatbuffers::String> pinionActor = 0,
    const DeepSeaPhysics::Vector3f *pinionAxis = nullptr,
    ::flatbuffers::Offset<::flatbuffers::String> pinionConstraint = 0,
    float ratio = 0.0f) {
  RackAndPinionConstraintBuilder builder_(_fbb);
  builder_.add_ratio(ratio);
  builder_.add_pinionConstraint(pinionConstraint);
  builder_.add_pinionAxis(pinionAxis);
  builder_.add_pinionActor(pinionActor);
  builder_.add_rackConstraint(rackConstraint);
  builder_.add_rackAxis(rackAxis);
  builder_.add_rackActor(rackActor);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<RackAndPinionConstraint> CreateRackAndPinionConstraintDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *rackActor = nullptr,
    const DeepSeaPhysics::Vector3f *rackAxis = nullptr,
    const char *rackConstraint = nullptr,
    const char *pinionActor = nullptr,
    const DeepSeaPhysics::Vector3f *pinionAxis = nullptr,
    const char *pinionConstraint = nullptr,
    float ratio = 0.0f) {
  auto rackActor__ = rackActor ? _fbb.CreateString(rackActor) : 0;
  auto rackConstraint__ = rackConstraint ? _fbb.CreateString(rackConstraint) : 0;
  auto pinionActor__ = pinionActor ? _fbb.CreateString(pinionActor) : 0;
  auto pinionConstraint__ = pinionConstraint ? _fbb.CreateString(pinionConstraint) : 0;
  return DeepSeaPhysics::CreateRackAndPinionConstraint(
      _fbb,
      rackActor__,
      rackAxis,
      rackConstraint__,
      pinionActor__,
      pinionAxis,
      pinionConstraint__,
      ratio);
}

}  // namespace DeepSeaPhysics

#endif  // FLATBUFFERS_GENERATED_RACKANDPINIONPHYSICSCONSTRAINT_DEEPSEAPHYSICS_H_

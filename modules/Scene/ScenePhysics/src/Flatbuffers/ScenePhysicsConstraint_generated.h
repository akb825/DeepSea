// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENEPHYSICSCONSTRAINT_DEEPSEASCENEPHYSICS_H_
#define FLATBUFFERS_GENERATED_SCENEPHYSICSCONSTRAINT_DEEPSEASCENEPHYSICS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace DeepSeaScenePhysics {

struct PhysicsConstraint;
struct PhysicsConstraintBuilder;

struct PhysicsConstraint FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef PhysicsConstraintBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CONSTRAINT = 4,
    VT_FIRSTRIGIDBODYINSTANCE = 6,
    VT_FIRSTCONNECTEDCONSTRAINTINSTANCE = 8,
    VT_SECONDRIGIDBODYINSTANCE = 10,
    VT_SECONDCONNECTEDCONSTRAINTINSTANCE = 12
  };
  const ::flatbuffers::Vector<uint8_t> *constraint() const {
    return GetPointer<const ::flatbuffers::Vector<uint8_t> *>(VT_CONSTRAINT);
  }
  const ::flatbuffers::String *firstRigidBodyInstance() const {
    return GetPointer<const ::flatbuffers::String *>(VT_FIRSTRIGIDBODYINSTANCE);
  }
  const ::flatbuffers::String *firstConnectedConstraintInstance() const {
    return GetPointer<const ::flatbuffers::String *>(VT_FIRSTCONNECTEDCONSTRAINTINSTANCE);
  }
  const ::flatbuffers::String *secondRigidBodyInstance() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SECONDRIGIDBODYINSTANCE);
  }
  const ::flatbuffers::String *secondConnectedConstraintInstance() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SECONDCONNECTEDCONSTRAINTINSTANCE);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_CONSTRAINT) &&
           verifier.VerifyVector(constraint()) &&
           VerifyOffset(verifier, VT_FIRSTRIGIDBODYINSTANCE) &&
           verifier.VerifyString(firstRigidBodyInstance()) &&
           VerifyOffset(verifier, VT_FIRSTCONNECTEDCONSTRAINTINSTANCE) &&
           verifier.VerifyString(firstConnectedConstraintInstance()) &&
           VerifyOffset(verifier, VT_SECONDRIGIDBODYINSTANCE) &&
           verifier.VerifyString(secondRigidBodyInstance()) &&
           VerifyOffset(verifier, VT_SECONDCONNECTEDCONSTRAINTINSTANCE) &&
           verifier.VerifyString(secondConnectedConstraintInstance()) &&
           verifier.EndTable();
  }
};

struct PhysicsConstraintBuilder {
  typedef PhysicsConstraint Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_constraint(::flatbuffers::Offset<::flatbuffers::Vector<uint8_t>> constraint) {
    fbb_.AddOffset(PhysicsConstraint::VT_CONSTRAINT, constraint);
  }
  void add_firstRigidBodyInstance(::flatbuffers::Offset<::flatbuffers::String> firstRigidBodyInstance) {
    fbb_.AddOffset(PhysicsConstraint::VT_FIRSTRIGIDBODYINSTANCE, firstRigidBodyInstance);
  }
  void add_firstConnectedConstraintInstance(::flatbuffers::Offset<::flatbuffers::String> firstConnectedConstraintInstance) {
    fbb_.AddOffset(PhysicsConstraint::VT_FIRSTCONNECTEDCONSTRAINTINSTANCE, firstConnectedConstraintInstance);
  }
  void add_secondRigidBodyInstance(::flatbuffers::Offset<::flatbuffers::String> secondRigidBodyInstance) {
    fbb_.AddOffset(PhysicsConstraint::VT_SECONDRIGIDBODYINSTANCE, secondRigidBodyInstance);
  }
  void add_secondConnectedConstraintInstance(::flatbuffers::Offset<::flatbuffers::String> secondConnectedConstraintInstance) {
    fbb_.AddOffset(PhysicsConstraint::VT_SECONDCONNECTEDCONSTRAINTINSTANCE, secondConnectedConstraintInstance);
  }
  explicit PhysicsConstraintBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<PhysicsConstraint> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<PhysicsConstraint>(end);
    fbb_.Required(o, PhysicsConstraint::VT_CONSTRAINT);
    return o;
  }
};

inline ::flatbuffers::Offset<PhysicsConstraint> CreatePhysicsConstraint(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<uint8_t>> constraint = 0,
    ::flatbuffers::Offset<::flatbuffers::String> firstRigidBodyInstance = 0,
    ::flatbuffers::Offset<::flatbuffers::String> firstConnectedConstraintInstance = 0,
    ::flatbuffers::Offset<::flatbuffers::String> secondRigidBodyInstance = 0,
    ::flatbuffers::Offset<::flatbuffers::String> secondConnectedConstraintInstance = 0) {
  PhysicsConstraintBuilder builder_(_fbb);
  builder_.add_secondConnectedConstraintInstance(secondConnectedConstraintInstance);
  builder_.add_secondRigidBodyInstance(secondRigidBodyInstance);
  builder_.add_firstConnectedConstraintInstance(firstConnectedConstraintInstance);
  builder_.add_firstRigidBodyInstance(firstRigidBodyInstance);
  builder_.add_constraint(constraint);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<PhysicsConstraint> CreatePhysicsConstraintDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint8_t> *constraint = nullptr,
    const char *firstRigidBodyInstance = nullptr,
    const char *firstConnectedConstraintInstance = nullptr,
    const char *secondRigidBodyInstance = nullptr,
    const char *secondConnectedConstraintInstance = nullptr) {
  auto constraint__ = constraint ? _fbb.CreateVector<uint8_t>(*constraint) : 0;
  auto firstRigidBodyInstance__ = firstRigidBodyInstance ? _fbb.CreateString(firstRigidBodyInstance) : 0;
  auto firstConnectedConstraintInstance__ = firstConnectedConstraintInstance ? _fbb.CreateString(firstConnectedConstraintInstance) : 0;
  auto secondRigidBodyInstance__ = secondRigidBodyInstance ? _fbb.CreateString(secondRigidBodyInstance) : 0;
  auto secondConnectedConstraintInstance__ = secondConnectedConstraintInstance ? _fbb.CreateString(secondConnectedConstraintInstance) : 0;
  return DeepSeaScenePhysics::CreatePhysicsConstraint(
      _fbb,
      constraint__,
      firstRigidBodyInstance__,
      firstConnectedConstraintInstance__,
      secondRigidBodyInstance__,
      secondConnectedConstraintInstance__);
}

inline const DeepSeaScenePhysics::PhysicsConstraint *GetPhysicsConstraint(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaScenePhysics::PhysicsConstraint>(buf);
}

inline const DeepSeaScenePhysics::PhysicsConstraint *GetSizePrefixedPhysicsConstraint(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaScenePhysics::PhysicsConstraint>(buf);
}

inline bool VerifyPhysicsConstraintBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScenePhysics::PhysicsConstraint>(nullptr);
}

inline bool VerifySizePrefixedPhysicsConstraintBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScenePhysics::PhysicsConstraint>(nullptr);
}

inline void FinishPhysicsConstraintBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScenePhysics::PhysicsConstraint> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedPhysicsConstraintBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScenePhysics::PhysicsConstraint> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScenePhysics

#endif  // FLATBUFFERS_GENERATED_SCENEPHYSICSCONSTRAINT_DEEPSEASCENEPHYSICS_H_

// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RIGIDBODYTEMPLATE_DEEPSEAPHYSICS_H_
#define FLATBUFFERS_GENERATED_RIGIDBODYTEMPLATE_DEEPSEAPHYSICS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "PhysicsCommon_generated.h"
#include "PhysicsShape_generated.h"

namespace DeepSeaPhysics {

struct RigidBodyTemplate;
struct RigidBodyTemplateBuilder;

struct RigidBodyTemplate FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RigidBodyTemplateBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_FLAGS = 4,
    VT_MOTIONTYPE = 6,
    VT_DOFMASK = 8,
    VT_LAYER = 10,
    VT_COLLISIONGROUP = 12,
    VT_FRICTION = 14,
    VT_RESTITUTION = 16,
    VT_HARDNESS = 18,
    VT_LINEARDAMPING = 20,
    VT_ANGULARDAMPING = 22,
    VT_MAXLINEARVELOCITY = 24,
    VT_MAXANGULARVELOCITY = 26,
    VT_SHAPES = 28
  };
  DeepSeaPhysics::RigidBodyFlags flags() const {
    return static_cast<DeepSeaPhysics::RigidBodyFlags>(GetField<uint32_t>(VT_FLAGS, 0));
  }
  DeepSeaPhysics::MotionType motionType() const {
    return static_cast<DeepSeaPhysics::MotionType>(GetField<uint8_t>(VT_MOTIONTYPE, 0));
  }
  DeepSeaPhysics::DOFMask dofMask() const {
    return static_cast<DeepSeaPhysics::DOFMask>(GetField<uint8_t>(VT_DOFMASK, 0));
  }
  DeepSeaPhysics::PhysicsLayer layer() const {
    return static_cast<DeepSeaPhysics::PhysicsLayer>(GetField<uint8_t>(VT_LAYER, 0));
  }
  uint64_t collisionGroup() const {
    return GetField<uint64_t>(VT_COLLISIONGROUP, 0);
  }
  float friction() const {
    return GetField<float>(VT_FRICTION, 0.0f);
  }
  float restitution() const {
    return GetField<float>(VT_RESTITUTION, 0.0f);
  }
  float hardness() const {
    return GetField<float>(VT_HARDNESS, 0.0f);
  }
  float linearDamping() const {
    return GetField<float>(VT_LINEARDAMPING, -1.0f);
  }
  float angularDamping() const {
    return GetField<float>(VT_ANGULARDAMPING, -1.0f);
  }
  float maxLinearVelocity() const {
    return GetField<float>(VT_MAXLINEARVELOCITY, 0.0f);
  }
  float maxAngularVelocity() const {
    return GetField<float>(VT_MAXANGULARVELOCITY, 0.0f);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaPhysics::ShapeInstance>> *shapes() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaPhysics::ShapeInstance>> *>(VT_SHAPES);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_FLAGS, 4) &&
           VerifyField<uint8_t>(verifier, VT_MOTIONTYPE, 1) &&
           VerifyField<uint8_t>(verifier, VT_DOFMASK, 1) &&
           VerifyField<uint8_t>(verifier, VT_LAYER, 1) &&
           VerifyField<uint64_t>(verifier, VT_COLLISIONGROUP, 8) &&
           VerifyField<float>(verifier, VT_FRICTION, 4) &&
           VerifyField<float>(verifier, VT_RESTITUTION, 4) &&
           VerifyField<float>(verifier, VT_HARDNESS, 4) &&
           VerifyField<float>(verifier, VT_LINEARDAMPING, 4) &&
           VerifyField<float>(verifier, VT_ANGULARDAMPING, 4) &&
           VerifyField<float>(verifier, VT_MAXLINEARVELOCITY, 4) &&
           VerifyField<float>(verifier, VT_MAXANGULARVELOCITY, 4) &&
           VerifyOffset(verifier, VT_SHAPES) &&
           verifier.VerifyVector(shapes()) &&
           verifier.VerifyVectorOfTables(shapes()) &&
           verifier.EndTable();
  }
};

struct RigidBodyTemplateBuilder {
  typedef RigidBodyTemplate Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_flags(DeepSeaPhysics::RigidBodyFlags flags) {
    fbb_.AddElement<uint32_t>(RigidBodyTemplate::VT_FLAGS, static_cast<uint32_t>(flags), 0);
  }
  void add_motionType(DeepSeaPhysics::MotionType motionType) {
    fbb_.AddElement<uint8_t>(RigidBodyTemplate::VT_MOTIONTYPE, static_cast<uint8_t>(motionType), 0);
  }
  void add_dofMask(DeepSeaPhysics::DOFMask dofMask) {
    fbb_.AddElement<uint8_t>(RigidBodyTemplate::VT_DOFMASK, static_cast<uint8_t>(dofMask), 0);
  }
  void add_layer(DeepSeaPhysics::PhysicsLayer layer) {
    fbb_.AddElement<uint8_t>(RigidBodyTemplate::VT_LAYER, static_cast<uint8_t>(layer), 0);
  }
  void add_collisionGroup(uint64_t collisionGroup) {
    fbb_.AddElement<uint64_t>(RigidBodyTemplate::VT_COLLISIONGROUP, collisionGroup, 0);
  }
  void add_friction(float friction) {
    fbb_.AddElement<float>(RigidBodyTemplate::VT_FRICTION, friction, 0.0f);
  }
  void add_restitution(float restitution) {
    fbb_.AddElement<float>(RigidBodyTemplate::VT_RESTITUTION, restitution, 0.0f);
  }
  void add_hardness(float hardness) {
    fbb_.AddElement<float>(RigidBodyTemplate::VT_HARDNESS, hardness, 0.0f);
  }
  void add_linearDamping(float linearDamping) {
    fbb_.AddElement<float>(RigidBodyTemplate::VT_LINEARDAMPING, linearDamping, -1.0f);
  }
  void add_angularDamping(float angularDamping) {
    fbb_.AddElement<float>(RigidBodyTemplate::VT_ANGULARDAMPING, angularDamping, -1.0f);
  }
  void add_maxLinearVelocity(float maxLinearVelocity) {
    fbb_.AddElement<float>(RigidBodyTemplate::VT_MAXLINEARVELOCITY, maxLinearVelocity, 0.0f);
  }
  void add_maxAngularVelocity(float maxAngularVelocity) {
    fbb_.AddElement<float>(RigidBodyTemplate::VT_MAXANGULARVELOCITY, maxAngularVelocity, 0.0f);
  }
  void add_shapes(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaPhysics::ShapeInstance>>> shapes) {
    fbb_.AddOffset(RigidBodyTemplate::VT_SHAPES, shapes);
  }
  explicit RigidBodyTemplateBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<RigidBodyTemplate> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<RigidBodyTemplate>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<RigidBodyTemplate> CreateRigidBodyTemplate(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaPhysics::RigidBodyFlags flags = static_cast<DeepSeaPhysics::RigidBodyFlags>(0),
    DeepSeaPhysics::MotionType motionType = DeepSeaPhysics::MotionType::Static,
    DeepSeaPhysics::DOFMask dofMask = DeepSeaPhysics::DOFMask::TransX,
    DeepSeaPhysics::PhysicsLayer layer = DeepSeaPhysics::PhysicsLayer::StaticWorld,
    uint64_t collisionGroup = 0,
    float friction = 0.0f,
    float restitution = 0.0f,
    float hardness = 0.0f,
    float linearDamping = -1.0f,
    float angularDamping = -1.0f,
    float maxLinearVelocity = 0.0f,
    float maxAngularVelocity = 0.0f,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaPhysics::ShapeInstance>>> shapes = 0) {
  RigidBodyTemplateBuilder builder_(_fbb);
  builder_.add_collisionGroup(collisionGroup);
  builder_.add_shapes(shapes);
  builder_.add_maxAngularVelocity(maxAngularVelocity);
  builder_.add_maxLinearVelocity(maxLinearVelocity);
  builder_.add_angularDamping(angularDamping);
  builder_.add_linearDamping(linearDamping);
  builder_.add_hardness(hardness);
  builder_.add_restitution(restitution);
  builder_.add_friction(friction);
  builder_.add_flags(flags);
  builder_.add_layer(layer);
  builder_.add_dofMask(dofMask);
  builder_.add_motionType(motionType);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<RigidBodyTemplate> CreateRigidBodyTemplateDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaPhysics::RigidBodyFlags flags = static_cast<DeepSeaPhysics::RigidBodyFlags>(0),
    DeepSeaPhysics::MotionType motionType = DeepSeaPhysics::MotionType::Static,
    DeepSeaPhysics::DOFMask dofMask = DeepSeaPhysics::DOFMask::TransX,
    DeepSeaPhysics::PhysicsLayer layer = DeepSeaPhysics::PhysicsLayer::StaticWorld,
    uint64_t collisionGroup = 0,
    float friction = 0.0f,
    float restitution = 0.0f,
    float hardness = 0.0f,
    float linearDamping = -1.0f,
    float angularDamping = -1.0f,
    float maxLinearVelocity = 0.0f,
    float maxAngularVelocity = 0.0f,
    const std::vector<::flatbuffers::Offset<DeepSeaPhysics::ShapeInstance>> *shapes = nullptr) {
  auto shapes__ = shapes ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaPhysics::ShapeInstance>>(*shapes) : 0;
  return DeepSeaPhysics::CreateRigidBodyTemplate(
      _fbb,
      flags,
      motionType,
      dofMask,
      layer,
      collisionGroup,
      friction,
      restitution,
      hardness,
      linearDamping,
      angularDamping,
      maxLinearVelocity,
      maxAngularVelocity,
      shapes__);
}

inline const DeepSeaPhysics::RigidBodyTemplate *GetRigidBodyTemplate(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaPhysics::RigidBodyTemplate>(buf);
}

inline const DeepSeaPhysics::RigidBodyTemplate *GetSizePrefixedRigidBodyTemplate(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaPhysics::RigidBodyTemplate>(buf);
}

inline bool VerifyRigidBodyTemplateBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaPhysics::RigidBodyTemplate>(nullptr);
}

inline bool VerifySizePrefixedRigidBodyTemplateBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaPhysics::RigidBodyTemplate>(nullptr);
}

inline void FinishRigidBodyTemplateBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaPhysics::RigidBodyTemplate> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedRigidBodyTemplateBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaPhysics::RigidBodyTemplate> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaPhysics

#endif  // FLATBUFFERS_GENERATED_RIGIDBODYTEMPLATE_DEEPSEAPHYSICS_H_
// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENERIGIDBODYGROUPNODE_DEEPSEASCENEPHYSICS_H_
#define FLATBUFFERS_GENERATED_SCENERIGIDBODYGROUPNODE_DEEPSEASCENEPHYSICS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Physics/Flatbuffers/PhysicsCommon_generated.h"

namespace DeepSeaScenePhysics {

struct RigidBodyGroupNode;
struct RigidBodyGroupNodeBuilder;

struct RigidBodyGroupNode FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RigidBodyGroupNodeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MOTIONTYPE = 4,
    VT_RIGIDBODYTEMPLATES = 6,
    VT_CONSTRAINTS = 8,
    VT_ITEMLISTS = 10
  };
  DeepSeaPhysics::MotionType motionType() const {
    return static_cast<DeepSeaPhysics::MotionType>(GetField<uint8_t>(VT_MOTIONTYPE, 0));
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *rigidBodyTemplates() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *>(VT_RIGIDBODYTEMPLATES);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *constraints() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *>(VT_CONSTRAINTS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *itemLists() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *>(VT_ITEMLISTS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_MOTIONTYPE, 1) &&
           VerifyOffset(verifier, VT_RIGIDBODYTEMPLATES) &&
           verifier.VerifyVector(rigidBodyTemplates()) &&
           verifier.VerifyVectorOfStrings(rigidBodyTemplates()) &&
           VerifyOffset(verifier, VT_CONSTRAINTS) &&
           verifier.VerifyVector(constraints()) &&
           verifier.VerifyVectorOfStrings(constraints()) &&
           VerifyOffset(verifier, VT_ITEMLISTS) &&
           verifier.VerifyVector(itemLists()) &&
           verifier.VerifyVectorOfStrings(itemLists()) &&
           verifier.EndTable();
  }
};

struct RigidBodyGroupNodeBuilder {
  typedef RigidBodyGroupNode Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_motionType(DeepSeaPhysics::MotionType motionType) {
    fbb_.AddElement<uint8_t>(RigidBodyGroupNode::VT_MOTIONTYPE, static_cast<uint8_t>(motionType), 0);
  }
  void add_rigidBodyTemplates(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> rigidBodyTemplates) {
    fbb_.AddOffset(RigidBodyGroupNode::VT_RIGIDBODYTEMPLATES, rigidBodyTemplates);
  }
  void add_constraints(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> constraints) {
    fbb_.AddOffset(RigidBodyGroupNode::VT_CONSTRAINTS, constraints);
  }
  void add_itemLists(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> itemLists) {
    fbb_.AddOffset(RigidBodyGroupNode::VT_ITEMLISTS, itemLists);
  }
  explicit RigidBodyGroupNodeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<RigidBodyGroupNode> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<RigidBodyGroupNode>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<RigidBodyGroupNode> CreateRigidBodyGroupNode(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaPhysics::MotionType motionType = DeepSeaPhysics::MotionType::Static,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> rigidBodyTemplates = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> constraints = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> itemLists = 0) {
  RigidBodyGroupNodeBuilder builder_(_fbb);
  builder_.add_itemLists(itemLists);
  builder_.add_constraints(constraints);
  builder_.add_rigidBodyTemplates(rigidBodyTemplates);
  builder_.add_motionType(motionType);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<RigidBodyGroupNode> CreateRigidBodyGroupNodeDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaPhysics::MotionType motionType = DeepSeaPhysics::MotionType::Static,
    const std::vector<::flatbuffers::Offset<::flatbuffers::String>> *rigidBodyTemplates = nullptr,
    const std::vector<::flatbuffers::Offset<::flatbuffers::String>> *constraints = nullptr,
    const std::vector<::flatbuffers::Offset<::flatbuffers::String>> *itemLists = nullptr) {
  auto rigidBodyTemplates__ = rigidBodyTemplates ? _fbb.CreateVector<::flatbuffers::Offset<::flatbuffers::String>>(*rigidBodyTemplates) : 0;
  auto constraints__ = constraints ? _fbb.CreateVector<::flatbuffers::Offset<::flatbuffers::String>>(*constraints) : 0;
  auto itemLists__ = itemLists ? _fbb.CreateVector<::flatbuffers::Offset<::flatbuffers::String>>(*itemLists) : 0;
  return DeepSeaScenePhysics::CreateRigidBodyGroupNode(
      _fbb,
      motionType,
      rigidBodyTemplates__,
      constraints__,
      itemLists__);
}

inline const DeepSeaScenePhysics::RigidBodyGroupNode *GetRigidBodyGroupNode(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaScenePhysics::RigidBodyGroupNode>(buf);
}

inline const DeepSeaScenePhysics::RigidBodyGroupNode *GetSizePrefixedRigidBodyGroupNode(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaScenePhysics::RigidBodyGroupNode>(buf);
}

inline bool VerifyRigidBodyGroupNodeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScenePhysics::RigidBodyGroupNode>(nullptr);
}

inline bool VerifySizePrefixedRigidBodyGroupNodeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScenePhysics::RigidBodyGroupNode>(nullptr);
}

inline void FinishRigidBodyGroupNodeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScenePhysics::RigidBodyGroupNode> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedRigidBodyGroupNodeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScenePhysics::RigidBodyGroupNode> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScenePhysics

#endif  // FLATBUFFERS_GENERATED_SCENERIGIDBODYGROUPNODE_DEEPSEASCENEPHYSICS_H_
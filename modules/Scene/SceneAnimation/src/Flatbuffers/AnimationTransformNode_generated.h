// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_ANIMATIONTRANSFORMNODE_DEEPSEASCENEANIMATION_H_
#define FLATBUFFERS_GENERATED_ANIMATIONTRANSFORMNODE_DEEPSEASCENEANIMATION_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 3,
             "Non-compatible flatbuffers version included");

namespace DeepSeaSceneAnimation {

struct AnimationTransformNode;
struct AnimationTransformNodeBuilder;

struct AnimationTransformNode FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef AnimationTransformNodeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ANIMATINNODE = 4,
    VT_ITEMLISTS = 6
  };
  const ::flatbuffers::String *animatinNode() const {
    return GetPointer<const ::flatbuffers::String *>(VT_ANIMATINNODE);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *itemLists() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *>(VT_ITEMLISTS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_ANIMATINNODE) &&
           verifier.VerifyString(animatinNode()) &&
           VerifyOffset(verifier, VT_ITEMLISTS) &&
           verifier.VerifyVector(itemLists()) &&
           verifier.VerifyVectorOfStrings(itemLists()) &&
           verifier.EndTable();
  }
};

struct AnimationTransformNodeBuilder {
  typedef AnimationTransformNode Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_animatinNode(::flatbuffers::Offset<::flatbuffers::String> animatinNode) {
    fbb_.AddOffset(AnimationTransformNode::VT_ANIMATINNODE, animatinNode);
  }
  void add_itemLists(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> itemLists) {
    fbb_.AddOffset(AnimationTransformNode::VT_ITEMLISTS, itemLists);
  }
  explicit AnimationTransformNodeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<AnimationTransformNode> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<AnimationTransformNode>(end);
    fbb_.Required(o, AnimationTransformNode::VT_ANIMATINNODE);
    return o;
  }
};

inline ::flatbuffers::Offset<AnimationTransformNode> CreateAnimationTransformNode(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> animatinNode = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> itemLists = 0) {
  AnimationTransformNodeBuilder builder_(_fbb);
  builder_.add_itemLists(itemLists);
  builder_.add_animatinNode(animatinNode);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<AnimationTransformNode> CreateAnimationTransformNodeDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *animatinNode = nullptr,
    const std::vector<::flatbuffers::Offset<::flatbuffers::String>> *itemLists = nullptr) {
  auto animatinNode__ = animatinNode ? _fbb.CreateString(animatinNode) : 0;
  auto itemLists__ = itemLists ? _fbb.CreateVector<::flatbuffers::Offset<::flatbuffers::String>>(*itemLists) : 0;
  return DeepSeaSceneAnimation::CreateAnimationTransformNode(
      _fbb,
      animatinNode__,
      itemLists__);
}

inline const DeepSeaSceneAnimation::AnimationTransformNode *GetAnimationTransformNode(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneAnimation::AnimationTransformNode>(buf);
}

inline const DeepSeaSceneAnimation::AnimationTransformNode *GetSizePrefixedAnimationTransformNode(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneAnimation::AnimationTransformNode>(buf);
}

inline bool VerifyAnimationTransformNodeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneAnimation::AnimationTransformNode>(nullptr);
}

inline bool VerifySizePrefixedAnimationTransformNodeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneAnimation::AnimationTransformNode>(nullptr);
}

inline void FinishAnimationTransformNodeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneAnimation::AnimationTransformNode> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedAnimationTransformNodeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneAnimation::AnimationTransformNode> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneAnimation

#endif  // FLATBUFFERS_GENERATED_ANIMATIONTRANSFORMNODE_DEEPSEASCENEANIMATION_H_

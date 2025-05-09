// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_HANDOFFNODE_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_HANDOFFNODE_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct HandoffNode;
struct HandoffNodeBuilder;

struct HandoffNode FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef HandoffNodeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TRANSITIONTIME = 4,
    VT_CHILDREN = 6,
    VT_ITEMLISTS = 8
  };
  float transitionTime() const {
    return GetField<float>(VT_TRANSITIONTIME, 0.0f);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *children() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *>(VT_CHILDREN);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *itemLists() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *>(VT_ITEMLISTS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_TRANSITIONTIME, 4) &&
           VerifyOffset(verifier, VT_CHILDREN) &&
           verifier.VerifyVector(children()) &&
           verifier.VerifyVectorOfTables(children()) &&
           VerifyOffset(verifier, VT_ITEMLISTS) &&
           verifier.VerifyVector(itemLists()) &&
           verifier.VerifyVectorOfStrings(itemLists()) &&
           verifier.EndTable();
  }
};

struct HandoffNodeBuilder {
  typedef HandoffNode Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_transitionTime(float transitionTime) {
    fbb_.AddElement<float>(HandoffNode::VT_TRANSITIONTIME, transitionTime, 0.0f);
  }
  void add_children(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> children) {
    fbb_.AddOffset(HandoffNode::VT_CHILDREN, children);
  }
  void add_itemLists(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> itemLists) {
    fbb_.AddOffset(HandoffNode::VT_ITEMLISTS, itemLists);
  }
  explicit HandoffNodeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<HandoffNode> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<HandoffNode>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<HandoffNode> CreateHandoffNode(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    float transitionTime = 0.0f,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> children = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> itemLists = 0) {
  HandoffNodeBuilder builder_(_fbb);
  builder_.add_itemLists(itemLists);
  builder_.add_children(children);
  builder_.add_transitionTime(transitionTime);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<HandoffNode> CreateHandoffNodeDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    float transitionTime = 0.0f,
    const std::vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *children = nullptr,
    const std::vector<::flatbuffers::Offset<::flatbuffers::String>> *itemLists = nullptr) {
  auto children__ = children ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>(*children) : 0;
  auto itemLists__ = itemLists ? _fbb.CreateVector<::flatbuffers::Offset<::flatbuffers::String>>(*itemLists) : 0;
  return DeepSeaScene::CreateHandoffNode(
      _fbb,
      transitionTime,
      children__,
      itemLists__);
}

inline const DeepSeaScene::HandoffNode *GetHandoffNode(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaScene::HandoffNode>(buf);
}

inline const DeepSeaScene::HandoffNode *GetSizePrefixedHandoffNode(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaScene::HandoffNode>(buf);
}

inline bool VerifyHandoffNodeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::HandoffNode>(nullptr);
}

inline bool VerifySizePrefixedHandoffNodeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::HandoffNode>(nullptr);
}

inline void FinishHandoffNodeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::HandoffNode> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedHandoffNodeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::HandoffNode> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_HANDOFFNODE_DEEPSEASCENE_H_

// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_TRANSFORMNODE_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_TRANSFORMNODE_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct TransformNode;
struct TransformNodeBuilder;

struct TransformNode FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef TransformNodeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TRANSFORM = 4,
    VT_CHILDREN = 6
  };
  const DeepSeaScene::Matrix44f *transform() const {
    return GetStruct<const DeepSeaScene::Matrix44f *>(VT_TRANSFORM);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *children() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *>(VT_CHILDREN);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<DeepSeaScene::Matrix44f>(verifier, VT_TRANSFORM, 4) &&
           VerifyOffset(verifier, VT_CHILDREN) &&
           verifier.VerifyVector(children()) &&
           verifier.VerifyVectorOfTables(children()) &&
           verifier.EndTable();
  }
};

struct TransformNodeBuilder {
  typedef TransformNode Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_transform(const DeepSeaScene::Matrix44f *transform) {
    fbb_.AddStruct(TransformNode::VT_TRANSFORM, transform);
  }
  void add_children(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> children) {
    fbb_.AddOffset(TransformNode::VT_CHILDREN, children);
  }
  explicit TransformNodeBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<TransformNode> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<TransformNode>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<TransformNode> CreateTransformNode(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Matrix44f *transform = nullptr,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> children = 0) {
  TransformNodeBuilder builder_(_fbb);
  builder_.add_children(children);
  builder_.add_transform(transform);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<TransformNode> CreateTransformNodeDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Matrix44f *transform = nullptr,
    const std::vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *children = nullptr) {
  auto children__ = children ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>(*children) : 0;
  return DeepSeaScene::CreateTransformNode(
      _fbb,
      transform,
      children__);
}

inline const DeepSeaScene::TransformNode *GetTransformNode(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaScene::TransformNode>(buf);
}

inline const DeepSeaScene::TransformNode *GetSizePrefixedTransformNode(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaScene::TransformNode>(buf);
}

inline bool VerifyTransformNodeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::TransformNode>(nullptr);
}

inline bool VerifySizePrefixedTransformNodeBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::TransformNode>(nullptr);
}

inline void FinishTransformNodeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::TransformNode> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedTransformNodeBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::TransformNode> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_TRANSFORMNODE_DEEPSEASCENE_H_

// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_VECTORIMAGENODE_DEEPSEASCENEVECTORDRAW_H_
#define FLATBUFFERS_GENERATED_VECTORIMAGENODE_DEEPSEASCENEVECTORDRAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 22 &&
              FLATBUFFERS_VERSION_MINOR == 12 &&
              FLATBUFFERS_VERSION_REVISION == 6,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"
#include "SceneVectorCommon_generated.h"

namespace DeepSeaSceneVectorDraw {

struct VectorImageNode;
struct VectorImageNodeBuilder;

struct VectorImageNode FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef VectorImageNodeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_EMBEDDEDRESOURCES = 4,
    VT_VECTORIMAGE = 6,
    VT_SIZE = 8,
    VT_Z = 10,
    VT_VECTORSHADERS = 12,
    VT_MATERIAL = 14,
    VT_ITEMLISTS = 16
  };
  const flatbuffers::Vector<uint8_t> *embeddedResources() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_EMBEDDEDRESOURCES);
  }
  const flatbuffers::String *vectorImage() const {
    return GetPointer<const flatbuffers::String *>(VT_VECTORIMAGE);
  }
  const DeepSeaScene::Vector2f *size() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_SIZE);
  }
  int32_t z() const {
    return GetField<int32_t>(VT_Z, 0);
  }
  const flatbuffers::String *vectorShaders() const {
    return GetPointer<const flatbuffers::String *>(VT_VECTORSHADERS);
  }
  const flatbuffers::String *material() const {
    return GetPointer<const flatbuffers::String *>(VT_MATERIAL);
  }
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *itemLists() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_ITEMLISTS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_EMBEDDEDRESOURCES) &&
           verifier.VerifyVector(embeddedResources()) &&
           VerifyOffsetRequired(verifier, VT_VECTORIMAGE) &&
           verifier.VerifyString(vectorImage()) &&
           VerifyField<DeepSeaScene::Vector2f>(verifier, VT_SIZE, 4) &&
           VerifyField<int32_t>(verifier, VT_Z, 4) &&
           VerifyOffsetRequired(verifier, VT_VECTORSHADERS) &&
           verifier.VerifyString(vectorShaders()) &&
           VerifyOffsetRequired(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           VerifyOffset(verifier, VT_ITEMLISTS) &&
           verifier.VerifyVector(itemLists()) &&
           verifier.VerifyVectorOfStrings(itemLists()) &&
           verifier.EndTable();
  }
};

struct VectorImageNodeBuilder {
  typedef VectorImageNode Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_embeddedResources(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> embeddedResources) {
    fbb_.AddOffset(VectorImageNode::VT_EMBEDDEDRESOURCES, embeddedResources);
  }
  void add_vectorImage(flatbuffers::Offset<flatbuffers::String> vectorImage) {
    fbb_.AddOffset(VectorImageNode::VT_VECTORIMAGE, vectorImage);
  }
  void add_size(const DeepSeaScene::Vector2f *size) {
    fbb_.AddStruct(VectorImageNode::VT_SIZE, size);
  }
  void add_z(int32_t z) {
    fbb_.AddElement<int32_t>(VectorImageNode::VT_Z, z, 0);
  }
  void add_vectorShaders(flatbuffers::Offset<flatbuffers::String> vectorShaders) {
    fbb_.AddOffset(VectorImageNode::VT_VECTORSHADERS, vectorShaders);
  }
  void add_material(flatbuffers::Offset<flatbuffers::String> material) {
    fbb_.AddOffset(VectorImageNode::VT_MATERIAL, material);
  }
  void add_itemLists(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> itemLists) {
    fbb_.AddOffset(VectorImageNode::VT_ITEMLISTS, itemLists);
  }
  explicit VectorImageNodeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<VectorImageNode> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<VectorImageNode>(end);
    fbb_.Required(o, VectorImageNode::VT_VECTORIMAGE);
    fbb_.Required(o, VectorImageNode::VT_VECTORSHADERS);
    fbb_.Required(o, VectorImageNode::VT_MATERIAL);
    return o;
  }
};

inline flatbuffers::Offset<VectorImageNode> CreateVectorImageNode(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> embeddedResources = 0,
    flatbuffers::Offset<flatbuffers::String> vectorImage = 0,
    const DeepSeaScene::Vector2f *size = nullptr,
    int32_t z = 0,
    flatbuffers::Offset<flatbuffers::String> vectorShaders = 0,
    flatbuffers::Offset<flatbuffers::String> material = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> itemLists = 0) {
  VectorImageNodeBuilder builder_(_fbb);
  builder_.add_itemLists(itemLists);
  builder_.add_material(material);
  builder_.add_vectorShaders(vectorShaders);
  builder_.add_z(z);
  builder_.add_size(size);
  builder_.add_vectorImage(vectorImage);
  builder_.add_embeddedResources(embeddedResources);
  return builder_.Finish();
}

inline flatbuffers::Offset<VectorImageNode> CreateVectorImageNodeDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint8_t> *embeddedResources = nullptr,
    const char *vectorImage = nullptr,
    const DeepSeaScene::Vector2f *size = nullptr,
    int32_t z = 0,
    const char *vectorShaders = nullptr,
    const char *material = nullptr,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *itemLists = nullptr) {
  auto embeddedResources__ = embeddedResources ? _fbb.CreateVector<uint8_t>(*embeddedResources) : 0;
  auto vectorImage__ = vectorImage ? _fbb.CreateString(vectorImage) : 0;
  auto vectorShaders__ = vectorShaders ? _fbb.CreateString(vectorShaders) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  auto itemLists__ = itemLists ? _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*itemLists) : 0;
  return DeepSeaSceneVectorDraw::CreateVectorImageNode(
      _fbb,
      embeddedResources__,
      vectorImage__,
      size,
      z,
      vectorShaders__,
      material__,
      itemLists__);
}

inline const DeepSeaSceneVectorDraw::VectorImageNode *GetVectorImageNode(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaSceneVectorDraw::VectorImageNode>(buf);
}

inline const DeepSeaSceneVectorDraw::VectorImageNode *GetSizePrefixedVectorImageNode(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaSceneVectorDraw::VectorImageNode>(buf);
}

inline bool VerifyVectorImageNodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneVectorDraw::VectorImageNode>(nullptr);
}

inline bool VerifySizePrefixedVectorImageNodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneVectorDraw::VectorImageNode>(nullptr);
}

inline void FinishVectorImageNodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorImageNode> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedVectorImageNodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorImageNode> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneVectorDraw

#endif  // FLATBUFFERS_GENERATED_VECTORIMAGENODE_DEEPSEASCENEVECTORDRAW_H_

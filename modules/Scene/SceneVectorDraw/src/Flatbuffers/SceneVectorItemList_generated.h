// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENEVECTORITEMLIST_DEEPSEASCENEVECTORDRAW_H_
#define FLATBUFFERS_GENERATED_SCENEVECTORITEMLIST_DEEPSEASCENEVECTORDRAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaSceneVectorDraw {

struct VectorItemList;
struct VectorItemListBuilder;

struct VectorItemList FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef VectorItemListBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_INSTANCEDATA = 4,
    VT_DYNAMICRENDERSTATES = 6
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *instanceData() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *>(VT_INSTANCEDATA);
  }
  const DeepSeaScene::DynamicRenderStates *dynamicRenderStates() const {
    return GetPointer<const DeepSeaScene::DynamicRenderStates *>(VT_DYNAMICRENDERSTATES);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_INSTANCEDATA) &&
           verifier.VerifyVector(instanceData()) &&
           verifier.VerifyVectorOfTables(instanceData()) &&
           VerifyOffset(verifier, VT_DYNAMICRENDERSTATES) &&
           verifier.VerifyTable(dynamicRenderStates()) &&
           verifier.EndTable();
  }
};

struct VectorItemListBuilder {
  typedef VectorItemList Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_instanceData(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> instanceData) {
    fbb_.AddOffset(VectorItemList::VT_INSTANCEDATA, instanceData);
  }
  void add_dynamicRenderStates(::flatbuffers::Offset<DeepSeaScene::DynamicRenderStates> dynamicRenderStates) {
    fbb_.AddOffset(VectorItemList::VT_DYNAMICRENDERSTATES, dynamicRenderStates);
  }
  explicit VectorItemListBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<VectorItemList> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<VectorItemList>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<VectorItemList> CreateVectorItemList(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> instanceData = 0,
    ::flatbuffers::Offset<DeepSeaScene::DynamicRenderStates> dynamicRenderStates = 0) {
  VectorItemListBuilder builder_(_fbb);
  builder_.add_dynamicRenderStates(dynamicRenderStates);
  builder_.add_instanceData(instanceData);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<VectorItemList> CreateVectorItemListDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *instanceData = nullptr,
    ::flatbuffers::Offset<DeepSeaScene::DynamicRenderStates> dynamicRenderStates = 0) {
  auto instanceData__ = instanceData ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>(*instanceData) : 0;
  return DeepSeaSceneVectorDraw::CreateVectorItemList(
      _fbb,
      instanceData__,
      dynamicRenderStates);
}

inline const DeepSeaSceneVectorDraw::VectorItemList *GetVectorItemList(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneVectorDraw::VectorItemList>(buf);
}

inline const DeepSeaSceneVectorDraw::VectorItemList *GetSizePrefixedVectorItemList(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneVectorDraw::VectorItemList>(buf);
}

inline bool VerifyVectorItemListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneVectorDraw::VectorItemList>(nullptr);
}

inline bool VerifySizePrefixedVectorItemListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneVectorDraw::VectorItemList>(nullptr);
}

inline void FinishVectorItemListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorItemList> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedVectorItemListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorItemList> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneVectorDraw

#endif  // FLATBUFFERS_GENERATED_SCENEVECTORITEMLIST_DEEPSEASCENEVECTORDRAW_H_

// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_MODELLIST_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_MODELLIST_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct ModelList;
struct ModelListBuilder;

enum class SortType : uint8_t {
  None = 0,
  Material = 1,
  BackToFront = 2,
  FrontToBack = 3,
  MIN = None,
  MAX = FrontToBack
};

inline const SortType (&EnumValuesSortType())[4] {
  static const SortType values[] = {
    SortType::None,
    SortType::Material,
    SortType::BackToFront,
    SortType::FrontToBack
  };
  return values;
}

inline const char * const *EnumNamesSortType() {
  static const char * const names[5] = {
    "None",
    "Material",
    "BackToFront",
    "FrontToBack",
    nullptr
  };
  return names;
}

inline const char *EnumNameSortType(SortType e) {
  if (::flatbuffers::IsOutRange(e, SortType::None, SortType::FrontToBack)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesSortType()[index];
}

struct ModelList FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ModelListBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_INSTANCEDATA = 4,
    VT_SORTTYPE = 6,
    VT_DYNAMICRENDERSTATES = 8,
    VT_CULLLIST = 10
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *instanceData() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *>(VT_INSTANCEDATA);
  }
  DeepSeaScene::SortType sortType() const {
    return static_cast<DeepSeaScene::SortType>(GetField<uint8_t>(VT_SORTTYPE, 0));
  }
  const DeepSeaScene::DynamicRenderStates *dynamicRenderStates() const {
    return GetPointer<const DeepSeaScene::DynamicRenderStates *>(VT_DYNAMICRENDERSTATES);
  }
  const ::flatbuffers::String *cullList() const {
    return GetPointer<const ::flatbuffers::String *>(VT_CULLLIST);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_INSTANCEDATA) &&
           verifier.VerifyVector(instanceData()) &&
           verifier.VerifyVectorOfTables(instanceData()) &&
           VerifyField<uint8_t>(verifier, VT_SORTTYPE, 1) &&
           VerifyOffset(verifier, VT_DYNAMICRENDERSTATES) &&
           verifier.VerifyTable(dynamicRenderStates()) &&
           VerifyOffset(verifier, VT_CULLLIST) &&
           verifier.VerifyString(cullList()) &&
           verifier.EndTable();
  }
};

struct ModelListBuilder {
  typedef ModelList Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_instanceData(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> instanceData) {
    fbb_.AddOffset(ModelList::VT_INSTANCEDATA, instanceData);
  }
  void add_sortType(DeepSeaScene::SortType sortType) {
    fbb_.AddElement<uint8_t>(ModelList::VT_SORTTYPE, static_cast<uint8_t>(sortType), 0);
  }
  void add_dynamicRenderStates(::flatbuffers::Offset<DeepSeaScene::DynamicRenderStates> dynamicRenderStates) {
    fbb_.AddOffset(ModelList::VT_DYNAMICRENDERSTATES, dynamicRenderStates);
  }
  void add_cullList(::flatbuffers::Offset<::flatbuffers::String> cullList) {
    fbb_.AddOffset(ModelList::VT_CULLLIST, cullList);
  }
  explicit ModelListBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ModelList> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ModelList>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ModelList> CreateModelList(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> instanceData = 0,
    DeepSeaScene::SortType sortType = DeepSeaScene::SortType::None,
    ::flatbuffers::Offset<DeepSeaScene::DynamicRenderStates> dynamicRenderStates = 0,
    ::flatbuffers::Offset<::flatbuffers::String> cullList = 0) {
  ModelListBuilder builder_(_fbb);
  builder_.add_cullList(cullList);
  builder_.add_dynamicRenderStates(dynamicRenderStates);
  builder_.add_instanceData(instanceData);
  builder_.add_sortType(sortType);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ModelList> CreateModelListDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *instanceData = nullptr,
    DeepSeaScene::SortType sortType = DeepSeaScene::SortType::None,
    ::flatbuffers::Offset<DeepSeaScene::DynamicRenderStates> dynamicRenderStates = 0,
    const char *cullList = nullptr) {
  auto instanceData__ = instanceData ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>(*instanceData) : 0;
  auto cullList__ = cullList ? _fbb.CreateString(cullList) : 0;
  return DeepSeaScene::CreateModelList(
      _fbb,
      instanceData__,
      sortType,
      dynamicRenderStates,
      cullList__);
}

inline const DeepSeaScene::ModelList *GetModelList(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaScene::ModelList>(buf);
}

inline const DeepSeaScene::ModelList *GetSizePrefixedModelList(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaScene::ModelList>(buf);
}

inline bool VerifyModelListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::ModelList>(nullptr);
}

inline bool VerifySizePrefixedModelListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::ModelList>(nullptr);
}

inline void FinishModelListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::ModelList> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedModelListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::ModelList> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_MODELLIST_DEEPSEASCENE_H_

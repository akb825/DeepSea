// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_MODELLIST_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_MODELLIST_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct DynamicRenderStates;

struct ModelList;

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
  static const char * const names[] = {
    "None",
    "Material",
    "BackToFront",
    "FrontToBack",
    nullptr
  };
  return names;
}

inline const char *EnumNameSortType(SortType e) {
  if (e < SortType::None || e > SortType::FrontToBack) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesSortType()[index];
}

struct DynamicRenderStates FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_LINEWIDTH = 4,
    VT_DEPTHBIASCONSTANTFACTOR = 6,
    VT_DEPTHBIASCLAMP = 8,
    VT_DEPTHBIASSLOPEFACTOR = 10,
    VT_BLENDCONSTANTS = 12,
    VT_DEPTHBOUNDS = 14,
    VT_FRONTSTENCILCOMPAREMASK = 16,
    VT_BACKSTENCILCOMPAREMASK = 18,
    VT_FRONTSTENCILWRITEMASK = 20,
    VT_BACKSTENCILWRITEMASK = 22,
    VT_FRONTSTENCILREFERENCE = 24,
    VT_BACKSTENCILREFERENCE = 26
  };
  float lineWidth() const {
    return GetField<float>(VT_LINEWIDTH, 0.0f);
  }
  float depthBiasConstantFactor() const {
    return GetField<float>(VT_DEPTHBIASCONSTANTFACTOR, 0.0f);
  }
  float depthBiasClamp() const {
    return GetField<float>(VT_DEPTHBIASCLAMP, 0.0f);
  }
  float depthBiasSlopeFactor() const {
    return GetField<float>(VT_DEPTHBIASSLOPEFACTOR, 0.0f);
  }
  const Color4f *blendConstants() const {
    return GetStruct<const Color4f *>(VT_BLENDCONSTANTS);
  }
  const Vector2f *depthBounds() const {
    return GetStruct<const Vector2f *>(VT_DEPTHBOUNDS);
  }
  uint32_t frontStencilCompareMask() const {
    return GetField<uint32_t>(VT_FRONTSTENCILCOMPAREMASK, 0);
  }
  uint32_t backStencilCompareMask() const {
    return GetField<uint32_t>(VT_BACKSTENCILCOMPAREMASK, 0);
  }
  uint32_t frontStencilWriteMask() const {
    return GetField<uint32_t>(VT_FRONTSTENCILWRITEMASK, 0);
  }
  uint32_t backStencilWriteMask() const {
    return GetField<uint32_t>(VT_BACKSTENCILWRITEMASK, 0);
  }
  uint32_t frontStencilReference() const {
    return GetField<uint32_t>(VT_FRONTSTENCILREFERENCE, 0);
  }
  uint32_t backStencilReference() const {
    return GetField<uint32_t>(VT_BACKSTENCILREFERENCE, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_LINEWIDTH) &&
           VerifyField<float>(verifier, VT_DEPTHBIASCONSTANTFACTOR) &&
           VerifyField<float>(verifier, VT_DEPTHBIASCLAMP) &&
           VerifyField<float>(verifier, VT_DEPTHBIASSLOPEFACTOR) &&
           VerifyField<Color4f>(verifier, VT_BLENDCONSTANTS) &&
           VerifyField<Vector2f>(verifier, VT_DEPTHBOUNDS) &&
           VerifyField<uint32_t>(verifier, VT_FRONTSTENCILCOMPAREMASK) &&
           VerifyField<uint32_t>(verifier, VT_BACKSTENCILCOMPAREMASK) &&
           VerifyField<uint32_t>(verifier, VT_FRONTSTENCILWRITEMASK) &&
           VerifyField<uint32_t>(verifier, VT_BACKSTENCILWRITEMASK) &&
           VerifyField<uint32_t>(verifier, VT_FRONTSTENCILREFERENCE) &&
           VerifyField<uint32_t>(verifier, VT_BACKSTENCILREFERENCE) &&
           verifier.EndTable();
  }
};

struct DynamicRenderStatesBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_lineWidth(float lineWidth) {
    fbb_.AddElement<float>(DynamicRenderStates::VT_LINEWIDTH, lineWidth, 0.0f);
  }
  void add_depthBiasConstantFactor(float depthBiasConstantFactor) {
    fbb_.AddElement<float>(DynamicRenderStates::VT_DEPTHBIASCONSTANTFACTOR, depthBiasConstantFactor, 0.0f);
  }
  void add_depthBiasClamp(float depthBiasClamp) {
    fbb_.AddElement<float>(DynamicRenderStates::VT_DEPTHBIASCLAMP, depthBiasClamp, 0.0f);
  }
  void add_depthBiasSlopeFactor(float depthBiasSlopeFactor) {
    fbb_.AddElement<float>(DynamicRenderStates::VT_DEPTHBIASSLOPEFACTOR, depthBiasSlopeFactor, 0.0f);
  }
  void add_blendConstants(const Color4f *blendConstants) {
    fbb_.AddStruct(DynamicRenderStates::VT_BLENDCONSTANTS, blendConstants);
  }
  void add_depthBounds(const Vector2f *depthBounds) {
    fbb_.AddStruct(DynamicRenderStates::VT_DEPTHBOUNDS, depthBounds);
  }
  void add_frontStencilCompareMask(uint32_t frontStencilCompareMask) {
    fbb_.AddElement<uint32_t>(DynamicRenderStates::VT_FRONTSTENCILCOMPAREMASK, frontStencilCompareMask, 0);
  }
  void add_backStencilCompareMask(uint32_t backStencilCompareMask) {
    fbb_.AddElement<uint32_t>(DynamicRenderStates::VT_BACKSTENCILCOMPAREMASK, backStencilCompareMask, 0);
  }
  void add_frontStencilWriteMask(uint32_t frontStencilWriteMask) {
    fbb_.AddElement<uint32_t>(DynamicRenderStates::VT_FRONTSTENCILWRITEMASK, frontStencilWriteMask, 0);
  }
  void add_backStencilWriteMask(uint32_t backStencilWriteMask) {
    fbb_.AddElement<uint32_t>(DynamicRenderStates::VT_BACKSTENCILWRITEMASK, backStencilWriteMask, 0);
  }
  void add_frontStencilReference(uint32_t frontStencilReference) {
    fbb_.AddElement<uint32_t>(DynamicRenderStates::VT_FRONTSTENCILREFERENCE, frontStencilReference, 0);
  }
  void add_backStencilReference(uint32_t backStencilReference) {
    fbb_.AddElement<uint32_t>(DynamicRenderStates::VT_BACKSTENCILREFERENCE, backStencilReference, 0);
  }
  explicit DynamicRenderStatesBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  DynamicRenderStatesBuilder &operator=(const DynamicRenderStatesBuilder &);
  flatbuffers::Offset<DynamicRenderStates> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<DynamicRenderStates>(end);
    return o;
  }
};

inline flatbuffers::Offset<DynamicRenderStates> CreateDynamicRenderStates(
    flatbuffers::FlatBufferBuilder &_fbb,
    float lineWidth = 0.0f,
    float depthBiasConstantFactor = 0.0f,
    float depthBiasClamp = 0.0f,
    float depthBiasSlopeFactor = 0.0f,
    const Color4f *blendConstants = 0,
    const Vector2f *depthBounds = 0,
    uint32_t frontStencilCompareMask = 0,
    uint32_t backStencilCompareMask = 0,
    uint32_t frontStencilWriteMask = 0,
    uint32_t backStencilWriteMask = 0,
    uint32_t frontStencilReference = 0,
    uint32_t backStencilReference = 0) {
  DynamicRenderStatesBuilder builder_(_fbb);
  builder_.add_backStencilReference(backStencilReference);
  builder_.add_frontStencilReference(frontStencilReference);
  builder_.add_backStencilWriteMask(backStencilWriteMask);
  builder_.add_frontStencilWriteMask(frontStencilWriteMask);
  builder_.add_backStencilCompareMask(backStencilCompareMask);
  builder_.add_frontStencilCompareMask(frontStencilCompareMask);
  builder_.add_depthBounds(depthBounds);
  builder_.add_blendConstants(blendConstants);
  builder_.add_depthBiasSlopeFactor(depthBiasSlopeFactor);
  builder_.add_depthBiasClamp(depthBiasClamp);
  builder_.add_depthBiasConstantFactor(depthBiasConstantFactor);
  builder_.add_lineWidth(lineWidth);
  return builder_.Finish();
}

struct ModelList FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_INSTANCEDATA = 4,
    VT_SORTTYPE = 6,
    VT_DYNAMICRENDERSTATES = 8,
    VT_CULLNAME = 10
  };
  const flatbuffers::Vector<flatbuffers::Offset<ObjectData>> *instanceData() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<ObjectData>> *>(VT_INSTANCEDATA);
  }
  SortType sortType() const {
    return static_cast<SortType>(GetField<uint8_t>(VT_SORTTYPE, 0));
  }
  const DynamicRenderStates *dynamicRenderStates() const {
    return GetPointer<const DynamicRenderStates *>(VT_DYNAMICRENDERSTATES);
  }
  const flatbuffers::String *cullName() const {
    return GetPointer<const flatbuffers::String *>(VT_CULLNAME);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_INSTANCEDATA) &&
           verifier.VerifyVector(instanceData()) &&
           verifier.VerifyVectorOfTables(instanceData()) &&
           VerifyField<uint8_t>(verifier, VT_SORTTYPE) &&
           VerifyOffset(verifier, VT_DYNAMICRENDERSTATES) &&
           verifier.VerifyTable(dynamicRenderStates()) &&
           VerifyOffset(verifier, VT_CULLNAME) &&
           verifier.VerifyString(cullName()) &&
           verifier.EndTable();
  }
};

struct ModelListBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_instanceData(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ObjectData>>> instanceData) {
    fbb_.AddOffset(ModelList::VT_INSTANCEDATA, instanceData);
  }
  void add_sortType(SortType sortType) {
    fbb_.AddElement<uint8_t>(ModelList::VT_SORTTYPE, static_cast<uint8_t>(sortType), 0);
  }
  void add_dynamicRenderStates(flatbuffers::Offset<DynamicRenderStates> dynamicRenderStates) {
    fbb_.AddOffset(ModelList::VT_DYNAMICRENDERSTATES, dynamicRenderStates);
  }
  void add_cullName(flatbuffers::Offset<flatbuffers::String> cullName) {
    fbb_.AddOffset(ModelList::VT_CULLNAME, cullName);
  }
  explicit ModelListBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ModelListBuilder &operator=(const ModelListBuilder &);
  flatbuffers::Offset<ModelList> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ModelList>(end);
    return o;
  }
};

inline flatbuffers::Offset<ModelList> CreateModelList(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ObjectData>>> instanceData = 0,
    SortType sortType = SortType::None,
    flatbuffers::Offset<DynamicRenderStates> dynamicRenderStates = 0,
    flatbuffers::Offset<flatbuffers::String> cullName = 0) {
  ModelListBuilder builder_(_fbb);
  builder_.add_cullName(cullName);
  builder_.add_dynamicRenderStates(dynamicRenderStates);
  builder_.add_instanceData(instanceData);
  builder_.add_sortType(sortType);
  return builder_.Finish();
}

inline flatbuffers::Offset<ModelList> CreateModelListDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<ObjectData>> *instanceData = nullptr,
    SortType sortType = SortType::None,
    flatbuffers::Offset<DynamicRenderStates> dynamicRenderStates = 0,
    const char *cullName = nullptr) {
  auto instanceData__ = instanceData ? _fbb.CreateVector<flatbuffers::Offset<ObjectData>>(*instanceData) : 0;
  auto cullName__ = cullName ? _fbb.CreateString(cullName) : 0;
  return DeepSeaScene::CreateModelList(
      _fbb,
      instanceData__,
      sortType,
      dynamicRenderStates,
      cullName__);
}

inline const DeepSeaScene::ModelList *GetModelList(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::ModelList>(buf);
}

inline const DeepSeaScene::ModelList *GetSizePrefixedModelList(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::ModelList>(buf);
}

inline bool VerifyModelListBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::ModelList>(nullptr);
}

inline bool VerifySizePrefixedModelListBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::ModelList>(nullptr);
}

inline void FinishModelListBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ModelList> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedModelListBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ModelList> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_MODELLIST_DEEPSEASCENE_H_
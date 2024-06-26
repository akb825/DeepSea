// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_MODELNODERECONFIG_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_MODELNODERECONFIG_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct ModelReconfig;
struct ModelReconfigBuilder;

struct ModelNodeReconfig;
struct ModelNodeReconfigBuilder;

struct ModelReconfig FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ModelReconfigBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_SHADER = 6,
    VT_MATERIAL = 8,
    VT_DISTANCERANGE = 10,
    VT_MODELLIST = 12
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  const ::flatbuffers::String *shader() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHADER);
  }
  const ::flatbuffers::String *material() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MATERIAL);
  }
  const DeepSeaScene::Vector2f *distanceRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_DISTANCERANGE);
  }
  const ::flatbuffers::String *modelList() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MODELLIST);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffsetRequired(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffsetRequired(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_DISTANCERANGE, 4) &&
           VerifyOffsetRequired(verifier, VT_MODELLIST) &&
           verifier.VerifyString(modelList()) &&
           verifier.EndTable();
  }
};

struct ModelReconfigBuilder {
  typedef ModelReconfig Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(ModelReconfig::VT_NAME, name);
  }
  void add_shader(::flatbuffers::Offset<::flatbuffers::String> shader) {
    fbb_.AddOffset(ModelReconfig::VT_SHADER, shader);
  }
  void add_material(::flatbuffers::Offset<::flatbuffers::String> material) {
    fbb_.AddOffset(ModelReconfig::VT_MATERIAL, material);
  }
  void add_distanceRange(const DeepSeaScene::Vector2f *distanceRange) {
    fbb_.AddStruct(ModelReconfig::VT_DISTANCERANGE, distanceRange);
  }
  void add_modelList(::flatbuffers::Offset<::flatbuffers::String> modelList) {
    fbb_.AddOffset(ModelReconfig::VT_MODELLIST, modelList);
  }
  explicit ModelReconfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ModelReconfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ModelReconfig>(end);
    fbb_.Required(o, ModelReconfig::VT_NAME);
    fbb_.Required(o, ModelReconfig::VT_SHADER);
    fbb_.Required(o, ModelReconfig::VT_MATERIAL);
    fbb_.Required(o, ModelReconfig::VT_DISTANCERANGE);
    fbb_.Required(o, ModelReconfig::VT_MODELLIST);
    return o;
  }
};

inline ::flatbuffers::Offset<ModelReconfig> CreateModelReconfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    ::flatbuffers::Offset<::flatbuffers::String> shader = 0,
    ::flatbuffers::Offset<::flatbuffers::String> material = 0,
    const DeepSeaScene::Vector2f *distanceRange = nullptr,
    ::flatbuffers::Offset<::flatbuffers::String> modelList = 0) {
  ModelReconfigBuilder builder_(_fbb);
  builder_.add_modelList(modelList);
  builder_.add_distanceRange(distanceRange);
  builder_.add_material(material);
  builder_.add_shader(shader);
  builder_.add_name(name);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ModelReconfig> CreateModelReconfigDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const char *shader = nullptr,
    const char *material = nullptr,
    const DeepSeaScene::Vector2f *distanceRange = nullptr,
    const char *modelList = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  auto modelList__ = modelList ? _fbb.CreateString(modelList) : 0;
  return DeepSeaScene::CreateModelReconfig(
      _fbb,
      name__,
      shader__,
      material__,
      distanceRange,
      modelList__);
}

struct ModelNodeReconfig FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ModelNodeReconfigBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_MODELS = 6,
    VT_EXTRAITEMLISTS = 8
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ModelReconfig>> *models() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ModelReconfig>> *>(VT_MODELS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *extraItemLists() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *>(VT_EXTRAITEMLISTS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffsetRequired(verifier, VT_MODELS) &&
           verifier.VerifyVector(models()) &&
           verifier.VerifyVectorOfTables(models()) &&
           VerifyOffset(verifier, VT_EXTRAITEMLISTS) &&
           verifier.VerifyVector(extraItemLists()) &&
           verifier.VerifyVectorOfStrings(extraItemLists()) &&
           verifier.EndTable();
  }
};

struct ModelNodeReconfigBuilder {
  typedef ModelNodeReconfig Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(ModelNodeReconfig::VT_NAME, name);
  }
  void add_models(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ModelReconfig>>> models) {
    fbb_.AddOffset(ModelNodeReconfig::VT_MODELS, models);
  }
  void add_extraItemLists(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> extraItemLists) {
    fbb_.AddOffset(ModelNodeReconfig::VT_EXTRAITEMLISTS, extraItemLists);
  }
  explicit ModelNodeReconfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ModelNodeReconfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ModelNodeReconfig>(end);
    fbb_.Required(o, ModelNodeReconfig::VT_NAME);
    fbb_.Required(o, ModelNodeReconfig::VT_MODELS);
    return o;
  }
};

inline ::flatbuffers::Offset<ModelNodeReconfig> CreateModelNodeReconfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ModelReconfig>>> models = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> extraItemLists = 0) {
  ModelNodeReconfigBuilder builder_(_fbb);
  builder_.add_extraItemLists(extraItemLists);
  builder_.add_models(models);
  builder_.add_name(name);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ModelNodeReconfig> CreateModelNodeReconfigDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const std::vector<::flatbuffers::Offset<DeepSeaScene::ModelReconfig>> *models = nullptr,
    const std::vector<::flatbuffers::Offset<::flatbuffers::String>> *extraItemLists = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto models__ = models ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaScene::ModelReconfig>>(*models) : 0;
  auto extraItemLists__ = extraItemLists ? _fbb.CreateVector<::flatbuffers::Offset<::flatbuffers::String>>(*extraItemLists) : 0;
  return DeepSeaScene::CreateModelNodeReconfig(
      _fbb,
      name__,
      models__,
      extraItemLists__);
}

inline const DeepSeaScene::ModelNodeReconfig *GetModelNodeReconfig(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaScene::ModelNodeReconfig>(buf);
}

inline const DeepSeaScene::ModelNodeReconfig *GetSizePrefixedModelNodeReconfig(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaScene::ModelNodeReconfig>(buf);
}

inline bool VerifyModelNodeReconfigBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::ModelNodeReconfig>(nullptr);
}

inline bool VerifySizePrefixedModelNodeReconfigBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::ModelNodeReconfig>(nullptr);
}

inline void FinishModelNodeReconfigBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::ModelNodeReconfig> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedModelNodeReconfigBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::ModelNodeReconfig> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_MODELNODERECONFIG_DEEPSEASCENE_H_

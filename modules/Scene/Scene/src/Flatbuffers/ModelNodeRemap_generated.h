// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_MODELNODEREMAP_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_MODELNODEREMAP_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 1 &&
              FLATBUFFERS_VERSION_REVISION == 4,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct MaterialRemap;
struct MaterialRemapBuilder;

struct ModelNodeRemap;
struct ModelNodeRemapBuilder;

struct MaterialRemap FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef MaterialRemapBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_MODELLIST = 6,
    VT_SHADER = 8,
    VT_MATERIAL = 10
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  const flatbuffers::String *modelList() const {
    return GetPointer<const flatbuffers::String *>(VT_MODELLIST);
  }
  const flatbuffers::String *shader() const {
    return GetPointer<const flatbuffers::String *>(VT_SHADER);
  }
  const flatbuffers::String *material() const {
    return GetPointer<const flatbuffers::String *>(VT_MATERIAL);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_MODELLIST) &&
           verifier.VerifyString(modelList()) &&
           VerifyOffset(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffset(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           verifier.EndTable();
  }
};

struct MaterialRemapBuilder {
  typedef MaterialRemap Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(MaterialRemap::VT_NAME, name);
  }
  void add_modelList(flatbuffers::Offset<flatbuffers::String> modelList) {
    fbb_.AddOffset(MaterialRemap::VT_MODELLIST, modelList);
  }
  void add_shader(flatbuffers::Offset<flatbuffers::String> shader) {
    fbb_.AddOffset(MaterialRemap::VT_SHADER, shader);
  }
  void add_material(flatbuffers::Offset<flatbuffers::String> material) {
    fbb_.AddOffset(MaterialRemap::VT_MATERIAL, material);
  }
  explicit MaterialRemapBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<MaterialRemap> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<MaterialRemap>(end);
    fbb_.Required(o, MaterialRemap::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<MaterialRemap> CreateMaterialRemap(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::String> modelList = 0,
    flatbuffers::Offset<flatbuffers::String> shader = 0,
    flatbuffers::Offset<flatbuffers::String> material = 0) {
  MaterialRemapBuilder builder_(_fbb);
  builder_.add_material(material);
  builder_.add_shader(shader);
  builder_.add_modelList(modelList);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<MaterialRemap> CreateMaterialRemapDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const char *modelList = nullptr,
    const char *shader = nullptr,
    const char *material = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto modelList__ = modelList ? _fbb.CreateString(modelList) : 0;
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  return DeepSeaScene::CreateMaterialRemap(
      _fbb,
      name__,
      modelList__,
      shader__,
      material__);
}

struct ModelNodeRemap FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ModelNodeRemapBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_MATERIALREMAPS = 6
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::MaterialRemap>> *materialRemaps() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::MaterialRemap>> *>(VT_MATERIALREMAPS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_MATERIALREMAPS) &&
           verifier.VerifyVector(materialRemaps()) &&
           verifier.VerifyVectorOfTables(materialRemaps()) &&
           verifier.EndTable();
  }
};

struct ModelNodeRemapBuilder {
  typedef ModelNodeRemap Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(ModelNodeRemap::VT_NAME, name);
  }
  void add_materialRemaps(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::MaterialRemap>>> materialRemaps) {
    fbb_.AddOffset(ModelNodeRemap::VT_MATERIALREMAPS, materialRemaps);
  }
  explicit ModelNodeRemapBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ModelNodeRemap> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ModelNodeRemap>(end);
    fbb_.Required(o, ModelNodeRemap::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<ModelNodeRemap> CreateModelNodeRemap(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::MaterialRemap>>> materialRemaps = 0) {
  ModelNodeRemapBuilder builder_(_fbb);
  builder_.add_materialRemaps(materialRemaps);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<ModelNodeRemap> CreateModelNodeRemapDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const std::vector<flatbuffers::Offset<DeepSeaScene::MaterialRemap>> *materialRemaps = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto materialRemaps__ = materialRemaps ? _fbb.CreateVector<flatbuffers::Offset<DeepSeaScene::MaterialRemap>>(*materialRemaps) : 0;
  return DeepSeaScene::CreateModelNodeRemap(
      _fbb,
      name__,
      materialRemaps__);
}

inline const DeepSeaScene::ModelNodeRemap *GetModelNodeRemap(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::ModelNodeRemap>(buf);
}

inline const DeepSeaScene::ModelNodeRemap *GetSizePrefixedModelNodeRemap(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::ModelNodeRemap>(buf);
}

inline bool VerifyModelNodeRemapBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::ModelNodeRemap>(nullptr);
}

inline bool VerifySizePrefixedModelNodeRemapBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::ModelNodeRemap>(nullptr);
}

inline void FinishModelNodeRemapBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ModelNodeRemap> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedModelNodeRemapBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ModelNodeRemap> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_MODELNODEREMAP_DEEPSEASCENE_H_

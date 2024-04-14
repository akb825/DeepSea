// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENESSAO_DEEPSEASCENELIGHTING_H_
#define FLATBUFFERS_GENERATED_SCENESSAO_DEEPSEASCENELIGHTING_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace DeepSeaSceneLighting {

struct SceneSSAO;
struct SceneSSAOBuilder;

struct SceneSSAO FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef SceneSSAOBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SHADER = 4,
    VT_MATERIAL = 6
  };
  const ::flatbuffers::String *shader() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHADER);
  }
  const ::flatbuffers::String *material() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MATERIAL);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffsetRequired(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           verifier.EndTable();
  }
};

struct SceneSSAOBuilder {
  typedef SceneSSAO Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_shader(::flatbuffers::Offset<::flatbuffers::String> shader) {
    fbb_.AddOffset(SceneSSAO::VT_SHADER, shader);
  }
  void add_material(::flatbuffers::Offset<::flatbuffers::String> material) {
    fbb_.AddOffset(SceneSSAO::VT_MATERIAL, material);
  }
  explicit SceneSSAOBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<SceneSSAO> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<SceneSSAO>(end);
    fbb_.Required(o, SceneSSAO::VT_SHADER);
    fbb_.Required(o, SceneSSAO::VT_MATERIAL);
    return o;
  }
};

inline ::flatbuffers::Offset<SceneSSAO> CreateSceneSSAO(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> shader = 0,
    ::flatbuffers::Offset<::flatbuffers::String> material = 0) {
  SceneSSAOBuilder builder_(_fbb);
  builder_.add_material(material);
  builder_.add_shader(shader);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<SceneSSAO> CreateSceneSSAODirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *shader = nullptr,
    const char *material = nullptr) {
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  return DeepSeaSceneLighting::CreateSceneSSAO(
      _fbb,
      shader__,
      material__);
}

inline const DeepSeaSceneLighting::SceneSSAO *GetSceneSSAO(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneLighting::SceneSSAO>(buf);
}

inline const DeepSeaSceneLighting::SceneSSAO *GetSizePrefixedSceneSSAO(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneLighting::SceneSSAO>(buf);
}

inline bool VerifySceneSSAOBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneLighting::SceneSSAO>(nullptr);
}

inline bool VerifySizePrefixedSceneSSAOBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneLighting::SceneSSAO>(nullptr);
}

inline void FinishSceneSSAOBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneLighting::SceneSSAO> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedSceneSSAOBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneLighting::SceneSSAO> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneLighting

#endif  // FLATBUFFERS_GENERATED_SCENESSAO_DEEPSEASCENELIGHTING_H_

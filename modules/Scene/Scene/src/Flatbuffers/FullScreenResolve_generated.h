// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_FULLSCREENRESOLVE_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_FULLSCREENRESOLVE_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct FullScreenResolve;
struct FullScreenResolveBuilder;

struct FullScreenResolve FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef FullScreenResolveBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SHADER = 4,
    VT_MATERIAL = 6,
    VT_DYNAMICRENDERSTATES = 8
  };
  const flatbuffers::String *shader() const {
    return GetPointer<const flatbuffers::String *>(VT_SHADER);
  }
  const flatbuffers::String *material() const {
    return GetPointer<const flatbuffers::String *>(VT_MATERIAL);
  }
  const DeepSeaScene::DynamicRenderStates *dynamicRenderStates() const {
    return GetPointer<const DeepSeaScene::DynamicRenderStates *>(VT_DYNAMICRENDERSTATES);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffsetRequired(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           VerifyOffset(verifier, VT_DYNAMICRENDERSTATES) &&
           verifier.VerifyTable(dynamicRenderStates()) &&
           verifier.EndTable();
  }
};

struct FullScreenResolveBuilder {
  typedef FullScreenResolve Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_shader(flatbuffers::Offset<flatbuffers::String> shader) {
    fbb_.AddOffset(FullScreenResolve::VT_SHADER, shader);
  }
  void add_material(flatbuffers::Offset<flatbuffers::String> material) {
    fbb_.AddOffset(FullScreenResolve::VT_MATERIAL, material);
  }
  void add_dynamicRenderStates(flatbuffers::Offset<DeepSeaScene::DynamicRenderStates> dynamicRenderStates) {
    fbb_.AddOffset(FullScreenResolve::VT_DYNAMICRENDERSTATES, dynamicRenderStates);
  }
  explicit FullScreenResolveBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  FullScreenResolveBuilder &operator=(const FullScreenResolveBuilder &);
  flatbuffers::Offset<FullScreenResolve> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<FullScreenResolve>(end);
    fbb_.Required(o, FullScreenResolve::VT_SHADER);
    fbb_.Required(o, FullScreenResolve::VT_MATERIAL);
    return o;
  }
};

inline flatbuffers::Offset<FullScreenResolve> CreateFullScreenResolve(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> shader = 0,
    flatbuffers::Offset<flatbuffers::String> material = 0,
    flatbuffers::Offset<DeepSeaScene::DynamicRenderStates> dynamicRenderStates = 0) {
  FullScreenResolveBuilder builder_(_fbb);
  builder_.add_dynamicRenderStates(dynamicRenderStates);
  builder_.add_material(material);
  builder_.add_shader(shader);
  return builder_.Finish();
}

inline flatbuffers::Offset<FullScreenResolve> CreateFullScreenResolveDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *shader = nullptr,
    const char *material = nullptr,
    flatbuffers::Offset<DeepSeaScene::DynamicRenderStates> dynamicRenderStates = 0) {
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  return DeepSeaScene::CreateFullScreenResolve(
      _fbb,
      shader__,
      material__,
      dynamicRenderStates);
}

inline const DeepSeaScene::FullScreenResolve *GetFullScreenResolve(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::FullScreenResolve>(buf);
}

inline const DeepSeaScene::FullScreenResolve *GetSizePrefixedFullScreenResolve(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::FullScreenResolve>(buf);
}

inline bool VerifyFullScreenResolveBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::FullScreenResolve>(nullptr);
}

inline bool VerifySizePrefixedFullScreenResolveBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::FullScreenResolve>(nullptr);
}

inline void FinishFullScreenResolveBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::FullScreenResolve> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedFullScreenResolveBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::FullScreenResolve> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_FULLSCREENRESOLVE_DEEPSEASCENE_H_

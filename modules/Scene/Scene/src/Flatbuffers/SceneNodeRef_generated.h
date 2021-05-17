// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENENODEREF_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_SCENENODEREF_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct SceneNodeRef;
struct SceneNodeRefBuilder;

struct SceneNodeRef FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SceneNodeRefBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           verifier.EndTable();
  }
};

struct SceneNodeRefBuilder {
  typedef SceneNodeRef Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(SceneNodeRef::VT_NAME, name);
  }
  explicit SceneNodeRefBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<SceneNodeRef> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SceneNodeRef>(end);
    fbb_.Required(o, SceneNodeRef::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<SceneNodeRef> CreateSceneNodeRef(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0) {
  SceneNodeRefBuilder builder_(_fbb);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<SceneNodeRef> CreateSceneNodeRefDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaScene::CreateSceneNodeRef(
      _fbb,
      name__);
}

inline const DeepSeaScene::SceneNodeRef *GetSceneNodeRef(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::SceneNodeRef>(buf);
}

inline const DeepSeaScene::SceneNodeRef *GetSizePrefixedSceneNodeRef(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::SceneNodeRef>(buf);
}

inline bool VerifySceneNodeRefBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::SceneNodeRef>(nullptr);
}

inline bool VerifySizePrefixedSceneNodeRefBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::SceneNodeRef>(nullptr);
}

inline void FinishSceneNodeRefBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::SceneNodeRef> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedSceneNodeRefBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::SceneNodeRef> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_SCENENODEREF_DEEPSEASCENE_H_

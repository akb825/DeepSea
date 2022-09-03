// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_VIEWMIPMAPLIST_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_VIEWMIPMAPLIST_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 2 &&
              FLATBUFFERS_VERSION_MINOR == 0 &&
              FLATBUFFERS_VERSION_REVISION == 8,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct ViewMipmapList;
struct ViewMipmapListBuilder;

struct ViewMipmapList FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ViewMipmapListBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TEXTURES = 4
  };
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *textures() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_TEXTURES);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_TEXTURES) &&
           verifier.VerifyVector(textures()) &&
           verifier.VerifyVectorOfStrings(textures()) &&
           verifier.EndTable();
  }
};

struct ViewMipmapListBuilder {
  typedef ViewMipmapList Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_textures(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> textures) {
    fbb_.AddOffset(ViewMipmapList::VT_TEXTURES, textures);
  }
  explicit ViewMipmapListBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ViewMipmapList> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ViewMipmapList>(end);
    fbb_.Required(o, ViewMipmapList::VT_TEXTURES);
    return o;
  }
};

inline flatbuffers::Offset<ViewMipmapList> CreateViewMipmapList(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> textures = 0) {
  ViewMipmapListBuilder builder_(_fbb);
  builder_.add_textures(textures);
  return builder_.Finish();
}

inline flatbuffers::Offset<ViewMipmapList> CreateViewMipmapListDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *textures = nullptr) {
  auto textures__ = textures ? _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*textures) : 0;
  return DeepSeaScene::CreateViewMipmapList(
      _fbb,
      textures__);
}

inline const DeepSeaScene::ViewMipmapList *GetViewMipmapList(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::ViewMipmapList>(buf);
}

inline const DeepSeaScene::ViewMipmapList *GetSizePrefixedViewMipmapList(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::ViewMipmapList>(buf);
}

inline bool VerifyViewMipmapListBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::ViewMipmapList>(nullptr);
}

inline bool VerifySizePrefixedViewMipmapListBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::ViewMipmapList>(nullptr);
}

inline void FinishViewMipmapListBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ViewMipmapList> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedViewMipmapListBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ViewMipmapList> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_VIEWMIPMAPLIST_DEEPSEASCENE_H_

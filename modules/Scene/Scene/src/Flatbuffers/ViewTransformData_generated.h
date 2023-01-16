// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_VIEWTRANSFORMDATA_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_VIEWTRANSFORMDATA_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 1 &&
              FLATBUFFERS_VERSION_REVISION == 4,
             "Non-compatible flatbuffers version included");

namespace DeepSeaScene {

struct ViewTransformData;
struct ViewTransformDataBuilder;

struct ViewTransformData FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ViewTransformDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VARIABLEGROUPDESC = 4
  };
  const flatbuffers::String *variableGroupDesc() const {
    return GetPointer<const flatbuffers::String *>(VT_VARIABLEGROUPDESC);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_VARIABLEGROUPDESC) &&
           verifier.VerifyString(variableGroupDesc()) &&
           verifier.EndTable();
  }
};

struct ViewTransformDataBuilder {
  typedef ViewTransformData Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_variableGroupDesc(flatbuffers::Offset<flatbuffers::String> variableGroupDesc) {
    fbb_.AddOffset(ViewTransformData::VT_VARIABLEGROUPDESC, variableGroupDesc);
  }
  explicit ViewTransformDataBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ViewTransformData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ViewTransformData>(end);
    fbb_.Required(o, ViewTransformData::VT_VARIABLEGROUPDESC);
    return o;
  }
};

inline flatbuffers::Offset<ViewTransformData> CreateViewTransformData(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> variableGroupDesc = 0) {
  ViewTransformDataBuilder builder_(_fbb);
  builder_.add_variableGroupDesc(variableGroupDesc);
  return builder_.Finish();
}

inline flatbuffers::Offset<ViewTransformData> CreateViewTransformDataDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *variableGroupDesc = nullptr) {
  auto variableGroupDesc__ = variableGroupDesc ? _fbb.CreateString(variableGroupDesc) : 0;
  return DeepSeaScene::CreateViewTransformData(
      _fbb,
      variableGroupDesc__);
}

inline const DeepSeaScene::ViewTransformData *GetViewTransformData(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::ViewTransformData>(buf);
}

inline const DeepSeaScene::ViewTransformData *GetSizePrefixedViewTransformData(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::ViewTransformData>(buf);
}

inline bool VerifyViewTransformDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::ViewTransformData>(nullptr);
}

inline bool VerifySizePrefixedViewTransformDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::ViewTransformData>(nullptr);
}

inline void FinishViewTransformDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ViewTransformData> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedViewTransformDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ViewTransformData> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_VIEWTRANSFORMDATA_DEEPSEASCENE_H_

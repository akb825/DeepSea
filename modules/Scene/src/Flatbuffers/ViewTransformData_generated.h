// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_VIEWTRANSFORMDATA_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_VIEWTRANSFORMDATA_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

namespace DeepSeaScene {

struct ViewTransformData;

struct ViewTransformData FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VARIABLEGROUPDESCNAME = 4
  };
  const flatbuffers::String *variableGroupDescName() const {
    return GetPointer<const flatbuffers::String *>(VT_VARIABLEGROUPDESCNAME);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_VARIABLEGROUPDESCNAME) &&
           verifier.VerifyString(variableGroupDescName()) &&
           verifier.EndTable();
  }
};

struct ViewTransformDataBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_variableGroupDescName(flatbuffers::Offset<flatbuffers::String> variableGroupDescName) {
    fbb_.AddOffset(ViewTransformData::VT_VARIABLEGROUPDESCNAME, variableGroupDescName);
  }
  explicit ViewTransformDataBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ViewTransformDataBuilder &operator=(const ViewTransformDataBuilder &);
  flatbuffers::Offset<ViewTransformData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ViewTransformData>(end);
    fbb_.Required(o, ViewTransformData::VT_VARIABLEGROUPDESCNAME);
    return o;
  }
};

inline flatbuffers::Offset<ViewTransformData> CreateViewTransformData(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> variableGroupDescName = 0) {
  ViewTransformDataBuilder builder_(_fbb);
  builder_.add_variableGroupDescName(variableGroupDescName);
  return builder_.Finish();
}

inline flatbuffers::Offset<ViewTransformData> CreateViewTransformDataDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *variableGroupDescName = nullptr) {
  auto variableGroupDescName__ = variableGroupDescName ? _fbb.CreateString(variableGroupDescName) : 0;
  return DeepSeaScene::CreateViewTransformData(
      _fbb,
      variableGroupDescName__);
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
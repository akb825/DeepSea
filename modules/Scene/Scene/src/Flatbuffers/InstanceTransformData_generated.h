// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_INSTANCETRANSFORMDATA_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_INSTANCETRANSFORMDATA_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace DeepSeaScene {

struct InstanceTransformData;
struct InstanceTransformDataBuilder;

struct InstanceTransformData FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef InstanceTransformDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VARIABLEGROUPDESC = 4
  };
  const ::flatbuffers::String *variableGroupDesc() const {
    return GetPointer<const ::flatbuffers::String *>(VT_VARIABLEGROUPDESC);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_VARIABLEGROUPDESC) &&
           verifier.VerifyString(variableGroupDesc()) &&
           verifier.EndTable();
  }
};

struct InstanceTransformDataBuilder {
  typedef InstanceTransformData Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_variableGroupDesc(::flatbuffers::Offset<::flatbuffers::String> variableGroupDesc) {
    fbb_.AddOffset(InstanceTransformData::VT_VARIABLEGROUPDESC, variableGroupDesc);
  }
  explicit InstanceTransformDataBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<InstanceTransformData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<InstanceTransformData>(end);
    fbb_.Required(o, InstanceTransformData::VT_VARIABLEGROUPDESC);
    return o;
  }
};

inline ::flatbuffers::Offset<InstanceTransformData> CreateInstanceTransformData(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> variableGroupDesc = 0) {
  InstanceTransformDataBuilder builder_(_fbb);
  builder_.add_variableGroupDesc(variableGroupDesc);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<InstanceTransformData> CreateInstanceTransformDataDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *variableGroupDesc = nullptr) {
  auto variableGroupDesc__ = variableGroupDesc ? _fbb.CreateString(variableGroupDesc) : 0;
  return DeepSeaScene::CreateInstanceTransformData(
      _fbb,
      variableGroupDesc__);
}

inline const DeepSeaScene::InstanceTransformData *GetInstanceTransformData(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaScene::InstanceTransformData>(buf);
}

inline const DeepSeaScene::InstanceTransformData *GetSizePrefixedInstanceTransformData(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaScene::InstanceTransformData>(buf);
}

inline bool VerifyInstanceTransformDataBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::InstanceTransformData>(nullptr);
}

inline bool VerifySizePrefixedInstanceTransformDataBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::InstanceTransformData>(nullptr);
}

inline void FinishInstanceTransformDataBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::InstanceTransformData> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedInstanceTransformDataBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScene::InstanceTransformData> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_INSTANCETRANSFORMDATA_DEEPSEASCENE_H_

// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_INSTANCEFORWARDLIGHTDATA_DEEPSEASCENELIGHTING_H_
#define FLATBUFFERS_GENERATED_INSTANCEFORWARDLIGHTDATA_DEEPSEASCENELIGHTING_H_

#include "flatbuffers/flatbuffers.h"

namespace DeepSeaSceneLighting {

struct InstanceForwardLightData;
struct InstanceForwardLightDataBuilder;

struct InstanceForwardLightData FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef InstanceForwardLightDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VARIABLEGROUPDESC = 4,
    VT_LIGHTSET = 6
  };
  const flatbuffers::String *variableGroupDesc() const {
    return GetPointer<const flatbuffers::String *>(VT_VARIABLEGROUPDESC);
  }
  const flatbuffers::String *lightSet() const {
    return GetPointer<const flatbuffers::String *>(VT_LIGHTSET);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_VARIABLEGROUPDESC) &&
           verifier.VerifyString(variableGroupDesc()) &&
           VerifyOffsetRequired(verifier, VT_LIGHTSET) &&
           verifier.VerifyString(lightSet()) &&
           verifier.EndTable();
  }
};

struct InstanceForwardLightDataBuilder {
  typedef InstanceForwardLightData Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_variableGroupDesc(flatbuffers::Offset<flatbuffers::String> variableGroupDesc) {
    fbb_.AddOffset(InstanceForwardLightData::VT_VARIABLEGROUPDESC, variableGroupDesc);
  }
  void add_lightSet(flatbuffers::Offset<flatbuffers::String> lightSet) {
    fbb_.AddOffset(InstanceForwardLightData::VT_LIGHTSET, lightSet);
  }
  explicit InstanceForwardLightDataBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  InstanceForwardLightDataBuilder &operator=(const InstanceForwardLightDataBuilder &);
  flatbuffers::Offset<InstanceForwardLightData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<InstanceForwardLightData>(end);
    fbb_.Required(o, InstanceForwardLightData::VT_VARIABLEGROUPDESC);
    fbb_.Required(o, InstanceForwardLightData::VT_LIGHTSET);
    return o;
  }
};

inline flatbuffers::Offset<InstanceForwardLightData> CreateInstanceForwardLightData(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> variableGroupDesc = 0,
    flatbuffers::Offset<flatbuffers::String> lightSet = 0) {
  InstanceForwardLightDataBuilder builder_(_fbb);
  builder_.add_lightSet(lightSet);
  builder_.add_variableGroupDesc(variableGroupDesc);
  return builder_.Finish();
}

inline flatbuffers::Offset<InstanceForwardLightData> CreateInstanceForwardLightDataDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *variableGroupDesc = nullptr,
    const char *lightSet = nullptr) {
  auto variableGroupDesc__ = variableGroupDesc ? _fbb.CreateString(variableGroupDesc) : 0;
  auto lightSet__ = lightSet ? _fbb.CreateString(lightSet) : 0;
  return DeepSeaSceneLighting::CreateInstanceForwardLightData(
      _fbb,
      variableGroupDesc__,
      lightSet__);
}

inline const DeepSeaSceneLighting::InstanceForwardLightData *GetInstanceForwardLightData(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaSceneLighting::InstanceForwardLightData>(buf);
}

inline const DeepSeaSceneLighting::InstanceForwardLightData *GetSizePrefixedInstanceForwardLightData(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaSceneLighting::InstanceForwardLightData>(buf);
}

inline bool VerifyInstanceForwardLightDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneLighting::InstanceForwardLightData>(nullptr);
}

inline bool VerifySizePrefixedInstanceForwardLightDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneLighting::InstanceForwardLightData>(nullptr);
}

inline void FinishInstanceForwardLightDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::InstanceForwardLightData> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedInstanceForwardLightDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::InstanceForwardLightData> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneLighting

#endif  // FLATBUFFERS_GENERATED_INSTANCEFORWARDLIGHTDATA_DEEPSEASCENELIGHTING_H_

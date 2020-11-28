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
    VT_VARIABLEGROUPDESCNAME = 4,
    VT_LIGHTSETNAME = 6
  };
  const flatbuffers::String *variableGroupDescName() const {
    return GetPointer<const flatbuffers::String *>(VT_VARIABLEGROUPDESCNAME);
  }
  const flatbuffers::String *lightSetName() const {
    return GetPointer<const flatbuffers::String *>(VT_LIGHTSETNAME);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_VARIABLEGROUPDESCNAME) &&
           verifier.VerifyString(variableGroupDescName()) &&
           VerifyOffsetRequired(verifier, VT_LIGHTSETNAME) &&
           verifier.VerifyString(lightSetName()) &&
           verifier.EndTable();
  }
};

struct InstanceForwardLightDataBuilder {
  typedef InstanceForwardLightData Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_variableGroupDescName(flatbuffers::Offset<flatbuffers::String> variableGroupDescName) {
    fbb_.AddOffset(InstanceForwardLightData::VT_VARIABLEGROUPDESCNAME, variableGroupDescName);
  }
  void add_lightSetName(flatbuffers::Offset<flatbuffers::String> lightSetName) {
    fbb_.AddOffset(InstanceForwardLightData::VT_LIGHTSETNAME, lightSetName);
  }
  explicit InstanceForwardLightDataBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  InstanceForwardLightDataBuilder &operator=(const InstanceForwardLightDataBuilder &);
  flatbuffers::Offset<InstanceForwardLightData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<InstanceForwardLightData>(end);
    fbb_.Required(o, InstanceForwardLightData::VT_VARIABLEGROUPDESCNAME);
    fbb_.Required(o, InstanceForwardLightData::VT_LIGHTSETNAME);
    return o;
  }
};

inline flatbuffers::Offset<InstanceForwardLightData> CreateInstanceForwardLightData(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> variableGroupDescName = 0,
    flatbuffers::Offset<flatbuffers::String> lightSetName = 0) {
  InstanceForwardLightDataBuilder builder_(_fbb);
  builder_.add_lightSetName(lightSetName);
  builder_.add_variableGroupDescName(variableGroupDescName);
  return builder_.Finish();
}

inline flatbuffers::Offset<InstanceForwardLightData> CreateInstanceForwardLightDataDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *variableGroupDescName = nullptr,
    const char *lightSetName = nullptr) {
  auto variableGroupDescName__ = variableGroupDescName ? _fbb.CreateString(variableGroupDescName) : 0;
  auto lightSetName__ = lightSetName ? _fbb.CreateString(lightSetName) : 0;
  return DeepSeaSceneLighting::CreateInstanceForwardLightData(
      _fbb,
      variableGroupDescName__,
      lightSetName__);
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
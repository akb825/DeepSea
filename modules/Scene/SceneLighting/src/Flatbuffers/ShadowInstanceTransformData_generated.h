// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SHADOWINSTANCETRANSFORMDATA_DEEPSEASCENELIGHTING_H_
#define FLATBUFFERS_GENERATED_SHADOWINSTANCETRANSFORMDATA_DEEPSEASCENELIGHTING_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 1 &&
              FLATBUFFERS_VERSION_REVISION == 4,
             "Non-compatible flatbuffers version included");

namespace DeepSeaSceneLighting {

struct ShadowInstanceTransformData;
struct ShadowInstanceTransformDataBuilder;

struct ShadowInstanceTransformData FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ShadowInstanceTransformDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SHADOWMANAGER = 4,
    VT_SHADOWS = 6,
    VT_SURFACE = 8,
    VT_VARIABLEGROUPDESC = 10
  };
  const flatbuffers::String *shadowManager() const {
    return GetPointer<const flatbuffers::String *>(VT_SHADOWMANAGER);
  }
  const flatbuffers::String *shadows() const {
    return GetPointer<const flatbuffers::String *>(VT_SHADOWS);
  }
  uint8_t surface() const {
    return GetField<uint8_t>(VT_SURFACE, 0);
  }
  const flatbuffers::String *variableGroupDesc() const {
    return GetPointer<const flatbuffers::String *>(VT_VARIABLEGROUPDESC);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_SHADOWMANAGER) &&
           verifier.VerifyString(shadowManager()) &&
           VerifyOffsetRequired(verifier, VT_SHADOWS) &&
           verifier.VerifyString(shadows()) &&
           VerifyField<uint8_t>(verifier, VT_SURFACE, 1) &&
           VerifyOffsetRequired(verifier, VT_VARIABLEGROUPDESC) &&
           verifier.VerifyString(variableGroupDesc()) &&
           verifier.EndTable();
  }
};

struct ShadowInstanceTransformDataBuilder {
  typedef ShadowInstanceTransformData Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_shadowManager(flatbuffers::Offset<flatbuffers::String> shadowManager) {
    fbb_.AddOffset(ShadowInstanceTransformData::VT_SHADOWMANAGER, shadowManager);
  }
  void add_shadows(flatbuffers::Offset<flatbuffers::String> shadows) {
    fbb_.AddOffset(ShadowInstanceTransformData::VT_SHADOWS, shadows);
  }
  void add_surface(uint8_t surface) {
    fbb_.AddElement<uint8_t>(ShadowInstanceTransformData::VT_SURFACE, surface, 0);
  }
  void add_variableGroupDesc(flatbuffers::Offset<flatbuffers::String> variableGroupDesc) {
    fbb_.AddOffset(ShadowInstanceTransformData::VT_VARIABLEGROUPDESC, variableGroupDesc);
  }
  explicit ShadowInstanceTransformDataBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ShadowInstanceTransformData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ShadowInstanceTransformData>(end);
    fbb_.Required(o, ShadowInstanceTransformData::VT_SHADOWMANAGER);
    fbb_.Required(o, ShadowInstanceTransformData::VT_SHADOWS);
    fbb_.Required(o, ShadowInstanceTransformData::VT_VARIABLEGROUPDESC);
    return o;
  }
};

inline flatbuffers::Offset<ShadowInstanceTransformData> CreateShadowInstanceTransformData(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> shadowManager = 0,
    flatbuffers::Offset<flatbuffers::String> shadows = 0,
    uint8_t surface = 0,
    flatbuffers::Offset<flatbuffers::String> variableGroupDesc = 0) {
  ShadowInstanceTransformDataBuilder builder_(_fbb);
  builder_.add_variableGroupDesc(variableGroupDesc);
  builder_.add_shadows(shadows);
  builder_.add_shadowManager(shadowManager);
  builder_.add_surface(surface);
  return builder_.Finish();
}

inline flatbuffers::Offset<ShadowInstanceTransformData> CreateShadowInstanceTransformDataDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *shadowManager = nullptr,
    const char *shadows = nullptr,
    uint8_t surface = 0,
    const char *variableGroupDesc = nullptr) {
  auto shadowManager__ = shadowManager ? _fbb.CreateString(shadowManager) : 0;
  auto shadows__ = shadows ? _fbb.CreateString(shadows) : 0;
  auto variableGroupDesc__ = variableGroupDesc ? _fbb.CreateString(variableGroupDesc) : 0;
  return DeepSeaSceneLighting::CreateShadowInstanceTransformData(
      _fbb,
      shadowManager__,
      shadows__,
      surface,
      variableGroupDesc__);
}

inline const DeepSeaSceneLighting::ShadowInstanceTransformData *GetShadowInstanceTransformData(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaSceneLighting::ShadowInstanceTransformData>(buf);
}

inline const DeepSeaSceneLighting::ShadowInstanceTransformData *GetSizePrefixedShadowInstanceTransformData(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaSceneLighting::ShadowInstanceTransformData>(buf);
}

inline bool VerifyShadowInstanceTransformDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneLighting::ShadowInstanceTransformData>(nullptr);
}

inline bool VerifySizePrefixedShadowInstanceTransformDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneLighting::ShadowInstanceTransformData>(nullptr);
}

inline void FinishShadowInstanceTransformDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::ShadowInstanceTransformData> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedShadowInstanceTransformDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::ShadowInstanceTransformData> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneLighting

#endif  // FLATBUFFERS_GENERATED_SHADOWINSTANCETRANSFORMDATA_DEEPSEASCENELIGHTING_H_

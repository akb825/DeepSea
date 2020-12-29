// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_LIGHTSETPREPARE_DEEPSEASCENELIGHTING_H_
#define FLATBUFFERS_GENERATED_LIGHTSETPREPARE_DEEPSEASCENELIGHTING_H_

#include "flatbuffers/flatbuffers.h"

namespace DeepSeaSceneLighting {

struct LightSetPrepare;
struct LightSetPrepareBuilder;

struct LightSetPrepare FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef LightSetPrepareBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_LIGHTSETS = 4,
    VT_INTENSITYTHRESHOLD = 6
  };
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *lightSets() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_LIGHTSETS);
  }
  float intensityThreshold() const {
    return GetField<float>(VT_INTENSITYTHRESHOLD, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_LIGHTSETS) &&
           verifier.VerifyVector(lightSets()) &&
           verifier.VerifyVectorOfStrings(lightSets()) &&
           VerifyField<float>(verifier, VT_INTENSITYTHRESHOLD) &&
           verifier.EndTable();
  }
};

struct LightSetPrepareBuilder {
  typedef LightSetPrepare Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_lightSets(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> lightSets) {
    fbb_.AddOffset(LightSetPrepare::VT_LIGHTSETS, lightSets);
  }
  void add_intensityThreshold(float intensityThreshold) {
    fbb_.AddElement<float>(LightSetPrepare::VT_INTENSITYTHRESHOLD, intensityThreshold, 0.0f);
  }
  explicit LightSetPrepareBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  LightSetPrepareBuilder &operator=(const LightSetPrepareBuilder &);
  flatbuffers::Offset<LightSetPrepare> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<LightSetPrepare>(end);
    fbb_.Required(o, LightSetPrepare::VT_LIGHTSETS);
    return o;
  }
};

inline flatbuffers::Offset<LightSetPrepare> CreateLightSetPrepare(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> lightSets = 0,
    float intensityThreshold = 0.0f) {
  LightSetPrepareBuilder builder_(_fbb);
  builder_.add_intensityThreshold(intensityThreshold);
  builder_.add_lightSets(lightSets);
  return builder_.Finish();
}

inline flatbuffers::Offset<LightSetPrepare> CreateLightSetPrepareDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *lightSets = nullptr,
    float intensityThreshold = 0.0f) {
  auto lightSets__ = lightSets ? _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*lightSets) : 0;
  return DeepSeaSceneLighting::CreateLightSetPrepare(
      _fbb,
      lightSets__,
      intensityThreshold);
}

inline const DeepSeaSceneLighting::LightSetPrepare *GetLightSetPrepare(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaSceneLighting::LightSetPrepare>(buf);
}

inline const DeepSeaSceneLighting::LightSetPrepare *GetSizePrefixedLightSetPrepare(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaSceneLighting::LightSetPrepare>(buf);
}

inline bool VerifyLightSetPrepareBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneLighting::LightSetPrepare>(nullptr);
}

inline bool VerifySizePrefixedLightSetPrepareBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneLighting::LightSetPrepare>(nullptr);
}

inline void FinishLightSetPrepareBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::LightSetPrepare> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedLightSetPrepareBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::LightSetPrepare> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneLighting

#endif  // FLATBUFFERS_GENERATED_LIGHTSETPREPARE_DEEPSEASCENELIGHTING_H_

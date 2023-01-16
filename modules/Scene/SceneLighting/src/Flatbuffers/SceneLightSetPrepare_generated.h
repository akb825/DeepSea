// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENELIGHTSETPREPARE_DEEPSEASCENELIGHTING_H_
#define FLATBUFFERS_GENERATED_SCENELIGHTSETPREPARE_DEEPSEASCENELIGHTING_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 1 &&
              FLATBUFFERS_VERSION_REVISION == 4,
             "Non-compatible flatbuffers version included");

namespace DeepSeaSceneLighting {

struct SceneLightSetPrepare;
struct SceneLightSetPrepareBuilder;

struct SceneLightSetPrepare FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SceneLightSetPrepareBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_LIGHTSET = 4,
    VT_INTENSITYTHRESHOLD = 6
  };
  const flatbuffers::String *lightSet() const {
    return GetPointer<const flatbuffers::String *>(VT_LIGHTSET);
  }
  float intensityThreshold() const {
    return GetField<float>(VT_INTENSITYTHRESHOLD, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_LIGHTSET) &&
           verifier.VerifyString(lightSet()) &&
           VerifyField<float>(verifier, VT_INTENSITYTHRESHOLD, 4) &&
           verifier.EndTable();
  }
};

struct SceneLightSetPrepareBuilder {
  typedef SceneLightSetPrepare Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_lightSet(flatbuffers::Offset<flatbuffers::String> lightSet) {
    fbb_.AddOffset(SceneLightSetPrepare::VT_LIGHTSET, lightSet);
  }
  void add_intensityThreshold(float intensityThreshold) {
    fbb_.AddElement<float>(SceneLightSetPrepare::VT_INTENSITYTHRESHOLD, intensityThreshold, 0.0f);
  }
  explicit SceneLightSetPrepareBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<SceneLightSetPrepare> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SceneLightSetPrepare>(end);
    fbb_.Required(o, SceneLightSetPrepare::VT_LIGHTSET);
    return o;
  }
};

inline flatbuffers::Offset<SceneLightSetPrepare> CreateSceneLightSetPrepare(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> lightSet = 0,
    float intensityThreshold = 0.0f) {
  SceneLightSetPrepareBuilder builder_(_fbb);
  builder_.add_intensityThreshold(intensityThreshold);
  builder_.add_lightSet(lightSet);
  return builder_.Finish();
}

inline flatbuffers::Offset<SceneLightSetPrepare> CreateSceneLightSetPrepareDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *lightSet = nullptr,
    float intensityThreshold = 0.0f) {
  auto lightSet__ = lightSet ? _fbb.CreateString(lightSet) : 0;
  return DeepSeaSceneLighting::CreateSceneLightSetPrepare(
      _fbb,
      lightSet__,
      intensityThreshold);
}

inline const DeepSeaSceneLighting::SceneLightSetPrepare *GetSceneLightSetPrepare(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaSceneLighting::SceneLightSetPrepare>(buf);
}

inline const DeepSeaSceneLighting::SceneLightSetPrepare *GetSizePrefixedSceneLightSetPrepare(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaSceneLighting::SceneLightSetPrepare>(buf);
}

inline bool VerifySceneLightSetPrepareBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneLighting::SceneLightSetPrepare>(nullptr);
}

inline bool VerifySizePrefixedSceneLightSetPrepareBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneLighting::SceneLightSetPrepare>(nullptr);
}

inline void FinishSceneLightSetPrepareBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::SceneLightSetPrepare> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedSceneLightSetPrepareBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::SceneLightSetPrepare> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneLighting

#endif  // FLATBUFFERS_GENERATED_SCENELIGHTSETPREPARE_DEEPSEASCENELIGHTING_H_

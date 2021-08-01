// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENELIGHTSHADOWS_DEEPSEASCENELIGHTING_H_
#define FLATBUFFERS_GENERATED_SCENELIGHTSHADOWS_DEEPSEASCENELIGHTING_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaSceneLighting {

struct SceneLightShadows;
struct SceneLightShadowsBuilder;

enum class LightType : uint8_t {
  Directional = 0,
  Point = 1,
  Spot = 2,
  MIN = Directional,
  MAX = Spot
};

inline const LightType (&EnumValuesLightType())[3] {
  static const LightType values[] = {
    LightType::Directional,
    LightType::Point,
    LightType::Spot
  };
  return values;
}

inline const char * const *EnumNamesLightType() {
  static const char * const names[4] = {
    "Directional",
    "Point",
    "Spot",
    nullptr
  };
  return names;
}

inline const char *EnumNameLightType(LightType e) {
  if (flatbuffers::IsOutRange(e, LightType::Directional, LightType::Spot)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesLightType()[index];
}

struct SceneLightShadows FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SceneLightShadowsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_LIGHTSET = 4,
    VT_LIGHTTYPE = 6,
    VT_LIGHT = 8,
    VT_TRANSFORMGROUPDESC = 10,
    VT_MAXCASCADES = 12,
    VT_MAXFIRSTSPLITDISTANCE = 14,
    VT_CASCADEEXPFACTOR = 16,
    VT_FADESTARTDISTANCE = 18,
    VT_MAXDISTANCE = 20
  };
  const flatbuffers::String *lightSet() const {
    return GetPointer<const flatbuffers::String *>(VT_LIGHTSET);
  }
  DeepSeaSceneLighting::LightType lightType() const {
    return static_cast<DeepSeaSceneLighting::LightType>(GetField<uint8_t>(VT_LIGHTTYPE, 0));
  }
  const flatbuffers::String *light() const {
    return GetPointer<const flatbuffers::String *>(VT_LIGHT);
  }
  const flatbuffers::String *transformGroupDesc() const {
    return GetPointer<const flatbuffers::String *>(VT_TRANSFORMGROUPDESC);
  }
  uint32_t maxCascades() const {
    return GetField<uint32_t>(VT_MAXCASCADES, 0);
  }
  float maxFirstSplitDistance() const {
    return GetField<float>(VT_MAXFIRSTSPLITDISTANCE, 0.0f);
  }
  float cascadeExpFactor() const {
    return GetField<float>(VT_CASCADEEXPFACTOR, 0.0f);
  }
  float fadeStartDistance() const {
    return GetField<float>(VT_FADESTARTDISTANCE, 0.0f);
  }
  float maxDistance() const {
    return GetField<float>(VT_MAXDISTANCE, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_LIGHTSET) &&
           verifier.VerifyString(lightSet()) &&
           VerifyField<uint8_t>(verifier, VT_LIGHTTYPE) &&
           VerifyOffset(verifier, VT_LIGHT) &&
           verifier.VerifyString(light()) &&
           VerifyOffsetRequired(verifier, VT_TRANSFORMGROUPDESC) &&
           verifier.VerifyString(transformGroupDesc()) &&
           VerifyField<uint32_t>(verifier, VT_MAXCASCADES) &&
           VerifyField<float>(verifier, VT_MAXFIRSTSPLITDISTANCE) &&
           VerifyField<float>(verifier, VT_CASCADEEXPFACTOR) &&
           VerifyField<float>(verifier, VT_FADESTARTDISTANCE) &&
           VerifyField<float>(verifier, VT_MAXDISTANCE) &&
           verifier.EndTable();
  }
};

struct SceneLightShadowsBuilder {
  typedef SceneLightShadows Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_lightSet(flatbuffers::Offset<flatbuffers::String> lightSet) {
    fbb_.AddOffset(SceneLightShadows::VT_LIGHTSET, lightSet);
  }
  void add_lightType(DeepSeaSceneLighting::LightType lightType) {
    fbb_.AddElement<uint8_t>(SceneLightShadows::VT_LIGHTTYPE, static_cast<uint8_t>(lightType), 0);
  }
  void add_light(flatbuffers::Offset<flatbuffers::String> light) {
    fbb_.AddOffset(SceneLightShadows::VT_LIGHT, light);
  }
  void add_transformGroupDesc(flatbuffers::Offset<flatbuffers::String> transformGroupDesc) {
    fbb_.AddOffset(SceneLightShadows::VT_TRANSFORMGROUPDESC, transformGroupDesc);
  }
  void add_maxCascades(uint32_t maxCascades) {
    fbb_.AddElement<uint32_t>(SceneLightShadows::VT_MAXCASCADES, maxCascades, 0);
  }
  void add_maxFirstSplitDistance(float maxFirstSplitDistance) {
    fbb_.AddElement<float>(SceneLightShadows::VT_MAXFIRSTSPLITDISTANCE, maxFirstSplitDistance, 0.0f);
  }
  void add_cascadeExpFactor(float cascadeExpFactor) {
    fbb_.AddElement<float>(SceneLightShadows::VT_CASCADEEXPFACTOR, cascadeExpFactor, 0.0f);
  }
  void add_fadeStartDistance(float fadeStartDistance) {
    fbb_.AddElement<float>(SceneLightShadows::VT_FADESTARTDISTANCE, fadeStartDistance, 0.0f);
  }
  void add_maxDistance(float maxDistance) {
    fbb_.AddElement<float>(SceneLightShadows::VT_MAXDISTANCE, maxDistance, 0.0f);
  }
  explicit SceneLightShadowsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<SceneLightShadows> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SceneLightShadows>(end);
    fbb_.Required(o, SceneLightShadows::VT_LIGHTSET);
    fbb_.Required(o, SceneLightShadows::VT_TRANSFORMGROUPDESC);
    return o;
  }
};

inline flatbuffers::Offset<SceneLightShadows> CreateSceneLightShadows(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> lightSet = 0,
    DeepSeaSceneLighting::LightType lightType = DeepSeaSceneLighting::LightType::Directional,
    flatbuffers::Offset<flatbuffers::String> light = 0,
    flatbuffers::Offset<flatbuffers::String> transformGroupDesc = 0,
    uint32_t maxCascades = 0,
    float maxFirstSplitDistance = 0.0f,
    float cascadeExpFactor = 0.0f,
    float fadeStartDistance = 0.0f,
    float maxDistance = 0.0f) {
  SceneLightShadowsBuilder builder_(_fbb);
  builder_.add_maxDistance(maxDistance);
  builder_.add_fadeStartDistance(fadeStartDistance);
  builder_.add_cascadeExpFactor(cascadeExpFactor);
  builder_.add_maxFirstSplitDistance(maxFirstSplitDistance);
  builder_.add_maxCascades(maxCascades);
  builder_.add_transformGroupDesc(transformGroupDesc);
  builder_.add_light(light);
  builder_.add_lightSet(lightSet);
  builder_.add_lightType(lightType);
  return builder_.Finish();
}

inline flatbuffers::Offset<SceneLightShadows> CreateSceneLightShadowsDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *lightSet = nullptr,
    DeepSeaSceneLighting::LightType lightType = DeepSeaSceneLighting::LightType::Directional,
    const char *light = nullptr,
    const char *transformGroupDesc = nullptr,
    uint32_t maxCascades = 0,
    float maxFirstSplitDistance = 0.0f,
    float cascadeExpFactor = 0.0f,
    float fadeStartDistance = 0.0f,
    float maxDistance = 0.0f) {
  auto lightSet__ = lightSet ? _fbb.CreateString(lightSet) : 0;
  auto light__ = light ? _fbb.CreateString(light) : 0;
  auto transformGroupDesc__ = transformGroupDesc ? _fbb.CreateString(transformGroupDesc) : 0;
  return DeepSeaSceneLighting::CreateSceneLightShadows(
      _fbb,
      lightSet__,
      lightType,
      light__,
      transformGroupDesc__,
      maxCascades,
      maxFirstSplitDistance,
      cascadeExpFactor,
      fadeStartDistance,
      maxDistance);
}

inline const DeepSeaSceneLighting::SceneLightShadows *GetSceneLightShadows(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaSceneLighting::SceneLightShadows>(buf);
}

inline const DeepSeaSceneLighting::SceneLightShadows *GetSizePrefixedSceneLightShadows(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaSceneLighting::SceneLightShadows>(buf);
}

inline bool VerifySceneLightShadowsBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneLighting::SceneLightShadows>(nullptr);
}

inline bool VerifySizePrefixedSceneLightShadowsBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneLighting::SceneLightShadows>(nullptr);
}

inline void FinishSceneLightShadowsBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::SceneLightShadows> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedSceneLightShadowsBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::SceneLightShadows> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneLighting

#endif  // FLATBUFFERS_GENERATED_SCENELIGHTSHADOWS_DEEPSEASCENELIGHTING_H_

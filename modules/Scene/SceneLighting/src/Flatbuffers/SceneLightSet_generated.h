// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENELIGHTSET_DEEPSEASCENELIGHTING_H_
#define FLATBUFFERS_GENERATED_SCENELIGHTSET_DEEPSEASCENELIGHTING_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaSceneLighting {

struct DirectionalLight;
struct DirectionalLightBuilder;

struct PointLight;
struct PointLightBuilder;

struct SpotLight;
struct SpotLightBuilder;

struct Light;
struct LightBuilder;

struct SceneLightSet;
struct SceneLightSetBuilder;

enum class LightUnion : uint8_t {
  NONE = 0,
  DirectionalLight = 1,
  PointLight = 2,
  SpotLight = 3,
  MIN = NONE,
  MAX = SpotLight
};

inline const LightUnion (&EnumValuesLightUnion())[4] {
  static const LightUnion values[] = {
    LightUnion::NONE,
    LightUnion::DirectionalLight,
    LightUnion::PointLight,
    LightUnion::SpotLight
  };
  return values;
}

inline const char * const *EnumNamesLightUnion() {
  static const char * const names[5] = {
    "NONE",
    "DirectionalLight",
    "PointLight",
    "SpotLight",
    nullptr
  };
  return names;
}

inline const char *EnumNameLightUnion(LightUnion e) {
  if (flatbuffers::IsOutRange(e, LightUnion::NONE, LightUnion::SpotLight)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesLightUnion()[index];
}

template<typename T> struct LightUnionTraits {
  static const LightUnion enum_value = LightUnion::NONE;
};

template<> struct LightUnionTraits<DeepSeaSceneLighting::DirectionalLight> {
  static const LightUnion enum_value = LightUnion::DirectionalLight;
};

template<> struct LightUnionTraits<DeepSeaSceneLighting::PointLight> {
  static const LightUnion enum_value = LightUnion::PointLight;
};

template<> struct LightUnionTraits<DeepSeaSceneLighting::SpotLight> {
  static const LightUnion enum_value = LightUnion::SpotLight;
};

bool VerifyLightUnion(flatbuffers::Verifier &verifier, const void *obj, LightUnion type);
bool VerifyLightUnionVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

struct DirectionalLight FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef DirectionalLightBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_DIRECTION = 4,
    VT_COLOR = 6,
    VT_INTENSITY = 8
  };
  const DeepSeaScene::Vector3f *direction() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_DIRECTION);
  }
  const DeepSeaScene::Color3f *color() const {
    return GetStruct<const DeepSeaScene::Color3f *>(VT_COLOR);
  }
  float intensity() const {
    return GetField<float>(VT_INTENSITY, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_DIRECTION) &&
           VerifyFieldRequired<DeepSeaScene::Color3f>(verifier, VT_COLOR) &&
           VerifyField<float>(verifier, VT_INTENSITY) &&
           verifier.EndTable();
  }
};

struct DirectionalLightBuilder {
  typedef DirectionalLight Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_direction(const DeepSeaScene::Vector3f *direction) {
    fbb_.AddStruct(DirectionalLight::VT_DIRECTION, direction);
  }
  void add_color(const DeepSeaScene::Color3f *color) {
    fbb_.AddStruct(DirectionalLight::VT_COLOR, color);
  }
  void add_intensity(float intensity) {
    fbb_.AddElement<float>(DirectionalLight::VT_INTENSITY, intensity, 0.0f);
  }
  explicit DirectionalLightBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<DirectionalLight> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<DirectionalLight>(end);
    fbb_.Required(o, DirectionalLight::VT_DIRECTION);
    fbb_.Required(o, DirectionalLight::VT_COLOR);
    return o;
  }
};

inline flatbuffers::Offset<DirectionalLight> CreateDirectionalLight(
    flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector3f *direction = 0,
    const DeepSeaScene::Color3f *color = 0,
    float intensity = 0.0f) {
  DirectionalLightBuilder builder_(_fbb);
  builder_.add_intensity(intensity);
  builder_.add_color(color);
  builder_.add_direction(direction);
  return builder_.Finish();
}

struct PointLight FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef PointLightBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_POSITION = 4,
    VT_COLOR = 6,
    VT_INTENSITY = 8,
    VT_LINEARFALLOFF = 10,
    VT_QUADRATICFALLOFF = 12
  };
  const DeepSeaScene::Vector3f *position() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_POSITION);
  }
  const DeepSeaScene::Color3f *color() const {
    return GetStruct<const DeepSeaScene::Color3f *>(VT_COLOR);
  }
  float intensity() const {
    return GetField<float>(VT_INTENSITY, 0.0f);
  }
  float linearFalloff() const {
    return GetField<float>(VT_LINEARFALLOFF, 0.0f);
  }
  float quadraticFalloff() const {
    return GetField<float>(VT_QUADRATICFALLOFF, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_POSITION) &&
           VerifyFieldRequired<DeepSeaScene::Color3f>(verifier, VT_COLOR) &&
           VerifyField<float>(verifier, VT_INTENSITY) &&
           VerifyField<float>(verifier, VT_LINEARFALLOFF) &&
           VerifyField<float>(verifier, VT_QUADRATICFALLOFF) &&
           verifier.EndTable();
  }
};

struct PointLightBuilder {
  typedef PointLight Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_position(const DeepSeaScene::Vector3f *position) {
    fbb_.AddStruct(PointLight::VT_POSITION, position);
  }
  void add_color(const DeepSeaScene::Color3f *color) {
    fbb_.AddStruct(PointLight::VT_COLOR, color);
  }
  void add_intensity(float intensity) {
    fbb_.AddElement<float>(PointLight::VT_INTENSITY, intensity, 0.0f);
  }
  void add_linearFalloff(float linearFalloff) {
    fbb_.AddElement<float>(PointLight::VT_LINEARFALLOFF, linearFalloff, 0.0f);
  }
  void add_quadraticFalloff(float quadraticFalloff) {
    fbb_.AddElement<float>(PointLight::VT_QUADRATICFALLOFF, quadraticFalloff, 0.0f);
  }
  explicit PointLightBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<PointLight> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<PointLight>(end);
    fbb_.Required(o, PointLight::VT_POSITION);
    fbb_.Required(o, PointLight::VT_COLOR);
    return o;
  }
};

inline flatbuffers::Offset<PointLight> CreatePointLight(
    flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector3f *position = 0,
    const DeepSeaScene::Color3f *color = 0,
    float intensity = 0.0f,
    float linearFalloff = 0.0f,
    float quadraticFalloff = 0.0f) {
  PointLightBuilder builder_(_fbb);
  builder_.add_quadraticFalloff(quadraticFalloff);
  builder_.add_linearFalloff(linearFalloff);
  builder_.add_intensity(intensity);
  builder_.add_color(color);
  builder_.add_position(position);
  return builder_.Finish();
}

struct SpotLight FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SpotLightBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_POSITION = 4,
    VT_DIRECTION = 6,
    VT_COLOR = 8,
    VT_INTENSITY = 10,
    VT_LINEARFALLOFF = 12,
    VT_QUADRATICFALLOFF = 14,
    VT_INNERSPOTANGLE = 16,
    VT_OUTERSPOTANGLE = 18
  };
  const DeepSeaScene::Vector3f *position() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_POSITION);
  }
  const DeepSeaScene::Vector3f *direction() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_DIRECTION);
  }
  const DeepSeaScene::Color3f *color() const {
    return GetStruct<const DeepSeaScene::Color3f *>(VT_COLOR);
  }
  float intensity() const {
    return GetField<float>(VT_INTENSITY, 0.0f);
  }
  float linearFalloff() const {
    return GetField<float>(VT_LINEARFALLOFF, 0.0f);
  }
  float quadraticFalloff() const {
    return GetField<float>(VT_QUADRATICFALLOFF, 0.0f);
  }
  float innerSpotAngle() const {
    return GetField<float>(VT_INNERSPOTANGLE, 0.0f);
  }
  float outerSpotAngle() const {
    return GetField<float>(VT_OUTERSPOTANGLE, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_POSITION) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_DIRECTION) &&
           VerifyFieldRequired<DeepSeaScene::Color3f>(verifier, VT_COLOR) &&
           VerifyField<float>(verifier, VT_INTENSITY) &&
           VerifyField<float>(verifier, VT_LINEARFALLOFF) &&
           VerifyField<float>(verifier, VT_QUADRATICFALLOFF) &&
           VerifyField<float>(verifier, VT_INNERSPOTANGLE) &&
           VerifyField<float>(verifier, VT_OUTERSPOTANGLE) &&
           verifier.EndTable();
  }
};

struct SpotLightBuilder {
  typedef SpotLight Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_position(const DeepSeaScene::Vector3f *position) {
    fbb_.AddStruct(SpotLight::VT_POSITION, position);
  }
  void add_direction(const DeepSeaScene::Vector3f *direction) {
    fbb_.AddStruct(SpotLight::VT_DIRECTION, direction);
  }
  void add_color(const DeepSeaScene::Color3f *color) {
    fbb_.AddStruct(SpotLight::VT_COLOR, color);
  }
  void add_intensity(float intensity) {
    fbb_.AddElement<float>(SpotLight::VT_INTENSITY, intensity, 0.0f);
  }
  void add_linearFalloff(float linearFalloff) {
    fbb_.AddElement<float>(SpotLight::VT_LINEARFALLOFF, linearFalloff, 0.0f);
  }
  void add_quadraticFalloff(float quadraticFalloff) {
    fbb_.AddElement<float>(SpotLight::VT_QUADRATICFALLOFF, quadraticFalloff, 0.0f);
  }
  void add_innerSpotAngle(float innerSpotAngle) {
    fbb_.AddElement<float>(SpotLight::VT_INNERSPOTANGLE, innerSpotAngle, 0.0f);
  }
  void add_outerSpotAngle(float outerSpotAngle) {
    fbb_.AddElement<float>(SpotLight::VT_OUTERSPOTANGLE, outerSpotAngle, 0.0f);
  }
  explicit SpotLightBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<SpotLight> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SpotLight>(end);
    fbb_.Required(o, SpotLight::VT_POSITION);
    fbb_.Required(o, SpotLight::VT_DIRECTION);
    fbb_.Required(o, SpotLight::VT_COLOR);
    return o;
  }
};

inline flatbuffers::Offset<SpotLight> CreateSpotLight(
    flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector3f *position = 0,
    const DeepSeaScene::Vector3f *direction = 0,
    const DeepSeaScene::Color3f *color = 0,
    float intensity = 0.0f,
    float linearFalloff = 0.0f,
    float quadraticFalloff = 0.0f,
    float innerSpotAngle = 0.0f,
    float outerSpotAngle = 0.0f) {
  SpotLightBuilder builder_(_fbb);
  builder_.add_outerSpotAngle(outerSpotAngle);
  builder_.add_innerSpotAngle(innerSpotAngle);
  builder_.add_quadraticFalloff(quadraticFalloff);
  builder_.add_linearFalloff(linearFalloff);
  builder_.add_intensity(intensity);
  builder_.add_color(color);
  builder_.add_direction(direction);
  builder_.add_position(position);
  return builder_.Finish();
}

struct Light FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef LightBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_LIGHT_TYPE = 6,
    VT_LIGHT = 8
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  DeepSeaSceneLighting::LightUnion light_type() const {
    return static_cast<DeepSeaSceneLighting::LightUnion>(GetField<uint8_t>(VT_LIGHT_TYPE, 0));
  }
  const void *light() const {
    return GetPointer<const void *>(VT_LIGHT);
  }
  template<typename T> const T *light_as() const;
  const DeepSeaSceneLighting::DirectionalLight *light_as_DirectionalLight() const {
    return light_type() == DeepSeaSceneLighting::LightUnion::DirectionalLight ? static_cast<const DeepSeaSceneLighting::DirectionalLight *>(light()) : nullptr;
  }
  const DeepSeaSceneLighting::PointLight *light_as_PointLight() const {
    return light_type() == DeepSeaSceneLighting::LightUnion::PointLight ? static_cast<const DeepSeaSceneLighting::PointLight *>(light()) : nullptr;
  }
  const DeepSeaSceneLighting::SpotLight *light_as_SpotLight() const {
    return light_type() == DeepSeaSceneLighting::LightUnion::SpotLight ? static_cast<const DeepSeaSceneLighting::SpotLight *>(light()) : nullptr;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint8_t>(verifier, VT_LIGHT_TYPE) &&
           VerifyOffsetRequired(verifier, VT_LIGHT) &&
           VerifyLightUnion(verifier, light(), light_type()) &&
           verifier.EndTable();
  }
};

template<> inline const DeepSeaSceneLighting::DirectionalLight *Light::light_as<DeepSeaSceneLighting::DirectionalLight>() const {
  return light_as_DirectionalLight();
}

template<> inline const DeepSeaSceneLighting::PointLight *Light::light_as<DeepSeaSceneLighting::PointLight>() const {
  return light_as_PointLight();
}

template<> inline const DeepSeaSceneLighting::SpotLight *Light::light_as<DeepSeaSceneLighting::SpotLight>() const {
  return light_as_SpotLight();
}

struct LightBuilder {
  typedef Light Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Light::VT_NAME, name);
  }
  void add_light_type(DeepSeaSceneLighting::LightUnion light_type) {
    fbb_.AddElement<uint8_t>(Light::VT_LIGHT_TYPE, static_cast<uint8_t>(light_type), 0);
  }
  void add_light(flatbuffers::Offset<void> light) {
    fbb_.AddOffset(Light::VT_LIGHT, light);
  }
  explicit LightBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Light> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Light>(end);
    fbb_.Required(o, Light::VT_NAME);
    fbb_.Required(o, Light::VT_LIGHT);
    return o;
  }
};

inline flatbuffers::Offset<Light> CreateLight(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    DeepSeaSceneLighting::LightUnion light_type = DeepSeaSceneLighting::LightUnion::NONE,
    flatbuffers::Offset<void> light = 0) {
  LightBuilder builder_(_fbb);
  builder_.add_light(light);
  builder_.add_name(name);
  builder_.add_light_type(light_type);
  return builder_.Finish();
}

inline flatbuffers::Offset<Light> CreateLightDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    DeepSeaSceneLighting::LightUnion light_type = DeepSeaSceneLighting::LightUnion::NONE,
    flatbuffers::Offset<void> light = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaSceneLighting::CreateLight(
      _fbb,
      name__,
      light_type,
      light);
}

struct SceneLightSet FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SceneLightSetBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_LIGHTS = 4,
    VT_MAXLIGHTS = 6,
    VT_AMBIENTCOLOR = 8,
    VT_AMBIENTINTENSITY = 10
  };
  const flatbuffers::Vector<flatbuffers::Offset<DeepSeaSceneLighting::Light>> *lights() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<DeepSeaSceneLighting::Light>> *>(VT_LIGHTS);
  }
  uint32_t maxLights() const {
    return GetField<uint32_t>(VT_MAXLIGHTS, 0);
  }
  const DeepSeaScene::Color3f *ambientColor() const {
    return GetStruct<const DeepSeaScene::Color3f *>(VT_AMBIENTCOLOR);
  }
  float ambientIntensity() const {
    return GetField<float>(VT_AMBIENTINTENSITY, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_LIGHTS) &&
           verifier.VerifyVector(lights()) &&
           verifier.VerifyVectorOfTables(lights()) &&
           VerifyField<uint32_t>(verifier, VT_MAXLIGHTS) &&
           VerifyField<DeepSeaScene::Color3f>(verifier, VT_AMBIENTCOLOR) &&
           VerifyField<float>(verifier, VT_AMBIENTINTENSITY) &&
           verifier.EndTable();
  }
};

struct SceneLightSetBuilder {
  typedef SceneLightSet Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_lights(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaSceneLighting::Light>>> lights) {
    fbb_.AddOffset(SceneLightSet::VT_LIGHTS, lights);
  }
  void add_maxLights(uint32_t maxLights) {
    fbb_.AddElement<uint32_t>(SceneLightSet::VT_MAXLIGHTS, maxLights, 0);
  }
  void add_ambientColor(const DeepSeaScene::Color3f *ambientColor) {
    fbb_.AddStruct(SceneLightSet::VT_AMBIENTCOLOR, ambientColor);
  }
  void add_ambientIntensity(float ambientIntensity) {
    fbb_.AddElement<float>(SceneLightSet::VT_AMBIENTINTENSITY, ambientIntensity, 0.0f);
  }
  explicit SceneLightSetBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<SceneLightSet> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SceneLightSet>(end);
    return o;
  }
};

inline flatbuffers::Offset<SceneLightSet> CreateSceneLightSet(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaSceneLighting::Light>>> lights = 0,
    uint32_t maxLights = 0,
    const DeepSeaScene::Color3f *ambientColor = 0,
    float ambientIntensity = 0.0f) {
  SceneLightSetBuilder builder_(_fbb);
  builder_.add_ambientIntensity(ambientIntensity);
  builder_.add_ambientColor(ambientColor);
  builder_.add_maxLights(maxLights);
  builder_.add_lights(lights);
  return builder_.Finish();
}

inline flatbuffers::Offset<SceneLightSet> CreateSceneLightSetDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<DeepSeaSceneLighting::Light>> *lights = nullptr,
    uint32_t maxLights = 0,
    const DeepSeaScene::Color3f *ambientColor = 0,
    float ambientIntensity = 0.0f) {
  auto lights__ = lights ? _fbb.CreateVector<flatbuffers::Offset<DeepSeaSceneLighting::Light>>(*lights) : 0;
  return DeepSeaSceneLighting::CreateSceneLightSet(
      _fbb,
      lights__,
      maxLights,
      ambientColor,
      ambientIntensity);
}

inline bool VerifyLightUnion(flatbuffers::Verifier &verifier, const void *obj, LightUnion type) {
  switch (type) {
    case LightUnion::NONE: {
      return true;
    }
    case LightUnion::DirectionalLight: {
      auto ptr = reinterpret_cast<const DeepSeaSceneLighting::DirectionalLight *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case LightUnion::PointLight: {
      auto ptr = reinterpret_cast<const DeepSeaSceneLighting::PointLight *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case LightUnion::SpotLight: {
      auto ptr = reinterpret_cast<const DeepSeaSceneLighting::SpotLight *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyLightUnionVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyLightUnion(
        verifier,  values->Get(i), types->GetEnum<LightUnion>(i))) {
      return false;
    }
  }
  return true;
}

inline const DeepSeaSceneLighting::SceneLightSet *GetSceneLightSet(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaSceneLighting::SceneLightSet>(buf);
}

inline const DeepSeaSceneLighting::SceneLightSet *GetSizePrefixedSceneLightSet(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaSceneLighting::SceneLightSet>(buf);
}

inline bool VerifySceneLightSetBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneLighting::SceneLightSet>(nullptr);
}

inline bool VerifySizePrefixedSceneLightSetBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneLighting::SceneLightSet>(nullptr);
}

inline void FinishSceneLightSetBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::SceneLightSet> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedSceneLightSetBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaSceneLighting::SceneLightSet> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneLighting

#endif  // FLATBUFFERS_GENERATED_SCENELIGHTSET_DEEPSEASCENELIGHTING_H_

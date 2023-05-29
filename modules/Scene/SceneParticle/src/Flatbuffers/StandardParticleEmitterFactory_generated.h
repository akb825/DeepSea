// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_STANDARDPARTICLEEMITTERFACTORY_DEEPSEASCENEPARTICLE_H_
#define FLATBUFFERS_GENERATED_STANDARDPARTICLEEMITTERFACTORY_DEEPSEASCENEPARTICLE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 26,
             "Non-compatible flatbuffers version included");

#include "DeepSea/SceneParticle/Flatbuffers/SceneParticleCommon_generated.h"

namespace DeepSeaSceneParticle {

struct StandardParticleEmitterFactory;
struct StandardParticleEmitterFactoryBuilder;

struct StandardParticleEmitterFactory FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef StandardParticleEmitterFactoryBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_PARAMS = 4,
    VT_SPAWNVOLUME_TYPE = 6,
    VT_SPAWNVOLUME = 8,
    VT_SPAWNVOLUMEMATRIX = 10,
    VT_WIDTHRANGE = 12,
    VT_HEIGHTRANGE = 14,
    VT_ROTATIONRANGE = 16,
    VT_BASEDIRECTION = 18,
    VT_DIRECTIONSPREAD = 20,
    VT_SPAWNTIMERANGE = 22,
    VT_ACTIVETIMERANGE = 24,
    VT_SPEEDRANGE = 26,
    VT_ROTATIONSPEEDRANGE = 28,
    VT_TEXTURERANGE = 30,
    VT_COLORHUERANGE = 32,
    VT_COLORSATURATIONRANGE = 34,
    VT_COLORVALUERANGE = 36,
    VT_COLORALPHARANGE = 38,
    VT_INTENSITYRANGE = 40,
    VT_RELATIVENODE = 42,
    VT_SEED = 44,
    VT_ENABLED = 46,
    VT_STARTTIME = 48
  };
  const DeepSeaSceneParticle::ParticleEmitterParams *params() const {
    return GetPointer<const DeepSeaSceneParticle::ParticleEmitterParams *>(VT_PARAMS);
  }
  DeepSeaSceneParticle::ParticleVolume spawnVolume_type() const {
    return static_cast<DeepSeaSceneParticle::ParticleVolume>(GetField<uint8_t>(VT_SPAWNVOLUME_TYPE, 0));
  }
  const void *spawnVolume() const {
    return GetPointer<const void *>(VT_SPAWNVOLUME);
  }
  template<typename T> const T *spawnVolume_as() const;
  const DeepSeaSceneParticle::ParticleBox *spawnVolume_as_ParticleBox() const {
    return spawnVolume_type() == DeepSeaSceneParticle::ParticleVolume::ParticleBox ? static_cast<const DeepSeaSceneParticle::ParticleBox *>(spawnVolume()) : nullptr;
  }
  const DeepSeaSceneParticle::ParticleSphere *spawnVolume_as_ParticleSphere() const {
    return spawnVolume_type() == DeepSeaSceneParticle::ParticleVolume::ParticleSphere ? static_cast<const DeepSeaSceneParticle::ParticleSphere *>(spawnVolume()) : nullptr;
  }
  const DeepSeaSceneParticle::ParticleCylinder *spawnVolume_as_ParticleCylinder() const {
    return spawnVolume_type() == DeepSeaSceneParticle::ParticleVolume::ParticleCylinder ? static_cast<const DeepSeaSceneParticle::ParticleCylinder *>(spawnVolume()) : nullptr;
  }
  const DeepSeaScene::Matrix44f *spawnVolumeMatrix() const {
    return GetStruct<const DeepSeaScene::Matrix44f *>(VT_SPAWNVOLUMEMATRIX);
  }
  const DeepSeaScene::Vector2f *widthRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_WIDTHRANGE);
  }
  const DeepSeaScene::Vector2f *heightRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_HEIGHTRANGE);
  }
  const DeepSeaScene::Vector2f *rotationRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_ROTATIONRANGE);
  }
  const DeepSeaScene::Vector3f *baseDirection() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_BASEDIRECTION);
  }
  float directionSpread() const {
    return GetField<float>(VT_DIRECTIONSPREAD, 0.0f);
  }
  const DeepSeaScene::Vector2f *spawnTimeRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_SPAWNTIMERANGE);
  }
  const DeepSeaScene::Vector2f *activeTimeRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_ACTIVETIMERANGE);
  }
  const DeepSeaScene::Vector2f *speedRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_SPEEDRANGE);
  }
  const DeepSeaScene::Vector2f *rotationSpeedRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_ROTATIONSPEEDRANGE);
  }
  const DeepSeaSceneParticle::Vector2u *textureRange() const {
    return GetStruct<const DeepSeaSceneParticle::Vector2u *>(VT_TEXTURERANGE);
  }
  const DeepSeaScene::Vector2f *colorHueRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_COLORHUERANGE);
  }
  const DeepSeaScene::Vector2f *colorSaturationRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_COLORSATURATIONRANGE);
  }
  const DeepSeaScene::Vector2f *colorValueRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_COLORVALUERANGE);
  }
  const DeepSeaScene::Vector2f *colorAlphaRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_COLORALPHARANGE);
  }
  const DeepSeaScene::Vector2f *intensityRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_INTENSITYRANGE);
  }
  const ::flatbuffers::String *relativeNode() const {
    return GetPointer<const ::flatbuffers::String *>(VT_RELATIVENODE);
  }
  uint64_t seed() const {
    return GetField<uint64_t>(VT_SEED, 0);
  }
  bool enabled() const {
    return GetField<uint8_t>(VT_ENABLED, 0) != 0;
  }
  float startTime() const {
    return GetField<float>(VT_STARTTIME, 0.0f);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_PARAMS) &&
           verifier.VerifyTable(params()) &&
           VerifyField<uint8_t>(verifier, VT_SPAWNVOLUME_TYPE, 1) &&
           VerifyOffsetRequired(verifier, VT_SPAWNVOLUME) &&
           VerifyParticleVolume(verifier, spawnVolume(), spawnVolume_type()) &&
           VerifyFieldRequired<DeepSeaScene::Matrix44f>(verifier, VT_SPAWNVOLUMEMATRIX, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_WIDTHRANGE, 4) &&
           VerifyField<DeepSeaScene::Vector2f>(verifier, VT_HEIGHTRANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_ROTATIONRANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_BASEDIRECTION, 4) &&
           VerifyField<float>(verifier, VT_DIRECTIONSPREAD, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_SPAWNTIMERANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_ACTIVETIMERANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_SPEEDRANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_ROTATIONSPEEDRANGE, 4) &&
           VerifyFieldRequired<DeepSeaSceneParticle::Vector2u>(verifier, VT_TEXTURERANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_COLORHUERANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_COLORSATURATIONRANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_COLORVALUERANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_COLORALPHARANGE, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_INTENSITYRANGE, 4) &&
           VerifyOffset(verifier, VT_RELATIVENODE) &&
           verifier.VerifyString(relativeNode()) &&
           VerifyField<uint64_t>(verifier, VT_SEED, 8) &&
           VerifyField<uint8_t>(verifier, VT_ENABLED, 1) &&
           VerifyField<float>(verifier, VT_STARTTIME, 4) &&
           verifier.EndTable();
  }
};

template<> inline const DeepSeaSceneParticle::ParticleBox *StandardParticleEmitterFactory::spawnVolume_as<DeepSeaSceneParticle::ParticleBox>() const {
  return spawnVolume_as_ParticleBox();
}

template<> inline const DeepSeaSceneParticle::ParticleSphere *StandardParticleEmitterFactory::spawnVolume_as<DeepSeaSceneParticle::ParticleSphere>() const {
  return spawnVolume_as_ParticleSphere();
}

template<> inline const DeepSeaSceneParticle::ParticleCylinder *StandardParticleEmitterFactory::spawnVolume_as<DeepSeaSceneParticle::ParticleCylinder>() const {
  return spawnVolume_as_ParticleCylinder();
}

struct StandardParticleEmitterFactoryBuilder {
  typedef StandardParticleEmitterFactory Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_params(::flatbuffers::Offset<DeepSeaSceneParticle::ParticleEmitterParams> params) {
    fbb_.AddOffset(StandardParticleEmitterFactory::VT_PARAMS, params);
  }
  void add_spawnVolume_type(DeepSeaSceneParticle::ParticleVolume spawnVolume_type) {
    fbb_.AddElement<uint8_t>(StandardParticleEmitterFactory::VT_SPAWNVOLUME_TYPE, static_cast<uint8_t>(spawnVolume_type), 0);
  }
  void add_spawnVolume(::flatbuffers::Offset<void> spawnVolume) {
    fbb_.AddOffset(StandardParticleEmitterFactory::VT_SPAWNVOLUME, spawnVolume);
  }
  void add_spawnVolumeMatrix(const DeepSeaScene::Matrix44f *spawnVolumeMatrix) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_SPAWNVOLUMEMATRIX, spawnVolumeMatrix);
  }
  void add_widthRange(const DeepSeaScene::Vector2f *widthRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_WIDTHRANGE, widthRange);
  }
  void add_heightRange(const DeepSeaScene::Vector2f *heightRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_HEIGHTRANGE, heightRange);
  }
  void add_rotationRange(const DeepSeaScene::Vector2f *rotationRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_ROTATIONRANGE, rotationRange);
  }
  void add_baseDirection(const DeepSeaScene::Vector3f *baseDirection) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_BASEDIRECTION, baseDirection);
  }
  void add_directionSpread(float directionSpread) {
    fbb_.AddElement<float>(StandardParticleEmitterFactory::VT_DIRECTIONSPREAD, directionSpread, 0.0f);
  }
  void add_spawnTimeRange(const DeepSeaScene::Vector2f *spawnTimeRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_SPAWNTIMERANGE, spawnTimeRange);
  }
  void add_activeTimeRange(const DeepSeaScene::Vector2f *activeTimeRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_ACTIVETIMERANGE, activeTimeRange);
  }
  void add_speedRange(const DeepSeaScene::Vector2f *speedRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_SPEEDRANGE, speedRange);
  }
  void add_rotationSpeedRange(const DeepSeaScene::Vector2f *rotationSpeedRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_ROTATIONSPEEDRANGE, rotationSpeedRange);
  }
  void add_textureRange(const DeepSeaSceneParticle::Vector2u *textureRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_TEXTURERANGE, textureRange);
  }
  void add_colorHueRange(const DeepSeaScene::Vector2f *colorHueRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_COLORHUERANGE, colorHueRange);
  }
  void add_colorSaturationRange(const DeepSeaScene::Vector2f *colorSaturationRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_COLORSATURATIONRANGE, colorSaturationRange);
  }
  void add_colorValueRange(const DeepSeaScene::Vector2f *colorValueRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_COLORVALUERANGE, colorValueRange);
  }
  void add_colorAlphaRange(const DeepSeaScene::Vector2f *colorAlphaRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_COLORALPHARANGE, colorAlphaRange);
  }
  void add_intensityRange(const DeepSeaScene::Vector2f *intensityRange) {
    fbb_.AddStruct(StandardParticleEmitterFactory::VT_INTENSITYRANGE, intensityRange);
  }
  void add_relativeNode(::flatbuffers::Offset<::flatbuffers::String> relativeNode) {
    fbb_.AddOffset(StandardParticleEmitterFactory::VT_RELATIVENODE, relativeNode);
  }
  void add_seed(uint64_t seed) {
    fbb_.AddElement<uint64_t>(StandardParticleEmitterFactory::VT_SEED, seed, 0);
  }
  void add_enabled(bool enabled) {
    fbb_.AddElement<uint8_t>(StandardParticleEmitterFactory::VT_ENABLED, static_cast<uint8_t>(enabled), 0);
  }
  void add_startTime(float startTime) {
    fbb_.AddElement<float>(StandardParticleEmitterFactory::VT_STARTTIME, startTime, 0.0f);
  }
  explicit StandardParticleEmitterFactoryBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<StandardParticleEmitterFactory> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<StandardParticleEmitterFactory>(end);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_PARAMS);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_SPAWNVOLUME);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_SPAWNVOLUMEMATRIX);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_WIDTHRANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_ROTATIONRANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_BASEDIRECTION);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_SPAWNTIMERANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_ACTIVETIMERANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_SPEEDRANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_ROTATIONSPEEDRANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_TEXTURERANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_COLORHUERANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_COLORSATURATIONRANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_COLORVALUERANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_COLORALPHARANGE);
    fbb_.Required(o, StandardParticleEmitterFactory::VT_INTENSITYRANGE);
    return o;
  }
};

inline ::flatbuffers::Offset<StandardParticleEmitterFactory> CreateStandardParticleEmitterFactory(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<DeepSeaSceneParticle::ParticleEmitterParams> params = 0,
    DeepSeaSceneParticle::ParticleVolume spawnVolume_type = DeepSeaSceneParticle::ParticleVolume::NONE,
    ::flatbuffers::Offset<void> spawnVolume = 0,
    const DeepSeaScene::Matrix44f *spawnVolumeMatrix = nullptr,
    const DeepSeaScene::Vector2f *widthRange = nullptr,
    const DeepSeaScene::Vector2f *heightRange = nullptr,
    const DeepSeaScene::Vector2f *rotationRange = nullptr,
    const DeepSeaScene::Vector3f *baseDirection = nullptr,
    float directionSpread = 0.0f,
    const DeepSeaScene::Vector2f *spawnTimeRange = nullptr,
    const DeepSeaScene::Vector2f *activeTimeRange = nullptr,
    const DeepSeaScene::Vector2f *speedRange = nullptr,
    const DeepSeaScene::Vector2f *rotationSpeedRange = nullptr,
    const DeepSeaSceneParticle::Vector2u *textureRange = nullptr,
    const DeepSeaScene::Vector2f *colorHueRange = nullptr,
    const DeepSeaScene::Vector2f *colorSaturationRange = nullptr,
    const DeepSeaScene::Vector2f *colorValueRange = nullptr,
    const DeepSeaScene::Vector2f *colorAlphaRange = nullptr,
    const DeepSeaScene::Vector2f *intensityRange = nullptr,
    ::flatbuffers::Offset<::flatbuffers::String> relativeNode = 0,
    uint64_t seed = 0,
    bool enabled = false,
    float startTime = 0.0f) {
  StandardParticleEmitterFactoryBuilder builder_(_fbb);
  builder_.add_seed(seed);
  builder_.add_startTime(startTime);
  builder_.add_relativeNode(relativeNode);
  builder_.add_intensityRange(intensityRange);
  builder_.add_colorAlphaRange(colorAlphaRange);
  builder_.add_colorValueRange(colorValueRange);
  builder_.add_colorSaturationRange(colorSaturationRange);
  builder_.add_colorHueRange(colorHueRange);
  builder_.add_textureRange(textureRange);
  builder_.add_rotationSpeedRange(rotationSpeedRange);
  builder_.add_speedRange(speedRange);
  builder_.add_activeTimeRange(activeTimeRange);
  builder_.add_spawnTimeRange(spawnTimeRange);
  builder_.add_directionSpread(directionSpread);
  builder_.add_baseDirection(baseDirection);
  builder_.add_rotationRange(rotationRange);
  builder_.add_heightRange(heightRange);
  builder_.add_widthRange(widthRange);
  builder_.add_spawnVolumeMatrix(spawnVolumeMatrix);
  builder_.add_spawnVolume(spawnVolume);
  builder_.add_params(params);
  builder_.add_enabled(enabled);
  builder_.add_spawnVolume_type(spawnVolume_type);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<StandardParticleEmitterFactory> CreateStandardParticleEmitterFactoryDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<DeepSeaSceneParticle::ParticleEmitterParams> params = 0,
    DeepSeaSceneParticle::ParticleVolume spawnVolume_type = DeepSeaSceneParticle::ParticleVolume::NONE,
    ::flatbuffers::Offset<void> spawnVolume = 0,
    const DeepSeaScene::Matrix44f *spawnVolumeMatrix = nullptr,
    const DeepSeaScene::Vector2f *widthRange = nullptr,
    const DeepSeaScene::Vector2f *heightRange = nullptr,
    const DeepSeaScene::Vector2f *rotationRange = nullptr,
    const DeepSeaScene::Vector3f *baseDirection = nullptr,
    float directionSpread = 0.0f,
    const DeepSeaScene::Vector2f *spawnTimeRange = nullptr,
    const DeepSeaScene::Vector2f *activeTimeRange = nullptr,
    const DeepSeaScene::Vector2f *speedRange = nullptr,
    const DeepSeaScene::Vector2f *rotationSpeedRange = nullptr,
    const DeepSeaSceneParticle::Vector2u *textureRange = nullptr,
    const DeepSeaScene::Vector2f *colorHueRange = nullptr,
    const DeepSeaScene::Vector2f *colorSaturationRange = nullptr,
    const DeepSeaScene::Vector2f *colorValueRange = nullptr,
    const DeepSeaScene::Vector2f *colorAlphaRange = nullptr,
    const DeepSeaScene::Vector2f *intensityRange = nullptr,
    const char *relativeNode = nullptr,
    uint64_t seed = 0,
    bool enabled = false,
    float startTime = 0.0f) {
  auto relativeNode__ = relativeNode ? _fbb.CreateString(relativeNode) : 0;
  return DeepSeaSceneParticle::CreateStandardParticleEmitterFactory(
      _fbb,
      params,
      spawnVolume_type,
      spawnVolume,
      spawnVolumeMatrix,
      widthRange,
      heightRange,
      rotationRange,
      baseDirection,
      directionSpread,
      spawnTimeRange,
      activeTimeRange,
      speedRange,
      rotationSpeedRange,
      textureRange,
      colorHueRange,
      colorSaturationRange,
      colorValueRange,
      colorAlphaRange,
      intensityRange,
      relativeNode__,
      seed,
      enabled,
      startTime);
}

inline const DeepSeaSceneParticle::StandardParticleEmitterFactory *GetStandardParticleEmitterFactory(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneParticle::StandardParticleEmitterFactory>(buf);
}

inline const DeepSeaSceneParticle::StandardParticleEmitterFactory *GetSizePrefixedStandardParticleEmitterFactory(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneParticle::StandardParticleEmitterFactory>(buf);
}

inline bool VerifyStandardParticleEmitterFactoryBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneParticle::StandardParticleEmitterFactory>(nullptr);
}

inline bool VerifySizePrefixedStandardParticleEmitterFactoryBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneParticle::StandardParticleEmitterFactory>(nullptr);
}

inline void FinishStandardParticleEmitterFactoryBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneParticle::StandardParticleEmitterFactory> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedStandardParticleEmitterFactoryBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneParticle::StandardParticleEmitterFactory> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneParticle

#endif  // FLATBUFFERS_GENERATED_STANDARDPARTICLEEMITTERFACTORY_DEEPSEASCENEPARTICLE_H_

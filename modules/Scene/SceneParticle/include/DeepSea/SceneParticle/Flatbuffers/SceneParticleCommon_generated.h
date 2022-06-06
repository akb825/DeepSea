// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENEPARTICLECOMMON_DEEPSEASCENEPARTICLE_H_
#define FLATBUFFERS_GENERATED_SCENEPARTICLECOMMON_DEEPSEASCENEPARTICLE_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaSceneParticle {

struct Vector2u;

struct ParticleEmitterParams;
struct ParticleEmitterParamsBuilder;

struct ParticleBox;
struct ParticleBoxBuilder;

struct ParticleSphere;
struct ParticleSphereBuilder;

struct ParticleCylinder;
struct ParticleCylinderBuilder;

enum class ParticleVolume : uint8_t {
  NONE = 0,
  ParticleBox = 1,
  ParticleSphere = 2,
  ParticleCylinder = 3,
  MIN = NONE,
  MAX = ParticleCylinder
};

inline const ParticleVolume (&EnumValuesParticleVolume())[4] {
  static const ParticleVolume values[] = {
    ParticleVolume::NONE,
    ParticleVolume::ParticleBox,
    ParticleVolume::ParticleSphere,
    ParticleVolume::ParticleCylinder
  };
  return values;
}

inline const char * const *EnumNamesParticleVolume() {
  static const char * const names[5] = {
    "NONE",
    "ParticleBox",
    "ParticleSphere",
    "ParticleCylinder",
    nullptr
  };
  return names;
}

inline const char *EnumNameParticleVolume(ParticleVolume e) {
  if (flatbuffers::IsOutRange(e, ParticleVolume::NONE, ParticleVolume::ParticleCylinder)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesParticleVolume()[index];
}

template<typename T> struct ParticleVolumeTraits {
  static const ParticleVolume enum_value = ParticleVolume::NONE;
};

template<> struct ParticleVolumeTraits<DeepSeaSceneParticle::ParticleBox> {
  static const ParticleVolume enum_value = ParticleVolume::ParticleBox;
};

template<> struct ParticleVolumeTraits<DeepSeaSceneParticle::ParticleSphere> {
  static const ParticleVolume enum_value = ParticleVolume::ParticleSphere;
};

template<> struct ParticleVolumeTraits<DeepSeaSceneParticle::ParticleCylinder> {
  static const ParticleVolume enum_value = ParticleVolume::ParticleCylinder;
};

bool VerifyParticleVolume(flatbuffers::Verifier &verifier, const void *obj, ParticleVolume type);
bool VerifyParticleVolumeVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<ParticleVolume> *types);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Vector2u FLATBUFFERS_FINAL_CLASS {
 private:
  uint32_t x_;
  uint32_t y_;

 public:
  Vector2u()
      : x_(0),
        y_(0) {
  }
  Vector2u(uint32_t _x, uint32_t _y)
      : x_(flatbuffers::EndianScalar(_x)),
        y_(flatbuffers::EndianScalar(_y)) {
  }
  uint32_t x() const {
    return flatbuffers::EndianScalar(x_);
  }
  uint32_t y() const {
    return flatbuffers::EndianScalar(y_);
  }
};
FLATBUFFERS_STRUCT_END(Vector2u, 8);

struct ParticleEmitterParams FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ParticleEmitterParamsBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MAXPARTICLES = 4,
    VT_SHADER = 6,
    VT_MATERIAL = 8,
    VT_INSTANCEVALUECOUNT = 10
  };
  uint32_t maxParticles() const {
    return GetField<uint32_t>(VT_MAXPARTICLES, 0);
  }
  const flatbuffers::String *shader() const {
    return GetPointer<const flatbuffers::String *>(VT_SHADER);
  }
  const flatbuffers::String *material() const {
    return GetPointer<const flatbuffers::String *>(VT_MATERIAL);
  }
  uint32_t instanceValueCount() const {
    return GetField<uint32_t>(VT_INSTANCEVALUECOUNT, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_MAXPARTICLES, 4) &&
           VerifyOffsetRequired(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffsetRequired(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           VerifyField<uint32_t>(verifier, VT_INSTANCEVALUECOUNT, 4) &&
           verifier.EndTable();
  }
};

struct ParticleEmitterParamsBuilder {
  typedef ParticleEmitterParams Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_maxParticles(uint32_t maxParticles) {
    fbb_.AddElement<uint32_t>(ParticleEmitterParams::VT_MAXPARTICLES, maxParticles, 0);
  }
  void add_shader(flatbuffers::Offset<flatbuffers::String> shader) {
    fbb_.AddOffset(ParticleEmitterParams::VT_SHADER, shader);
  }
  void add_material(flatbuffers::Offset<flatbuffers::String> material) {
    fbb_.AddOffset(ParticleEmitterParams::VT_MATERIAL, material);
  }
  void add_instanceValueCount(uint32_t instanceValueCount) {
    fbb_.AddElement<uint32_t>(ParticleEmitterParams::VT_INSTANCEVALUECOUNT, instanceValueCount, 0);
  }
  explicit ParticleEmitterParamsBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ParticleEmitterParams> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ParticleEmitterParams>(end);
    fbb_.Required(o, ParticleEmitterParams::VT_SHADER);
    fbb_.Required(o, ParticleEmitterParams::VT_MATERIAL);
    return o;
  }
};

inline flatbuffers::Offset<ParticleEmitterParams> CreateParticleEmitterParams(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t maxParticles = 0,
    flatbuffers::Offset<flatbuffers::String> shader = 0,
    flatbuffers::Offset<flatbuffers::String> material = 0,
    uint32_t instanceValueCount = 0) {
  ParticleEmitterParamsBuilder builder_(_fbb);
  builder_.add_instanceValueCount(instanceValueCount);
  builder_.add_material(material);
  builder_.add_shader(shader);
  builder_.add_maxParticles(maxParticles);
  return builder_.Finish();
}

inline flatbuffers::Offset<ParticleEmitterParams> CreateParticleEmitterParamsDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t maxParticles = 0,
    const char *shader = nullptr,
    const char *material = nullptr,
    uint32_t instanceValueCount = 0) {
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  return DeepSeaSceneParticle::CreateParticleEmitterParams(
      _fbb,
      maxParticles,
      shader__,
      material__,
      instanceValueCount);
}

struct ParticleBox FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ParticleBoxBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MIN = 4,
    VT_MAX = 6
  };
  const DeepSeaScene::Vector3f *min() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_MIN);
  }
  const DeepSeaScene::Vector3f *max() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_MAX);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_MIN, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_MAX, 4) &&
           verifier.EndTable();
  }
};

struct ParticleBoxBuilder {
  typedef ParticleBox Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_min(const DeepSeaScene::Vector3f *min) {
    fbb_.AddStruct(ParticleBox::VT_MIN, min);
  }
  void add_max(const DeepSeaScene::Vector3f *max) {
    fbb_.AddStruct(ParticleBox::VT_MAX, max);
  }
  explicit ParticleBoxBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ParticleBox> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ParticleBox>(end);
    fbb_.Required(o, ParticleBox::VT_MIN);
    fbb_.Required(o, ParticleBox::VT_MAX);
    return o;
  }
};

inline flatbuffers::Offset<ParticleBox> CreateParticleBox(
    flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector3f *min = nullptr,
    const DeepSeaScene::Vector3f *max = nullptr) {
  ParticleBoxBuilder builder_(_fbb);
  builder_.add_max(max);
  builder_.add_min(min);
  return builder_.Finish();
}

struct ParticleSphere FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ParticleSphereBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CENTER = 4,
    VT_RADIUS = 6
  };
  const DeepSeaScene::Vector3f *center() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_CENTER);
  }
  float radius() const {
    return GetField<float>(VT_RADIUS, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_CENTER, 4) &&
           VerifyField<float>(verifier, VT_RADIUS, 4) &&
           verifier.EndTable();
  }
};

struct ParticleSphereBuilder {
  typedef ParticleSphere Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_center(const DeepSeaScene::Vector3f *center) {
    fbb_.AddStruct(ParticleSphere::VT_CENTER, center);
  }
  void add_radius(float radius) {
    fbb_.AddElement<float>(ParticleSphere::VT_RADIUS, radius, 0.0f);
  }
  explicit ParticleSphereBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ParticleSphere> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ParticleSphere>(end);
    fbb_.Required(o, ParticleSphere::VT_CENTER);
    return o;
  }
};

inline flatbuffers::Offset<ParticleSphere> CreateParticleSphere(
    flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector3f *center = nullptr,
    float radius = 0.0f) {
  ParticleSphereBuilder builder_(_fbb);
  builder_.add_radius(radius);
  builder_.add_center(center);
  return builder_.Finish();
}

struct ParticleCylinder FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ParticleCylinderBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CENTER = 4,
    VT_RADIUS = 6,
    VT_HEIGHT = 8
  };
  const DeepSeaScene::Vector3f *center() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_CENTER);
  }
  float radius() const {
    return GetField<float>(VT_RADIUS, 0.0f);
  }
  float height() const {
    return GetField<float>(VT_HEIGHT, 0.0f);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_CENTER, 4) &&
           VerifyField<float>(verifier, VT_RADIUS, 4) &&
           VerifyField<float>(verifier, VT_HEIGHT, 4) &&
           verifier.EndTable();
  }
};

struct ParticleCylinderBuilder {
  typedef ParticleCylinder Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_center(const DeepSeaScene::Vector3f *center) {
    fbb_.AddStruct(ParticleCylinder::VT_CENTER, center);
  }
  void add_radius(float radius) {
    fbb_.AddElement<float>(ParticleCylinder::VT_RADIUS, radius, 0.0f);
  }
  void add_height(float height) {
    fbb_.AddElement<float>(ParticleCylinder::VT_HEIGHT, height, 0.0f);
  }
  explicit ParticleCylinderBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ParticleCylinder> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ParticleCylinder>(end);
    fbb_.Required(o, ParticleCylinder::VT_CENTER);
    return o;
  }
};

inline flatbuffers::Offset<ParticleCylinder> CreateParticleCylinder(
    flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector3f *center = nullptr,
    float radius = 0.0f,
    float height = 0.0f) {
  ParticleCylinderBuilder builder_(_fbb);
  builder_.add_height(height);
  builder_.add_radius(radius);
  builder_.add_center(center);
  return builder_.Finish();
}

inline bool VerifyParticleVolume(flatbuffers::Verifier &verifier, const void *obj, ParticleVolume type) {
  switch (type) {
    case ParticleVolume::NONE: {
      return true;
    }
    case ParticleVolume::ParticleBox: {
      auto ptr = reinterpret_cast<const DeepSeaSceneParticle::ParticleBox *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ParticleVolume::ParticleSphere: {
      auto ptr = reinterpret_cast<const DeepSeaSceneParticle::ParticleSphere *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ParticleVolume::ParticleCylinder: {
      auto ptr = reinterpret_cast<const DeepSeaSceneParticle::ParticleCylinder *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyParticleVolumeVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<ParticleVolume> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyParticleVolume(
        verifier,  values->Get(i), types->GetEnum<ParticleVolume>(i))) {
      return false;
    }
  }
  return true;
}

}  // namespace DeepSeaSceneParticle

#endif  // FLATBUFFERS_GENERATED_SCENEPARTICLECOMMON_DEEPSEASCENEPARTICLE_H_

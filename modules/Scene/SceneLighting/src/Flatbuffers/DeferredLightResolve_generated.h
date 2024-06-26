// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_DEFERREDLIGHTRESOLVE_DEEPSEASCENELIGHTING_H_
#define FLATBUFFERS_GENERATED_DEFERREDLIGHTRESOLVE_DEEPSEASCENELIGHTING_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace DeepSeaSceneLighting {

struct DeferredLightInfo;
struct DeferredLightInfoBuilder;

struct DeferredShadowLightInfo;
struct DeferredShadowLightInfoBuilder;

struct DeferredLightResolve;
struct DeferredLightResolveBuilder;

struct DeferredLightInfo FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef DeferredLightInfoBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SHADER = 4,
    VT_MATERIAL = 6
  };
  const ::flatbuffers::String *shader() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHADER);
  }
  const ::flatbuffers::String *material() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MATERIAL);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffsetRequired(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           verifier.EndTable();
  }
};

struct DeferredLightInfoBuilder {
  typedef DeferredLightInfo Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_shader(::flatbuffers::Offset<::flatbuffers::String> shader) {
    fbb_.AddOffset(DeferredLightInfo::VT_SHADER, shader);
  }
  void add_material(::flatbuffers::Offset<::flatbuffers::String> material) {
    fbb_.AddOffset(DeferredLightInfo::VT_MATERIAL, material);
  }
  explicit DeferredLightInfoBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<DeferredLightInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<DeferredLightInfo>(end);
    fbb_.Required(o, DeferredLightInfo::VT_SHADER);
    fbb_.Required(o, DeferredLightInfo::VT_MATERIAL);
    return o;
  }
};

inline ::flatbuffers::Offset<DeferredLightInfo> CreateDeferredLightInfo(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> shader = 0,
    ::flatbuffers::Offset<::flatbuffers::String> material = 0) {
  DeferredLightInfoBuilder builder_(_fbb);
  builder_.add_material(material);
  builder_.add_shader(shader);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<DeferredLightInfo> CreateDeferredLightInfoDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *shader = nullptr,
    const char *material = nullptr) {
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  return DeepSeaSceneLighting::CreateDeferredLightInfo(
      _fbb,
      shader__,
      material__);
}

struct DeferredShadowLightInfo FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef DeferredShadowLightInfoBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SHADER = 4,
    VT_MATERIAL = 6,
    VT_TRANSFORMGROUP = 8,
    VT_SHADOWTEXTURE = 10
  };
  const ::flatbuffers::String *shader() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHADER);
  }
  const ::flatbuffers::String *material() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MATERIAL);
  }
  const ::flatbuffers::String *transformGroup() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TRANSFORMGROUP);
  }
  const ::flatbuffers::String *shadowTexture() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHADOWTEXTURE);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffsetRequired(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           VerifyOffsetRequired(verifier, VT_TRANSFORMGROUP) &&
           verifier.VerifyString(transformGroup()) &&
           VerifyOffsetRequired(verifier, VT_SHADOWTEXTURE) &&
           verifier.VerifyString(shadowTexture()) &&
           verifier.EndTable();
  }
};

struct DeferredShadowLightInfoBuilder {
  typedef DeferredShadowLightInfo Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_shader(::flatbuffers::Offset<::flatbuffers::String> shader) {
    fbb_.AddOffset(DeferredShadowLightInfo::VT_SHADER, shader);
  }
  void add_material(::flatbuffers::Offset<::flatbuffers::String> material) {
    fbb_.AddOffset(DeferredShadowLightInfo::VT_MATERIAL, material);
  }
  void add_transformGroup(::flatbuffers::Offset<::flatbuffers::String> transformGroup) {
    fbb_.AddOffset(DeferredShadowLightInfo::VT_TRANSFORMGROUP, transformGroup);
  }
  void add_shadowTexture(::flatbuffers::Offset<::flatbuffers::String> shadowTexture) {
    fbb_.AddOffset(DeferredShadowLightInfo::VT_SHADOWTEXTURE, shadowTexture);
  }
  explicit DeferredShadowLightInfoBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<DeferredShadowLightInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<DeferredShadowLightInfo>(end);
    fbb_.Required(o, DeferredShadowLightInfo::VT_SHADER);
    fbb_.Required(o, DeferredShadowLightInfo::VT_MATERIAL);
    fbb_.Required(o, DeferredShadowLightInfo::VT_TRANSFORMGROUP);
    fbb_.Required(o, DeferredShadowLightInfo::VT_SHADOWTEXTURE);
    return o;
  }
};

inline ::flatbuffers::Offset<DeferredShadowLightInfo> CreateDeferredShadowLightInfo(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> shader = 0,
    ::flatbuffers::Offset<::flatbuffers::String> material = 0,
    ::flatbuffers::Offset<::flatbuffers::String> transformGroup = 0,
    ::flatbuffers::Offset<::flatbuffers::String> shadowTexture = 0) {
  DeferredShadowLightInfoBuilder builder_(_fbb);
  builder_.add_shadowTexture(shadowTexture);
  builder_.add_transformGroup(transformGroup);
  builder_.add_material(material);
  builder_.add_shader(shader);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<DeferredShadowLightInfo> CreateDeferredShadowLightInfoDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *shader = nullptr,
    const char *material = nullptr,
    const char *transformGroup = nullptr,
    const char *shadowTexture = nullptr) {
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  auto transformGroup__ = transformGroup ? _fbb.CreateString(transformGroup) : 0;
  auto shadowTexture__ = shadowTexture ? _fbb.CreateString(shadowTexture) : 0;
  return DeepSeaSceneLighting::CreateDeferredShadowLightInfo(
      _fbb,
      shader__,
      material__,
      transformGroup__,
      shadowTexture__);
}

struct DeferredLightResolve FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef DeferredLightResolveBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_LIGHTSET = 4,
    VT_SHADOWMANAGER = 6,
    VT_AMBIENT = 8,
    VT_DIRECTIONAL = 10,
    VT_POINT = 12,
    VT_SPOT = 14,
    VT_SHADOWDIRECTIONAL = 16,
    VT_SHADOWPOINT = 18,
    VT_SHADOWSPOT = 20,
    VT_INTENSITYTHRESHOLD = 22
  };
  const ::flatbuffers::String *lightSet() const {
    return GetPointer<const ::flatbuffers::String *>(VT_LIGHTSET);
  }
  const ::flatbuffers::String *shadowManager() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHADOWMANAGER);
  }
  const DeepSeaSceneLighting::DeferredLightInfo *ambient() const {
    return GetPointer<const DeepSeaSceneLighting::DeferredLightInfo *>(VT_AMBIENT);
  }
  const DeepSeaSceneLighting::DeferredLightInfo *directional() const {
    return GetPointer<const DeepSeaSceneLighting::DeferredLightInfo *>(VT_DIRECTIONAL);
  }
  const DeepSeaSceneLighting::DeferredLightInfo *point() const {
    return GetPointer<const DeepSeaSceneLighting::DeferredLightInfo *>(VT_POINT);
  }
  const DeepSeaSceneLighting::DeferredLightInfo *spot() const {
    return GetPointer<const DeepSeaSceneLighting::DeferredLightInfo *>(VT_SPOT);
  }
  const DeepSeaSceneLighting::DeferredShadowLightInfo *shadowDirectional() const {
    return GetPointer<const DeepSeaSceneLighting::DeferredShadowLightInfo *>(VT_SHADOWDIRECTIONAL);
  }
  const DeepSeaSceneLighting::DeferredShadowLightInfo *shadowPoint() const {
    return GetPointer<const DeepSeaSceneLighting::DeferredShadowLightInfo *>(VT_SHADOWPOINT);
  }
  const DeepSeaSceneLighting::DeferredShadowLightInfo *shadowSpot() const {
    return GetPointer<const DeepSeaSceneLighting::DeferredShadowLightInfo *>(VT_SHADOWSPOT);
  }
  float intensityThreshold() const {
    return GetField<float>(VT_INTENSITYTHRESHOLD, 0.0f);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_LIGHTSET) &&
           verifier.VerifyString(lightSet()) &&
           VerifyOffset(verifier, VT_SHADOWMANAGER) &&
           verifier.VerifyString(shadowManager()) &&
           VerifyOffset(verifier, VT_AMBIENT) &&
           verifier.VerifyTable(ambient()) &&
           VerifyOffset(verifier, VT_DIRECTIONAL) &&
           verifier.VerifyTable(directional()) &&
           VerifyOffset(verifier, VT_POINT) &&
           verifier.VerifyTable(point()) &&
           VerifyOffset(verifier, VT_SPOT) &&
           verifier.VerifyTable(spot()) &&
           VerifyOffset(verifier, VT_SHADOWDIRECTIONAL) &&
           verifier.VerifyTable(shadowDirectional()) &&
           VerifyOffset(verifier, VT_SHADOWPOINT) &&
           verifier.VerifyTable(shadowPoint()) &&
           VerifyOffset(verifier, VT_SHADOWSPOT) &&
           verifier.VerifyTable(shadowSpot()) &&
           VerifyField<float>(verifier, VT_INTENSITYTHRESHOLD, 4) &&
           verifier.EndTable();
  }
};

struct DeferredLightResolveBuilder {
  typedef DeferredLightResolve Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_lightSet(::flatbuffers::Offset<::flatbuffers::String> lightSet) {
    fbb_.AddOffset(DeferredLightResolve::VT_LIGHTSET, lightSet);
  }
  void add_shadowManager(::flatbuffers::Offset<::flatbuffers::String> shadowManager) {
    fbb_.AddOffset(DeferredLightResolve::VT_SHADOWMANAGER, shadowManager);
  }
  void add_ambient(::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> ambient) {
    fbb_.AddOffset(DeferredLightResolve::VT_AMBIENT, ambient);
  }
  void add_directional(::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> directional) {
    fbb_.AddOffset(DeferredLightResolve::VT_DIRECTIONAL, directional);
  }
  void add_point(::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> point) {
    fbb_.AddOffset(DeferredLightResolve::VT_POINT, point);
  }
  void add_spot(::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> spot) {
    fbb_.AddOffset(DeferredLightResolve::VT_SPOT, spot);
  }
  void add_shadowDirectional(::flatbuffers::Offset<DeepSeaSceneLighting::DeferredShadowLightInfo> shadowDirectional) {
    fbb_.AddOffset(DeferredLightResolve::VT_SHADOWDIRECTIONAL, shadowDirectional);
  }
  void add_shadowPoint(::flatbuffers::Offset<DeepSeaSceneLighting::DeferredShadowLightInfo> shadowPoint) {
    fbb_.AddOffset(DeferredLightResolve::VT_SHADOWPOINT, shadowPoint);
  }
  void add_shadowSpot(::flatbuffers::Offset<DeepSeaSceneLighting::DeferredShadowLightInfo> shadowSpot) {
    fbb_.AddOffset(DeferredLightResolve::VT_SHADOWSPOT, shadowSpot);
  }
  void add_intensityThreshold(float intensityThreshold) {
    fbb_.AddElement<float>(DeferredLightResolve::VT_INTENSITYTHRESHOLD, intensityThreshold, 0.0f);
  }
  explicit DeferredLightResolveBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<DeferredLightResolve> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<DeferredLightResolve>(end);
    fbb_.Required(o, DeferredLightResolve::VT_LIGHTSET);
    return o;
  }
};

inline ::flatbuffers::Offset<DeferredLightResolve> CreateDeferredLightResolve(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> lightSet = 0,
    ::flatbuffers::Offset<::flatbuffers::String> shadowManager = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> ambient = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> directional = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> point = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> spot = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredShadowLightInfo> shadowDirectional = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredShadowLightInfo> shadowPoint = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredShadowLightInfo> shadowSpot = 0,
    float intensityThreshold = 0.0f) {
  DeferredLightResolveBuilder builder_(_fbb);
  builder_.add_intensityThreshold(intensityThreshold);
  builder_.add_shadowSpot(shadowSpot);
  builder_.add_shadowPoint(shadowPoint);
  builder_.add_shadowDirectional(shadowDirectional);
  builder_.add_spot(spot);
  builder_.add_point(point);
  builder_.add_directional(directional);
  builder_.add_ambient(ambient);
  builder_.add_shadowManager(shadowManager);
  builder_.add_lightSet(lightSet);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<DeferredLightResolve> CreateDeferredLightResolveDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *lightSet = nullptr,
    const char *shadowManager = nullptr,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> ambient = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> directional = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> point = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightInfo> spot = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredShadowLightInfo> shadowDirectional = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredShadowLightInfo> shadowPoint = 0,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredShadowLightInfo> shadowSpot = 0,
    float intensityThreshold = 0.0f) {
  auto lightSet__ = lightSet ? _fbb.CreateString(lightSet) : 0;
  auto shadowManager__ = shadowManager ? _fbb.CreateString(shadowManager) : 0;
  return DeepSeaSceneLighting::CreateDeferredLightResolve(
      _fbb,
      lightSet__,
      shadowManager__,
      ambient,
      directional,
      point,
      spot,
      shadowDirectional,
      shadowPoint,
      shadowSpot,
      intensityThreshold);
}

inline const DeepSeaSceneLighting::DeferredLightResolve *GetDeferredLightResolve(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneLighting::DeferredLightResolve>(buf);
}

inline const DeepSeaSceneLighting::DeferredLightResolve *GetSizePrefixedDeferredLightResolve(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneLighting::DeferredLightResolve>(buf);
}

inline bool VerifyDeferredLightResolveBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneLighting::DeferredLightResolve>(nullptr);
}

inline bool VerifySizePrefixedDeferredLightResolveBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneLighting::DeferredLightResolve>(nullptr);
}

inline void FinishDeferredLightResolveBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightResolve> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedDeferredLightResolveBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneLighting::DeferredLightResolve> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneLighting

#endif  // FLATBUFFERS_GENERATED_DEFERREDLIGHTRESOLVE_DEEPSEASCENELIGHTING_H_

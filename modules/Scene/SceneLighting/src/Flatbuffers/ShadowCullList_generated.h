// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SHADOWCULLLIST_DEEPSEASCENELIGHTING_H_
#define FLATBUFFERS_GENERATED_SHADOWCULLLIST_DEEPSEASCENELIGHTING_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace DeepSeaSceneLighting {

struct ShadowCullList;
struct ShadowCullListBuilder;

struct ShadowCullList FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ShadowCullListBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SHADOWMANAGER = 4,
    VT_SHADOWS = 6,
    VT_SURFACE = 8
  };
  const ::flatbuffers::String *shadowManager() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHADOWMANAGER);
  }
  const ::flatbuffers::String *shadows() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHADOWS);
  }
  uint8_t surface() const {
    return GetField<uint8_t>(VT_SURFACE, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_SHADOWMANAGER) &&
           verifier.VerifyString(shadowManager()) &&
           VerifyOffsetRequired(verifier, VT_SHADOWS) &&
           verifier.VerifyString(shadows()) &&
           VerifyField<uint8_t>(verifier, VT_SURFACE, 1) &&
           verifier.EndTable();
  }
};

struct ShadowCullListBuilder {
  typedef ShadowCullList Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_shadowManager(::flatbuffers::Offset<::flatbuffers::String> shadowManager) {
    fbb_.AddOffset(ShadowCullList::VT_SHADOWMANAGER, shadowManager);
  }
  void add_shadows(::flatbuffers::Offset<::flatbuffers::String> shadows) {
    fbb_.AddOffset(ShadowCullList::VT_SHADOWS, shadows);
  }
  void add_surface(uint8_t surface) {
    fbb_.AddElement<uint8_t>(ShadowCullList::VT_SURFACE, surface, 0);
  }
  explicit ShadowCullListBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ShadowCullList> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ShadowCullList>(end);
    fbb_.Required(o, ShadowCullList::VT_SHADOWMANAGER);
    fbb_.Required(o, ShadowCullList::VT_SHADOWS);
    return o;
  }
};

inline ::flatbuffers::Offset<ShadowCullList> CreateShadowCullList(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> shadowManager = 0,
    ::flatbuffers::Offset<::flatbuffers::String> shadows = 0,
    uint8_t surface = 0) {
  ShadowCullListBuilder builder_(_fbb);
  builder_.add_shadows(shadows);
  builder_.add_shadowManager(shadowManager);
  builder_.add_surface(surface);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ShadowCullList> CreateShadowCullListDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *shadowManager = nullptr,
    const char *shadows = nullptr,
    uint8_t surface = 0) {
  auto shadowManager__ = shadowManager ? _fbb.CreateString(shadowManager) : 0;
  auto shadows__ = shadows ? _fbb.CreateString(shadows) : 0;
  return DeepSeaSceneLighting::CreateShadowCullList(
      _fbb,
      shadowManager__,
      shadows__,
      surface);
}

inline const DeepSeaSceneLighting::ShadowCullList *GetShadowCullList(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneLighting::ShadowCullList>(buf);
}

inline const DeepSeaSceneLighting::ShadowCullList *GetSizePrefixedShadowCullList(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneLighting::ShadowCullList>(buf);
}

inline bool VerifyShadowCullListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneLighting::ShadowCullList>(nullptr);
}

inline bool VerifySizePrefixedShadowCullListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneLighting::ShadowCullList>(nullptr);
}

inline void FinishShadowCullListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneLighting::ShadowCullList> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedShadowCullListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneLighting::ShadowCullList> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneLighting

#endif  // FLATBUFFERS_GENERATED_SHADOWCULLLIST_DEEPSEASCENELIGHTING_H_

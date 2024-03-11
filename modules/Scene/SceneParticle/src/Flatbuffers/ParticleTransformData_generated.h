// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PARTICLETRANSFORMDATA_DEEPSEASCENEPARTICLE_H_
#define FLATBUFFERS_GENERATED_PARTICLETRANSFORMDATA_DEEPSEASCENEPARTICLE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 7,
             "Non-compatible flatbuffers version included");

namespace DeepSeaSceneParticle {

struct ParticleTransformData;
struct ParticleTransformDataBuilder;

struct ParticleTransformData FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ParticleTransformDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VARIABLEGROUPDESC = 4
  };
  const ::flatbuffers::String *variableGroupDesc() const {
    return GetPointer<const ::flatbuffers::String *>(VT_VARIABLEGROUPDESC);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_VARIABLEGROUPDESC) &&
           verifier.VerifyString(variableGroupDesc()) &&
           verifier.EndTable();
  }
};

struct ParticleTransformDataBuilder {
  typedef ParticleTransformData Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_variableGroupDesc(::flatbuffers::Offset<::flatbuffers::String> variableGroupDesc) {
    fbb_.AddOffset(ParticleTransformData::VT_VARIABLEGROUPDESC, variableGroupDesc);
  }
  explicit ParticleTransformDataBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ParticleTransformData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ParticleTransformData>(end);
    fbb_.Required(o, ParticleTransformData::VT_VARIABLEGROUPDESC);
    return o;
  }
};

inline ::flatbuffers::Offset<ParticleTransformData> CreateParticleTransformData(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> variableGroupDesc = 0) {
  ParticleTransformDataBuilder builder_(_fbb);
  builder_.add_variableGroupDesc(variableGroupDesc);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ParticleTransformData> CreateParticleTransformDataDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *variableGroupDesc = nullptr) {
  auto variableGroupDesc__ = variableGroupDesc ? _fbb.CreateString(variableGroupDesc) : 0;
  return DeepSeaSceneParticle::CreateParticleTransformData(
      _fbb,
      variableGroupDesc__);
}

inline const DeepSeaSceneParticle::ParticleTransformData *GetParticleTransformData(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneParticle::ParticleTransformData>(buf);
}

inline const DeepSeaSceneParticle::ParticleTransformData *GetSizePrefixedParticleTransformData(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneParticle::ParticleTransformData>(buf);
}

inline bool VerifyParticleTransformDataBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneParticle::ParticleTransformData>(nullptr);
}

inline bool VerifySizePrefixedParticleTransformDataBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneParticle::ParticleTransformData>(nullptr);
}

inline void FinishParticleTransformDataBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneParticle::ParticleTransformData> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedParticleTransformDataBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneParticle::ParticleTransformData> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneParticle

#endif  // FLATBUFFERS_GENERATED_PARTICLETRANSFORMDATA_DEEPSEASCENEPARTICLE_H_

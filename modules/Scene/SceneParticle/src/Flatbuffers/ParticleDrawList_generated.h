// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PARTICLEDRAWLIST_DEEPSEASCENEPARTICLE_H_
#define FLATBUFFERS_GENERATED_PARTICLEDRAWLIST_DEEPSEASCENEPARTICLE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 26,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaSceneParticle {

struct ParticleDrawList;
struct ParticleDrawListBuilder;

struct ParticleDrawList FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ParticleDrawListBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_INSTANCEDATA = 4,
    VT_CULLLIST = 6
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *instanceData() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *>(VT_INSTANCEDATA);
  }
  const ::flatbuffers::String *cullList() const {
    return GetPointer<const ::flatbuffers::String *>(VT_CULLLIST);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_INSTANCEDATA) &&
           verifier.VerifyVector(instanceData()) &&
           verifier.VerifyVectorOfTables(instanceData()) &&
           VerifyOffset(verifier, VT_CULLLIST) &&
           verifier.VerifyString(cullList()) &&
           verifier.EndTable();
  }
};

struct ParticleDrawListBuilder {
  typedef ParticleDrawList Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_instanceData(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> instanceData) {
    fbb_.AddOffset(ParticleDrawList::VT_INSTANCEDATA, instanceData);
  }
  void add_cullList(::flatbuffers::Offset<::flatbuffers::String> cullList) {
    fbb_.AddOffset(ParticleDrawList::VT_CULLLIST, cullList);
  }
  explicit ParticleDrawListBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ParticleDrawList> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ParticleDrawList>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ParticleDrawList> CreateParticleDrawList(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>> instanceData = 0,
    ::flatbuffers::Offset<::flatbuffers::String> cullList = 0) {
  ParticleDrawListBuilder builder_(_fbb);
  builder_.add_cullList(cullList);
  builder_.add_instanceData(instanceData);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<ParticleDrawList> CreateParticleDrawListDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<DeepSeaScene::ObjectData>> *instanceData = nullptr,
    const char *cullList = nullptr) {
  auto instanceData__ = instanceData ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaScene::ObjectData>>(*instanceData) : 0;
  auto cullList__ = cullList ? _fbb.CreateString(cullList) : 0;
  return DeepSeaSceneParticle::CreateParticleDrawList(
      _fbb,
      instanceData__,
      cullList__);
}

inline const DeepSeaSceneParticle::ParticleDrawList *GetParticleDrawList(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneParticle::ParticleDrawList>(buf);
}

inline const DeepSeaSceneParticle::ParticleDrawList *GetSizePrefixedParticleDrawList(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneParticle::ParticleDrawList>(buf);
}

inline bool VerifyParticleDrawListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneParticle::ParticleDrawList>(nullptr);
}

inline bool VerifySizePrefixedParticleDrawListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneParticle::ParticleDrawList>(nullptr);
}

inline void FinishParticleDrawListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneParticle::ParticleDrawList> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedParticleDrawListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneParticle::ParticleDrawList> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneParticle

#endif  // FLATBUFFERS_GENERATED_PARTICLEDRAWLIST_DEEPSEASCENEPARTICLE_H_

// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENEVECTORRESOURCES_DEEPSEASCENEVECTORDRAW_H_
#define FLATBUFFERS_GENERATED_SCENEVECTORRESOURCES_DEEPSEASCENEVECTORDRAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaSceneVectorDraw {

struct VectorResources;
struct VectorResourcesBuilder;

struct VectorResources FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef VectorResourcesBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_RESOURCES_TYPE = 4,
    VT_RESOURCES = 6
  };
  DeepSeaScene::FileOrData resources_type() const {
    return static_cast<DeepSeaScene::FileOrData>(GetField<uint8_t>(VT_RESOURCES_TYPE, 0));
  }
  const void *resources() const {
    return GetPointer<const void *>(VT_RESOURCES);
  }
  template<typename T> const T *resources_as() const;
  const DeepSeaScene::FileReference *resources_as_FileReference() const {
    return resources_type() == DeepSeaScene::FileOrData::FileReference ? static_cast<const DeepSeaScene::FileReference *>(resources()) : nullptr;
  }
  const DeepSeaScene::RelativePathReference *resources_as_RelativePathReference() const {
    return resources_type() == DeepSeaScene::FileOrData::RelativePathReference ? static_cast<const DeepSeaScene::RelativePathReference *>(resources()) : nullptr;
  }
  const DeepSeaScene::RawData *resources_as_RawData() const {
    return resources_type() == DeepSeaScene::FileOrData::RawData ? static_cast<const DeepSeaScene::RawData *>(resources()) : nullptr;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_RESOURCES_TYPE, 1) &&
           VerifyOffsetRequired(verifier, VT_RESOURCES) &&
           VerifyFileOrData(verifier, resources(), resources_type()) &&
           verifier.EndTable();
  }
};

template<> inline const DeepSeaScene::FileReference *VectorResources::resources_as<DeepSeaScene::FileReference>() const {
  return resources_as_FileReference();
}

template<> inline const DeepSeaScene::RelativePathReference *VectorResources::resources_as<DeepSeaScene::RelativePathReference>() const {
  return resources_as_RelativePathReference();
}

template<> inline const DeepSeaScene::RawData *VectorResources::resources_as<DeepSeaScene::RawData>() const {
  return resources_as_RawData();
}

struct VectorResourcesBuilder {
  typedef VectorResources Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_resources_type(DeepSeaScene::FileOrData resources_type) {
    fbb_.AddElement<uint8_t>(VectorResources::VT_RESOURCES_TYPE, static_cast<uint8_t>(resources_type), 0);
  }
  void add_resources(::flatbuffers::Offset<void> resources) {
    fbb_.AddOffset(VectorResources::VT_RESOURCES, resources);
  }
  explicit VectorResourcesBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<VectorResources> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<VectorResources>(end);
    fbb_.Required(o, VectorResources::VT_RESOURCES);
    return o;
  }
};

inline ::flatbuffers::Offset<VectorResources> CreateVectorResources(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaScene::FileOrData resources_type = DeepSeaScene::FileOrData::NONE,
    ::flatbuffers::Offset<void> resources = 0) {
  VectorResourcesBuilder builder_(_fbb);
  builder_.add_resources(resources);
  builder_.add_resources_type(resources_type);
  return builder_.Finish();
}

inline const DeepSeaSceneVectorDraw::VectorResources *GetVectorResources(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneVectorDraw::VectorResources>(buf);
}

inline const DeepSeaSceneVectorDraw::VectorResources *GetSizePrefixedVectorResources(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneVectorDraw::VectorResources>(buf);
}

inline bool VerifyVectorResourcesBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneVectorDraw::VectorResources>(nullptr);
}

inline bool VerifySizePrefixedVectorResourcesBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneVectorDraw::VectorResources>(nullptr);
}

inline void FinishVectorResourcesBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorResources> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedVectorResourcesBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorResources> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneVectorDraw

#endif  // FLATBUFFERS_GENERATED_SCENEVECTORRESOURCES_DEEPSEASCENEVECTORDRAW_H_

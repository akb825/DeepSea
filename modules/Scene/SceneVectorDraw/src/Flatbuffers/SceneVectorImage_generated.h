// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENEVECTORIMAGE_DEEPSEASCENEVECTORDRAW_H_
#define FLATBUFFERS_GENERATED_SCENEVECTORIMAGE_DEEPSEASCENEVECTORDRAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 1 &&
              FLATBUFFERS_VERSION_REVISION == 21,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaSceneVectorDraw {

struct VectorImage;
struct VectorImageBuilder;

struct VectorImage FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef VectorImageBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_IMAGE_TYPE = 4,
    VT_IMAGE = 6,
    VT_TARGETSIZE = 8,
    VT_SHAREDMATERIALS = 10,
    VT_VECTORSHADERS = 12,
    VT_RESOURCES = 14,
    VT_SRGB = 16
  };
  DeepSeaScene::FileOrData image_type() const {
    return static_cast<DeepSeaScene::FileOrData>(GetField<uint8_t>(VT_IMAGE_TYPE, 0));
  }
  const void *image() const {
    return GetPointer<const void *>(VT_IMAGE);
  }
  template<typename T> const T *image_as() const;
  const DeepSeaScene::FileReference *image_as_FileReference() const {
    return image_type() == DeepSeaScene::FileOrData::FileReference ? static_cast<const DeepSeaScene::FileReference *>(image()) : nullptr;
  }
  const DeepSeaScene::RawData *image_as_RawData() const {
    return image_type() == DeepSeaScene::FileOrData::RawData ? static_cast<const DeepSeaScene::RawData *>(image()) : nullptr;
  }
  const DeepSeaScene::Vector2f *targetSize() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_TARGETSIZE);
  }
  const ::flatbuffers::String *sharedMaterials() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHAREDMATERIALS);
  }
  const ::flatbuffers::String *vectorShaders() const {
    return GetPointer<const ::flatbuffers::String *>(VT_VECTORSHADERS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *resources() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *>(VT_RESOURCES);
  }
  bool srgb() const {
    return GetField<uint8_t>(VT_SRGB, 0) != 0;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_IMAGE_TYPE, 1) &&
           VerifyOffsetRequired(verifier, VT_IMAGE) &&
           VerifyFileOrData(verifier, image(), image_type()) &&
           VerifyField<DeepSeaScene::Vector2f>(verifier, VT_TARGETSIZE, 4) &&
           VerifyOffset(verifier, VT_SHAREDMATERIALS) &&
           verifier.VerifyString(sharedMaterials()) &&
           VerifyOffsetRequired(verifier, VT_VECTORSHADERS) &&
           verifier.VerifyString(vectorShaders()) &&
           VerifyOffset(verifier, VT_RESOURCES) &&
           verifier.VerifyVector(resources()) &&
           verifier.VerifyVectorOfStrings(resources()) &&
           VerifyField<uint8_t>(verifier, VT_SRGB, 1) &&
           verifier.EndTable();
  }
};

template<> inline const DeepSeaScene::FileReference *VectorImage::image_as<DeepSeaScene::FileReference>() const {
  return image_as_FileReference();
}

template<> inline const DeepSeaScene::RawData *VectorImage::image_as<DeepSeaScene::RawData>() const {
  return image_as_RawData();
}

struct VectorImageBuilder {
  typedef VectorImage Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_image_type(DeepSeaScene::FileOrData image_type) {
    fbb_.AddElement<uint8_t>(VectorImage::VT_IMAGE_TYPE, static_cast<uint8_t>(image_type), 0);
  }
  void add_image(::flatbuffers::Offset<void> image) {
    fbb_.AddOffset(VectorImage::VT_IMAGE, image);
  }
  void add_targetSize(const DeepSeaScene::Vector2f *targetSize) {
    fbb_.AddStruct(VectorImage::VT_TARGETSIZE, targetSize);
  }
  void add_sharedMaterials(::flatbuffers::Offset<::flatbuffers::String> sharedMaterials) {
    fbb_.AddOffset(VectorImage::VT_SHAREDMATERIALS, sharedMaterials);
  }
  void add_vectorShaders(::flatbuffers::Offset<::flatbuffers::String> vectorShaders) {
    fbb_.AddOffset(VectorImage::VT_VECTORSHADERS, vectorShaders);
  }
  void add_resources(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> resources) {
    fbb_.AddOffset(VectorImage::VT_RESOURCES, resources);
  }
  void add_srgb(bool srgb) {
    fbb_.AddElement<uint8_t>(VectorImage::VT_SRGB, static_cast<uint8_t>(srgb), 0);
  }
  explicit VectorImageBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<VectorImage> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<VectorImage>(end);
    fbb_.Required(o, VectorImage::VT_IMAGE);
    fbb_.Required(o, VectorImage::VT_VECTORSHADERS);
    return o;
  }
};

inline ::flatbuffers::Offset<VectorImage> CreateVectorImage(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaScene::FileOrData image_type = DeepSeaScene::FileOrData::NONE,
    ::flatbuffers::Offset<void> image = 0,
    const DeepSeaScene::Vector2f *targetSize = nullptr,
    ::flatbuffers::Offset<::flatbuffers::String> sharedMaterials = 0,
    ::flatbuffers::Offset<::flatbuffers::String> vectorShaders = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> resources = 0,
    bool srgb = false) {
  VectorImageBuilder builder_(_fbb);
  builder_.add_resources(resources);
  builder_.add_vectorShaders(vectorShaders);
  builder_.add_sharedMaterials(sharedMaterials);
  builder_.add_targetSize(targetSize);
  builder_.add_image(image);
  builder_.add_srgb(srgb);
  builder_.add_image_type(image_type);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<VectorImage> CreateVectorImageDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaScene::FileOrData image_type = DeepSeaScene::FileOrData::NONE,
    ::flatbuffers::Offset<void> image = 0,
    const DeepSeaScene::Vector2f *targetSize = nullptr,
    const char *sharedMaterials = nullptr,
    const char *vectorShaders = nullptr,
    const std::vector<::flatbuffers::Offset<::flatbuffers::String>> *resources = nullptr,
    bool srgb = false) {
  auto sharedMaterials__ = sharedMaterials ? _fbb.CreateString(sharedMaterials) : 0;
  auto vectorShaders__ = vectorShaders ? _fbb.CreateString(vectorShaders) : 0;
  auto resources__ = resources ? _fbb.CreateVector<::flatbuffers::Offset<::flatbuffers::String>>(*resources) : 0;
  return DeepSeaSceneVectorDraw::CreateVectorImage(
      _fbb,
      image_type,
      image,
      targetSize,
      sharedMaterials__,
      vectorShaders__,
      resources__,
      srgb);
}

inline const DeepSeaSceneVectorDraw::VectorImage *GetVectorImage(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneVectorDraw::VectorImage>(buf);
}

inline const DeepSeaSceneVectorDraw::VectorImage *GetSizePrefixedVectorImage(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneVectorDraw::VectorImage>(buf);
}

inline bool VerifyVectorImageBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneVectorDraw::VectorImage>(nullptr);
}

inline bool VerifySizePrefixedVectorImageBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneVectorDraw::VectorImage>(nullptr);
}

inline void FinishVectorImageBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorImage> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedVectorImageBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorImage> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneVectorDraw

#endif  // FLATBUFFERS_GENERATED_SCENEVECTORIMAGE_DEEPSEASCENEVECTORDRAW_H_

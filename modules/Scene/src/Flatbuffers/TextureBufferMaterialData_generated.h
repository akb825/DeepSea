// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_TEXTUREBUFFERMATERIALDATA_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_TEXTUREBUFFERMATERIALDATA_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct TextureBufferMaterialData;
struct TextureBufferMaterialDataBuilder;

struct TextureBufferMaterialData FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef TextureBufferMaterialDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_FORMAT = 6,
    VT_DECORATION = 8,
    VT_OFFSET = 10,
    VT_COUNT = 12
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  DeepSeaScene::TextureFormat format() const {
    return static_cast<DeepSeaScene::TextureFormat>(GetField<uint8_t>(VT_FORMAT, 0));
  }
  DeepSeaScene::FormatDecoration decoration() const {
    return static_cast<DeepSeaScene::FormatDecoration>(GetField<uint8_t>(VT_DECORATION, 0));
  }
  uint32_t offset() const {
    return GetField<uint32_t>(VT_OFFSET, 0);
  }
  uint32_t count() const {
    return GetField<uint32_t>(VT_COUNT, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint8_t>(verifier, VT_FORMAT) &&
           VerifyField<uint8_t>(verifier, VT_DECORATION) &&
           VerifyField<uint32_t>(verifier, VT_OFFSET) &&
           VerifyField<uint32_t>(verifier, VT_COUNT) &&
           verifier.EndTable();
  }
};

struct TextureBufferMaterialDataBuilder {
  typedef TextureBufferMaterialData Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(TextureBufferMaterialData::VT_NAME, name);
  }
  void add_format(DeepSeaScene::TextureFormat format) {
    fbb_.AddElement<uint8_t>(TextureBufferMaterialData::VT_FORMAT, static_cast<uint8_t>(format), 0);
  }
  void add_decoration(DeepSeaScene::FormatDecoration decoration) {
    fbb_.AddElement<uint8_t>(TextureBufferMaterialData::VT_DECORATION, static_cast<uint8_t>(decoration), 0);
  }
  void add_offset(uint32_t offset) {
    fbb_.AddElement<uint32_t>(TextureBufferMaterialData::VT_OFFSET, offset, 0);
  }
  void add_count(uint32_t count) {
    fbb_.AddElement<uint32_t>(TextureBufferMaterialData::VT_COUNT, count, 0);
  }
  explicit TextureBufferMaterialDataBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  TextureBufferMaterialDataBuilder &operator=(const TextureBufferMaterialDataBuilder &);
  flatbuffers::Offset<TextureBufferMaterialData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<TextureBufferMaterialData>(end);
    fbb_.Required(o, TextureBufferMaterialData::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<TextureBufferMaterialData> CreateTextureBufferMaterialData(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    DeepSeaScene::TextureFormat format = DeepSeaScene::TextureFormat::R4G4,
    DeepSeaScene::FormatDecoration decoration = DeepSeaScene::FormatDecoration::UNorm,
    uint32_t offset = 0,
    uint32_t count = 0) {
  TextureBufferMaterialDataBuilder builder_(_fbb);
  builder_.add_count(count);
  builder_.add_offset(offset);
  builder_.add_name(name);
  builder_.add_decoration(decoration);
  builder_.add_format(format);
  return builder_.Finish();
}

inline flatbuffers::Offset<TextureBufferMaterialData> CreateTextureBufferMaterialDataDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    DeepSeaScene::TextureFormat format = DeepSeaScene::TextureFormat::R4G4,
    DeepSeaScene::FormatDecoration decoration = DeepSeaScene::FormatDecoration::UNorm,
    uint32_t offset = 0,
    uint32_t count = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaScene::CreateTextureBufferMaterialData(
      _fbb,
      name__,
      format,
      decoration,
      offset,
      count);
}

inline const DeepSeaScene::TextureBufferMaterialData *GetTextureBufferMaterialData(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::TextureBufferMaterialData>(buf);
}

inline const DeepSeaScene::TextureBufferMaterialData *GetSizePrefixedTextureBufferMaterialData(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::TextureBufferMaterialData>(buf);
}

inline bool VerifyTextureBufferMaterialDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::TextureBufferMaterialData>(nullptr);
}

inline bool VerifySizePrefixedTextureBufferMaterialDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::TextureBufferMaterialData>(nullptr);
}

inline void FinishTextureBufferMaterialDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::TextureBufferMaterialData> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedTextureBufferMaterialDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::TextureBufferMaterialData> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_TEXTUREBUFFERMATERIALDATA_DEEPSEASCENE_H_

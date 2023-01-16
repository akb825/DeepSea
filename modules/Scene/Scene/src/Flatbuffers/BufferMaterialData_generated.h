// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_BUFFERMATERIALDATA_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_BUFFERMATERIALDATA_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 1 &&
              FLATBUFFERS_VERSION_REVISION == 4,
             "Non-compatible flatbuffers version included");

namespace DeepSeaScene {

struct BufferMaterialData;
struct BufferMaterialDataBuilder;

struct BufferMaterialData FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef BufferMaterialDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_OFFSET = 6,
    VT_SIZE = 8
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  uint32_t offset() const {
    return GetField<uint32_t>(VT_OFFSET, 0);
  }
  uint32_t size() const {
    return GetField<uint32_t>(VT_SIZE, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint32_t>(verifier, VT_OFFSET, 4) &&
           VerifyField<uint32_t>(verifier, VT_SIZE, 4) &&
           verifier.EndTable();
  }
};

struct BufferMaterialDataBuilder {
  typedef BufferMaterialData Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(BufferMaterialData::VT_NAME, name);
  }
  void add_offset(uint32_t offset) {
    fbb_.AddElement<uint32_t>(BufferMaterialData::VT_OFFSET, offset, 0);
  }
  void add_size(uint32_t size) {
    fbb_.AddElement<uint32_t>(BufferMaterialData::VT_SIZE, size, 0);
  }
  explicit BufferMaterialDataBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<BufferMaterialData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<BufferMaterialData>(end);
    fbb_.Required(o, BufferMaterialData::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<BufferMaterialData> CreateBufferMaterialData(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    uint32_t offset = 0,
    uint32_t size = 0) {
  BufferMaterialDataBuilder builder_(_fbb);
  builder_.add_size(size);
  builder_.add_offset(offset);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<BufferMaterialData> CreateBufferMaterialDataDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    uint32_t offset = 0,
    uint32_t size = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaScene::CreateBufferMaterialData(
      _fbb,
      name__,
      offset,
      size);
}

inline const DeepSeaScene::BufferMaterialData *GetBufferMaterialData(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::BufferMaterialData>(buf);
}

inline const DeepSeaScene::BufferMaterialData *GetSizePrefixedBufferMaterialData(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::BufferMaterialData>(buf);
}

inline bool VerifyBufferMaterialDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::BufferMaterialData>(nullptr);
}

inline bool VerifySizePrefixedBufferMaterialDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::BufferMaterialData>(nullptr);
}

inline void FinishBufferMaterialDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::BufferMaterialData> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedBufferMaterialDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::BufferMaterialData> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_BUFFERMATERIALDATA_DEEPSEASCENE_H_

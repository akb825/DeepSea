// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENEVECTORCOMMON_DEEPSEASCENEVECTORDRAW_H_
#define FLATBUFFERS_GENERATED_SCENEVECTORCOMMON_DEEPSEASCENEVECTORDRAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

namespace DeepSeaSceneVectorDraw {

struct Color;

struct VectorResourceRef;
struct VectorResourceRefBuilder;

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(1) Color FLATBUFFERS_FINAL_CLASS {
 private:
  uint8_t red_;
  uint8_t green_;
  uint8_t blue_;
  uint8_t alpha_;

 public:
  Color()
      : red_(0),
        green_(0),
        blue_(0),
        alpha_(0) {
  }
  Color(uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _alpha)
      : red_(::flatbuffers::EndianScalar(_red)),
        green_(::flatbuffers::EndianScalar(_green)),
        blue_(::flatbuffers::EndianScalar(_blue)),
        alpha_(::flatbuffers::EndianScalar(_alpha)) {
  }
  uint8_t red() const {
    return ::flatbuffers::EndianScalar(red_);
  }
  uint8_t green() const {
    return ::flatbuffers::EndianScalar(green_);
  }
  uint8_t blue() const {
    return ::flatbuffers::EndianScalar(blue_);
  }
  uint8_t alpha() const {
    return ::flatbuffers::EndianScalar(alpha_);
  }
};
FLATBUFFERS_STRUCT_END(Color, 4);

struct VectorResourceRef FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef VectorResourceRefBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_RESOURCES = 4,
    VT_NAME = 6
  };
  const ::flatbuffers::String *resources() const {
    return GetPointer<const ::flatbuffers::String *>(VT_RESOURCES);
  }
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_RESOURCES) &&
           verifier.VerifyString(resources()) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           verifier.EndTable();
  }
};

struct VectorResourceRefBuilder {
  typedef VectorResourceRef Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_resources(::flatbuffers::Offset<::flatbuffers::String> resources) {
    fbb_.AddOffset(VectorResourceRef::VT_RESOURCES, resources);
  }
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(VectorResourceRef::VT_NAME, name);
  }
  explicit VectorResourceRefBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<VectorResourceRef> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<VectorResourceRef>(end);
    fbb_.Required(o, VectorResourceRef::VT_RESOURCES);
    fbb_.Required(o, VectorResourceRef::VT_NAME);
    return o;
  }
};

inline ::flatbuffers::Offset<VectorResourceRef> CreateVectorResourceRef(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> resources = 0,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0) {
  VectorResourceRefBuilder builder_(_fbb);
  builder_.add_name(name);
  builder_.add_resources(resources);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<VectorResourceRef> CreateVectorResourceRefDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *resources = nullptr,
    const char *name = nullptr) {
  auto resources__ = resources ? _fbb.CreateString(resources) : 0;
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaSceneVectorDraw::CreateVectorResourceRef(
      _fbb,
      resources__,
      name__);
}

}  // namespace DeepSeaSceneVectorDraw

#endif  // FLATBUFFERS_GENERATED_SCENEVECTORCOMMON_DEEPSEASCENEVECTORDRAW_H_

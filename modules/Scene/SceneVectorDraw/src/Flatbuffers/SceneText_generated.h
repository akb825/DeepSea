// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENETEXT_DEEPSEASCENEVECTORDRAW_H_
#define FLATBUFFERS_GENERATED_SCENETEXT_DEEPSEASCENEVECTORDRAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 7,
             "Non-compatible flatbuffers version included");

#include "SceneVectorCommon_generated.h"

namespace DeepSeaSceneVectorDraw {

struct SceneTextStyle;
struct SceneTextStyleBuilder;

struct SceneText;
struct SceneTextBuilder;

struct SceneTextStyle FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef SceneTextStyleBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_START = 4,
    VT_COUNT = 6,
    VT_SIZE = 8,
    VT_EMBOLDEN = 10,
    VT_SLANT = 12,
    VT_OUTLINEWIDTH = 14,
    VT_FUZINESS = 16,
    VT_VERTICALOFFSET = 18,
    VT_COLOR = 20,
    VT_OUTLINECOLOR = 22
  };
  uint32_t start() const {
    return GetField<uint32_t>(VT_START, 0);
  }
  uint32_t count() const {
    return GetField<uint32_t>(VT_COUNT, 0);
  }
  float size() const {
    return GetField<float>(VT_SIZE, 0.0f);
  }
  float embolden() const {
    return GetField<float>(VT_EMBOLDEN, 0.0f);
  }
  float slant() const {
    return GetField<float>(VT_SLANT, 0.0f);
  }
  float outlineWidth() const {
    return GetField<float>(VT_OUTLINEWIDTH, 0.0f);
  }
  float fuziness() const {
    return GetField<float>(VT_FUZINESS, 0.0f);
  }
  float verticalOffset() const {
    return GetField<float>(VT_VERTICALOFFSET, 0.0f);
  }
  const DeepSeaSceneVectorDraw::Color *color() const {
    return GetStruct<const DeepSeaSceneVectorDraw::Color *>(VT_COLOR);
  }
  const DeepSeaSceneVectorDraw::Color *outlineColor() const {
    return GetStruct<const DeepSeaSceneVectorDraw::Color *>(VT_OUTLINECOLOR);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_START, 4) &&
           VerifyField<uint32_t>(verifier, VT_COUNT, 4) &&
           VerifyField<float>(verifier, VT_SIZE, 4) &&
           VerifyField<float>(verifier, VT_EMBOLDEN, 4) &&
           VerifyField<float>(verifier, VT_SLANT, 4) &&
           VerifyField<float>(verifier, VT_OUTLINEWIDTH, 4) &&
           VerifyField<float>(verifier, VT_FUZINESS, 4) &&
           VerifyField<float>(verifier, VT_VERTICALOFFSET, 4) &&
           VerifyField<DeepSeaSceneVectorDraw::Color>(verifier, VT_COLOR, 1) &&
           VerifyField<DeepSeaSceneVectorDraw::Color>(verifier, VT_OUTLINECOLOR, 1) &&
           verifier.EndTable();
  }
};

struct SceneTextStyleBuilder {
  typedef SceneTextStyle Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_start(uint32_t start) {
    fbb_.AddElement<uint32_t>(SceneTextStyle::VT_START, start, 0);
  }
  void add_count(uint32_t count) {
    fbb_.AddElement<uint32_t>(SceneTextStyle::VT_COUNT, count, 0);
  }
  void add_size(float size) {
    fbb_.AddElement<float>(SceneTextStyle::VT_SIZE, size, 0.0f);
  }
  void add_embolden(float embolden) {
    fbb_.AddElement<float>(SceneTextStyle::VT_EMBOLDEN, embolden, 0.0f);
  }
  void add_slant(float slant) {
    fbb_.AddElement<float>(SceneTextStyle::VT_SLANT, slant, 0.0f);
  }
  void add_outlineWidth(float outlineWidth) {
    fbb_.AddElement<float>(SceneTextStyle::VT_OUTLINEWIDTH, outlineWidth, 0.0f);
  }
  void add_fuziness(float fuziness) {
    fbb_.AddElement<float>(SceneTextStyle::VT_FUZINESS, fuziness, 0.0f);
  }
  void add_verticalOffset(float verticalOffset) {
    fbb_.AddElement<float>(SceneTextStyle::VT_VERTICALOFFSET, verticalOffset, 0.0f);
  }
  void add_color(const DeepSeaSceneVectorDraw::Color *color) {
    fbb_.AddStruct(SceneTextStyle::VT_COLOR, color);
  }
  void add_outlineColor(const DeepSeaSceneVectorDraw::Color *outlineColor) {
    fbb_.AddStruct(SceneTextStyle::VT_OUTLINECOLOR, outlineColor);
  }
  explicit SceneTextStyleBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<SceneTextStyle> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<SceneTextStyle>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<SceneTextStyle> CreateSceneTextStyle(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t start = 0,
    uint32_t count = 0,
    float size = 0.0f,
    float embolden = 0.0f,
    float slant = 0.0f,
    float outlineWidth = 0.0f,
    float fuziness = 0.0f,
    float verticalOffset = 0.0f,
    const DeepSeaSceneVectorDraw::Color *color = nullptr,
    const DeepSeaSceneVectorDraw::Color *outlineColor = nullptr) {
  SceneTextStyleBuilder builder_(_fbb);
  builder_.add_outlineColor(outlineColor);
  builder_.add_color(color);
  builder_.add_verticalOffset(verticalOffset);
  builder_.add_fuziness(fuziness);
  builder_.add_outlineWidth(outlineWidth);
  builder_.add_slant(slant);
  builder_.add_embolden(embolden);
  builder_.add_size(size);
  builder_.add_count(count);
  builder_.add_start(start);
  return builder_.Finish();
}

struct SceneText FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef SceneTextBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_FONT = 4,
    VT_TEXT = 6,
    VT_STYLES = 8
  };
  const DeepSeaSceneVectorDraw::VectorResourceRef *font() const {
    return GetPointer<const DeepSeaSceneVectorDraw::VectorResourceRef *>(VT_FONT);
  }
  const ::flatbuffers::String *text() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TEXT);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::SceneTextStyle>> *styles() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::SceneTextStyle>> *>(VT_STYLES);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_FONT) &&
           verifier.VerifyTable(font()) &&
           VerifyOffsetRequired(verifier, VT_TEXT) &&
           verifier.VerifyString(text()) &&
           VerifyOffsetRequired(verifier, VT_STYLES) &&
           verifier.VerifyVector(styles()) &&
           verifier.VerifyVectorOfTables(styles()) &&
           verifier.EndTable();
  }
};

struct SceneTextBuilder {
  typedef SceneText Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_font(::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorResourceRef> font) {
    fbb_.AddOffset(SceneText::VT_FONT, font);
  }
  void add_text(::flatbuffers::Offset<::flatbuffers::String> text) {
    fbb_.AddOffset(SceneText::VT_TEXT, text);
  }
  void add_styles(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::SceneTextStyle>>> styles) {
    fbb_.AddOffset(SceneText::VT_STYLES, styles);
  }
  explicit SceneTextBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<SceneText> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<SceneText>(end);
    fbb_.Required(o, SceneText::VT_FONT);
    fbb_.Required(o, SceneText::VT_TEXT);
    fbb_.Required(o, SceneText::VT_STYLES);
    return o;
  }
};

inline ::flatbuffers::Offset<SceneText> CreateSceneText(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorResourceRef> font = 0,
    ::flatbuffers::Offset<::flatbuffers::String> text = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::SceneTextStyle>>> styles = 0) {
  SceneTextBuilder builder_(_fbb);
  builder_.add_styles(styles);
  builder_.add_text(text);
  builder_.add_font(font);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<SceneText> CreateSceneTextDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorResourceRef> font = 0,
    const char *text = nullptr,
    const std::vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::SceneTextStyle>> *styles = nullptr) {
  auto text__ = text ? _fbb.CreateString(text) : 0;
  auto styles__ = styles ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::SceneTextStyle>>(*styles) : 0;
  return DeepSeaSceneVectorDraw::CreateSceneText(
      _fbb,
      font,
      text__,
      styles__);
}

inline const DeepSeaSceneVectorDraw::SceneText *GetSceneText(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneVectorDraw::SceneText>(buf);
}

inline const DeepSeaSceneVectorDraw::SceneText *GetSizePrefixedSceneText(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneVectorDraw::SceneText>(buf);
}

inline bool VerifySceneTextBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneVectorDraw::SceneText>(nullptr);
}

inline bool VerifySizePrefixedSceneTextBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneVectorDraw::SceneText>(nullptr);
}

inline void FinishSceneTextBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::SceneText> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedSceneTextBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::SceneText> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneVectorDraw

#endif  // FLATBUFFERS_GENERATED_SCENETEXT_DEEPSEASCENEVECTORDRAW_H_

// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_TEXTNODE_DEEPSEAVECTORDRAWSCENE_H_
#define FLATBUFFERS_GENERATED_TEXTNODE_DEEPSEAVECTORDRAWSCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 22 &&
              FLATBUFFERS_VERSION_MINOR == 9 &&
              FLATBUFFERS_VERSION_REVISION == 29,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"
#include "VectorSceneCommon_generated.h"

namespace DeepSeaVectorDrawScene {

struct TextNode;
struct TextNodeBuilder;

enum class TextAlign : uint8_t {
  Start = 0,
  End = 1,
  Left = 2,
  Right = 3,
  Center = 4,
  MIN = Start,
  MAX = Center
};

inline const TextAlign (&EnumValuesTextAlign())[5] {
  static const TextAlign values[] = {
    TextAlign::Start,
    TextAlign::End,
    TextAlign::Left,
    TextAlign::Right,
    TextAlign::Center
  };
  return values;
}

inline const char * const *EnumNamesTextAlign() {
  static const char * const names[6] = {
    "Start",
    "End",
    "Left",
    "Right",
    "Center",
    nullptr
  };
  return names;
}

inline const char *EnumNameTextAlign(TextAlign e) {
  if (flatbuffers::IsOutRange(e, TextAlign::Start, TextAlign::Center)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesTextAlign()[index];
}

struct TextNode FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef TextNodeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_EMBEDDEDRESOURCES = 4,
    VT_TEXT = 6,
    VT_ALIGNMENT = 8,
    VT_MAXWIDTH = 10,
    VT_LINESCALE = 12,
    VT_Z = 14,
    VT_FIRSTCHAR = 16,
    VT_CHARCOUNT = 18,
    VT_SHADER = 20,
    VT_MATERIAL = 22,
    VT_FONTTEXTURE = 24,
    VT_ITEMLISTS = 26
  };
  const flatbuffers::Vector<uint8_t> *embeddedResources() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_EMBEDDEDRESOURCES);
  }
  const flatbuffers::String *text() const {
    return GetPointer<const flatbuffers::String *>(VT_TEXT);
  }
  DeepSeaVectorDrawScene::TextAlign alignment() const {
    return static_cast<DeepSeaVectorDrawScene::TextAlign>(GetField<uint8_t>(VT_ALIGNMENT, 0));
  }
  float maxWidth() const {
    return GetField<float>(VT_MAXWIDTH, 0.0f);
  }
  float lineScale() const {
    return GetField<float>(VT_LINESCALE, 0.0f);
  }
  int32_t z() const {
    return GetField<int32_t>(VT_Z, 0);
  }
  uint32_t firstChar() const {
    return GetField<uint32_t>(VT_FIRSTCHAR, 0);
  }
  uint32_t charCount() const {
    return GetField<uint32_t>(VT_CHARCOUNT, 0);
  }
  const flatbuffers::String *shader() const {
    return GetPointer<const flatbuffers::String *>(VT_SHADER);
  }
  const flatbuffers::String *material() const {
    return GetPointer<const flatbuffers::String *>(VT_MATERIAL);
  }
  const flatbuffers::String *fontTexture() const {
    return GetPointer<const flatbuffers::String *>(VT_FONTTEXTURE);
  }
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *itemLists() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_ITEMLISTS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_EMBEDDEDRESOURCES) &&
           verifier.VerifyVector(embeddedResources()) &&
           VerifyOffsetRequired(verifier, VT_TEXT) &&
           verifier.VerifyString(text()) &&
           VerifyField<uint8_t>(verifier, VT_ALIGNMENT, 1) &&
           VerifyField<float>(verifier, VT_MAXWIDTH, 4) &&
           VerifyField<float>(verifier, VT_LINESCALE, 4) &&
           VerifyField<int32_t>(verifier, VT_Z, 4) &&
           VerifyField<uint32_t>(verifier, VT_FIRSTCHAR, 4) &&
           VerifyField<uint32_t>(verifier, VT_CHARCOUNT, 4) &&
           VerifyOffsetRequired(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffsetRequired(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           VerifyOffsetRequired(verifier, VT_FONTTEXTURE) &&
           verifier.VerifyString(fontTexture()) &&
           VerifyOffset(verifier, VT_ITEMLISTS) &&
           verifier.VerifyVector(itemLists()) &&
           verifier.VerifyVectorOfStrings(itemLists()) &&
           verifier.EndTable();
  }
};

struct TextNodeBuilder {
  typedef TextNode Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_embeddedResources(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> embeddedResources) {
    fbb_.AddOffset(TextNode::VT_EMBEDDEDRESOURCES, embeddedResources);
  }
  void add_text(flatbuffers::Offset<flatbuffers::String> text) {
    fbb_.AddOffset(TextNode::VT_TEXT, text);
  }
  void add_alignment(DeepSeaVectorDrawScene::TextAlign alignment) {
    fbb_.AddElement<uint8_t>(TextNode::VT_ALIGNMENT, static_cast<uint8_t>(alignment), 0);
  }
  void add_maxWidth(float maxWidth) {
    fbb_.AddElement<float>(TextNode::VT_MAXWIDTH, maxWidth, 0.0f);
  }
  void add_lineScale(float lineScale) {
    fbb_.AddElement<float>(TextNode::VT_LINESCALE, lineScale, 0.0f);
  }
  void add_z(int32_t z) {
    fbb_.AddElement<int32_t>(TextNode::VT_Z, z, 0);
  }
  void add_firstChar(uint32_t firstChar) {
    fbb_.AddElement<uint32_t>(TextNode::VT_FIRSTCHAR, firstChar, 0);
  }
  void add_charCount(uint32_t charCount) {
    fbb_.AddElement<uint32_t>(TextNode::VT_CHARCOUNT, charCount, 0);
  }
  void add_shader(flatbuffers::Offset<flatbuffers::String> shader) {
    fbb_.AddOffset(TextNode::VT_SHADER, shader);
  }
  void add_material(flatbuffers::Offset<flatbuffers::String> material) {
    fbb_.AddOffset(TextNode::VT_MATERIAL, material);
  }
  void add_fontTexture(flatbuffers::Offset<flatbuffers::String> fontTexture) {
    fbb_.AddOffset(TextNode::VT_FONTTEXTURE, fontTexture);
  }
  void add_itemLists(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> itemLists) {
    fbb_.AddOffset(TextNode::VT_ITEMLISTS, itemLists);
  }
  explicit TextNodeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<TextNode> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<TextNode>(end);
    fbb_.Required(o, TextNode::VT_TEXT);
    fbb_.Required(o, TextNode::VT_SHADER);
    fbb_.Required(o, TextNode::VT_MATERIAL);
    fbb_.Required(o, TextNode::VT_FONTTEXTURE);
    return o;
  }
};

inline flatbuffers::Offset<TextNode> CreateTextNode(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> embeddedResources = 0,
    flatbuffers::Offset<flatbuffers::String> text = 0,
    DeepSeaVectorDrawScene::TextAlign alignment = DeepSeaVectorDrawScene::TextAlign::Start,
    float maxWidth = 0.0f,
    float lineScale = 0.0f,
    int32_t z = 0,
    uint32_t firstChar = 0,
    uint32_t charCount = 0,
    flatbuffers::Offset<flatbuffers::String> shader = 0,
    flatbuffers::Offset<flatbuffers::String> material = 0,
    flatbuffers::Offset<flatbuffers::String> fontTexture = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> itemLists = 0) {
  TextNodeBuilder builder_(_fbb);
  builder_.add_itemLists(itemLists);
  builder_.add_fontTexture(fontTexture);
  builder_.add_material(material);
  builder_.add_shader(shader);
  builder_.add_charCount(charCount);
  builder_.add_firstChar(firstChar);
  builder_.add_z(z);
  builder_.add_lineScale(lineScale);
  builder_.add_maxWidth(maxWidth);
  builder_.add_text(text);
  builder_.add_embeddedResources(embeddedResources);
  builder_.add_alignment(alignment);
  return builder_.Finish();
}

inline flatbuffers::Offset<TextNode> CreateTextNodeDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint8_t> *embeddedResources = nullptr,
    const char *text = nullptr,
    DeepSeaVectorDrawScene::TextAlign alignment = DeepSeaVectorDrawScene::TextAlign::Start,
    float maxWidth = 0.0f,
    float lineScale = 0.0f,
    int32_t z = 0,
    uint32_t firstChar = 0,
    uint32_t charCount = 0,
    const char *shader = nullptr,
    const char *material = nullptr,
    const char *fontTexture = nullptr,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *itemLists = nullptr) {
  auto embeddedResources__ = embeddedResources ? _fbb.CreateVector<uint8_t>(*embeddedResources) : 0;
  auto text__ = text ? _fbb.CreateString(text) : 0;
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  auto fontTexture__ = fontTexture ? _fbb.CreateString(fontTexture) : 0;
  auto itemLists__ = itemLists ? _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*itemLists) : 0;
  return DeepSeaVectorDrawScene::CreateTextNode(
      _fbb,
      embeddedResources__,
      text__,
      alignment,
      maxWidth,
      lineScale,
      z,
      firstChar,
      charCount,
      shader__,
      material__,
      fontTexture__,
      itemLists__);
}

inline const DeepSeaVectorDrawScene::TextNode *GetTextNode(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaVectorDrawScene::TextNode>(buf);
}

inline const DeepSeaVectorDrawScene::TextNode *GetSizePrefixedTextNode(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaVectorDrawScene::TextNode>(buf);
}

inline bool VerifyTextNodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaVectorDrawScene::TextNode>(nullptr);
}

inline bool VerifySizePrefixedTextNodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaVectorDrawScene::TextNode>(nullptr);
}

inline void FinishTextNodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaVectorDrawScene::TextNode> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedTextNodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaVectorDrawScene::TextNode> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaVectorDrawScene

#endif  // FLATBUFFERS_GENERATED_TEXTNODE_DEEPSEAVECTORDRAWSCENE_H_

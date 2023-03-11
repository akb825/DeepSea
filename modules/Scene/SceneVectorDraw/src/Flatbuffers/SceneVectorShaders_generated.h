// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENEVECTORSHADERS_DEEPSEASCENEVECTORDRAW_H_
#define FLATBUFFERS_GENERATED_SCENEVECTORSHADERS_DEEPSEASCENEVECTORDRAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 3,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaSceneVectorDraw {

struct MaterialElement;
struct MaterialElementBuilder;

struct VectorShaders;
struct VectorShadersBuilder;

struct MaterialElement FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef MaterialElementBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_TYPE = 6,
    VT_COUNT = 8,
    VT_BINDING = 10,
    VT_SHADERVARIABLEGROUPDESC = 12
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  DeepSeaScene::MaterialType type() const {
    return static_cast<DeepSeaScene::MaterialType>(GetField<uint8_t>(VT_TYPE, 0));
  }
  uint32_t count() const {
    return GetField<uint32_t>(VT_COUNT, 0);
  }
  DeepSeaScene::MaterialBinding binding() const {
    return static_cast<DeepSeaScene::MaterialBinding>(GetField<uint8_t>(VT_BINDING, 0));
  }
  const ::flatbuffers::String *shaderVariableGroupDesc() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SHADERVARIABLEGROUPDESC);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint8_t>(verifier, VT_TYPE, 1) &&
           VerifyField<uint32_t>(verifier, VT_COUNT, 4) &&
           VerifyField<uint8_t>(verifier, VT_BINDING, 1) &&
           VerifyOffset(verifier, VT_SHADERVARIABLEGROUPDESC) &&
           verifier.VerifyString(shaderVariableGroupDesc()) &&
           verifier.EndTable();
  }
};

struct MaterialElementBuilder {
  typedef MaterialElement Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(MaterialElement::VT_NAME, name);
  }
  void add_type(DeepSeaScene::MaterialType type) {
    fbb_.AddElement<uint8_t>(MaterialElement::VT_TYPE, static_cast<uint8_t>(type), 0);
  }
  void add_count(uint32_t count) {
    fbb_.AddElement<uint32_t>(MaterialElement::VT_COUNT, count, 0);
  }
  void add_binding(DeepSeaScene::MaterialBinding binding) {
    fbb_.AddElement<uint8_t>(MaterialElement::VT_BINDING, static_cast<uint8_t>(binding), 0);
  }
  void add_shaderVariableGroupDesc(::flatbuffers::Offset<::flatbuffers::String> shaderVariableGroupDesc) {
    fbb_.AddOffset(MaterialElement::VT_SHADERVARIABLEGROUPDESC, shaderVariableGroupDesc);
  }
  explicit MaterialElementBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<MaterialElement> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<MaterialElement>(end);
    fbb_.Required(o, MaterialElement::VT_NAME);
    return o;
  }
};

inline ::flatbuffers::Offset<MaterialElement> CreateMaterialElement(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    DeepSeaScene::MaterialType type = DeepSeaScene::MaterialType::Float,
    uint32_t count = 0,
    DeepSeaScene::MaterialBinding binding = DeepSeaScene::MaterialBinding::Material,
    ::flatbuffers::Offset<::flatbuffers::String> shaderVariableGroupDesc = 0) {
  MaterialElementBuilder builder_(_fbb);
  builder_.add_shaderVariableGroupDesc(shaderVariableGroupDesc);
  builder_.add_count(count);
  builder_.add_name(name);
  builder_.add_binding(binding);
  builder_.add_type(type);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<MaterialElement> CreateMaterialElementDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    DeepSeaScene::MaterialType type = DeepSeaScene::MaterialType::Float,
    uint32_t count = 0,
    DeepSeaScene::MaterialBinding binding = DeepSeaScene::MaterialBinding::Material,
    const char *shaderVariableGroupDesc = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto shaderVariableGroupDesc__ = shaderVariableGroupDesc ? _fbb.CreateString(shaderVariableGroupDesc) : 0;
  return DeepSeaSceneVectorDraw::CreateMaterialElement(
      _fbb,
      name__,
      type,
      count,
      binding,
      shaderVariableGroupDesc__);
}

struct VectorShaders FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef VectorShadersBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MODULES = 4,
    VT_EXTRAELEMENTS = 6,
    VT_MATERIALDESC = 8,
    VT_FILLCOLOR = 10,
    VT_FILLLINEARGRADIENT = 12,
    VT_FILLRADIALGRADIENT = 14,
    VT_LINE = 16,
    VT_IMAGE = 18,
    VT_TEXTCOLOR = 20,
    VT_TEXTCOLOROUTLINE = 22,
    VT_TEXTGRADIENT = 24,
    VT_TEXTGRADIENTOUTLINE = 26
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::VersionedShaderModule>> *modules() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::VersionedShaderModule>> *>(VT_MODULES);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::MaterialElement>> *extraElements() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::MaterialElement>> *>(VT_EXTRAELEMENTS);
  }
  const ::flatbuffers::String *materialDesc() const {
    return GetPointer<const ::flatbuffers::String *>(VT_MATERIALDESC);
  }
  const ::flatbuffers::String *fillColor() const {
    return GetPointer<const ::flatbuffers::String *>(VT_FILLCOLOR);
  }
  const ::flatbuffers::String *fillLinearGradient() const {
    return GetPointer<const ::flatbuffers::String *>(VT_FILLLINEARGRADIENT);
  }
  const ::flatbuffers::String *fillRadialGradient() const {
    return GetPointer<const ::flatbuffers::String *>(VT_FILLRADIALGRADIENT);
  }
  const ::flatbuffers::String *line() const {
    return GetPointer<const ::flatbuffers::String *>(VT_LINE);
  }
  const ::flatbuffers::String *image() const {
    return GetPointer<const ::flatbuffers::String *>(VT_IMAGE);
  }
  const ::flatbuffers::String *textColor() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TEXTCOLOR);
  }
  const ::flatbuffers::String *textColorOutline() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TEXTCOLOROUTLINE);
  }
  const ::flatbuffers::String *textGradient() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TEXTGRADIENT);
  }
  const ::flatbuffers::String *textGradientOutline() const {
    return GetPointer<const ::flatbuffers::String *>(VT_TEXTGRADIENTOUTLINE);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_MODULES) &&
           verifier.VerifyVector(modules()) &&
           verifier.VerifyVectorOfTables(modules()) &&
           VerifyOffsetRequired(verifier, VT_EXTRAELEMENTS) &&
           verifier.VerifyVector(extraElements()) &&
           verifier.VerifyVectorOfTables(extraElements()) &&
           VerifyOffsetRequired(verifier, VT_MATERIALDESC) &&
           verifier.VerifyString(materialDesc()) &&
           VerifyOffset(verifier, VT_FILLCOLOR) &&
           verifier.VerifyString(fillColor()) &&
           VerifyOffset(verifier, VT_FILLLINEARGRADIENT) &&
           verifier.VerifyString(fillLinearGradient()) &&
           VerifyOffset(verifier, VT_FILLRADIALGRADIENT) &&
           verifier.VerifyString(fillRadialGradient()) &&
           VerifyOffset(verifier, VT_LINE) &&
           verifier.VerifyString(line()) &&
           VerifyOffset(verifier, VT_IMAGE) &&
           verifier.VerifyString(image()) &&
           VerifyOffset(verifier, VT_TEXTCOLOR) &&
           verifier.VerifyString(textColor()) &&
           VerifyOffset(verifier, VT_TEXTCOLOROUTLINE) &&
           verifier.VerifyString(textColorOutline()) &&
           VerifyOffset(verifier, VT_TEXTGRADIENT) &&
           verifier.VerifyString(textGradient()) &&
           VerifyOffset(verifier, VT_TEXTGRADIENTOUTLINE) &&
           verifier.VerifyString(textGradientOutline()) &&
           verifier.EndTable();
  }
};

struct VectorShadersBuilder {
  typedef VectorShaders Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_modules(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::VersionedShaderModule>>> modules) {
    fbb_.AddOffset(VectorShaders::VT_MODULES, modules);
  }
  void add_extraElements(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::MaterialElement>>> extraElements) {
    fbb_.AddOffset(VectorShaders::VT_EXTRAELEMENTS, extraElements);
  }
  void add_materialDesc(::flatbuffers::Offset<::flatbuffers::String> materialDesc) {
    fbb_.AddOffset(VectorShaders::VT_MATERIALDESC, materialDesc);
  }
  void add_fillColor(::flatbuffers::Offset<::flatbuffers::String> fillColor) {
    fbb_.AddOffset(VectorShaders::VT_FILLCOLOR, fillColor);
  }
  void add_fillLinearGradient(::flatbuffers::Offset<::flatbuffers::String> fillLinearGradient) {
    fbb_.AddOffset(VectorShaders::VT_FILLLINEARGRADIENT, fillLinearGradient);
  }
  void add_fillRadialGradient(::flatbuffers::Offset<::flatbuffers::String> fillRadialGradient) {
    fbb_.AddOffset(VectorShaders::VT_FILLRADIALGRADIENT, fillRadialGradient);
  }
  void add_line(::flatbuffers::Offset<::flatbuffers::String> line) {
    fbb_.AddOffset(VectorShaders::VT_LINE, line);
  }
  void add_image(::flatbuffers::Offset<::flatbuffers::String> image) {
    fbb_.AddOffset(VectorShaders::VT_IMAGE, image);
  }
  void add_textColor(::flatbuffers::Offset<::flatbuffers::String> textColor) {
    fbb_.AddOffset(VectorShaders::VT_TEXTCOLOR, textColor);
  }
  void add_textColorOutline(::flatbuffers::Offset<::flatbuffers::String> textColorOutline) {
    fbb_.AddOffset(VectorShaders::VT_TEXTCOLOROUTLINE, textColorOutline);
  }
  void add_textGradient(::flatbuffers::Offset<::flatbuffers::String> textGradient) {
    fbb_.AddOffset(VectorShaders::VT_TEXTGRADIENT, textGradient);
  }
  void add_textGradientOutline(::flatbuffers::Offset<::flatbuffers::String> textGradientOutline) {
    fbb_.AddOffset(VectorShaders::VT_TEXTGRADIENTOUTLINE, textGradientOutline);
  }
  explicit VectorShadersBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<VectorShaders> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<VectorShaders>(end);
    fbb_.Required(o, VectorShaders::VT_MODULES);
    fbb_.Required(o, VectorShaders::VT_EXTRAELEMENTS);
    fbb_.Required(o, VectorShaders::VT_MATERIALDESC);
    return o;
  }
};

inline ::flatbuffers::Offset<VectorShaders> CreateVectorShaders(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaScene::VersionedShaderModule>>> modules = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::MaterialElement>>> extraElements = 0,
    ::flatbuffers::Offset<::flatbuffers::String> materialDesc = 0,
    ::flatbuffers::Offset<::flatbuffers::String> fillColor = 0,
    ::flatbuffers::Offset<::flatbuffers::String> fillLinearGradient = 0,
    ::flatbuffers::Offset<::flatbuffers::String> fillRadialGradient = 0,
    ::flatbuffers::Offset<::flatbuffers::String> line = 0,
    ::flatbuffers::Offset<::flatbuffers::String> image = 0,
    ::flatbuffers::Offset<::flatbuffers::String> textColor = 0,
    ::flatbuffers::Offset<::flatbuffers::String> textColorOutline = 0,
    ::flatbuffers::Offset<::flatbuffers::String> textGradient = 0,
    ::flatbuffers::Offset<::flatbuffers::String> textGradientOutline = 0) {
  VectorShadersBuilder builder_(_fbb);
  builder_.add_textGradientOutline(textGradientOutline);
  builder_.add_textGradient(textGradient);
  builder_.add_textColorOutline(textColorOutline);
  builder_.add_textColor(textColor);
  builder_.add_image(image);
  builder_.add_line(line);
  builder_.add_fillRadialGradient(fillRadialGradient);
  builder_.add_fillLinearGradient(fillLinearGradient);
  builder_.add_fillColor(fillColor);
  builder_.add_materialDesc(materialDesc);
  builder_.add_extraElements(extraElements);
  builder_.add_modules(modules);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<VectorShaders> CreateVectorShadersDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<DeepSeaScene::VersionedShaderModule>> *modules = nullptr,
    const std::vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::MaterialElement>> *extraElements = nullptr,
    const char *materialDesc = nullptr,
    const char *fillColor = nullptr,
    const char *fillLinearGradient = nullptr,
    const char *fillRadialGradient = nullptr,
    const char *line = nullptr,
    const char *image = nullptr,
    const char *textColor = nullptr,
    const char *textColorOutline = nullptr,
    const char *textGradient = nullptr,
    const char *textGradientOutline = nullptr) {
  auto modules__ = modules ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaScene::VersionedShaderModule>>(*modules) : 0;
  auto extraElements__ = extraElements ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::MaterialElement>>(*extraElements) : 0;
  auto materialDesc__ = materialDesc ? _fbb.CreateString(materialDesc) : 0;
  auto fillColor__ = fillColor ? _fbb.CreateString(fillColor) : 0;
  auto fillLinearGradient__ = fillLinearGradient ? _fbb.CreateString(fillLinearGradient) : 0;
  auto fillRadialGradient__ = fillRadialGradient ? _fbb.CreateString(fillRadialGradient) : 0;
  auto line__ = line ? _fbb.CreateString(line) : 0;
  auto image__ = image ? _fbb.CreateString(image) : 0;
  auto textColor__ = textColor ? _fbb.CreateString(textColor) : 0;
  auto textColorOutline__ = textColorOutline ? _fbb.CreateString(textColorOutline) : 0;
  auto textGradient__ = textGradient ? _fbb.CreateString(textGradient) : 0;
  auto textGradientOutline__ = textGradientOutline ? _fbb.CreateString(textGradientOutline) : 0;
  return DeepSeaSceneVectorDraw::CreateVectorShaders(
      _fbb,
      modules__,
      extraElements__,
      materialDesc__,
      fillColor__,
      fillLinearGradient__,
      fillRadialGradient__,
      line__,
      image__,
      textColor__,
      textColorOutline__,
      textGradient__,
      textGradientOutline__);
}

inline const DeepSeaSceneVectorDraw::VectorShaders *GetVectorShaders(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneVectorDraw::VectorShaders>(buf);
}

inline const DeepSeaSceneVectorDraw::VectorShaders *GetSizePrefixedVectorShaders(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneVectorDraw::VectorShaders>(buf);
}

inline bool VerifyVectorShadersBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneVectorDraw::VectorShaders>(nullptr);
}

inline bool VerifySizePrefixedVectorShadersBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneVectorDraw::VectorShaders>(nullptr);
}

inline void FinishVectorShadersBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorShaders> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedVectorShadersBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorShaders> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneVectorDraw

#endif  // FLATBUFFERS_GENERATED_SCENEVECTORSHADERS_DEEPSEASCENEVECTORDRAW_H_

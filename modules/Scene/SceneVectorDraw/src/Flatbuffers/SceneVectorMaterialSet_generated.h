// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENEVECTORMATERIALSET_DEEPSEASCENEVECTORDRAW_H_
#define FLATBUFFERS_GENERATED_SCENEVECTORMATERIALSET_DEEPSEASCENEVECTORDRAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"
#include "SceneVectorCommon_generated.h"

namespace DeepSeaSceneVectorDraw {

struct ColorTable;
struct ColorTableBuilder;

struct GradientStop;
struct GradientStopBuilder;

struct LinearGradient;
struct LinearGradientBuilder;

struct RadialGradient;
struct RadialGradientBuilder;

struct Material;
struct MaterialBuilder;

struct VectorMaterialSet;
struct VectorMaterialSetBuilder;

enum class GradientEdge : uint8_t {
  Clamp = 0,
  Repeat = 1,
  Mirror = 2,
  MIN = Clamp,
  MAX = Mirror
};

inline const GradientEdge (&EnumValuesGradientEdge())[3] {
  static const GradientEdge values[] = {
    GradientEdge::Clamp,
    GradientEdge::Repeat,
    GradientEdge::Mirror
  };
  return values;
}

inline const char * const *EnumNamesGradientEdge() {
  static const char * const names[4] = {
    "Clamp",
    "Repeat",
    "Mirror",
    nullptr
  };
  return names;
}

inline const char *EnumNameGradientEdge(GradientEdge e) {
  if (::flatbuffers::IsOutRange(e, GradientEdge::Clamp, GradientEdge::Mirror)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesGradientEdge()[index];
}

enum class MaterialSpace : uint8_t {
  Local = 0,
  Bounds = 1,
  MIN = Local,
  MAX = Bounds
};

inline const MaterialSpace (&EnumValuesMaterialSpace())[2] {
  static const MaterialSpace values[] = {
    MaterialSpace::Local,
    MaterialSpace::Bounds
  };
  return values;
}

inline const char * const *EnumNamesMaterialSpace() {
  static const char * const names[3] = {
    "Local",
    "Bounds",
    nullptr
  };
  return names;
}

inline const char *EnumNameMaterialSpace(MaterialSpace e) {
  if (::flatbuffers::IsOutRange(e, MaterialSpace::Local, MaterialSpace::Bounds)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMaterialSpace()[index];
}

enum class MaterialValue : uint8_t {
  NONE = 0,
  ColorTable = 1,
  LinearGradient = 2,
  RadialGradient = 3,
  MIN = NONE,
  MAX = RadialGradient
};

inline const MaterialValue (&EnumValuesMaterialValue())[4] {
  static const MaterialValue values[] = {
    MaterialValue::NONE,
    MaterialValue::ColorTable,
    MaterialValue::LinearGradient,
    MaterialValue::RadialGradient
  };
  return values;
}

inline const char * const *EnumNamesMaterialValue() {
  static const char * const names[5] = {
    "NONE",
    "ColorTable",
    "LinearGradient",
    "RadialGradient",
    nullptr
  };
  return names;
}

inline const char *EnumNameMaterialValue(MaterialValue e) {
  if (::flatbuffers::IsOutRange(e, MaterialValue::NONE, MaterialValue::RadialGradient)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMaterialValue()[index];
}

template<typename T> struct MaterialValueTraits {
  static const MaterialValue enum_value = MaterialValue::NONE;
};

template<> struct MaterialValueTraits<DeepSeaSceneVectorDraw::ColorTable> {
  static const MaterialValue enum_value = MaterialValue::ColorTable;
};

template<> struct MaterialValueTraits<DeepSeaSceneVectorDraw::LinearGradient> {
  static const MaterialValue enum_value = MaterialValue::LinearGradient;
};

template<> struct MaterialValueTraits<DeepSeaSceneVectorDraw::RadialGradient> {
  static const MaterialValue enum_value = MaterialValue::RadialGradient;
};

bool VerifyMaterialValue(::flatbuffers::Verifier &verifier, const void *obj, MaterialValue type);
bool VerifyMaterialValueVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<MaterialValue> *types);

struct ColorTable FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ColorTableBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_RED = 4,
    VT_GREEN = 6,
    VT_BLUE = 8,
    VT_ALPHA = 10
  };
  uint8_t red() const {
    return GetField<uint8_t>(VT_RED, 0);
  }
  uint8_t green() const {
    return GetField<uint8_t>(VT_GREEN, 0);
  }
  uint8_t blue() const {
    return GetField<uint8_t>(VT_BLUE, 0);
  }
  uint8_t alpha() const {
    return GetField<uint8_t>(VT_ALPHA, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_RED, 1) &&
           VerifyField<uint8_t>(verifier, VT_GREEN, 1) &&
           VerifyField<uint8_t>(verifier, VT_BLUE, 1) &&
           VerifyField<uint8_t>(verifier, VT_ALPHA, 1) &&
           verifier.EndTable();
  }
};

struct ColorTableBuilder {
  typedef ColorTable Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_red(uint8_t red) {
    fbb_.AddElement<uint8_t>(ColorTable::VT_RED, red, 0);
  }
  void add_green(uint8_t green) {
    fbb_.AddElement<uint8_t>(ColorTable::VT_GREEN, green, 0);
  }
  void add_blue(uint8_t blue) {
    fbb_.AddElement<uint8_t>(ColorTable::VT_BLUE, blue, 0);
  }
  void add_alpha(uint8_t alpha) {
    fbb_.AddElement<uint8_t>(ColorTable::VT_ALPHA, alpha, 0);
  }
  explicit ColorTableBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<ColorTable> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<ColorTable>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<ColorTable> CreateColorTable(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint8_t red = 0,
    uint8_t green = 0,
    uint8_t blue = 0,
    uint8_t alpha = 0) {
  ColorTableBuilder builder_(_fbb);
  builder_.add_alpha(alpha);
  builder_.add_blue(blue);
  builder_.add_green(green);
  builder_.add_red(red);
  return builder_.Finish();
}

struct GradientStop FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef GradientStopBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_POSITION = 4,
    VT_COLOR = 6
  };
  float position() const {
    return GetField<float>(VT_POSITION, 0.0f);
  }
  const DeepSeaSceneVectorDraw::Color *color() const {
    return GetStruct<const DeepSeaSceneVectorDraw::Color *>(VT_COLOR);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<float>(verifier, VT_POSITION, 4) &&
           VerifyFieldRequired<DeepSeaSceneVectorDraw::Color>(verifier, VT_COLOR, 1) &&
           verifier.EndTable();
  }
};

struct GradientStopBuilder {
  typedef GradientStop Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_position(float position) {
    fbb_.AddElement<float>(GradientStop::VT_POSITION, position, 0.0f);
  }
  void add_color(const DeepSeaSceneVectorDraw::Color *color) {
    fbb_.AddStruct(GradientStop::VT_COLOR, color);
  }
  explicit GradientStopBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<GradientStop> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<GradientStop>(end);
    fbb_.Required(o, GradientStop::VT_COLOR);
    return o;
  }
};

inline ::flatbuffers::Offset<GradientStop> CreateGradientStop(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    float position = 0.0f,
    const DeepSeaSceneVectorDraw::Color *color = nullptr) {
  GradientStopBuilder builder_(_fbb);
  builder_.add_color(color);
  builder_.add_position(position);
  return builder_.Finish();
}

struct LinearGradient FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef LinearGradientBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_START = 4,
    VT_END = 6,
    VT_EDGE = 8,
    VT_COORDINATESPACE = 10,
    VT_TRANSFORM = 12,
    VT_STOPS = 14
  };
  const DeepSeaScene::Vector2f *start() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_START);
  }
  const DeepSeaScene::Vector2f *end() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_END);
  }
  DeepSeaSceneVectorDraw::GradientEdge edge() const {
    return static_cast<DeepSeaSceneVectorDraw::GradientEdge>(GetField<uint8_t>(VT_EDGE, 0));
  }
  DeepSeaSceneVectorDraw::MaterialSpace coordinateSpace() const {
    return static_cast<DeepSeaSceneVectorDraw::MaterialSpace>(GetField<uint8_t>(VT_COORDINATESPACE, 0));
  }
  const DeepSeaScene::Matrix33f *transform() const {
    return GetStruct<const DeepSeaScene::Matrix33f *>(VT_TRANSFORM);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>> *stops() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>> *>(VT_STOPS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<DeepSeaScene::Vector2f>(verifier, VT_START, 4) &&
           VerifyField<DeepSeaScene::Vector2f>(verifier, VT_END, 4) &&
           VerifyField<uint8_t>(verifier, VT_EDGE, 1) &&
           VerifyField<uint8_t>(verifier, VT_COORDINATESPACE, 1) &&
           VerifyField<DeepSeaScene::Matrix33f>(verifier, VT_TRANSFORM, 4) &&
           VerifyOffsetRequired(verifier, VT_STOPS) &&
           verifier.VerifyVector(stops()) &&
           verifier.VerifyVectorOfTables(stops()) &&
           verifier.EndTable();
  }
};

struct LinearGradientBuilder {
  typedef LinearGradient Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_start(const DeepSeaScene::Vector2f *start) {
    fbb_.AddStruct(LinearGradient::VT_START, start);
  }
  void add_end(const DeepSeaScene::Vector2f *end) {
    fbb_.AddStruct(LinearGradient::VT_END, end);
  }
  void add_edge(DeepSeaSceneVectorDraw::GradientEdge edge) {
    fbb_.AddElement<uint8_t>(LinearGradient::VT_EDGE, static_cast<uint8_t>(edge), 0);
  }
  void add_coordinateSpace(DeepSeaSceneVectorDraw::MaterialSpace coordinateSpace) {
    fbb_.AddElement<uint8_t>(LinearGradient::VT_COORDINATESPACE, static_cast<uint8_t>(coordinateSpace), 0);
  }
  void add_transform(const DeepSeaScene::Matrix33f *transform) {
    fbb_.AddStruct(LinearGradient::VT_TRANSFORM, transform);
  }
  void add_stops(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>>> stops) {
    fbb_.AddOffset(LinearGradient::VT_STOPS, stops);
  }
  explicit LinearGradientBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<LinearGradient> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<LinearGradient>(end);
    fbb_.Required(o, LinearGradient::VT_STOPS);
    return o;
  }
};

inline ::flatbuffers::Offset<LinearGradient> CreateLinearGradient(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector2f *start = nullptr,
    const DeepSeaScene::Vector2f *end = nullptr,
    DeepSeaSceneVectorDraw::GradientEdge edge = DeepSeaSceneVectorDraw::GradientEdge::Clamp,
    DeepSeaSceneVectorDraw::MaterialSpace coordinateSpace = DeepSeaSceneVectorDraw::MaterialSpace::Local,
    const DeepSeaScene::Matrix33f *transform = nullptr,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>>> stops = 0) {
  LinearGradientBuilder builder_(_fbb);
  builder_.add_stops(stops);
  builder_.add_transform(transform);
  builder_.add_end(end);
  builder_.add_start(start);
  builder_.add_coordinateSpace(coordinateSpace);
  builder_.add_edge(edge);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<LinearGradient> CreateLinearGradientDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector2f *start = nullptr,
    const DeepSeaScene::Vector2f *end = nullptr,
    DeepSeaSceneVectorDraw::GradientEdge edge = DeepSeaSceneVectorDraw::GradientEdge::Clamp,
    DeepSeaSceneVectorDraw::MaterialSpace coordinateSpace = DeepSeaSceneVectorDraw::MaterialSpace::Local,
    const DeepSeaScene::Matrix33f *transform = nullptr,
    const std::vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>> *stops = nullptr) {
  auto stops__ = stops ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>>(*stops) : 0;
  return DeepSeaSceneVectorDraw::CreateLinearGradient(
      _fbb,
      start,
      end,
      edge,
      coordinateSpace,
      transform,
      stops__);
}

struct RadialGradient FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RadialGradientBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_CENTER = 4,
    VT_RADIUS = 6,
    VT_FOCUS = 8,
    VT_FOCUSRADIUS = 10,
    VT_EDGE = 12,
    VT_COORDINATESPACE = 14,
    VT_TRANSFORM = 16,
    VT_STOPS = 18
  };
  const DeepSeaScene::Vector2f *center() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_CENTER);
  }
  float radius() const {
    return GetField<float>(VT_RADIUS, 0.0f);
  }
  const DeepSeaScene::Vector2f *focus() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_FOCUS);
  }
  float focusRadius() const {
    return GetField<float>(VT_FOCUSRADIUS, 0.0f);
  }
  DeepSeaSceneVectorDraw::GradientEdge edge() const {
    return static_cast<DeepSeaSceneVectorDraw::GradientEdge>(GetField<uint8_t>(VT_EDGE, 0));
  }
  DeepSeaSceneVectorDraw::MaterialSpace coordinateSpace() const {
    return static_cast<DeepSeaSceneVectorDraw::MaterialSpace>(GetField<uint8_t>(VT_COORDINATESPACE, 0));
  }
  const DeepSeaScene::Matrix33f *transform() const {
    return GetStruct<const DeepSeaScene::Matrix33f *>(VT_TRANSFORM);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>> *stops() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>> *>(VT_STOPS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<DeepSeaScene::Vector2f>(verifier, VT_CENTER, 4) &&
           VerifyField<float>(verifier, VT_RADIUS, 4) &&
           VerifyField<DeepSeaScene::Vector2f>(verifier, VT_FOCUS, 4) &&
           VerifyField<float>(verifier, VT_FOCUSRADIUS, 4) &&
           VerifyField<uint8_t>(verifier, VT_EDGE, 1) &&
           VerifyField<uint8_t>(verifier, VT_COORDINATESPACE, 1) &&
           VerifyField<DeepSeaScene::Matrix33f>(verifier, VT_TRANSFORM, 4) &&
           VerifyOffsetRequired(verifier, VT_STOPS) &&
           verifier.VerifyVector(stops()) &&
           verifier.VerifyVectorOfTables(stops()) &&
           verifier.EndTable();
  }
};

struct RadialGradientBuilder {
  typedef RadialGradient Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_center(const DeepSeaScene::Vector2f *center) {
    fbb_.AddStruct(RadialGradient::VT_CENTER, center);
  }
  void add_radius(float radius) {
    fbb_.AddElement<float>(RadialGradient::VT_RADIUS, radius, 0.0f);
  }
  void add_focus(const DeepSeaScene::Vector2f *focus) {
    fbb_.AddStruct(RadialGradient::VT_FOCUS, focus);
  }
  void add_focusRadius(float focusRadius) {
    fbb_.AddElement<float>(RadialGradient::VT_FOCUSRADIUS, focusRadius, 0.0f);
  }
  void add_edge(DeepSeaSceneVectorDraw::GradientEdge edge) {
    fbb_.AddElement<uint8_t>(RadialGradient::VT_EDGE, static_cast<uint8_t>(edge), 0);
  }
  void add_coordinateSpace(DeepSeaSceneVectorDraw::MaterialSpace coordinateSpace) {
    fbb_.AddElement<uint8_t>(RadialGradient::VT_COORDINATESPACE, static_cast<uint8_t>(coordinateSpace), 0);
  }
  void add_transform(const DeepSeaScene::Matrix33f *transform) {
    fbb_.AddStruct(RadialGradient::VT_TRANSFORM, transform);
  }
  void add_stops(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>>> stops) {
    fbb_.AddOffset(RadialGradient::VT_STOPS, stops);
  }
  explicit RadialGradientBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<RadialGradient> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<RadialGradient>(end);
    fbb_.Required(o, RadialGradient::VT_STOPS);
    return o;
  }
};

inline ::flatbuffers::Offset<RadialGradient> CreateRadialGradient(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector2f *center = nullptr,
    float radius = 0.0f,
    const DeepSeaScene::Vector2f *focus = nullptr,
    float focusRadius = 0.0f,
    DeepSeaSceneVectorDraw::GradientEdge edge = DeepSeaSceneVectorDraw::GradientEdge::Clamp,
    DeepSeaSceneVectorDraw::MaterialSpace coordinateSpace = DeepSeaSceneVectorDraw::MaterialSpace::Local,
    const DeepSeaScene::Matrix33f *transform = nullptr,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>>> stops = 0) {
  RadialGradientBuilder builder_(_fbb);
  builder_.add_stops(stops);
  builder_.add_transform(transform);
  builder_.add_focusRadius(focusRadius);
  builder_.add_focus(focus);
  builder_.add_radius(radius);
  builder_.add_center(center);
  builder_.add_coordinateSpace(coordinateSpace);
  builder_.add_edge(edge);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<RadialGradient> CreateRadialGradientDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const DeepSeaScene::Vector2f *center = nullptr,
    float radius = 0.0f,
    const DeepSeaScene::Vector2f *focus = nullptr,
    float focusRadius = 0.0f,
    DeepSeaSceneVectorDraw::GradientEdge edge = DeepSeaSceneVectorDraw::GradientEdge::Clamp,
    DeepSeaSceneVectorDraw::MaterialSpace coordinateSpace = DeepSeaSceneVectorDraw::MaterialSpace::Local,
    const DeepSeaScene::Matrix33f *transform = nullptr,
    const std::vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>> *stops = nullptr) {
  auto stops__ = stops ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::GradientStop>>(*stops) : 0;
  return DeepSeaSceneVectorDraw::CreateRadialGradient(
      _fbb,
      center,
      radius,
      focus,
      focusRadius,
      edge,
      coordinateSpace,
      transform,
      stops__);
}

struct Material FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef MaterialBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_VALUE_TYPE = 6,
    VT_VALUE = 8
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  DeepSeaSceneVectorDraw::MaterialValue value_type() const {
    return static_cast<DeepSeaSceneVectorDraw::MaterialValue>(GetField<uint8_t>(VT_VALUE_TYPE, 0));
  }
  const void *value() const {
    return GetPointer<const void *>(VT_VALUE);
  }
  template<typename T> const T *value_as() const;
  const DeepSeaSceneVectorDraw::ColorTable *value_as_ColorTable() const {
    return value_type() == DeepSeaSceneVectorDraw::MaterialValue::ColorTable ? static_cast<const DeepSeaSceneVectorDraw::ColorTable *>(value()) : nullptr;
  }
  const DeepSeaSceneVectorDraw::LinearGradient *value_as_LinearGradient() const {
    return value_type() == DeepSeaSceneVectorDraw::MaterialValue::LinearGradient ? static_cast<const DeepSeaSceneVectorDraw::LinearGradient *>(value()) : nullptr;
  }
  const DeepSeaSceneVectorDraw::RadialGradient *value_as_RadialGradient() const {
    return value_type() == DeepSeaSceneVectorDraw::MaterialValue::RadialGradient ? static_cast<const DeepSeaSceneVectorDraw::RadialGradient *>(value()) : nullptr;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint8_t>(verifier, VT_VALUE_TYPE, 1) &&
           VerifyOffsetRequired(verifier, VT_VALUE) &&
           VerifyMaterialValue(verifier, value(), value_type()) &&
           verifier.EndTable();
  }
};

template<> inline const DeepSeaSceneVectorDraw::ColorTable *Material::value_as<DeepSeaSceneVectorDraw::ColorTable>() const {
  return value_as_ColorTable();
}

template<> inline const DeepSeaSceneVectorDraw::LinearGradient *Material::value_as<DeepSeaSceneVectorDraw::LinearGradient>() const {
  return value_as_LinearGradient();
}

template<> inline const DeepSeaSceneVectorDraw::RadialGradient *Material::value_as<DeepSeaSceneVectorDraw::RadialGradient>() const {
  return value_as_RadialGradient();
}

struct MaterialBuilder {
  typedef Material Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(Material::VT_NAME, name);
  }
  void add_value_type(DeepSeaSceneVectorDraw::MaterialValue value_type) {
    fbb_.AddElement<uint8_t>(Material::VT_VALUE_TYPE, static_cast<uint8_t>(value_type), 0);
  }
  void add_value(::flatbuffers::Offset<void> value) {
    fbb_.AddOffset(Material::VT_VALUE, value);
  }
  explicit MaterialBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Material> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Material>(end);
    fbb_.Required(o, Material::VT_NAME);
    fbb_.Required(o, Material::VT_VALUE);
    return o;
  }
};

inline ::flatbuffers::Offset<Material> CreateMaterial(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    DeepSeaSceneVectorDraw::MaterialValue value_type = DeepSeaSceneVectorDraw::MaterialValue::NONE,
    ::flatbuffers::Offset<void> value = 0) {
  MaterialBuilder builder_(_fbb);
  builder_.add_value(value);
  builder_.add_name(name);
  builder_.add_value_type(value_type);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Material> CreateMaterialDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    DeepSeaSceneVectorDraw::MaterialValue value_type = DeepSeaSceneVectorDraw::MaterialValue::NONE,
    ::flatbuffers::Offset<void> value = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaSceneVectorDraw::CreateMaterial(
      _fbb,
      name__,
      value_type,
      value);
}

struct VectorMaterialSet FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef VectorMaterialSetBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MATERIALS = 4,
    VT_SRGB = 6
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::Material>> *materials() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::Material>> *>(VT_MATERIALS);
  }
  bool srgb() const {
    return GetField<uint8_t>(VT_SRGB, 0) != 0;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_MATERIALS) &&
           verifier.VerifyVector(materials()) &&
           verifier.VerifyVectorOfTables(materials()) &&
           VerifyField<uint8_t>(verifier, VT_SRGB, 1) &&
           verifier.EndTable();
  }
};

struct VectorMaterialSetBuilder {
  typedef VectorMaterialSet Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_materials(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::Material>>> materials) {
    fbb_.AddOffset(VectorMaterialSet::VT_MATERIALS, materials);
  }
  void add_srgb(bool srgb) {
    fbb_.AddElement<uint8_t>(VectorMaterialSet::VT_SRGB, static_cast<uint8_t>(srgb), 0);
  }
  explicit VectorMaterialSetBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<VectorMaterialSet> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<VectorMaterialSet>(end);
    fbb_.Required(o, VectorMaterialSet::VT_MATERIALS);
    return o;
  }
};

inline ::flatbuffers::Offset<VectorMaterialSet> CreateVectorMaterialSet(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::Material>>> materials = 0,
    bool srgb = false) {
  VectorMaterialSetBuilder builder_(_fbb);
  builder_.add_materials(materials);
  builder_.add_srgb(srgb);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<VectorMaterialSet> CreateVectorMaterialSetDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::Material>> *materials = nullptr,
    bool srgb = false) {
  auto materials__ = materials ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaSceneVectorDraw::Material>>(*materials) : 0;
  return DeepSeaSceneVectorDraw::CreateVectorMaterialSet(
      _fbb,
      materials__,
      srgb);
}

inline bool VerifyMaterialValue(::flatbuffers::Verifier &verifier, const void *obj, MaterialValue type) {
  switch (type) {
    case MaterialValue::NONE: {
      return true;
    }
    case MaterialValue::ColorTable: {
      auto ptr = reinterpret_cast<const DeepSeaSceneVectorDraw::ColorTable *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case MaterialValue::LinearGradient: {
      auto ptr = reinterpret_cast<const DeepSeaSceneVectorDraw::LinearGradient *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case MaterialValue::RadialGradient: {
      auto ptr = reinterpret_cast<const DeepSeaSceneVectorDraw::RadialGradient *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyMaterialValueVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<MaterialValue> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (::flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyMaterialValue(
        verifier,  values->Get(i), types->GetEnum<MaterialValue>(i))) {
      return false;
    }
  }
  return true;
}

inline const DeepSeaSceneVectorDraw::VectorMaterialSet *GetVectorMaterialSet(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaSceneVectorDraw::VectorMaterialSet>(buf);
}

inline const DeepSeaSceneVectorDraw::VectorMaterialSet *GetSizePrefixedVectorMaterialSet(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaSceneVectorDraw::VectorMaterialSet>(buf);
}

inline bool VerifyVectorMaterialSetBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaSceneVectorDraw::VectorMaterialSet>(nullptr);
}

inline bool VerifySizePrefixedVectorMaterialSetBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaSceneVectorDraw::VectorMaterialSet>(nullptr);
}

inline void FinishVectorMaterialSetBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorMaterialSet> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedVectorMaterialSetBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaSceneVectorDraw::VectorMaterialSet> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaSceneVectorDraw

#endif  // FLATBUFFERS_GENERATED_SCENEVECTORMATERIALSET_DEEPSEASCENEVECTORDRAW_H_

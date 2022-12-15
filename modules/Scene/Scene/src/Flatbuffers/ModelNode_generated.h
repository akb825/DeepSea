// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_MODELNODE_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_MODELNODE_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 22 &&
              FLATBUFFERS_VERSION_MINOR == 12 &&
              FLATBUFFERS_VERSION_REVISION == 6,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct DrawRange;
struct DrawRangeBuilder;

struct DrawIndexedRange;
struct DrawIndexedRangeBuilder;

struct ModelDrawRange;
struct ModelDrawRangeBuilder;

struct ModelInfo;
struct ModelInfoBuilder;

struct ModelNode;
struct ModelNodeBuilder;

enum class PrimitiveType : uint8_t {
  PointList = 0,
  LineList = 1,
  LineStrip = 2,
  TriangleList = 3,
  TriangleStrip = 4,
  TriangleFan = 5,
  LineListAdjacency = 6,
  TriangleListAdjacency = 7,
  TriangleStripAdjacency = 8,
  PatchList = 9,
  MIN = PointList,
  MAX = PatchList
};

inline const PrimitiveType (&EnumValuesPrimitiveType())[10] {
  static const PrimitiveType values[] = {
    PrimitiveType::PointList,
    PrimitiveType::LineList,
    PrimitiveType::LineStrip,
    PrimitiveType::TriangleList,
    PrimitiveType::TriangleStrip,
    PrimitiveType::TriangleFan,
    PrimitiveType::LineListAdjacency,
    PrimitiveType::TriangleListAdjacency,
    PrimitiveType::TriangleStripAdjacency,
    PrimitiveType::PatchList
  };
  return values;
}

inline const char * const *EnumNamesPrimitiveType() {
  static const char * const names[11] = {
    "PointList",
    "LineList",
    "LineStrip",
    "TriangleList",
    "TriangleStrip",
    "TriangleFan",
    "LineListAdjacency",
    "TriangleListAdjacency",
    "TriangleStripAdjacency",
    "PatchList",
    nullptr
  };
  return names;
}

inline const char *EnumNamePrimitiveType(PrimitiveType e) {
  if (flatbuffers::IsOutRange(e, PrimitiveType::PointList, PrimitiveType::PatchList)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesPrimitiveType()[index];
}

enum class ModelDrawRangeUnion : uint8_t {
  NONE = 0,
  DrawRange = 1,
  DrawIndexedRange = 2,
  MIN = NONE,
  MAX = DrawIndexedRange
};

inline const ModelDrawRangeUnion (&EnumValuesModelDrawRangeUnion())[3] {
  static const ModelDrawRangeUnion values[] = {
    ModelDrawRangeUnion::NONE,
    ModelDrawRangeUnion::DrawRange,
    ModelDrawRangeUnion::DrawIndexedRange
  };
  return values;
}

inline const char * const *EnumNamesModelDrawRangeUnion() {
  static const char * const names[4] = {
    "NONE",
    "DrawRange",
    "DrawIndexedRange",
    nullptr
  };
  return names;
}

inline const char *EnumNameModelDrawRangeUnion(ModelDrawRangeUnion e) {
  if (flatbuffers::IsOutRange(e, ModelDrawRangeUnion::NONE, ModelDrawRangeUnion::DrawIndexedRange)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesModelDrawRangeUnion()[index];
}

template<typename T> struct ModelDrawRangeUnionTraits {
  static const ModelDrawRangeUnion enum_value = ModelDrawRangeUnion::NONE;
};

template<> struct ModelDrawRangeUnionTraits<DeepSeaScene::DrawRange> {
  static const ModelDrawRangeUnion enum_value = ModelDrawRangeUnion::DrawRange;
};

template<> struct ModelDrawRangeUnionTraits<DeepSeaScene::DrawIndexedRange> {
  static const ModelDrawRangeUnion enum_value = ModelDrawRangeUnion::DrawIndexedRange;
};

bool VerifyModelDrawRangeUnion(flatbuffers::Verifier &verifier, const void *obj, ModelDrawRangeUnion type);
bool VerifyModelDrawRangeUnionVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<ModelDrawRangeUnion> *types);

struct DrawRange FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef DrawRangeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VERTEXCOUNT = 4,
    VT_INSTANCECOUNT = 6,
    VT_FIRSTVERTEX = 8,
    VT_FIRSTINSTANCE = 10
  };
  uint32_t vertexCount() const {
    return GetField<uint32_t>(VT_VERTEXCOUNT, 0);
  }
  uint32_t instanceCount() const {
    return GetField<uint32_t>(VT_INSTANCECOUNT, 0);
  }
  uint32_t firstVertex() const {
    return GetField<uint32_t>(VT_FIRSTVERTEX, 0);
  }
  uint32_t firstInstance() const {
    return GetField<uint32_t>(VT_FIRSTINSTANCE, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_VERTEXCOUNT, 4) &&
           VerifyField<uint32_t>(verifier, VT_INSTANCECOUNT, 4) &&
           VerifyField<uint32_t>(verifier, VT_FIRSTVERTEX, 4) &&
           VerifyField<uint32_t>(verifier, VT_FIRSTINSTANCE, 4) &&
           verifier.EndTable();
  }
};

struct DrawRangeBuilder {
  typedef DrawRange Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_vertexCount(uint32_t vertexCount) {
    fbb_.AddElement<uint32_t>(DrawRange::VT_VERTEXCOUNT, vertexCount, 0);
  }
  void add_instanceCount(uint32_t instanceCount) {
    fbb_.AddElement<uint32_t>(DrawRange::VT_INSTANCECOUNT, instanceCount, 0);
  }
  void add_firstVertex(uint32_t firstVertex) {
    fbb_.AddElement<uint32_t>(DrawRange::VT_FIRSTVERTEX, firstVertex, 0);
  }
  void add_firstInstance(uint32_t firstInstance) {
    fbb_.AddElement<uint32_t>(DrawRange::VT_FIRSTINSTANCE, firstInstance, 0);
  }
  explicit DrawRangeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<DrawRange> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<DrawRange>(end);
    return o;
  }
};

inline flatbuffers::Offset<DrawRange> CreateDrawRange(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t vertexCount = 0,
    uint32_t instanceCount = 0,
    uint32_t firstVertex = 0,
    uint32_t firstInstance = 0) {
  DrawRangeBuilder builder_(_fbb);
  builder_.add_firstInstance(firstInstance);
  builder_.add_firstVertex(firstVertex);
  builder_.add_instanceCount(instanceCount);
  builder_.add_vertexCount(vertexCount);
  return builder_.Finish();
}

struct DrawIndexedRange FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef DrawIndexedRangeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_INDEXCOUNT = 4,
    VT_INSTANCECOUNT = 6,
    VT_FIRSTINDEX = 8,
    VT_VERTEXOFFSET = 10,
    VT_FIRSTINSTANCE = 12
  };
  uint32_t indexCount() const {
    return GetField<uint32_t>(VT_INDEXCOUNT, 0);
  }
  uint32_t instanceCount() const {
    return GetField<uint32_t>(VT_INSTANCECOUNT, 0);
  }
  uint32_t firstIndex() const {
    return GetField<uint32_t>(VT_FIRSTINDEX, 0);
  }
  uint32_t vertexOffset() const {
    return GetField<uint32_t>(VT_VERTEXOFFSET, 0);
  }
  uint32_t firstInstance() const {
    return GetField<uint32_t>(VT_FIRSTINSTANCE, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_INDEXCOUNT, 4) &&
           VerifyField<uint32_t>(verifier, VT_INSTANCECOUNT, 4) &&
           VerifyField<uint32_t>(verifier, VT_FIRSTINDEX, 4) &&
           VerifyField<uint32_t>(verifier, VT_VERTEXOFFSET, 4) &&
           VerifyField<uint32_t>(verifier, VT_FIRSTINSTANCE, 4) &&
           verifier.EndTable();
  }
};

struct DrawIndexedRangeBuilder {
  typedef DrawIndexedRange Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_indexCount(uint32_t indexCount) {
    fbb_.AddElement<uint32_t>(DrawIndexedRange::VT_INDEXCOUNT, indexCount, 0);
  }
  void add_instanceCount(uint32_t instanceCount) {
    fbb_.AddElement<uint32_t>(DrawIndexedRange::VT_INSTANCECOUNT, instanceCount, 0);
  }
  void add_firstIndex(uint32_t firstIndex) {
    fbb_.AddElement<uint32_t>(DrawIndexedRange::VT_FIRSTINDEX, firstIndex, 0);
  }
  void add_vertexOffset(uint32_t vertexOffset) {
    fbb_.AddElement<uint32_t>(DrawIndexedRange::VT_VERTEXOFFSET, vertexOffset, 0);
  }
  void add_firstInstance(uint32_t firstInstance) {
    fbb_.AddElement<uint32_t>(DrawIndexedRange::VT_FIRSTINSTANCE, firstInstance, 0);
  }
  explicit DrawIndexedRangeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<DrawIndexedRange> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<DrawIndexedRange>(end);
    return o;
  }
};

inline flatbuffers::Offset<DrawIndexedRange> CreateDrawIndexedRange(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t indexCount = 0,
    uint32_t instanceCount = 0,
    uint32_t firstIndex = 0,
    uint32_t vertexOffset = 0,
    uint32_t firstInstance = 0) {
  DrawIndexedRangeBuilder builder_(_fbb);
  builder_.add_firstInstance(firstInstance);
  builder_.add_vertexOffset(vertexOffset);
  builder_.add_firstIndex(firstIndex);
  builder_.add_instanceCount(instanceCount);
  builder_.add_indexCount(indexCount);
  return builder_.Finish();
}

struct ModelDrawRange FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ModelDrawRangeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_DRAWRANGE_TYPE = 4,
    VT_DRAWRANGE = 6
  };
  DeepSeaScene::ModelDrawRangeUnion drawRange_type() const {
    return static_cast<DeepSeaScene::ModelDrawRangeUnion>(GetField<uint8_t>(VT_DRAWRANGE_TYPE, 0));
  }
  const void *drawRange() const {
    return GetPointer<const void *>(VT_DRAWRANGE);
  }
  template<typename T> const T *drawRange_as() const;
  const DeepSeaScene::DrawRange *drawRange_as_DrawRange() const {
    return drawRange_type() == DeepSeaScene::ModelDrawRangeUnion::DrawRange ? static_cast<const DeepSeaScene::DrawRange *>(drawRange()) : nullptr;
  }
  const DeepSeaScene::DrawIndexedRange *drawRange_as_DrawIndexedRange() const {
    return drawRange_type() == DeepSeaScene::ModelDrawRangeUnion::DrawIndexedRange ? static_cast<const DeepSeaScene::DrawIndexedRange *>(drawRange()) : nullptr;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_DRAWRANGE_TYPE, 1) &&
           VerifyOffsetRequired(verifier, VT_DRAWRANGE) &&
           VerifyModelDrawRangeUnion(verifier, drawRange(), drawRange_type()) &&
           verifier.EndTable();
  }
};

template<> inline const DeepSeaScene::DrawRange *ModelDrawRange::drawRange_as<DeepSeaScene::DrawRange>() const {
  return drawRange_as_DrawRange();
}

template<> inline const DeepSeaScene::DrawIndexedRange *ModelDrawRange::drawRange_as<DeepSeaScene::DrawIndexedRange>() const {
  return drawRange_as_DrawIndexedRange();
}

struct ModelDrawRangeBuilder {
  typedef ModelDrawRange Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_drawRange_type(DeepSeaScene::ModelDrawRangeUnion drawRange_type) {
    fbb_.AddElement<uint8_t>(ModelDrawRange::VT_DRAWRANGE_TYPE, static_cast<uint8_t>(drawRange_type), 0);
  }
  void add_drawRange(flatbuffers::Offset<void> drawRange) {
    fbb_.AddOffset(ModelDrawRange::VT_DRAWRANGE, drawRange);
  }
  explicit ModelDrawRangeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ModelDrawRange> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ModelDrawRange>(end);
    fbb_.Required(o, ModelDrawRange::VT_DRAWRANGE);
    return o;
  }
};

inline flatbuffers::Offset<ModelDrawRange> CreateModelDrawRange(
    flatbuffers::FlatBufferBuilder &_fbb,
    DeepSeaScene::ModelDrawRangeUnion drawRange_type = DeepSeaScene::ModelDrawRangeUnion::NONE,
    flatbuffers::Offset<void> drawRange = 0) {
  ModelDrawRangeBuilder builder_(_fbb);
  builder_.add_drawRange(drawRange);
  builder_.add_drawRange_type(drawRange_type);
  return builder_.Finish();
}

struct ModelInfo FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ModelInfoBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_SHADER = 6,
    VT_MATERIAL = 8,
    VT_GEOMETRY = 10,
    VT_DISTANCERANGE = 12,
    VT_DRAWRANGES = 14,
    VT_PRIMITIVETYPE = 16,
    VT_MODELLIST = 18
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  const flatbuffers::String *shader() const {
    return GetPointer<const flatbuffers::String *>(VT_SHADER);
  }
  const flatbuffers::String *material() const {
    return GetPointer<const flatbuffers::String *>(VT_MATERIAL);
  }
  const flatbuffers::String *geometry() const {
    return GetPointer<const flatbuffers::String *>(VT_GEOMETRY);
  }
  const DeepSeaScene::Vector2f *distanceRange() const {
    return GetStruct<const DeepSeaScene::Vector2f *>(VT_DISTANCERANGE);
  }
  const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::ModelDrawRange>> *drawRanges() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::ModelDrawRange>> *>(VT_DRAWRANGES);
  }
  DeepSeaScene::PrimitiveType primitiveType() const {
    return static_cast<DeepSeaScene::PrimitiveType>(GetField<uint8_t>(VT_PRIMITIVETYPE, 0));
  }
  const flatbuffers::String *modelList() const {
    return GetPointer<const flatbuffers::String *>(VT_MODELLIST);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffset(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           VerifyOffsetRequired(verifier, VT_GEOMETRY) &&
           verifier.VerifyString(geometry()) &&
           VerifyFieldRequired<DeepSeaScene::Vector2f>(verifier, VT_DISTANCERANGE, 4) &&
           VerifyOffsetRequired(verifier, VT_DRAWRANGES) &&
           verifier.VerifyVector(drawRanges()) &&
           verifier.VerifyVectorOfTables(drawRanges()) &&
           VerifyField<uint8_t>(verifier, VT_PRIMITIVETYPE, 1) &&
           VerifyOffset(verifier, VT_MODELLIST) &&
           verifier.VerifyString(modelList()) &&
           verifier.EndTable();
  }
};

struct ModelInfoBuilder {
  typedef ModelInfo Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(ModelInfo::VT_NAME, name);
  }
  void add_shader(flatbuffers::Offset<flatbuffers::String> shader) {
    fbb_.AddOffset(ModelInfo::VT_SHADER, shader);
  }
  void add_material(flatbuffers::Offset<flatbuffers::String> material) {
    fbb_.AddOffset(ModelInfo::VT_MATERIAL, material);
  }
  void add_geometry(flatbuffers::Offset<flatbuffers::String> geometry) {
    fbb_.AddOffset(ModelInfo::VT_GEOMETRY, geometry);
  }
  void add_distanceRange(const DeepSeaScene::Vector2f *distanceRange) {
    fbb_.AddStruct(ModelInfo::VT_DISTANCERANGE, distanceRange);
  }
  void add_drawRanges(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::ModelDrawRange>>> drawRanges) {
    fbb_.AddOffset(ModelInfo::VT_DRAWRANGES, drawRanges);
  }
  void add_primitiveType(DeepSeaScene::PrimitiveType primitiveType) {
    fbb_.AddElement<uint8_t>(ModelInfo::VT_PRIMITIVETYPE, static_cast<uint8_t>(primitiveType), 0);
  }
  void add_modelList(flatbuffers::Offset<flatbuffers::String> modelList) {
    fbb_.AddOffset(ModelInfo::VT_MODELLIST, modelList);
  }
  explicit ModelInfoBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ModelInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ModelInfo>(end);
    fbb_.Required(o, ModelInfo::VT_GEOMETRY);
    fbb_.Required(o, ModelInfo::VT_DISTANCERANGE);
    fbb_.Required(o, ModelInfo::VT_DRAWRANGES);
    return o;
  }
};

inline flatbuffers::Offset<ModelInfo> CreateModelInfo(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::String> shader = 0,
    flatbuffers::Offset<flatbuffers::String> material = 0,
    flatbuffers::Offset<flatbuffers::String> geometry = 0,
    const DeepSeaScene::Vector2f *distanceRange = nullptr,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::ModelDrawRange>>> drawRanges = 0,
    DeepSeaScene::PrimitiveType primitiveType = DeepSeaScene::PrimitiveType::PointList,
    flatbuffers::Offset<flatbuffers::String> modelList = 0) {
  ModelInfoBuilder builder_(_fbb);
  builder_.add_modelList(modelList);
  builder_.add_drawRanges(drawRanges);
  builder_.add_distanceRange(distanceRange);
  builder_.add_geometry(geometry);
  builder_.add_material(material);
  builder_.add_shader(shader);
  builder_.add_name(name);
  builder_.add_primitiveType(primitiveType);
  return builder_.Finish();
}

inline flatbuffers::Offset<ModelInfo> CreateModelInfoDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const char *shader = nullptr,
    const char *material = nullptr,
    const char *geometry = nullptr,
    const DeepSeaScene::Vector2f *distanceRange = nullptr,
    const std::vector<flatbuffers::Offset<DeepSeaScene::ModelDrawRange>> *drawRanges = nullptr,
    DeepSeaScene::PrimitiveType primitiveType = DeepSeaScene::PrimitiveType::PointList,
    const char *modelList = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  auto geometry__ = geometry ? _fbb.CreateString(geometry) : 0;
  auto drawRanges__ = drawRanges ? _fbb.CreateVector<flatbuffers::Offset<DeepSeaScene::ModelDrawRange>>(*drawRanges) : 0;
  auto modelList__ = modelList ? _fbb.CreateString(modelList) : 0;
  return DeepSeaScene::CreateModelInfo(
      _fbb,
      name__,
      shader__,
      material__,
      geometry__,
      distanceRange,
      drawRanges__,
      primitiveType,
      modelList__);
}

struct ModelNode FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ModelNodeBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_EMBEDDEDRESOURCES = 4,
    VT_MODELS = 6,
    VT_EXTRAITEMLISTS = 8,
    VT_BOUNDS = 10
  };
  const flatbuffers::Vector<uint8_t> *embeddedResources() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_EMBEDDEDRESOURCES);
  }
  const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::ModelInfo>> *models() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::ModelInfo>> *>(VT_MODELS);
  }
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *extraItemLists() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_EXTRAITEMLISTS);
  }
  const DeepSeaScene::OrientedBox3f *bounds() const {
    return GetStruct<const DeepSeaScene::OrientedBox3f *>(VT_BOUNDS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_EMBEDDEDRESOURCES) &&
           verifier.VerifyVector(embeddedResources()) &&
           VerifyOffsetRequired(verifier, VT_MODELS) &&
           verifier.VerifyVector(models()) &&
           verifier.VerifyVectorOfTables(models()) &&
           VerifyOffset(verifier, VT_EXTRAITEMLISTS) &&
           verifier.VerifyVector(extraItemLists()) &&
           verifier.VerifyVectorOfStrings(extraItemLists()) &&
           VerifyField<DeepSeaScene::OrientedBox3f>(verifier, VT_BOUNDS, 4) &&
           verifier.EndTable();
  }
};

struct ModelNodeBuilder {
  typedef ModelNode Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_embeddedResources(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> embeddedResources) {
    fbb_.AddOffset(ModelNode::VT_EMBEDDEDRESOURCES, embeddedResources);
  }
  void add_models(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::ModelInfo>>> models) {
    fbb_.AddOffset(ModelNode::VT_MODELS, models);
  }
  void add_extraItemLists(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> extraItemLists) {
    fbb_.AddOffset(ModelNode::VT_EXTRAITEMLISTS, extraItemLists);
  }
  void add_bounds(const DeepSeaScene::OrientedBox3f *bounds) {
    fbb_.AddStruct(ModelNode::VT_BOUNDS, bounds);
  }
  explicit ModelNodeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ModelNode> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ModelNode>(end);
    fbb_.Required(o, ModelNode::VT_MODELS);
    return o;
  }
};

inline flatbuffers::Offset<ModelNode> CreateModelNode(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> embeddedResources = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::ModelInfo>>> models = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> extraItemLists = 0,
    const DeepSeaScene::OrientedBox3f *bounds = nullptr) {
  ModelNodeBuilder builder_(_fbb);
  builder_.add_bounds(bounds);
  builder_.add_extraItemLists(extraItemLists);
  builder_.add_models(models);
  builder_.add_embeddedResources(embeddedResources);
  return builder_.Finish();
}

inline flatbuffers::Offset<ModelNode> CreateModelNodeDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint8_t> *embeddedResources = nullptr,
    const std::vector<flatbuffers::Offset<DeepSeaScene::ModelInfo>> *models = nullptr,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *extraItemLists = nullptr,
    const DeepSeaScene::OrientedBox3f *bounds = nullptr) {
  auto embeddedResources__ = embeddedResources ? _fbb.CreateVector<uint8_t>(*embeddedResources) : 0;
  auto models__ = models ? _fbb.CreateVector<flatbuffers::Offset<DeepSeaScene::ModelInfo>>(*models) : 0;
  auto extraItemLists__ = extraItemLists ? _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*extraItemLists) : 0;
  return DeepSeaScene::CreateModelNode(
      _fbb,
      embeddedResources__,
      models__,
      extraItemLists__,
      bounds);
}

inline bool VerifyModelDrawRangeUnion(flatbuffers::Verifier &verifier, const void *obj, ModelDrawRangeUnion type) {
  switch (type) {
    case ModelDrawRangeUnion::NONE: {
      return true;
    }
    case ModelDrawRangeUnion::DrawRange: {
      auto ptr = reinterpret_cast<const DeepSeaScene::DrawRange *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ModelDrawRangeUnion::DrawIndexedRange: {
      auto ptr = reinterpret_cast<const DeepSeaScene::DrawIndexedRange *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyModelDrawRangeUnionVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<ModelDrawRangeUnion> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyModelDrawRangeUnion(
        verifier,  values->Get(i), types->GetEnum<ModelDrawRangeUnion>(i))) {
      return false;
    }
  }
  return true;
}

inline const DeepSeaScene::ModelNode *GetModelNode(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::ModelNode>(buf);
}

inline const DeepSeaScene::ModelNode *GetSizePrefixedModelNode(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::ModelNode>(buf);
}

inline bool VerifyModelNodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::ModelNode>(nullptr);
}

inline bool VerifySizePrefixedModelNodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::ModelNode>(nullptr);
}

inline void FinishModelNodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ModelNode> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedModelNodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::ModelNode> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_MODELNODE_DEEPSEASCENE_H_

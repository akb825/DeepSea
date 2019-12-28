// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_MODELNODE_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_MODELNODE_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct DrawRange;

struct DrawIndexedRange;

struct ModelInfo;

struct ModelNode;

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
  static const char * const names[] = {
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
  if (e < PrimitiveType::PointList || e > PrimitiveType::PatchList) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesPrimitiveType()[index];
}

enum class ModelDrawRange : uint8_t {
  NONE = 0,
  DrawRange = 1,
  DrawIndexedRange = 2,
  MIN = NONE,
  MAX = DrawIndexedRange
};

inline const ModelDrawRange (&EnumValuesModelDrawRange())[3] {
  static const ModelDrawRange values[] = {
    ModelDrawRange::NONE,
    ModelDrawRange::DrawRange,
    ModelDrawRange::DrawIndexedRange
  };
  return values;
}

inline const char * const *EnumNamesModelDrawRange() {
  static const char * const names[] = {
    "NONE",
    "DrawRange",
    "DrawIndexedRange",
    nullptr
  };
  return names;
}

inline const char *EnumNameModelDrawRange(ModelDrawRange e) {
  if (e < ModelDrawRange::NONE || e > ModelDrawRange::DrawIndexedRange) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesModelDrawRange()[index];
}

template<typename T> struct ModelDrawRangeTraits {
  static const ModelDrawRange enum_value = ModelDrawRange::NONE;
};

template<> struct ModelDrawRangeTraits<DrawRange> {
  static const ModelDrawRange enum_value = ModelDrawRange::DrawRange;
};

template<> struct ModelDrawRangeTraits<DrawIndexedRange> {
  static const ModelDrawRange enum_value = ModelDrawRange::DrawIndexedRange;
};

bool VerifyModelDrawRange(flatbuffers::Verifier &verifier, const void *obj, ModelDrawRange type);
bool VerifyModelDrawRangeVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

struct DrawRange FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
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
           VerifyField<uint32_t>(verifier, VT_VERTEXCOUNT) &&
           VerifyField<uint32_t>(verifier, VT_INSTANCECOUNT) &&
           VerifyField<uint32_t>(verifier, VT_FIRSTVERTEX) &&
           VerifyField<uint32_t>(verifier, VT_FIRSTINSTANCE) &&
           verifier.EndTable();
  }
};

struct DrawRangeBuilder {
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
  DrawRangeBuilder &operator=(const DrawRangeBuilder &);
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
           VerifyField<uint32_t>(verifier, VT_INDEXCOUNT) &&
           VerifyField<uint32_t>(verifier, VT_INSTANCECOUNT) &&
           VerifyField<uint32_t>(verifier, VT_FIRSTINDEX) &&
           VerifyField<uint32_t>(verifier, VT_VERTEXOFFSET) &&
           VerifyField<uint32_t>(verifier, VT_FIRSTINSTANCE) &&
           verifier.EndTable();
  }
};

struct DrawIndexedRangeBuilder {
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
  DrawIndexedRangeBuilder &operator=(const DrawIndexedRangeBuilder &);
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

struct ModelInfo FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SHADER = 4,
    VT_MATERIAL = 6,
    VT_GEOMETRY = 8,
    VT_DISTANCERANGE = 10,
    VT_DRAWRANGE_TYPE = 12,
    VT_DRAWRANGE = 14,
    VT_PRIMITIVETYPE = 16,
    VT_LISTNAME = 18
  };
  const flatbuffers::String *shader() const {
    return GetPointer<const flatbuffers::String *>(VT_SHADER);
  }
  const flatbuffers::String *material() const {
    return GetPointer<const flatbuffers::String *>(VT_MATERIAL);
  }
  const flatbuffers::String *geometry() const {
    return GetPointer<const flatbuffers::String *>(VT_GEOMETRY);
  }
  const Vector2f *distanceRange() const {
    return GetStruct<const Vector2f *>(VT_DISTANCERANGE);
  }
  ModelDrawRange drawRange_type() const {
    return static_cast<ModelDrawRange>(GetField<uint8_t>(VT_DRAWRANGE_TYPE, 0));
  }
  const void *drawRange() const {
    return GetPointer<const void *>(VT_DRAWRANGE);
  }
  template<typename T> const T *drawRange_as() const;
  const DrawRange *drawRange_as_DrawRange() const {
    return drawRange_type() == ModelDrawRange::DrawRange ? static_cast<const DrawRange *>(drawRange()) : nullptr;
  }
  const DrawIndexedRange *drawRange_as_DrawIndexedRange() const {
    return drawRange_type() == ModelDrawRange::DrawIndexedRange ? static_cast<const DrawIndexedRange *>(drawRange()) : nullptr;
  }
  PrimitiveType primitiveType() const {
    return static_cast<PrimitiveType>(GetField<uint8_t>(VT_PRIMITIVETYPE, 0));
  }
  const flatbuffers::String *listName() const {
    return GetPointer<const flatbuffers::String *>(VT_LISTNAME);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_SHADER) &&
           verifier.VerifyString(shader()) &&
           VerifyOffsetRequired(verifier, VT_MATERIAL) &&
           verifier.VerifyString(material()) &&
           VerifyOffsetRequired(verifier, VT_GEOMETRY) &&
           verifier.VerifyString(geometry()) &&
           VerifyFieldRequired<Vector2f>(verifier, VT_DISTANCERANGE) &&
           VerifyField<uint8_t>(verifier, VT_DRAWRANGE_TYPE) &&
           VerifyOffsetRequired(verifier, VT_DRAWRANGE) &&
           VerifyModelDrawRange(verifier, drawRange(), drawRange_type()) &&
           VerifyField<uint8_t>(verifier, VT_PRIMITIVETYPE) &&
           VerifyOffsetRequired(verifier, VT_LISTNAME) &&
           verifier.VerifyString(listName()) &&
           verifier.EndTable();
  }
};

template<> inline const DrawRange *ModelInfo::drawRange_as<DrawRange>() const {
  return drawRange_as_DrawRange();
}

template<> inline const DrawIndexedRange *ModelInfo::drawRange_as<DrawIndexedRange>() const {
  return drawRange_as_DrawIndexedRange();
}

struct ModelInfoBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_shader(flatbuffers::Offset<flatbuffers::String> shader) {
    fbb_.AddOffset(ModelInfo::VT_SHADER, shader);
  }
  void add_material(flatbuffers::Offset<flatbuffers::String> material) {
    fbb_.AddOffset(ModelInfo::VT_MATERIAL, material);
  }
  void add_geometry(flatbuffers::Offset<flatbuffers::String> geometry) {
    fbb_.AddOffset(ModelInfo::VT_GEOMETRY, geometry);
  }
  void add_distanceRange(const Vector2f *distanceRange) {
    fbb_.AddStruct(ModelInfo::VT_DISTANCERANGE, distanceRange);
  }
  void add_drawRange_type(ModelDrawRange drawRange_type) {
    fbb_.AddElement<uint8_t>(ModelInfo::VT_DRAWRANGE_TYPE, static_cast<uint8_t>(drawRange_type), 0);
  }
  void add_drawRange(flatbuffers::Offset<void> drawRange) {
    fbb_.AddOffset(ModelInfo::VT_DRAWRANGE, drawRange);
  }
  void add_primitiveType(PrimitiveType primitiveType) {
    fbb_.AddElement<uint8_t>(ModelInfo::VT_PRIMITIVETYPE, static_cast<uint8_t>(primitiveType), 0);
  }
  void add_listName(flatbuffers::Offset<flatbuffers::String> listName) {
    fbb_.AddOffset(ModelInfo::VT_LISTNAME, listName);
  }
  explicit ModelInfoBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ModelInfoBuilder &operator=(const ModelInfoBuilder &);
  flatbuffers::Offset<ModelInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ModelInfo>(end);
    fbb_.Required(o, ModelInfo::VT_SHADER);
    fbb_.Required(o, ModelInfo::VT_MATERIAL);
    fbb_.Required(o, ModelInfo::VT_GEOMETRY);
    fbb_.Required(o, ModelInfo::VT_DISTANCERANGE);
    fbb_.Required(o, ModelInfo::VT_DRAWRANGE);
    fbb_.Required(o, ModelInfo::VT_LISTNAME);
    return o;
  }
};

inline flatbuffers::Offset<ModelInfo> CreateModelInfo(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> shader = 0,
    flatbuffers::Offset<flatbuffers::String> material = 0,
    flatbuffers::Offset<flatbuffers::String> geometry = 0,
    const Vector2f *distanceRange = 0,
    ModelDrawRange drawRange_type = ModelDrawRange::NONE,
    flatbuffers::Offset<void> drawRange = 0,
    PrimitiveType primitiveType = PrimitiveType::PointList,
    flatbuffers::Offset<flatbuffers::String> listName = 0) {
  ModelInfoBuilder builder_(_fbb);
  builder_.add_listName(listName);
  builder_.add_drawRange(drawRange);
  builder_.add_distanceRange(distanceRange);
  builder_.add_geometry(geometry);
  builder_.add_material(material);
  builder_.add_shader(shader);
  builder_.add_primitiveType(primitiveType);
  builder_.add_drawRange_type(drawRange_type);
  return builder_.Finish();
}

inline flatbuffers::Offset<ModelInfo> CreateModelInfoDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *shader = nullptr,
    const char *material = nullptr,
    const char *geometry = nullptr,
    const Vector2f *distanceRange = 0,
    ModelDrawRange drawRange_type = ModelDrawRange::NONE,
    flatbuffers::Offset<void> drawRange = 0,
    PrimitiveType primitiveType = PrimitiveType::PointList,
    const char *listName = nullptr) {
  auto shader__ = shader ? _fbb.CreateString(shader) : 0;
  auto material__ = material ? _fbb.CreateString(material) : 0;
  auto geometry__ = geometry ? _fbb.CreateString(geometry) : 0;
  auto listName__ = listName ? _fbb.CreateString(listName) : 0;
  return DeepSeaScene::CreateModelInfo(
      _fbb,
      shader__,
      material__,
      geometry__,
      distanceRange,
      drawRange_type,
      drawRange,
      primitiveType,
      listName__);
}

struct ModelNode FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_EMBEDDEDRESOURCES = 4,
    VT_EXTRAITEMLISTS = 6,
    VT_MODELS = 8,
    VT_BOUNDS = 10
  };
  const flatbuffers::Vector<uint8_t> *embeddedResources() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_EMBEDDEDRESOURCES);
  }
  const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *extraItemLists() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *>(VT_EXTRAITEMLISTS);
  }
  const flatbuffers::Vector<flatbuffers::Offset<ModelInfo>> *models() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<ModelInfo>> *>(VT_MODELS);
  }
  const OrientedBox3f *bounds() const {
    return GetStruct<const OrientedBox3f *>(VT_BOUNDS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_EMBEDDEDRESOURCES) &&
           verifier.VerifyVector(embeddedResources()) &&
           VerifyOffset(verifier, VT_EXTRAITEMLISTS) &&
           verifier.VerifyVector(extraItemLists()) &&
           verifier.VerifyVectorOfStrings(extraItemLists()) &&
           VerifyOffsetRequired(verifier, VT_MODELS) &&
           verifier.VerifyVector(models()) &&
           verifier.VerifyVectorOfTables(models()) &&
           VerifyField<OrientedBox3f>(verifier, VT_BOUNDS) &&
           verifier.EndTable();
  }
};

struct ModelNodeBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_embeddedResources(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> embeddedResources) {
    fbb_.AddOffset(ModelNode::VT_EMBEDDEDRESOURCES, embeddedResources);
  }
  void add_extraItemLists(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> extraItemLists) {
    fbb_.AddOffset(ModelNode::VT_EXTRAITEMLISTS, extraItemLists);
  }
  void add_models(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ModelInfo>>> models) {
    fbb_.AddOffset(ModelNode::VT_MODELS, models);
  }
  void add_bounds(const OrientedBox3f *bounds) {
    fbb_.AddStruct(ModelNode::VT_BOUNDS, bounds);
  }
  explicit ModelNodeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ModelNodeBuilder &operator=(const ModelNodeBuilder &);
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
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> extraItemLists = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ModelInfo>>> models = 0,
    const OrientedBox3f *bounds = 0) {
  ModelNodeBuilder builder_(_fbb);
  builder_.add_bounds(bounds);
  builder_.add_models(models);
  builder_.add_extraItemLists(extraItemLists);
  builder_.add_embeddedResources(embeddedResources);
  return builder_.Finish();
}

inline flatbuffers::Offset<ModelNode> CreateModelNodeDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint8_t> *embeddedResources = nullptr,
    const std::vector<flatbuffers::Offset<flatbuffers::String>> *extraItemLists = nullptr,
    const std::vector<flatbuffers::Offset<ModelInfo>> *models = nullptr,
    const OrientedBox3f *bounds = 0) {
  auto embeddedResources__ = embeddedResources ? _fbb.CreateVector<uint8_t>(*embeddedResources) : 0;
  auto extraItemLists__ = extraItemLists ? _fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(*extraItemLists) : 0;
  auto models__ = models ? _fbb.CreateVector<flatbuffers::Offset<ModelInfo>>(*models) : 0;
  return DeepSeaScene::CreateModelNode(
      _fbb,
      embeddedResources__,
      extraItemLists__,
      models__,
      bounds);
}

inline bool VerifyModelDrawRange(flatbuffers::Verifier &verifier, const void *obj, ModelDrawRange type) {
  switch (type) {
    case ModelDrawRange::NONE: {
      return true;
    }
    case ModelDrawRange::DrawRange: {
      auto ptr = reinterpret_cast<const DrawRange *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case ModelDrawRange::DrawIndexedRange: {
      auto ptr = reinterpret_cast<const DrawIndexedRange *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return false;
  }
}

inline bool VerifyModelDrawRangeVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyModelDrawRange(
        verifier,  values->Get(i), types->GetEnum<ModelDrawRange>(i))) {
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

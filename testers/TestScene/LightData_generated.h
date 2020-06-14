// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_LIGHTDATA_TESTSCENE_H_
#define FLATBUFFERS_GENERATED_LIGHTDATA_TESTSCENE_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace TestScene {

struct LightData;
struct LightDataBuilder;

struct LightData FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef LightDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VARIABLEGROUPDESCNAME = 4,
    VT_DIRECTION = 6,
    VT_COLOR = 8,
    VT_AMBIENT = 10
  };
  const flatbuffers::String *variableGroupDescName() const {
    return GetPointer<const flatbuffers::String *>(VT_VARIABLEGROUPDESCNAME);
  }
  const DeepSeaScene::Vector3f *direction() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_DIRECTION);
  }
  const DeepSeaScene::Vector3f *color() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_COLOR);
  }
  const DeepSeaScene::Vector3f *ambient() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_AMBIENT);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_VARIABLEGROUPDESCNAME) &&
           verifier.VerifyString(variableGroupDescName()) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_DIRECTION) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_COLOR) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_AMBIENT) &&
           verifier.EndTable();
  }
};

struct LightDataBuilder {
  typedef LightData Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_variableGroupDescName(flatbuffers::Offset<flatbuffers::String> variableGroupDescName) {
    fbb_.AddOffset(LightData::VT_VARIABLEGROUPDESCNAME, variableGroupDescName);
  }
  void add_direction(const DeepSeaScene::Vector3f *direction) {
    fbb_.AddStruct(LightData::VT_DIRECTION, direction);
  }
  void add_color(const DeepSeaScene::Vector3f *color) {
    fbb_.AddStruct(LightData::VT_COLOR, color);
  }
  void add_ambient(const DeepSeaScene::Vector3f *ambient) {
    fbb_.AddStruct(LightData::VT_AMBIENT, ambient);
  }
  explicit LightDataBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  LightDataBuilder &operator=(const LightDataBuilder &);
  flatbuffers::Offset<LightData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<LightData>(end);
    fbb_.Required(o, LightData::VT_VARIABLEGROUPDESCNAME);
    fbb_.Required(o, LightData::VT_DIRECTION);
    fbb_.Required(o, LightData::VT_COLOR);
    fbb_.Required(o, LightData::VT_AMBIENT);
    return o;
  }
};

inline flatbuffers::Offset<LightData> CreateLightData(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> variableGroupDescName = 0,
    const DeepSeaScene::Vector3f *direction = 0,
    const DeepSeaScene::Vector3f *color = 0,
    const DeepSeaScene::Vector3f *ambient = 0) {
  LightDataBuilder builder_(_fbb);
  builder_.add_ambient(ambient);
  builder_.add_color(color);
  builder_.add_direction(direction);
  builder_.add_variableGroupDescName(variableGroupDescName);
  return builder_.Finish();
}

inline flatbuffers::Offset<LightData> CreateLightDataDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *variableGroupDescName = nullptr,
    const DeepSeaScene::Vector3f *direction = 0,
    const DeepSeaScene::Vector3f *color = 0,
    const DeepSeaScene::Vector3f *ambient = 0) {
  auto variableGroupDescName__ = variableGroupDescName ? _fbb.CreateString(variableGroupDescName) : 0;
  return TestScene::CreateLightData(
      _fbb,
      variableGroupDescName__,
      direction,
      color,
      ambient);
}

inline const TestScene::LightData *GetLightData(const void *buf) {
  return flatbuffers::GetRoot<TestScene::LightData>(buf);
}

inline const TestScene::LightData *GetSizePrefixedLightData(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<TestScene::LightData>(buf);
}

inline bool VerifyLightDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<TestScene::LightData>(nullptr);
}

inline bool VerifySizePrefixedLightDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<TestScene::LightData>(nullptr);
}

inline void FinishLightDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<TestScene::LightData> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedLightDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<TestScene::LightData> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace TestScene

#endif  // FLATBUFFERS_GENERATED_LIGHTDATA_TESTSCENE_H_
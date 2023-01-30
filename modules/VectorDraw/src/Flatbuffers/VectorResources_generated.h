// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_VECTORRESOURCES_DEEPSEAVECTORDRAW_H_
#define FLATBUFFERS_GENERATED_VECTORRESOURCES_DEEPSEAVECTORDRAW_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 1 &&
              FLATBUFFERS_VERSION_REVISION == 21,
             "Non-compatible flatbuffers version included");

namespace DeepSeaVectorDraw {

struct FileReference;
struct FileReferenceBuilder;

struct RawData;
struct RawDataBuilder;

struct Resource;
struct ResourceBuilder;

struct FaceGroup;
struct FaceGroupBuilder;

struct Font;
struct FontBuilder;

struct VectorResources;
struct VectorResourcesBuilder;

enum class FileOrData : uint8_t {
  NONE = 0,
  FileReference = 1,
  RawData = 2,
  MIN = NONE,
  MAX = RawData
};

inline const FileOrData (&EnumValuesFileOrData())[3] {
  static const FileOrData values[] = {
    FileOrData::NONE,
    FileOrData::FileReference,
    FileOrData::RawData
  };
  return values;
}

inline const char * const *EnumNamesFileOrData() {
  static const char * const names[4] = {
    "NONE",
    "FileReference",
    "RawData",
    nullptr
  };
  return names;
}

inline const char *EnumNameFileOrData(FileOrData e) {
  if (::flatbuffers::IsOutRange(e, FileOrData::NONE, FileOrData::RawData)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesFileOrData()[index];
}

template<typename T> struct FileOrDataTraits {
  static const FileOrData enum_value = FileOrData::NONE;
};

template<> struct FileOrDataTraits<DeepSeaVectorDraw::FileReference> {
  static const FileOrData enum_value = FileOrData::FileReference;
};

template<> struct FileOrDataTraits<DeepSeaVectorDraw::RawData> {
  static const FileOrData enum_value = FileOrData::RawData;
};

bool VerifyFileOrData(::flatbuffers::Verifier &verifier, const void *obj, FileOrData type);
bool VerifyFileOrDataVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<FileOrData> *types);

enum class FontQuality : uint8_t {
  Low = 0,
  Medium = 1,
  High = 2,
  VeryHigh = 3,
  MIN = Low,
  MAX = VeryHigh
};

inline const FontQuality (&EnumValuesFontQuality())[4] {
  static const FontQuality values[] = {
    FontQuality::Low,
    FontQuality::Medium,
    FontQuality::High,
    FontQuality::VeryHigh
  };
  return values;
}

inline const char * const *EnumNamesFontQuality() {
  static const char * const names[5] = {
    "Low",
    "Medium",
    "High",
    "VeryHigh",
    nullptr
  };
  return names;
}

inline const char *EnumNameFontQuality(FontQuality e) {
  if (::flatbuffers::IsOutRange(e, FontQuality::Low, FontQuality::VeryHigh)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesFontQuality()[index];
}

enum class FontCacheSize : uint8_t {
  Small = 0,
  Large = 1,
  MIN = Small,
  MAX = Large
};

inline const FontCacheSize (&EnumValuesFontCacheSize())[2] {
  static const FontCacheSize values[] = {
    FontCacheSize::Small,
    FontCacheSize::Large
  };
  return values;
}

inline const char * const *EnumNamesFontCacheSize() {
  static const char * const names[3] = {
    "Small",
    "Large",
    nullptr
  };
  return names;
}

inline const char *EnumNameFontCacheSize(FontCacheSize e) {
  if (::flatbuffers::IsOutRange(e, FontCacheSize::Small, FontCacheSize::Large)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesFontCacheSize()[index];
}

struct FileReference FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FileReferenceBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_PATH = 4
  };
  const ::flatbuffers::String *path() const {
    return GetPointer<const ::flatbuffers::String *>(VT_PATH);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_PATH) &&
           verifier.VerifyString(path()) &&
           verifier.EndTable();
  }
};

struct FileReferenceBuilder {
  typedef FileReference Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_path(::flatbuffers::Offset<::flatbuffers::String> path) {
    fbb_.AddOffset(FileReference::VT_PATH, path);
  }
  explicit FileReferenceBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<FileReference> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<FileReference>(end);
    fbb_.Required(o, FileReference::VT_PATH);
    return o;
  }
};

inline ::flatbuffers::Offset<FileReference> CreateFileReference(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> path = 0) {
  FileReferenceBuilder builder_(_fbb);
  builder_.add_path(path);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<FileReference> CreateFileReferenceDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *path = nullptr) {
  auto path__ = path ? _fbb.CreateString(path) : 0;
  return DeepSeaVectorDraw::CreateFileReference(
      _fbb,
      path__);
}

struct RawData FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef RawDataBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_DATA = 4
  };
  const ::flatbuffers::Vector<uint8_t> *data() const {
    return GetPointer<const ::flatbuffers::Vector<uint8_t> *>(VT_DATA);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_DATA) &&
           verifier.VerifyVector(data()) &&
           verifier.EndTable();
  }
};

struct RawDataBuilder {
  typedef RawData Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_data(::flatbuffers::Offset<::flatbuffers::Vector<uint8_t>> data) {
    fbb_.AddOffset(RawData::VT_DATA, data);
  }
  explicit RawDataBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<RawData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<RawData>(end);
    fbb_.Required(o, RawData::VT_DATA);
    return o;
  }
};

inline ::flatbuffers::Offset<RawData> CreateRawData(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<uint8_t>> data = 0) {
  RawDataBuilder builder_(_fbb);
  builder_.add_data(data);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<RawData> CreateRawDataDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<uint8_t> *data = nullptr) {
  auto data__ = data ? _fbb.CreateVector<uint8_t>(*data) : 0;
  return DeepSeaVectorDraw::CreateRawData(
      _fbb,
      data__);
}

struct Resource FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef ResourceBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_DATA_TYPE = 6,
    VT_DATA = 8
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  DeepSeaVectorDraw::FileOrData data_type() const {
    return static_cast<DeepSeaVectorDraw::FileOrData>(GetField<uint8_t>(VT_DATA_TYPE, 0));
  }
  const void *data() const {
    return GetPointer<const void *>(VT_DATA);
  }
  template<typename T> const T *data_as() const;
  const DeepSeaVectorDraw::FileReference *data_as_FileReference() const {
    return data_type() == DeepSeaVectorDraw::FileOrData::FileReference ? static_cast<const DeepSeaVectorDraw::FileReference *>(data()) : nullptr;
  }
  const DeepSeaVectorDraw::RawData *data_as_RawData() const {
    return data_type() == DeepSeaVectorDraw::FileOrData::RawData ? static_cast<const DeepSeaVectorDraw::RawData *>(data()) : nullptr;
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint8_t>(verifier, VT_DATA_TYPE, 1) &&
           VerifyOffsetRequired(verifier, VT_DATA) &&
           VerifyFileOrData(verifier, data(), data_type()) &&
           verifier.EndTable();
  }
};

template<> inline const DeepSeaVectorDraw::FileReference *Resource::data_as<DeepSeaVectorDraw::FileReference>() const {
  return data_as_FileReference();
}

template<> inline const DeepSeaVectorDraw::RawData *Resource::data_as<DeepSeaVectorDraw::RawData>() const {
  return data_as_RawData();
}

struct ResourceBuilder {
  typedef Resource Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(Resource::VT_NAME, name);
  }
  void add_data_type(DeepSeaVectorDraw::FileOrData data_type) {
    fbb_.AddElement<uint8_t>(Resource::VT_DATA_TYPE, static_cast<uint8_t>(data_type), 0);
  }
  void add_data(::flatbuffers::Offset<void> data) {
    fbb_.AddOffset(Resource::VT_DATA, data);
  }
  explicit ResourceBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Resource> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Resource>(end);
    fbb_.Required(o, Resource::VT_NAME);
    fbb_.Required(o, Resource::VT_DATA);
    return o;
  }
};

inline ::flatbuffers::Offset<Resource> CreateResource(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    DeepSeaVectorDraw::FileOrData data_type = DeepSeaVectorDraw::FileOrData::NONE,
    ::flatbuffers::Offset<void> data = 0) {
  ResourceBuilder builder_(_fbb);
  builder_.add_data(data);
  builder_.add_name(name);
  builder_.add_data_type(data_type);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Resource> CreateResourceDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    DeepSeaVectorDraw::FileOrData data_type = DeepSeaVectorDraw::FileOrData::NONE,
    ::flatbuffers::Offset<void> data = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaVectorDraw::CreateResource(
      _fbb,
      name__,
      data_type,
      data);
}

struct FaceGroup FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FaceGroupBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_FACES = 6
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>> *faces() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>> *>(VT_FACES);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffsetRequired(verifier, VT_FACES) &&
           verifier.VerifyVector(faces()) &&
           verifier.VerifyVectorOfTables(faces()) &&
           verifier.EndTable();
  }
};

struct FaceGroupBuilder {
  typedef FaceGroup Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(FaceGroup::VT_NAME, name);
  }
  void add_faces(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>>> faces) {
    fbb_.AddOffset(FaceGroup::VT_FACES, faces);
  }
  explicit FaceGroupBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<FaceGroup> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<FaceGroup>(end);
    fbb_.Required(o, FaceGroup::VT_NAME);
    fbb_.Required(o, FaceGroup::VT_FACES);
    return o;
  }
};

inline ::flatbuffers::Offset<FaceGroup> CreateFaceGroup(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>>> faces = 0) {
  FaceGroupBuilder builder_(_fbb);
  builder_.add_faces(faces);
  builder_.add_name(name);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<FaceGroup> CreateFaceGroupDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const std::vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>> *faces = nullptr) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto faces__ = faces ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>>(*faces) : 0;
  return DeepSeaVectorDraw::CreateFaceGroup(
      _fbb,
      name__,
      faces__);
}

struct Font FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef FontBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_FACEGROUP = 6,
    VT_FACES = 8,
    VT_QUALITY = 10,
    VT_CACHESIZE = 12
  };
  const ::flatbuffers::String *name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_NAME);
  }
  const ::flatbuffers::String *faceGroup() const {
    return GetPointer<const ::flatbuffers::String *>(VT_FACEGROUP);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *faces() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>> *>(VT_FACES);
  }
  DeepSeaVectorDraw::FontQuality quality() const {
    return static_cast<DeepSeaVectorDraw::FontQuality>(GetField<uint8_t>(VT_QUALITY, 0));
  }
  DeepSeaVectorDraw::FontCacheSize cacheSize() const {
    return static_cast<DeepSeaVectorDraw::FontCacheSize>(GetField<uint8_t>(VT_CACHESIZE, 0));
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffsetRequired(verifier, VT_FACEGROUP) &&
           verifier.VerifyString(faceGroup()) &&
           VerifyOffsetRequired(verifier, VT_FACES) &&
           verifier.VerifyVector(faces()) &&
           verifier.VerifyVectorOfStrings(faces()) &&
           VerifyField<uint8_t>(verifier, VT_QUALITY, 1) &&
           VerifyField<uint8_t>(verifier, VT_CACHESIZE, 1) &&
           verifier.EndTable();
  }
};

struct FontBuilder {
  typedef Font Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_name(::flatbuffers::Offset<::flatbuffers::String> name) {
    fbb_.AddOffset(Font::VT_NAME, name);
  }
  void add_faceGroup(::flatbuffers::Offset<::flatbuffers::String> faceGroup) {
    fbb_.AddOffset(Font::VT_FACEGROUP, faceGroup);
  }
  void add_faces(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> faces) {
    fbb_.AddOffset(Font::VT_FACES, faces);
  }
  void add_quality(DeepSeaVectorDraw::FontQuality quality) {
    fbb_.AddElement<uint8_t>(Font::VT_QUALITY, static_cast<uint8_t>(quality), 0);
  }
  void add_cacheSize(DeepSeaVectorDraw::FontCacheSize cacheSize) {
    fbb_.AddElement<uint8_t>(Font::VT_CACHESIZE, static_cast<uint8_t>(cacheSize), 0);
  }
  explicit FontBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<Font> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<Font>(end);
    fbb_.Required(o, Font::VT_NAME);
    fbb_.Required(o, Font::VT_FACEGROUP);
    fbb_.Required(o, Font::VT_FACES);
    return o;
  }
};

inline ::flatbuffers::Offset<Font> CreateFont(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> name = 0,
    ::flatbuffers::Offset<::flatbuffers::String> faceGroup = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<::flatbuffers::String>>> faces = 0,
    DeepSeaVectorDraw::FontQuality quality = DeepSeaVectorDraw::FontQuality::Low,
    DeepSeaVectorDraw::FontCacheSize cacheSize = DeepSeaVectorDraw::FontCacheSize::Small) {
  FontBuilder builder_(_fbb);
  builder_.add_faces(faces);
  builder_.add_faceGroup(faceGroup);
  builder_.add_name(name);
  builder_.add_cacheSize(cacheSize);
  builder_.add_quality(quality);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<Font> CreateFontDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const char *faceGroup = nullptr,
    const std::vector<::flatbuffers::Offset<::flatbuffers::String>> *faces = nullptr,
    DeepSeaVectorDraw::FontQuality quality = DeepSeaVectorDraw::FontQuality::Low,
    DeepSeaVectorDraw::FontCacheSize cacheSize = DeepSeaVectorDraw::FontCacheSize::Small) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto faceGroup__ = faceGroup ? _fbb.CreateString(faceGroup) : 0;
  auto faces__ = faces ? _fbb.CreateVector<::flatbuffers::Offset<::flatbuffers::String>>(*faces) : 0;
  return DeepSeaVectorDraw::CreateFont(
      _fbb,
      name__,
      faceGroup__,
      faces__,
      quality,
      cacheSize);
}

struct VectorResources FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef VectorResourcesBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_TEXTURES = 4,
    VT_FACEGROUPS = 6,
    VT_FONTS = 8
  };
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>> *textures() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>> *>(VT_TEXTURES);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::FaceGroup>> *faceGroups() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::FaceGroup>> *>(VT_FACEGROUPS);
  }
  const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Font>> *fonts() const {
    return GetPointer<const ::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Font>> *>(VT_FONTS);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_TEXTURES) &&
           verifier.VerifyVector(textures()) &&
           verifier.VerifyVectorOfTables(textures()) &&
           VerifyOffset(verifier, VT_FACEGROUPS) &&
           verifier.VerifyVector(faceGroups()) &&
           verifier.VerifyVectorOfTables(faceGroups()) &&
           VerifyOffset(verifier, VT_FONTS) &&
           verifier.VerifyVector(fonts()) &&
           verifier.VerifyVectorOfTables(fonts()) &&
           verifier.EndTable();
  }
};

struct VectorResourcesBuilder {
  typedef VectorResources Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_textures(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>>> textures) {
    fbb_.AddOffset(VectorResources::VT_TEXTURES, textures);
  }
  void add_faceGroups(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::FaceGroup>>> faceGroups) {
    fbb_.AddOffset(VectorResources::VT_FACEGROUPS, faceGroups);
  }
  void add_fonts(::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Font>>> fonts) {
    fbb_.AddOffset(VectorResources::VT_FONTS, fonts);
  }
  explicit VectorResourcesBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<VectorResources> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<VectorResources>(end);
    return o;
  }
};

inline ::flatbuffers::Offset<VectorResources> CreateVectorResources(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>>> textures = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::FaceGroup>>> faceGroups = 0,
    ::flatbuffers::Offset<::flatbuffers::Vector<::flatbuffers::Offset<DeepSeaVectorDraw::Font>>> fonts = 0) {
  VectorResourcesBuilder builder_(_fbb);
  builder_.add_fonts(fonts);
  builder_.add_faceGroups(faceGroups);
  builder_.add_textures(textures);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<VectorResources> CreateVectorResourcesDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>> *textures = nullptr,
    const std::vector<::flatbuffers::Offset<DeepSeaVectorDraw::FaceGroup>> *faceGroups = nullptr,
    const std::vector<::flatbuffers::Offset<DeepSeaVectorDraw::Font>> *fonts = nullptr) {
  auto textures__ = textures ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaVectorDraw::Resource>>(*textures) : 0;
  auto faceGroups__ = faceGroups ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaVectorDraw::FaceGroup>>(*faceGroups) : 0;
  auto fonts__ = fonts ? _fbb.CreateVector<::flatbuffers::Offset<DeepSeaVectorDraw::Font>>(*fonts) : 0;
  return DeepSeaVectorDraw::CreateVectorResources(
      _fbb,
      textures__,
      faceGroups__,
      fonts__);
}

inline bool VerifyFileOrData(::flatbuffers::Verifier &verifier, const void *obj, FileOrData type) {
  switch (type) {
    case FileOrData::NONE: {
      return true;
    }
    case FileOrData::FileReference: {
      auto ptr = reinterpret_cast<const DeepSeaVectorDraw::FileReference *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case FileOrData::RawData: {
      auto ptr = reinterpret_cast<const DeepSeaVectorDraw::RawData *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyFileOrDataVector(::flatbuffers::Verifier &verifier, const ::flatbuffers::Vector<::flatbuffers::Offset<void>> *values, const ::flatbuffers::Vector<FileOrData> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (::flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyFileOrData(
        verifier,  values->Get(i), types->GetEnum<FileOrData>(i))) {
      return false;
    }
  }
  return true;
}

inline const DeepSeaVectorDraw::VectorResources *GetVectorResources(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaVectorDraw::VectorResources>(buf);
}

inline const DeepSeaVectorDraw::VectorResources *GetSizePrefixedVectorResources(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaVectorDraw::VectorResources>(buf);
}

inline bool VerifyVectorResourcesBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaVectorDraw::VectorResources>(nullptr);
}

inline bool VerifySizePrefixedVectorResourcesBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaVectorDraw::VectorResources>(nullptr);
}

inline void FinishVectorResourcesBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaVectorDraw::VectorResources> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedVectorResourcesBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaVectorDraw::VectorResources> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaVectorDraw

#endif  // FLATBUFFERS_GENERATED_VECTORRESOURCES_DEEPSEAVECTORDRAW_H_

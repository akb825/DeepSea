// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_VIEW_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_VIEW_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScene {

struct Surface;
struct SurfaceBuilder;

struct FramebufferSurface;
struct FramebufferSurfaceBuilder;

struct Framebuffer;
struct FramebufferBuilder;

struct View;
struct ViewBuilder;

enum class SurfaceType : uint8_t {
  Renderbuffer = 0,
  Offscreen = 1,
  MIN = Renderbuffer,
  MAX = Offscreen
};

inline const SurfaceType (&EnumValuesSurfaceType())[2] {
  static const SurfaceType values[] = {
    SurfaceType::Renderbuffer,
    SurfaceType::Offscreen
  };
  return values;
}

inline const char * const *EnumNamesSurfaceType() {
  static const char * const names[3] = {
    "Renderbuffer",
    "Offscreen",
    nullptr
  };
  return names;
}

inline const char *EnumNameSurfaceType(SurfaceType e) {
  if (flatbuffers::IsOutRange(e, SurfaceType::Renderbuffer, SurfaceType::Offscreen)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesSurfaceType()[index];
}

struct Surface FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef SurfaceBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_TYPE = 6,
    VT_USAGE = 8,
    VT_MEMORYHINTS = 10,
    VT_FORMAT = 12,
    VT_DECORATION = 14,
    VT_DIMENSION = 16,
    VT_WIDTH = 18,
    VT_WIDTHRATIO = 20,
    VT_HEIGHT = 22,
    VT_HEIGHTRATIO = 24,
    VT_DEPTH = 26,
    VT_MIPLEVELS = 28,
    VT_SAMPLES = 30,
    VT_RESOLVE = 32,
    VT_WINDOWFRAMEBUFFER = 34
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  DeepSeaScene::SurfaceType type() const {
    return static_cast<DeepSeaScene::SurfaceType>(GetField<uint8_t>(VT_TYPE, 0));
  }
  uint32_t usage() const {
    return GetField<uint32_t>(VT_USAGE, 0);
  }
  uint32_t memoryHints() const {
    return GetField<uint32_t>(VT_MEMORYHINTS, 0);
  }
  DeepSeaScene::TextureFormat format() const {
    return static_cast<DeepSeaScene::TextureFormat>(GetField<uint8_t>(VT_FORMAT, 0));
  }
  DeepSeaScene::FormatDecoration decoration() const {
    return static_cast<DeepSeaScene::FormatDecoration>(GetField<uint8_t>(VT_DECORATION, 0));
  }
  DeepSeaScene::TextureDim dimension() const {
    return static_cast<DeepSeaScene::TextureDim>(GetField<uint8_t>(VT_DIMENSION, 0));
  }
  uint32_t width() const {
    return GetField<uint32_t>(VT_WIDTH, 0);
  }
  float widthRatio() const {
    return GetField<float>(VT_WIDTHRATIO, 0.0f);
  }
  uint32_t height() const {
    return GetField<uint32_t>(VT_HEIGHT, 0);
  }
  float heightRatio() const {
    return GetField<float>(VT_HEIGHTRATIO, 0.0f);
  }
  uint32_t depth() const {
    return GetField<uint32_t>(VT_DEPTH, 0);
  }
  uint32_t mipLevels() const {
    return GetField<uint32_t>(VT_MIPLEVELS, 0);
  }
  uint32_t samples() const {
    return GetField<uint32_t>(VT_SAMPLES, 0);
  }
  bool resolve() const {
    return GetField<uint8_t>(VT_RESOLVE, 0) != 0;
  }
  bool windowFramebuffer() const {
    return GetField<uint8_t>(VT_WINDOWFRAMEBUFFER, 0) != 0;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint8_t>(verifier, VT_TYPE) &&
           VerifyField<uint32_t>(verifier, VT_USAGE) &&
           VerifyField<uint32_t>(verifier, VT_MEMORYHINTS) &&
           VerifyField<uint8_t>(verifier, VT_FORMAT) &&
           VerifyField<uint8_t>(verifier, VT_DECORATION) &&
           VerifyField<uint8_t>(verifier, VT_DIMENSION) &&
           VerifyField<uint32_t>(verifier, VT_WIDTH) &&
           VerifyField<float>(verifier, VT_WIDTHRATIO) &&
           VerifyField<uint32_t>(verifier, VT_HEIGHT) &&
           VerifyField<float>(verifier, VT_HEIGHTRATIO) &&
           VerifyField<uint32_t>(verifier, VT_DEPTH) &&
           VerifyField<uint32_t>(verifier, VT_MIPLEVELS) &&
           VerifyField<uint32_t>(verifier, VT_SAMPLES) &&
           VerifyField<uint8_t>(verifier, VT_RESOLVE) &&
           VerifyField<uint8_t>(verifier, VT_WINDOWFRAMEBUFFER) &&
           verifier.EndTable();
  }
};

struct SurfaceBuilder {
  typedef Surface Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Surface::VT_NAME, name);
  }
  void add_type(DeepSeaScene::SurfaceType type) {
    fbb_.AddElement<uint8_t>(Surface::VT_TYPE, static_cast<uint8_t>(type), 0);
  }
  void add_usage(uint32_t usage) {
    fbb_.AddElement<uint32_t>(Surface::VT_USAGE, usage, 0);
  }
  void add_memoryHints(uint32_t memoryHints) {
    fbb_.AddElement<uint32_t>(Surface::VT_MEMORYHINTS, memoryHints, 0);
  }
  void add_format(DeepSeaScene::TextureFormat format) {
    fbb_.AddElement<uint8_t>(Surface::VT_FORMAT, static_cast<uint8_t>(format), 0);
  }
  void add_decoration(DeepSeaScene::FormatDecoration decoration) {
    fbb_.AddElement<uint8_t>(Surface::VT_DECORATION, static_cast<uint8_t>(decoration), 0);
  }
  void add_dimension(DeepSeaScene::TextureDim dimension) {
    fbb_.AddElement<uint8_t>(Surface::VT_DIMENSION, static_cast<uint8_t>(dimension), 0);
  }
  void add_width(uint32_t width) {
    fbb_.AddElement<uint32_t>(Surface::VT_WIDTH, width, 0);
  }
  void add_widthRatio(float widthRatio) {
    fbb_.AddElement<float>(Surface::VT_WIDTHRATIO, widthRatio, 0.0f);
  }
  void add_height(uint32_t height) {
    fbb_.AddElement<uint32_t>(Surface::VT_HEIGHT, height, 0);
  }
  void add_heightRatio(float heightRatio) {
    fbb_.AddElement<float>(Surface::VT_HEIGHTRATIO, heightRatio, 0.0f);
  }
  void add_depth(uint32_t depth) {
    fbb_.AddElement<uint32_t>(Surface::VT_DEPTH, depth, 0);
  }
  void add_mipLevels(uint32_t mipLevels) {
    fbb_.AddElement<uint32_t>(Surface::VT_MIPLEVELS, mipLevels, 0);
  }
  void add_samples(uint32_t samples) {
    fbb_.AddElement<uint32_t>(Surface::VT_SAMPLES, samples, 0);
  }
  void add_resolve(bool resolve) {
    fbb_.AddElement<uint8_t>(Surface::VT_RESOLVE, static_cast<uint8_t>(resolve), 0);
  }
  void add_windowFramebuffer(bool windowFramebuffer) {
    fbb_.AddElement<uint8_t>(Surface::VT_WINDOWFRAMEBUFFER, static_cast<uint8_t>(windowFramebuffer), 0);
  }
  explicit SurfaceBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Surface> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Surface>(end);
    fbb_.Required(o, Surface::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<Surface> CreateSurface(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    DeepSeaScene::SurfaceType type = DeepSeaScene::SurfaceType::Renderbuffer,
    uint32_t usage = 0,
    uint32_t memoryHints = 0,
    DeepSeaScene::TextureFormat format = DeepSeaScene::TextureFormat::R4G4,
    DeepSeaScene::FormatDecoration decoration = DeepSeaScene::FormatDecoration::UNorm,
    DeepSeaScene::TextureDim dimension = DeepSeaScene::TextureDim::Dim1D,
    uint32_t width = 0,
    float widthRatio = 0.0f,
    uint32_t height = 0,
    float heightRatio = 0.0f,
    uint32_t depth = 0,
    uint32_t mipLevels = 0,
    uint32_t samples = 0,
    bool resolve = false,
    bool windowFramebuffer = false) {
  SurfaceBuilder builder_(_fbb);
  builder_.add_samples(samples);
  builder_.add_mipLevels(mipLevels);
  builder_.add_depth(depth);
  builder_.add_heightRatio(heightRatio);
  builder_.add_height(height);
  builder_.add_widthRatio(widthRatio);
  builder_.add_width(width);
  builder_.add_memoryHints(memoryHints);
  builder_.add_usage(usage);
  builder_.add_name(name);
  builder_.add_windowFramebuffer(windowFramebuffer);
  builder_.add_resolve(resolve);
  builder_.add_dimension(dimension);
  builder_.add_decoration(decoration);
  builder_.add_format(format);
  builder_.add_type(type);
  return builder_.Finish();
}

inline flatbuffers::Offset<Surface> CreateSurfaceDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    DeepSeaScene::SurfaceType type = DeepSeaScene::SurfaceType::Renderbuffer,
    uint32_t usage = 0,
    uint32_t memoryHints = 0,
    DeepSeaScene::TextureFormat format = DeepSeaScene::TextureFormat::R4G4,
    DeepSeaScene::FormatDecoration decoration = DeepSeaScene::FormatDecoration::UNorm,
    DeepSeaScene::TextureDim dimension = DeepSeaScene::TextureDim::Dim1D,
    uint32_t width = 0,
    float widthRatio = 0.0f,
    uint32_t height = 0,
    float heightRatio = 0.0f,
    uint32_t depth = 0,
    uint32_t mipLevels = 0,
    uint32_t samples = 0,
    bool resolve = false,
    bool windowFramebuffer = false) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaScene::CreateSurface(
      _fbb,
      name__,
      type,
      usage,
      memoryHints,
      format,
      decoration,
      dimension,
      width,
      widthRatio,
      height,
      heightRatio,
      depth,
      mipLevels,
      samples,
      resolve,
      windowFramebuffer);
}

struct FramebufferSurface FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef FramebufferSurfaceBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_FACE = 6,
    VT_LAYER = 8,
    VT_MIPLEVEL = 10
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  DeepSeaScene::CubeFace face() const {
    return static_cast<DeepSeaScene::CubeFace>(GetField<uint8_t>(VT_FACE, 0));
  }
  uint32_t layer() const {
    return GetField<uint32_t>(VT_LAYER, 0);
  }
  uint32_t mipLevel() const {
    return GetField<uint32_t>(VT_MIPLEVEL, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint8_t>(verifier, VT_FACE) &&
           VerifyField<uint32_t>(verifier, VT_LAYER) &&
           VerifyField<uint32_t>(verifier, VT_MIPLEVEL) &&
           verifier.EndTable();
  }
};

struct FramebufferSurfaceBuilder {
  typedef FramebufferSurface Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(FramebufferSurface::VT_NAME, name);
  }
  void add_face(DeepSeaScene::CubeFace face) {
    fbb_.AddElement<uint8_t>(FramebufferSurface::VT_FACE, static_cast<uint8_t>(face), 0);
  }
  void add_layer(uint32_t layer) {
    fbb_.AddElement<uint32_t>(FramebufferSurface::VT_LAYER, layer, 0);
  }
  void add_mipLevel(uint32_t mipLevel) {
    fbb_.AddElement<uint32_t>(FramebufferSurface::VT_MIPLEVEL, mipLevel, 0);
  }
  explicit FramebufferSurfaceBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<FramebufferSurface> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<FramebufferSurface>(end);
    fbb_.Required(o, FramebufferSurface::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<FramebufferSurface> CreateFramebufferSurface(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    DeepSeaScene::CubeFace face = DeepSeaScene::CubeFace::PosX,
    uint32_t layer = 0,
    uint32_t mipLevel = 0) {
  FramebufferSurfaceBuilder builder_(_fbb);
  builder_.add_mipLevel(mipLevel);
  builder_.add_layer(layer);
  builder_.add_name(name);
  builder_.add_face(face);
  return builder_.Finish();
}

inline flatbuffers::Offset<FramebufferSurface> CreateFramebufferSurfaceDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    DeepSeaScene::CubeFace face = DeepSeaScene::CubeFace::PosX,
    uint32_t layer = 0,
    uint32_t mipLevel = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  return DeepSeaScene::CreateFramebufferSurface(
      _fbb,
      name__,
      face,
      layer,
      mipLevel);
}

struct Framebuffer FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef FramebufferBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_SURFACES = 6,
    VT_WIDTH = 8,
    VT_HEIGHT = 10,
    VT_LAYERS = 12,
    VT_VIEWPORT = 14
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::FramebufferSurface>> *surfaces() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::FramebufferSurface>> *>(VT_SURFACES);
  }
  float width() const {
    return GetField<float>(VT_WIDTH, 0.0f);
  }
  float height() const {
    return GetField<float>(VT_HEIGHT, 0.0f);
  }
  uint32_t layers() const {
    return GetField<uint32_t>(VT_LAYERS, 0);
  }
  const DeepSeaScene::AlignedBox3f *viewport() const {
    return GetStruct<const DeepSeaScene::AlignedBox3f *>(VT_VIEWPORT);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyOffset(verifier, VT_SURFACES) &&
           verifier.VerifyVector(surfaces()) &&
           verifier.VerifyVectorOfTables(surfaces()) &&
           VerifyField<float>(verifier, VT_WIDTH) &&
           VerifyField<float>(verifier, VT_HEIGHT) &&
           VerifyField<uint32_t>(verifier, VT_LAYERS) &&
           VerifyField<DeepSeaScene::AlignedBox3f>(verifier, VT_VIEWPORT) &&
           verifier.EndTable();
  }
};

struct FramebufferBuilder {
  typedef Framebuffer Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Framebuffer::VT_NAME, name);
  }
  void add_surfaces(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::FramebufferSurface>>> surfaces) {
    fbb_.AddOffset(Framebuffer::VT_SURFACES, surfaces);
  }
  void add_width(float width) {
    fbb_.AddElement<float>(Framebuffer::VT_WIDTH, width, 0.0f);
  }
  void add_height(float height) {
    fbb_.AddElement<float>(Framebuffer::VT_HEIGHT, height, 0.0f);
  }
  void add_layers(uint32_t layers) {
    fbb_.AddElement<uint32_t>(Framebuffer::VT_LAYERS, layers, 0);
  }
  void add_viewport(const DeepSeaScene::AlignedBox3f *viewport) {
    fbb_.AddStruct(Framebuffer::VT_VIEWPORT, viewport);
  }
  explicit FramebufferBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Framebuffer> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Framebuffer>(end);
    fbb_.Required(o, Framebuffer::VT_NAME);
    return o;
  }
};

inline flatbuffers::Offset<Framebuffer> CreateFramebuffer(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::FramebufferSurface>>> surfaces = 0,
    float width = 0.0f,
    float height = 0.0f,
    uint32_t layers = 0,
    const DeepSeaScene::AlignedBox3f *viewport = 0) {
  FramebufferBuilder builder_(_fbb);
  builder_.add_viewport(viewport);
  builder_.add_layers(layers);
  builder_.add_height(height);
  builder_.add_width(width);
  builder_.add_surfaces(surfaces);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<Framebuffer> CreateFramebufferDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const std::vector<flatbuffers::Offset<DeepSeaScene::FramebufferSurface>> *surfaces = nullptr,
    float width = 0.0f,
    float height = 0.0f,
    uint32_t layers = 0,
    const DeepSeaScene::AlignedBox3f *viewport = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto surfaces__ = surfaces ? _fbb.CreateVector<flatbuffers::Offset<DeepSeaScene::FramebufferSurface>>(*surfaces) : 0;
  return DeepSeaScene::CreateFramebuffer(
      _fbb,
      name__,
      surfaces__,
      width,
      height,
      layers,
      viewport);
}

struct View FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ViewBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SURFACES = 4,
    VT_FRAMEBUFFERS = 6
  };
  const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::Surface>> *surfaces() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::Surface>> *>(VT_SURFACES);
  }
  const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::Framebuffer>> *framebuffers() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::Framebuffer>> *>(VT_FRAMEBUFFERS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SURFACES) &&
           verifier.VerifyVector(surfaces()) &&
           verifier.VerifyVectorOfTables(surfaces()) &&
           VerifyOffsetRequired(verifier, VT_FRAMEBUFFERS) &&
           verifier.VerifyVector(framebuffers()) &&
           verifier.VerifyVectorOfTables(framebuffers()) &&
           verifier.EndTable();
  }
};

struct ViewBuilder {
  typedef View Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_surfaces(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::Surface>>> surfaces) {
    fbb_.AddOffset(View::VT_SURFACES, surfaces);
  }
  void add_framebuffers(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::Framebuffer>>> framebuffers) {
    fbb_.AddOffset(View::VT_FRAMEBUFFERS, framebuffers);
  }
  explicit ViewBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<View> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<View>(end);
    fbb_.Required(o, View::VT_FRAMEBUFFERS);
    return o;
  }
};

inline flatbuffers::Offset<View> CreateView(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::Surface>>> surfaces = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<DeepSeaScene::Framebuffer>>> framebuffers = 0) {
  ViewBuilder builder_(_fbb);
  builder_.add_framebuffers(framebuffers);
  builder_.add_surfaces(surfaces);
  return builder_.Finish();
}

inline flatbuffers::Offset<View> CreateViewDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<DeepSeaScene::Surface>> *surfaces = nullptr,
    const std::vector<flatbuffers::Offset<DeepSeaScene::Framebuffer>> *framebuffers = nullptr) {
  auto surfaces__ = surfaces ? _fbb.CreateVector<flatbuffers::Offset<DeepSeaScene::Surface>>(*surfaces) : 0;
  auto framebuffers__ = framebuffers ? _fbb.CreateVector<flatbuffers::Offset<DeepSeaScene::Framebuffer>>(*framebuffers) : 0;
  return DeepSeaScene::CreateView(
      _fbb,
      surfaces__,
      framebuffers__);
}

inline const DeepSeaScene::View *GetView(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::View>(buf);
}

inline const DeepSeaScene::View *GetSizePrefixedView(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::View>(buf);
}

inline bool VerifyViewBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::View>(nullptr);
}

inline bool VerifySizePrefixedViewBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::View>(nullptr);
}

inline void FinishViewBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::View> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedViewBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::View> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_VIEW_DEEPSEASCENE_H_

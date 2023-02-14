// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_ANIMATIONCOMMON_DEEPSEAANIMATION_H_
#define FLATBUFFERS_GENERATED_ANIMATIONCOMMON_DEEPSEAANIMATION_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 1 &&
              FLATBUFFERS_VERSION_REVISION == 21,
             "Non-compatible flatbuffers version included");

namespace DeepSeaAnimation {

struct Vector3f;

struct Vector4f;

struct Quaternion4f;

struct Matrix44f;

enum class AnimationComponent : uint8_t {
  Translation = 0,
  Rotation = 1,
  Scale = 2,
  MIN = Translation,
  MAX = Scale
};

inline const AnimationComponent (&EnumValuesAnimationComponent())[3] {
  static const AnimationComponent values[] = {
    AnimationComponent::Translation,
    AnimationComponent::Rotation,
    AnimationComponent::Scale
  };
  return values;
}

inline const char * const *EnumNamesAnimationComponent() {
  static const char * const names[4] = {
    "Translation",
    "Rotation",
    "Scale",
    nullptr
  };
  return names;
}

inline const char *EnumNameAnimationComponent(AnimationComponent e) {
  if (::flatbuffers::IsOutRange(e, AnimationComponent::Translation, AnimationComponent::Scale)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesAnimationComponent()[index];
}

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Vector3f FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;
  float z_;

 public:
  Vector3f()
      : x_(0),
        y_(0),
        z_(0) {
  }
  Vector3f(float _x, float _y, float _z)
      : x_(::flatbuffers::EndianScalar(_x)),
        y_(::flatbuffers::EndianScalar(_y)),
        z_(::flatbuffers::EndianScalar(_z)) {
  }
  float x() const {
    return ::flatbuffers::EndianScalar(x_);
  }
  float y() const {
    return ::flatbuffers::EndianScalar(y_);
  }
  float z() const {
    return ::flatbuffers::EndianScalar(z_);
  }
};
FLATBUFFERS_STRUCT_END(Vector3f, 12);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Vector4f FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;
  float z_;
  float w_;

 public:
  Vector4f()
      : x_(0),
        y_(0),
        z_(0),
        w_(0) {
  }
  Vector4f(float _x, float _y, float _z, float _w)
      : x_(::flatbuffers::EndianScalar(_x)),
        y_(::flatbuffers::EndianScalar(_y)),
        z_(::flatbuffers::EndianScalar(_z)),
        w_(::flatbuffers::EndianScalar(_w)) {
  }
  float x() const {
    return ::flatbuffers::EndianScalar(x_);
  }
  float y() const {
    return ::flatbuffers::EndianScalar(y_);
  }
  float z() const {
    return ::flatbuffers::EndianScalar(z_);
  }
  float w() const {
    return ::flatbuffers::EndianScalar(w_);
  }
};
FLATBUFFERS_STRUCT_END(Vector4f, 16);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Quaternion4f FLATBUFFERS_FINAL_CLASS {
 private:
  float r_;
  float i_;
  float j_;
  float k_;

 public:
  Quaternion4f()
      : r_(0),
        i_(0),
        j_(0),
        k_(0) {
  }
  Quaternion4f(float _r, float _i, float _j, float _k)
      : r_(::flatbuffers::EndianScalar(_r)),
        i_(::flatbuffers::EndianScalar(_i)),
        j_(::flatbuffers::EndianScalar(_j)),
        k_(::flatbuffers::EndianScalar(_k)) {
  }
  float r() const {
    return ::flatbuffers::EndianScalar(r_);
  }
  float i() const {
    return ::flatbuffers::EndianScalar(i_);
  }
  float j() const {
    return ::flatbuffers::EndianScalar(j_);
  }
  float k() const {
    return ::flatbuffers::EndianScalar(k_);
  }
};
FLATBUFFERS_STRUCT_END(Quaternion4f, 16);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) Matrix44f FLATBUFFERS_FINAL_CLASS {
 private:
  DeepSeaAnimation::Vector4f column0_;
  DeepSeaAnimation::Vector4f column1_;
  DeepSeaAnimation::Vector4f column2_;
  DeepSeaAnimation::Vector4f column3_;

 public:
  Matrix44f()
      : column0_(),
        column1_(),
        column2_(),
        column3_() {
  }
  Matrix44f(const DeepSeaAnimation::Vector4f &_column0, const DeepSeaAnimation::Vector4f &_column1, const DeepSeaAnimation::Vector4f &_column2, const DeepSeaAnimation::Vector4f &_column3)
      : column0_(_column0),
        column1_(_column1),
        column2_(_column2),
        column3_(_column3) {
  }
  const DeepSeaAnimation::Vector4f &column0() const {
    return column0_;
  }
  const DeepSeaAnimation::Vector4f &column1() const {
    return column1_;
  }
  const DeepSeaAnimation::Vector4f &column2() const {
    return column2_;
  }
  const DeepSeaAnimation::Vector4f &column3() const {
    return column3_;
  }
};
FLATBUFFERS_STRUCT_END(Matrix44f, 64);

}  // namespace DeepSeaAnimation

#endif  // FLATBUFFERS_GENERATED_ANIMATIONCOMMON_DEEPSEAANIMATION_H_
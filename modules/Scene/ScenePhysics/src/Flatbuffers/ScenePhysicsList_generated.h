// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SCENEPHYSICSLIST_DEEPSEASCENEPHYSICS_H_
#define FLATBUFFERS_GENERATED_SCENEPHYSICSLIST_DEEPSEASCENEPHYSICS_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 24 &&
              FLATBUFFERS_VERSION_MINOR == 3 &&
              FLATBUFFERS_VERSION_REVISION == 25,
             "Non-compatible flatbuffers version included");

#include "DeepSea/Scene/Flatbuffers/SceneCommon_generated.h"

namespace DeepSeaScenePhysics {

struct PhysicsList;
struct PhysicsListBuilder;

struct PhysicsList FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef PhysicsListBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MAXSTATICBODIES = 4,
    VT_MAXDYNAMICBODIES = 6,
    VT_MAXCONSTRAINEDBODYGROUPS = 8,
    VT_MAXSTATICSHAPES = 10,
    VT_MAXDYNAMICSHAPES = 12,
    VT_MAXCONSTRAINTS = 14,
    VT_MAXBODYCOLLISIONPAIRS = 16,
    VT_MAXCONTACTPOINTS = 18,
    VT_GRAVITY = 20,
    VT_MULTITHREADEDMODIFICATIONS = 22,
    VT_TARGETSTEPTIME = 24
  };
  uint32_t maxStaticBodies() const {
    return GetField<uint32_t>(VT_MAXSTATICBODIES, 0);
  }
  uint32_t maxDynamicBodies() const {
    return GetField<uint32_t>(VT_MAXDYNAMICBODIES, 0);
  }
  uint32_t maxConstrainedBodyGroups() const {
    return GetField<uint32_t>(VT_MAXCONSTRAINEDBODYGROUPS, 0);
  }
  uint32_t maxStaticShapes() const {
    return GetField<uint32_t>(VT_MAXSTATICSHAPES, 0);
  }
  uint32_t maxDynamicShapes() const {
    return GetField<uint32_t>(VT_MAXDYNAMICSHAPES, 0);
  }
  uint32_t maxConstraints() const {
    return GetField<uint32_t>(VT_MAXCONSTRAINTS, 0);
  }
  uint32_t maxBodyCollisionPairs() const {
    return GetField<uint32_t>(VT_MAXBODYCOLLISIONPAIRS, 0);
  }
  uint32_t maxContactPoints() const {
    return GetField<uint32_t>(VT_MAXCONTACTPOINTS, 0);
  }
  const DeepSeaScene::Vector3f *gravity() const {
    return GetStruct<const DeepSeaScene::Vector3f *>(VT_GRAVITY);
  }
  bool multiThreadedModifications() const {
    return GetField<uint8_t>(VT_MULTITHREADEDMODIFICATIONS, 0) != 0;
  }
  float targetStepTime() const {
    return GetField<float>(VT_TARGETSTEPTIME, 0.01666667f);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_MAXSTATICBODIES, 4) &&
           VerifyField<uint32_t>(verifier, VT_MAXDYNAMICBODIES, 4) &&
           VerifyField<uint32_t>(verifier, VT_MAXCONSTRAINEDBODYGROUPS, 4) &&
           VerifyField<uint32_t>(verifier, VT_MAXSTATICSHAPES, 4) &&
           VerifyField<uint32_t>(verifier, VT_MAXDYNAMICSHAPES, 4) &&
           VerifyField<uint32_t>(verifier, VT_MAXCONSTRAINTS, 4) &&
           VerifyField<uint32_t>(verifier, VT_MAXBODYCOLLISIONPAIRS, 4) &&
           VerifyField<uint32_t>(verifier, VT_MAXCONTACTPOINTS, 4) &&
           VerifyFieldRequired<DeepSeaScene::Vector3f>(verifier, VT_GRAVITY, 4) &&
           VerifyField<uint8_t>(verifier, VT_MULTITHREADEDMODIFICATIONS, 1) &&
           VerifyField<float>(verifier, VT_TARGETSTEPTIME, 4) &&
           verifier.EndTable();
  }
};

struct PhysicsListBuilder {
  typedef PhysicsList Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_maxStaticBodies(uint32_t maxStaticBodies) {
    fbb_.AddElement<uint32_t>(PhysicsList::VT_MAXSTATICBODIES, maxStaticBodies, 0);
  }
  void add_maxDynamicBodies(uint32_t maxDynamicBodies) {
    fbb_.AddElement<uint32_t>(PhysicsList::VT_MAXDYNAMICBODIES, maxDynamicBodies, 0);
  }
  void add_maxConstrainedBodyGroups(uint32_t maxConstrainedBodyGroups) {
    fbb_.AddElement<uint32_t>(PhysicsList::VT_MAXCONSTRAINEDBODYGROUPS, maxConstrainedBodyGroups, 0);
  }
  void add_maxStaticShapes(uint32_t maxStaticShapes) {
    fbb_.AddElement<uint32_t>(PhysicsList::VT_MAXSTATICSHAPES, maxStaticShapes, 0);
  }
  void add_maxDynamicShapes(uint32_t maxDynamicShapes) {
    fbb_.AddElement<uint32_t>(PhysicsList::VT_MAXDYNAMICSHAPES, maxDynamicShapes, 0);
  }
  void add_maxConstraints(uint32_t maxConstraints) {
    fbb_.AddElement<uint32_t>(PhysicsList::VT_MAXCONSTRAINTS, maxConstraints, 0);
  }
  void add_maxBodyCollisionPairs(uint32_t maxBodyCollisionPairs) {
    fbb_.AddElement<uint32_t>(PhysicsList::VT_MAXBODYCOLLISIONPAIRS, maxBodyCollisionPairs, 0);
  }
  void add_maxContactPoints(uint32_t maxContactPoints) {
    fbb_.AddElement<uint32_t>(PhysicsList::VT_MAXCONTACTPOINTS, maxContactPoints, 0);
  }
  void add_gravity(const DeepSeaScene::Vector3f *gravity) {
    fbb_.AddStruct(PhysicsList::VT_GRAVITY, gravity);
  }
  void add_multiThreadedModifications(bool multiThreadedModifications) {
    fbb_.AddElement<uint8_t>(PhysicsList::VT_MULTITHREADEDMODIFICATIONS, static_cast<uint8_t>(multiThreadedModifications), 0);
  }
  void add_targetStepTime(float targetStepTime) {
    fbb_.AddElement<float>(PhysicsList::VT_TARGETSTEPTIME, targetStepTime, 0.01666667f);
  }
  explicit PhysicsListBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<PhysicsList> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<PhysicsList>(end);
    fbb_.Required(o, PhysicsList::VT_GRAVITY);
    return o;
  }
};

inline ::flatbuffers::Offset<PhysicsList> CreatePhysicsList(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t maxStaticBodies = 0,
    uint32_t maxDynamicBodies = 0,
    uint32_t maxConstrainedBodyGroups = 0,
    uint32_t maxStaticShapes = 0,
    uint32_t maxDynamicShapes = 0,
    uint32_t maxConstraints = 0,
    uint32_t maxBodyCollisionPairs = 0,
    uint32_t maxContactPoints = 0,
    const DeepSeaScene::Vector3f *gravity = nullptr,
    bool multiThreadedModifications = false,
    float targetStepTime = 0.01666667f) {
  PhysicsListBuilder builder_(_fbb);
  builder_.add_targetStepTime(targetStepTime);
  builder_.add_gravity(gravity);
  builder_.add_maxContactPoints(maxContactPoints);
  builder_.add_maxBodyCollisionPairs(maxBodyCollisionPairs);
  builder_.add_maxConstraints(maxConstraints);
  builder_.add_maxDynamicShapes(maxDynamicShapes);
  builder_.add_maxStaticShapes(maxStaticShapes);
  builder_.add_maxConstrainedBodyGroups(maxConstrainedBodyGroups);
  builder_.add_maxDynamicBodies(maxDynamicBodies);
  builder_.add_maxStaticBodies(maxStaticBodies);
  builder_.add_multiThreadedModifications(multiThreadedModifications);
  return builder_.Finish();
}

inline const DeepSeaScenePhysics::PhysicsList *GetPhysicsList(const void *buf) {
  return ::flatbuffers::GetRoot<DeepSeaScenePhysics::PhysicsList>(buf);
}

inline const DeepSeaScenePhysics::PhysicsList *GetSizePrefixedPhysicsList(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<DeepSeaScenePhysics::PhysicsList>(buf);
}

inline bool VerifyPhysicsListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScenePhysics::PhysicsList>(nullptr);
}

inline bool VerifySizePrefixedPhysicsListBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScenePhysics::PhysicsList>(nullptr);
}

inline void FinishPhysicsListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScenePhysics::PhysicsList> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedPhysicsListBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<DeepSeaScenePhysics::PhysicsList> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScenePhysics

#endif  // FLATBUFFERS_GENERATED_SCENEPHYSICSLIST_DEEPSEASCENEPHYSICS_H_

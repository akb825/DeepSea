# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLighting

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SceneLightShadows(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SceneLightShadows()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSceneLightShadows(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # SceneLightShadows
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SceneLightShadows
    def LightSet(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # SceneLightShadows
    def LightType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # SceneLightShadows
    def Light(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # SceneLightShadows
    def TransformGroupDesc(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # SceneLightShadows
    def MaxCascades(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # SceneLightShadows
    def MaxFirstSplitDistance(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SceneLightShadows
    def CascadeExpFactor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SceneLightShadows
    def FadeStartDistance(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SceneLightShadows
    def MaxDistance(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def Start(builder): builder.StartObject(9)
def SceneLightShadowsStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddLightSet(builder, lightSet): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(lightSet), 0)
def SceneLightShadowsAddLightSet(builder, lightSet):
    """This method is deprecated. Please switch to AddLightSet."""
    return AddLightSet(builder, lightSet)
def AddLightType(builder, lightType): builder.PrependUint8Slot(1, lightType, 0)
def SceneLightShadowsAddLightType(builder, lightType):
    """This method is deprecated. Please switch to AddLightType."""
    return AddLightType(builder, lightType)
def AddLight(builder, light): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(light), 0)
def SceneLightShadowsAddLight(builder, light):
    """This method is deprecated. Please switch to AddLight."""
    return AddLight(builder, light)
def AddTransformGroupDesc(builder, transformGroupDesc): builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(transformGroupDesc), 0)
def SceneLightShadowsAddTransformGroupDesc(builder, transformGroupDesc):
    """This method is deprecated. Please switch to AddTransformGroupDesc."""
    return AddTransformGroupDesc(builder, transformGroupDesc)
def AddMaxCascades(builder, maxCascades): builder.PrependUint32Slot(4, maxCascades, 0)
def SceneLightShadowsAddMaxCascades(builder, maxCascades):
    """This method is deprecated. Please switch to AddMaxCascades."""
    return AddMaxCascades(builder, maxCascades)
def AddMaxFirstSplitDistance(builder, maxFirstSplitDistance): builder.PrependFloat32Slot(5, maxFirstSplitDistance, 0.0)
def SceneLightShadowsAddMaxFirstSplitDistance(builder, maxFirstSplitDistance):
    """This method is deprecated. Please switch to AddMaxFirstSplitDistance."""
    return AddMaxFirstSplitDistance(builder, maxFirstSplitDistance)
def AddCascadeExpFactor(builder, cascadeExpFactor): builder.PrependFloat32Slot(6, cascadeExpFactor, 0.0)
def SceneLightShadowsAddCascadeExpFactor(builder, cascadeExpFactor):
    """This method is deprecated. Please switch to AddCascadeExpFactor."""
    return AddCascadeExpFactor(builder, cascadeExpFactor)
def AddFadeStartDistance(builder, fadeStartDistance): builder.PrependFloat32Slot(7, fadeStartDistance, 0.0)
def SceneLightShadowsAddFadeStartDistance(builder, fadeStartDistance):
    """This method is deprecated. Please switch to AddFadeStartDistance."""
    return AddFadeStartDistance(builder, fadeStartDistance)
def AddMaxDistance(builder, maxDistance): builder.PrependFloat32Slot(8, maxDistance, 0.0)
def SceneLightShadowsAddMaxDistance(builder, maxDistance):
    """This method is deprecated. Please switch to AddMaxDistance."""
    return AddMaxDistance(builder, maxDistance)
def End(builder): return builder.EndObject()
def SceneLightShadowsEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)
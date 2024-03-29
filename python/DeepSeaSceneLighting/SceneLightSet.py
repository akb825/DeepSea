# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLighting

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SceneLightSet(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SceneLightSet()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSceneLightSet(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # SceneLightSet
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SceneLightSet
    def Lights(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaSceneLighting.Light import Light
            obj = Light()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneLightSet
    def LightsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # SceneLightSet
    def LightsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # SceneLightSet
    def MaxLights(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # SceneLightSet
    def AmbientColor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Color3f import Color3f
            obj = Color3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SceneLightSet
    def AmbientIntensity(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SceneLightSet
    def MainLight(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def SceneLightSetStart(builder):
    builder.StartObject(5)

def Start(builder):
    SceneLightSetStart(builder)

def SceneLightSetAddLights(builder, lights):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(lights), 0)

def AddLights(builder, lights):
    SceneLightSetAddLights(builder, lights)

def SceneLightSetStartLightsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartLightsVector(builder, numElems):
    return SceneLightSetStartLightsVector(builder, numElems)

def SceneLightSetAddMaxLights(builder, maxLights):
    builder.PrependUint32Slot(1, maxLights, 0)

def AddMaxLights(builder, maxLights):
    SceneLightSetAddMaxLights(builder, maxLights)

def SceneLightSetAddAmbientColor(builder, ambientColor):
    builder.PrependStructSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(ambientColor), 0)

def AddAmbientColor(builder, ambientColor):
    SceneLightSetAddAmbientColor(builder, ambientColor)

def SceneLightSetAddAmbientIntensity(builder, ambientIntensity):
    builder.PrependFloat32Slot(3, ambientIntensity, 0.0)

def AddAmbientIntensity(builder, ambientIntensity):
    SceneLightSetAddAmbientIntensity(builder, ambientIntensity)

def SceneLightSetAddMainLight(builder, mainLight):
    builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(mainLight), 0)

def AddMainLight(builder, mainLight):
    SceneLightSetAddMainLight(builder, mainLight)

def SceneLightSetEnd(builder):
    return builder.EndObject()

def End(builder):
    return SceneLightSetEnd(builder)

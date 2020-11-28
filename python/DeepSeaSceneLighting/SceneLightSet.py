# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLighting

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SceneLightSet(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsSceneLightSet(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SceneLightSet()
        x.Init(buf, n + offset)
        return x

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

def SceneLightSetStart(builder): builder.StartObject(4)
def SceneLightSetAddLights(builder, lights): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(lights), 0)
def SceneLightSetStartLightsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def SceneLightSetAddMaxLights(builder, maxLights): builder.PrependUint32Slot(1, maxLights, 0)
def SceneLightSetAddAmbientColor(builder, ambientColor): builder.PrependStructSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(ambientColor), 0)
def SceneLightSetAddAmbientIntensity(builder, ambientIntensity): builder.PrependFloat32Slot(3, ambientIntensity, 0.0)
def SceneLightSetEnd(builder): return builder.EndObject()
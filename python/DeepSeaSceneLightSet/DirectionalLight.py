# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLightSet

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class DirectionalLight(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsDirectionalLight(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = DirectionalLight()
        x.Init(buf, n + offset)
        return x

    # DirectionalLight
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # DirectionalLight
    def Direction(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # DirectionalLight
    def Color(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaSceneLightSet.Color3f import Color3f
            obj = Color3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # DirectionalLight
    def Intensity(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def DirectionalLightStart(builder): builder.StartObject(3)
def DirectionalLightAddDirection(builder, direction): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(direction), 0)
def DirectionalLightAddColor(builder, color): builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(color), 0)
def DirectionalLightAddIntensity(builder, intensity): builder.PrependFloat32Slot(2, intensity, 0.0)
def DirectionalLightEnd(builder): return builder.EndObject()

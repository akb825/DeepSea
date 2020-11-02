# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLightSet

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SpotLight(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsSpotLight(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SpotLight()
        x.Init(buf, n + offset)
        return x

    # SpotLight
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SpotLight
    def Position(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SpotLight
    def Direction(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SpotLight
    def Color(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaSceneLightSet.Color3f import Color3f
            obj = Color3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SpotLight
    def Intensity(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SpotLight
    def LinearFalloff(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SpotLight
    def QuadraticFalloff(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SpotLight
    def MinSpotAngle(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SpotLight
    def MaxSpotAngle(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def SpotLightStart(builder): builder.StartObject(8)
def SpotLightAddPosition(builder, position): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(position), 0)
def SpotLightAddDirection(builder, direction): builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(direction), 0)
def SpotLightAddColor(builder, color): builder.PrependStructSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(color), 0)
def SpotLightAddIntensity(builder, intensity): builder.PrependFloat32Slot(3, intensity, 0.0)
def SpotLightAddLinearFalloff(builder, linearFalloff): builder.PrependFloat32Slot(4, linearFalloff, 0.0)
def SpotLightAddQuadraticFalloff(builder, quadraticFalloff): builder.PrependFloat32Slot(5, quadraticFalloff, 0.0)
def SpotLightAddMinSpotAngle(builder, minSpotAngle): builder.PrependFloat32Slot(6, minSpotAngle, 0.0)
def SpotLightAddMaxSpotAngle(builder, maxSpotAngle): builder.PrependFloat32Slot(7, maxSpotAngle, 0.0)
def SpotLightEnd(builder): return builder.EndObject()

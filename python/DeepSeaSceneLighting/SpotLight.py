# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaSceneLighting

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SpotLight(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SpotLight()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSpotLight(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
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
            from DeepSeaScene.Color3f import Color3f
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
    def InnerSpotAngle(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SpotLight
    def OuterSpotAngle(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def Start(builder): builder.StartObject(8)
def SpotLightStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddPosition(builder, position): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(position), 0)
def SpotLightAddPosition(builder, position):
    """This method is deprecated. Please switch to AddPosition."""
    return AddPosition(builder, position)
def AddDirection(builder, direction): builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(direction), 0)
def SpotLightAddDirection(builder, direction):
    """This method is deprecated. Please switch to AddDirection."""
    return AddDirection(builder, direction)
def AddColor(builder, color): builder.PrependStructSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(color), 0)
def SpotLightAddColor(builder, color):
    """This method is deprecated. Please switch to AddColor."""
    return AddColor(builder, color)
def AddIntensity(builder, intensity): builder.PrependFloat32Slot(3, intensity, 0.0)
def SpotLightAddIntensity(builder, intensity):
    """This method is deprecated. Please switch to AddIntensity."""
    return AddIntensity(builder, intensity)
def AddLinearFalloff(builder, linearFalloff): builder.PrependFloat32Slot(4, linearFalloff, 0.0)
def SpotLightAddLinearFalloff(builder, linearFalloff):
    """This method is deprecated. Please switch to AddLinearFalloff."""
    return AddLinearFalloff(builder, linearFalloff)
def AddQuadraticFalloff(builder, quadraticFalloff): builder.PrependFloat32Slot(5, quadraticFalloff, 0.0)
def SpotLightAddQuadraticFalloff(builder, quadraticFalloff):
    """This method is deprecated. Please switch to AddQuadraticFalloff."""
    return AddQuadraticFalloff(builder, quadraticFalloff)
def AddInnerSpotAngle(builder, innerSpotAngle): builder.PrependFloat32Slot(6, innerSpotAngle, 0.0)
def SpotLightAddInnerSpotAngle(builder, innerSpotAngle):
    """This method is deprecated. Please switch to AddInnerSpotAngle."""
    return AddInnerSpotAngle(builder, innerSpotAngle)
def AddOuterSpotAngle(builder, outerSpotAngle): builder.PrependFloat32Slot(7, outerSpotAngle, 0.0)
def SpotLightAddOuterSpotAngle(builder, outerSpotAngle):
    """This method is deprecated. Please switch to AddOuterSpotAngle."""
    return AddOuterSpotAngle(builder, outerSpotAngle)
def End(builder): return builder.EndObject()
def SpotLightEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)
# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDrawScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class GradientStop(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = GradientStop()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsGradientStop(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # GradientStop
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # GradientStop
    def Position(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # GradientStop
    def Color(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaVectorDrawScene.Color import Color
            obj = Color()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def Start(builder): builder.StartObject(2)
def GradientStopStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddPosition(builder, position): builder.PrependFloat32Slot(0, position, 0.0)
def GradientStopAddPosition(builder, position):
    """This method is deprecated. Please switch to AddPosition."""
    return AddPosition(builder, position)
def AddColor(builder, color): builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(color), 0)
def GradientStopAddColor(builder, color):
    """This method is deprecated. Please switch to AddColor."""
    return AddColor(builder, color)
def End(builder): return builder.EndObject()
def GradientStopEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)
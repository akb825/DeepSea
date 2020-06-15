# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDrawScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class GradientStop(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsGradientStop(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = GradientStop()
        x.Init(buf, n + offset)
        return x

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

def GradientStopStart(builder): builder.StartObject(2)
def GradientStopAddPosition(builder, position): builder.PrependFloat32Slot(0, position, 0.0)
def GradientStopAddColor(builder, color): builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(color), 0)
def GradientStopEnd(builder): return builder.EndObject()

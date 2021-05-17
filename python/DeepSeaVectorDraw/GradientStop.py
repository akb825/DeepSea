# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class GradientStop(object):
    __slots__ = ['_tab']

    @classmethod
    def SizeOf(cls):
        return 8

    # GradientStop
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # GradientStop
    def Position(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    # GradientStop
    def Color(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 4)
        return obj


def CreateGradientStop(builder, position, color_r, color_g, color_b, color_a):
    builder.Prep(4, 8)
    builder.Prep(1, 4)
    builder.PrependUint8(color_a)
    builder.PrependUint8(color_b)
    builder.PrependUint8(color_g)
    builder.PrependUint8(color_r)
    builder.PrependFloat32(position)
    return builder.Offset()

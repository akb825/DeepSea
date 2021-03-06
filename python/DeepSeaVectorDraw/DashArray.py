# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class DashArray(object):
    __slots__ = ['_tab']

    @classmethod
    def SizeOf(cls):
        return 16

    # DashArray
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # DashArray
    def Solid0(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    # DashArray
    def Gap0(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(4))
    # DashArray
    def Solid1(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(8))
    # DashArray
    def Gap1(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(12))

def CreateDashArray(builder, solid0, gap0, solid1, gap1):
    builder.Prep(4, 16)
    builder.PrependFloat32(gap1)
    builder.PrependFloat32(solid1)
    builder.PrependFloat32(gap0)
    builder.PrependFloat32(solid0)
    return builder.Offset()

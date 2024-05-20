# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaPhysics

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Vector3f(object):
    __slots__ = ['_tab']

    @classmethod
    def SizeOf(cls):
        return 12

    # Vector3f
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Vector3f
    def X(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    # Vector3f
    def Y(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(4))
    # Vector3f
    def Z(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(8))

def CreateVector3f(builder, x, y, z):
    builder.Prep(4, 12)
    builder.PrependFloat32(z)
    builder.PrependFloat32(y)
    builder.PrependFloat32(x)
    return builder.Offset()

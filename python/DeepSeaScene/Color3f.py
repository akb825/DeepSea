# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Color3f(object):
    __slots__ = ['_tab']

    # Color3f
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Color3f
    def Red(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    # Color3f
    def Green(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(4))
    # Color3f
    def Blue(self): return self._tab.Get(flatbuffers.number_types.Float32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(8))

def CreateColor3f(builder, red, green, blue):
    builder.Prep(4, 12)
    builder.PrependFloat32(blue)
    builder.PrependFloat32(green)
    builder.PrependFloat32(red)
    return builder.Offset()

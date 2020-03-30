# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Color(object):
    __slots__ = ['_tab']

    # Color
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Color
    def R(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    # Color
    def G(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(1))
    # Color
    def B(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(2))
    # Color
    def A(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(3))

def CreateColor(builder, r, g, b, a):
    builder.Prep(1, 4)
    builder.PrependUint8(a)
    builder.PrependUint8(b)
    builder.PrependUint8(g)
    builder.PrependUint8(r)
    return builder.Offset()

# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDrawScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Color(object):
    __slots__ = ['_tab']

    @classmethod
    def SizeOf(cls):
        return 4

    # Color
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Color
    def Red(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    # Color
    def Green(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(1))
    # Color
    def Blue(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(2))
    # Color
    def Alpha(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(3))

def CreateColor(builder, red, green, blue, alpha):
    builder.Prep(1, 4)
    builder.PrependUint8(alpha)
    builder.PrependUint8(blue)
    builder.PrependUint8(green)
    builder.PrependUint8(red)
    return builder.Offset()

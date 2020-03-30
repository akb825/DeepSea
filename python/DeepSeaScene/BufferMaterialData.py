# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class BufferMaterialData(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsBufferMaterialData(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = BufferMaterialData()
        x.Init(buf, n + offset)
        return x

    # BufferMaterialData
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # BufferMaterialData
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # BufferMaterialData
    def Offset(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # BufferMaterialData
    def Size(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

def BufferMaterialDataStart(builder): builder.StartObject(3)
def BufferMaterialDataAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def BufferMaterialDataAddOffset(builder, offset): builder.PrependUint32Slot(1, offset, 0)
def BufferMaterialDataAddSize(builder, size): builder.PrependUint32Slot(2, size, 0)
def BufferMaterialDataEnd(builder): return builder.EndObject()

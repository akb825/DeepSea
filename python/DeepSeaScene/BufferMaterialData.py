# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class BufferMaterialData(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = BufferMaterialData()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsBufferMaterialData(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
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

def BufferMaterialDataStart(builder):
    builder.StartObject(3)

def Start(builder):
    BufferMaterialDataStart(builder)

def BufferMaterialDataAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    BufferMaterialDataAddName(builder, name)

def BufferMaterialDataAddOffset(builder, offset):
    builder.PrependUint32Slot(1, offset, 0)

def AddOffset(builder, offset):
    BufferMaterialDataAddOffset(builder, offset)

def BufferMaterialDataAddSize(builder, size):
    builder.PrependUint32Slot(2, size, 0)

def AddSize(builder, size):
    BufferMaterialDataAddSize(builder, size)

def BufferMaterialDataEnd(builder):
    return builder.EndObject()

def End(builder):
    return BufferMaterialDataEnd(builder)

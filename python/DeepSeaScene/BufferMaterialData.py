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

def Start(builder): builder.StartObject(3)
def BufferMaterialDataStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def BufferMaterialDataAddName(builder, name):
    """This method is deprecated. Please switch to AddName."""
    return AddName(builder, name)
def AddOffset(builder, offset): builder.PrependUint32Slot(1, offset, 0)
def BufferMaterialDataAddOffset(builder, offset):
    """This method is deprecated. Please switch to AddOffset."""
    return AddOffset(builder, offset)
def AddSize(builder, size): builder.PrependUint32Slot(2, size, 0)
def BufferMaterialDataAddSize(builder, size):
    """This method is deprecated. Please switch to AddSize."""
    return AddSize(builder, size)
def End(builder): return builder.EndObject()
def BufferMaterialDataEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)
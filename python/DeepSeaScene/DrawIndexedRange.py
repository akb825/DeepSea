# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class DrawIndexedRange(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = DrawIndexedRange()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsDrawIndexedRange(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # DrawIndexedRange
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # DrawIndexedRange
    def IndexCount(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # DrawIndexedRange
    def InstanceCount(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # DrawIndexedRange
    def FirstIndex(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # DrawIndexedRange
    def VertexOffset(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # DrawIndexedRange
    def FirstInstance(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

def DrawIndexedRangeStart(builder):
    builder.StartObject(5)

def Start(builder):
    DrawIndexedRangeStart(builder)

def DrawIndexedRangeAddIndexCount(builder, indexCount):
    builder.PrependUint32Slot(0, indexCount, 0)

def AddIndexCount(builder, indexCount):
    DrawIndexedRangeAddIndexCount(builder, indexCount)

def DrawIndexedRangeAddInstanceCount(builder, instanceCount):
    builder.PrependUint32Slot(1, instanceCount, 0)

def AddInstanceCount(builder, instanceCount):
    DrawIndexedRangeAddInstanceCount(builder, instanceCount)

def DrawIndexedRangeAddFirstIndex(builder, firstIndex):
    builder.PrependUint32Slot(2, firstIndex, 0)

def AddFirstIndex(builder, firstIndex):
    DrawIndexedRangeAddFirstIndex(builder, firstIndex)

def DrawIndexedRangeAddVertexOffset(builder, vertexOffset):
    builder.PrependUint32Slot(3, vertexOffset, 0)

def AddVertexOffset(builder, vertexOffset):
    DrawIndexedRangeAddVertexOffset(builder, vertexOffset)

def DrawIndexedRangeAddFirstInstance(builder, firstInstance):
    builder.PrependUint32Slot(4, firstInstance, 0)

def AddFirstInstance(builder, firstInstance):
    DrawIndexedRangeAddFirstInstance(builder, firstInstance)

def DrawIndexedRangeEnd(builder):
    return builder.EndObject()

def End(builder):
    return DrawIndexedRangeEnd(builder)

# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ObjectData(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ObjectData()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsObjectData(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ObjectData
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ObjectData
    def Type(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # ObjectData
    def Data(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    # ObjectData
    def DataAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Uint8Flags, o)
        return 0

    # ObjectData
    def DataLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # ObjectData
    def DataIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def Start(builder): builder.StartObject(2)
def ObjectDataStart(builder):
    """This method is deprecated. Please switch to Start."""
    return Start(builder)
def AddType(builder, type): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(type), 0)
def ObjectDataAddType(builder, type):
    """This method is deprecated. Please switch to AddType."""
    return AddType(builder, type)
def AddData(builder, data): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(data), 0)
def ObjectDataAddData(builder, data):
    """This method is deprecated. Please switch to AddData."""
    return AddData(builder, data)
def StartDataVector(builder, numElems): return builder.StartVector(1, numElems, 1)
def ObjectDataStartDataVector(builder, numElems):
    """This method is deprecated. Please switch to Start."""
    return StartDataVector(builder, numElems)
def End(builder): return builder.EndObject()
def ObjectDataEnd(builder):
    """This method is deprecated. Please switch to End."""
    return End(builder)
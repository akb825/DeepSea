# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class VariableData(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = VariableData()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsVariableData(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # VariableData
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # VariableData
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VariableData
    def Type(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # VariableData
    def First(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # VariableData
    def Count(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # VariableData
    def Data(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 1))
        return 0

    # VariableData
    def DataAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Uint8Flags, o)
        return 0

    # VariableData
    def DataLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VariableData
    def DataIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        return o == 0

def VariableDataStart(builder):
    builder.StartObject(5)

def Start(builder):
    VariableDataStart(builder)

def VariableDataAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    VariableDataAddName(builder, name)

def VariableDataAddType(builder, type):
    builder.PrependUint8Slot(1, type, 0)

def AddType(builder, type):
    VariableDataAddType(builder, type)

def VariableDataAddFirst(builder, first):
    builder.PrependUint32Slot(2, first, 0)

def AddFirst(builder, first):
    VariableDataAddFirst(builder, first)

def VariableDataAddCount(builder, count):
    builder.PrependUint32Slot(3, count, 0)

def AddCount(builder, count):
    VariableDataAddCount(builder, count)

def VariableDataAddData(builder, data):
    builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(data), 0)

def AddData(builder, data):
    VariableDataAddData(builder, data)

def VariableDataStartDataVector(builder, numElems):
    return builder.StartVector(1, numElems, 1)

def StartDataVector(builder, numElems):
    return VariableDataStartDataVector(builder, numElems)

def VariableDataEnd(builder):
    return builder.EndObject()

def End(builder):
    return VariableDataEnd(builder)

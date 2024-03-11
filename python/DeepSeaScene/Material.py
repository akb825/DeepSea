# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Material(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Material()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsMaterial(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Material
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Material
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Material
    def Description(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Material
    def Data(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.VariableData import VariableData
            obj = VariableData()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # Material
    def DataLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Material
    def DataIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        return o == 0

def MaterialStart(builder):
    builder.StartObject(3)

def Start(builder):
    MaterialStart(builder)

def MaterialAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    MaterialAddName(builder, name)

def MaterialAddDescription(builder, description):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(description), 0)

def AddDescription(builder, description):
    MaterialAddDescription(builder, description)

def MaterialAddData(builder, data):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(data), 0)

def AddData(builder, data):
    MaterialAddData(builder, data)

def MaterialStartDataVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartDataVector(builder, numElems):
    return MaterialStartDataVector(builder, numElems)

def MaterialEnd(builder):
    return builder.EndObject()

def End(builder):
    return MaterialEnd(builder)

# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class MaterialDesc(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = MaterialDesc()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsMaterialDesc(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # MaterialDesc
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # MaterialDesc
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # MaterialDesc
    def Elements(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.MaterialElement import MaterialElement
            obj = MaterialElement()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # MaterialDesc
    def ElementsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # MaterialDesc
    def ElementsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def MaterialDescStart(builder):
    builder.StartObject(2)

def Start(builder):
    MaterialDescStart(builder)

def MaterialDescAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    MaterialDescAddName(builder, name)

def MaterialDescAddElements(builder, elements):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(elements), 0)

def AddElements(builder, elements):
    MaterialDescAddElements(builder, elements)

def MaterialDescStartElementsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartElementsVector(builder, numElems):
    return MaterialDescStartElementsVector(builder, numElems)

def MaterialDescEnd(builder):
    return builder.EndObject()

def End(builder):
    return MaterialDescEnd(builder)

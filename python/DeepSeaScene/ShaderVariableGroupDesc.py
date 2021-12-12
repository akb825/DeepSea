# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ShaderVariableGroupDesc(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ShaderVariableGroupDesc()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsShaderVariableGroupDesc(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ShaderVariableGroupDesc
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ShaderVariableGroupDesc
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # ShaderVariableGroupDesc
    def Elements(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.VariableElement import VariableElement
            obj = VariableElement()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ShaderVariableGroupDesc
    def ElementsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # ShaderVariableGroupDesc
    def ElementsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def ShaderVariableGroupDescStart(builder): builder.StartObject(2)
def Start(builder):
    return ShaderVariableGroupDescStart(builder)
def ShaderVariableGroupDescAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def AddName(builder, name):
    return ShaderVariableGroupDescAddName(builder, name)
def ShaderVariableGroupDescAddElements(builder, elements): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(elements), 0)
def AddElements(builder, elements):
    return ShaderVariableGroupDescAddElements(builder, elements)
def ShaderVariableGroupDescStartElementsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def StartElementsVector(builder, numElems):
    return ShaderVariableGroupDescStartElementsVector(builder, numElems)
def ShaderVariableGroupDescEnd(builder): return builder.EndObject()
def End(builder):
    return ShaderVariableGroupDescEnd(builder)
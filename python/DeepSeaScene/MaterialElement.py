# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class MaterialElement(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsMaterialElement(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = MaterialElement()
        x.Init(buf, n + offset)
        return x

    # MaterialElement
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # MaterialElement
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # MaterialElement
    def Type(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # MaterialElement
    def Count(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # MaterialElement
    def Binding(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # MaterialElement
    def ShaderVariableGroupDesc(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def MaterialElementStart(builder): builder.StartObject(5)
def MaterialElementAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def MaterialElementAddType(builder, type): builder.PrependUint8Slot(1, type, 0)
def MaterialElementAddCount(builder, count): builder.PrependUint32Slot(2, count, 0)
def MaterialElementAddBinding(builder, binding): builder.PrependUint8Slot(3, binding, 0)
def MaterialElementAddShaderVariableGroupDesc(builder, shaderVariableGroupDesc): builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(shaderVariableGroupDesc), 0)
def MaterialElementEnd(builder): return builder.EndObject()

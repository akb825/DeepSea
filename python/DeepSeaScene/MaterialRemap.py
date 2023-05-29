# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class MaterialRemap(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = MaterialRemap()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsMaterialRemap(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # MaterialRemap
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # MaterialRemap
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # MaterialRemap
    def ModelList(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # MaterialRemap
    def Shader(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # MaterialRemap
    def Material(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def MaterialRemapStart(builder):
    builder.StartObject(4)

def Start(builder):
    MaterialRemapStart(builder)

def MaterialRemapAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    MaterialRemapAddName(builder, name)

def MaterialRemapAddModelList(builder, modelList):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(modelList), 0)

def AddModelList(builder, modelList):
    MaterialRemapAddModelList(builder, modelList)

def MaterialRemapAddShader(builder, shader):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(shader), 0)

def AddShader(builder, shader):
    MaterialRemapAddShader(builder, shader)

def MaterialRemapAddMaterial(builder, material):
    builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(material), 0)

def AddMaterial(builder, material):
    MaterialRemapAddMaterial(builder, material)

def MaterialRemapEnd(builder):
    return builder.EndObject()

def End(builder):
    return MaterialRemapEnd(builder)

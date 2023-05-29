# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class NamedMaterialData(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = NamedMaterialData()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsNamedMaterialData(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # NamedMaterialData
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # NamedMaterialData
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def NamedMaterialDataStart(builder):
    builder.StartObject(1)

def Start(builder):
    NamedMaterialDataStart(builder)

def NamedMaterialDataAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    NamedMaterialDataAddName(builder, name)

def NamedMaterialDataEnd(builder):
    return builder.EndObject()

def End(builder):
    return NamedMaterialDataEnd(builder)

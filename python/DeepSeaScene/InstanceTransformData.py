# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class InstanceTransformData(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = InstanceTransformData()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsInstanceTransformData(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # InstanceTransformData
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # InstanceTransformData
    def VariableGroupDesc(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def InstanceTransformDataStart(builder):
    builder.StartObject(1)

def Start(builder):
    InstanceTransformDataStart(builder)

def InstanceTransformDataAddVariableGroupDesc(builder, variableGroupDesc):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(variableGroupDesc), 0)

def AddVariableGroupDesc(builder, variableGroupDesc):
    InstanceTransformDataAddVariableGroupDesc(builder, variableGroupDesc)

def InstanceTransformDataEnd(builder):
    return builder.EndObject()

def End(builder):
    return InstanceTransformDataEnd(builder)

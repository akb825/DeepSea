# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class InstanceTransformData(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsInstanceTransformData(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = InstanceTransformData()
        x.Init(buf, n + offset)
        return x

    # InstanceTransformData
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # InstanceTransformData
    def VariableGroupDescName(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def InstanceTransformDataStart(builder): builder.StartObject(1)
def InstanceTransformDataAddVariableGroupDescName(builder, variableGroupDescName): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(variableGroupDescName), 0)
def InstanceTransformDataEnd(builder): return builder.EndObject()
# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ViewTransformData(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsViewTransformData(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ViewTransformData()
        x.Init(buf, n + offset)
        return x

    # ViewTransformData
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ViewTransformData
    def VariableGroupDescName(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def ViewTransformDataStart(builder): builder.StartObject(1)
def ViewTransformDataAddVariableGroupDescName(builder, variableGroupDescName): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(variableGroupDescName), 0)
def ViewTransformDataEnd(builder): return builder.EndObject()

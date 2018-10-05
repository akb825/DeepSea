# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers

class ColorMaterial(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsColorMaterial(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ColorMaterial()
        x.Init(buf, n + offset)
        return x

    # ColorMaterial
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ColorMaterial
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # ColorMaterial
    def Color(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = o + self._tab.Pos
            from .Color import Color
            obj = Color()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def ColorMaterialStart(builder): builder.StartObject(2)
def ColorMaterialAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def ColorMaterialAddColor(builder, color): builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(color), 0)
def ColorMaterialEnd(builder): return builder.EndObject()

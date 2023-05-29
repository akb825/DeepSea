# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ColorMaterial(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ColorMaterial()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsColorMaterial(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
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
            from DeepSeaVectorDraw.Color import Color
            obj = Color()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def ColorMaterialStart(builder):
    builder.StartObject(2)

def Start(builder):
    ColorMaterialStart(builder)

def ColorMaterialAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    ColorMaterialAddName(builder, name)

def ColorMaterialAddColor(builder, color):
    builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(color), 0)

def AddColor(builder, color):
    ColorMaterialAddColor(builder, color)

def ColorMaterialEnd(builder):
    return builder.EndObject()

def End(builder):
    return ColorMaterialEnd(builder)

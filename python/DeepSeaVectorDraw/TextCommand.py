# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class TextCommand(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = TextCommand()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsTextCommand(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # TextCommand
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # TextCommand
    def Text(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # TextCommand
    def Font(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # TextCommand
    def Alignment(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # TextCommand
    def MaxLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # TextCommand
    def LineHeight(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # TextCommand
    def Transform(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaVectorDraw.Matrix33f import Matrix33f
            obj = Matrix33f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # TextCommand
    def RangeCount(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

def TextCommandStart(builder):
    builder.StartObject(7)

def Start(builder):
    TextCommandStart(builder)

def TextCommandAddText(builder, text):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(text), 0)

def AddText(builder, text):
    TextCommandAddText(builder, text)

def TextCommandAddFont(builder, font):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(font), 0)

def AddFont(builder, font):
    TextCommandAddFont(builder, font)

def TextCommandAddAlignment(builder, alignment):
    builder.PrependUint8Slot(2, alignment, 0)

def AddAlignment(builder, alignment):
    TextCommandAddAlignment(builder, alignment)

def TextCommandAddMaxLength(builder, maxLength):
    builder.PrependFloat32Slot(3, maxLength, 0.0)

def AddMaxLength(builder, maxLength):
    TextCommandAddMaxLength(builder, maxLength)

def TextCommandAddLineHeight(builder, lineHeight):
    builder.PrependFloat32Slot(4, lineHeight, 0.0)

def AddLineHeight(builder, lineHeight):
    TextCommandAddLineHeight(builder, lineHeight)

def TextCommandAddTransform(builder, transform):
    builder.PrependStructSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(transform), 0)

def AddTransform(builder, transform):
    TextCommandAddTransform(builder, transform)

def TextCommandAddRangeCount(builder, rangeCount):
    builder.PrependUint32Slot(6, rangeCount, 0)

def AddRangeCount(builder, rangeCount):
    TextCommandAddRangeCount(builder, rangeCount)

def TextCommandEnd(builder):
    return builder.EndObject()

def End(builder):
    return TextCommandEnd(builder)

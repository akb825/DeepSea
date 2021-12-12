# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class StartPathCommand(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = StartPathCommand()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsStartPathCommand(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # StartPathCommand
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # StartPathCommand
    def Transform(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaVectorDraw.Matrix33f import Matrix33f
            obj = Matrix33f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # StartPathCommand
    def Simple(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

def StartPathCommandStart(builder): builder.StartObject(2)
def Start(builder):
    return StartPathCommandStart(builder)
def StartPathCommandAddTransform(builder, transform): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(transform), 0)
def AddTransform(builder, transform):
    return StartPathCommandAddTransform(builder, transform)
def StartPathCommandAddSimple(builder, simple): builder.PrependBoolSlot(1, simple, 0)
def AddSimple(builder, simple):
    return StartPathCommandAddSimple(builder, simple)
def StartPathCommandEnd(builder): return builder.EndObject()
def End(builder):
    return StartPathCommandEnd(builder)
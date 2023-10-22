# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class DrawGeometry(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = DrawGeometry()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsDrawGeometry(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # DrawGeometry
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # DrawGeometry
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # DrawGeometry
    def VertexBuffers(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.VertexBuffer import VertexBuffer
            obj = VertexBuffer()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # DrawGeometry
    def VertexBuffersLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # DrawGeometry
    def VertexBuffersIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

    # DrawGeometry
    def IndexBuffer(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from DeepSeaScene.IndexBuffer import IndexBuffer
            obj = IndexBuffer()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def DrawGeometryStart(builder):
    builder.StartObject(3)

def Start(builder):
    DrawGeometryStart(builder)

def DrawGeometryAddName(builder, name):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)

def AddName(builder, name):
    DrawGeometryAddName(builder, name)

def DrawGeometryAddVertexBuffers(builder, vertexBuffers):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(vertexBuffers), 0)

def AddVertexBuffers(builder, vertexBuffers):
    DrawGeometryAddVertexBuffers(builder, vertexBuffers)

def DrawGeometryStartVertexBuffersVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartVertexBuffersVector(builder, numElems: int) -> int:
    return DrawGeometryStartVertexBuffersVector(builder, numElems)

def DrawGeometryAddIndexBuffer(builder, indexBuffer):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(indexBuffer), 0)

def AddIndexBuffer(builder, indexBuffer):
    DrawGeometryAddIndexBuffer(builder, indexBuffer)

def DrawGeometryEnd(builder):
    return builder.EndObject()

def End(builder):
    return DrawGeometryEnd(builder)

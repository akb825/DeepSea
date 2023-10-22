# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class NodeChildren(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = NodeChildren()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsNodeChildren(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # NodeChildren
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # NodeChildren
    def Node(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # NodeChildren
    def Children(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.ObjectData import ObjectData
            obj = ObjectData()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # NodeChildren
    def ChildrenLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # NodeChildren
    def ChildrenIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def NodeChildrenStart(builder):
    builder.StartObject(2)

def Start(builder):
    NodeChildrenStart(builder)

def NodeChildrenAddNode(builder, node):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(node), 0)

def AddNode(builder, node):
    NodeChildrenAddNode(builder, node)

def NodeChildrenAddChildren(builder, children):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(children), 0)

def AddChildren(builder, children):
    NodeChildrenAddChildren(builder, children)

def NodeChildrenStartChildrenVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartChildrenVector(builder, numElems: int) -> int:
    return NodeChildrenStartChildrenVector(builder, numElems)

def NodeChildrenEnd(builder):
    return builder.EndObject()

def End(builder):
    return NodeChildrenEnd(builder)

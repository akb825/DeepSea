# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaAnimation

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class AnimationTree(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = AnimationTree()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsAnimationTree(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # AnimationTree
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # AnimationTree
    def RootNodes(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaAnimation.AnimationNode import AnimationNode
            obj = AnimationNode()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # AnimationTree
    def RootNodesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # AnimationTree
    def RootNodesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # AnimationTree
    def JointNodes(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaAnimation.AnimationJointNode import AnimationJointNode
            obj = AnimationJointNode()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # AnimationTree
    def JointNodesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # AnimationTree
    def JointNodesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def AnimationTreeStart(builder): builder.StartObject(2)
def Start(builder):
    return AnimationTreeStart(builder)
def AnimationTreeAddRootNodes(builder, rootNodes): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(rootNodes), 0)
def AddRootNodes(builder, rootNodes):
    return AnimationTreeAddRootNodes(builder, rootNodes)
def AnimationTreeStartRootNodesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def StartRootNodesVector(builder, numElems):
    return AnimationTreeStartRootNodesVector(builder, numElems)
def AnimationTreeAddJointNodes(builder, jointNodes): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(jointNodes), 0)
def AddJointNodes(builder, jointNodes):
    return AnimationTreeAddJointNodes(builder, jointNodes)
def AnimationTreeStartJointNodesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def StartJointNodesVector(builder, numElems):
    return AnimationTreeStartJointNodesVector(builder, numElems)
def AnimationTreeEnd(builder): return builder.EndObject()
def End(builder):
    return AnimationTreeEnd(builder)
# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScenePhysics

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ConstraintResourceReference(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ConstraintResourceReference()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsConstraintResourceReference(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ConstraintResourceReference
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ConstraintResourceReference
    def Constraint(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def ConstraintResourceReferenceStart(builder):
    builder.StartObject(1)

def Start(builder):
    ConstraintResourceReferenceStart(builder)

def ConstraintResourceReferenceAddConstraint(builder, constraint):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(constraint), 0)

def AddConstraint(builder, constraint):
    ConstraintResourceReferenceAddConstraint(builder, constraint)

def ConstraintResourceReferenceEnd(builder):
    return builder.EndObject()

def End(builder):
    return ConstraintResourceReferenceEnd(builder)
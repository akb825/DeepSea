# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaPhysics

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class DistanceConstraint(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = DistanceConstraint()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsDistanceConstraint(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # DistanceConstraint
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # DistanceConstraint
    def FirstActor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # DistanceConstraint
    def FirstPosition(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # DistanceConstraint
    def SecondActor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # DistanceConstraint
    def SecondPosition(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # DistanceConstraint
    def MinDistance(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # DistanceConstraint
    def MaxDistance(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # DistanceConstraint
    def LimitStiffness(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # DistanceConstraint
    def LimitDamping(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def DistanceConstraintStart(builder):
    builder.StartObject(8)

def Start(builder):
    DistanceConstraintStart(builder)

def DistanceConstraintAddFirstActor(builder, firstActor):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(firstActor), 0)

def AddFirstActor(builder, firstActor):
    DistanceConstraintAddFirstActor(builder, firstActor)

def DistanceConstraintAddFirstPosition(builder, firstPosition):
    builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(firstPosition), 0)

def AddFirstPosition(builder, firstPosition):
    DistanceConstraintAddFirstPosition(builder, firstPosition)

def DistanceConstraintAddSecondActor(builder, secondActor):
    builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(secondActor), 0)

def AddSecondActor(builder, secondActor):
    DistanceConstraintAddSecondActor(builder, secondActor)

def DistanceConstraintAddSecondPosition(builder, secondPosition):
    builder.PrependStructSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(secondPosition), 0)

def AddSecondPosition(builder, secondPosition):
    DistanceConstraintAddSecondPosition(builder, secondPosition)

def DistanceConstraintAddMinDistance(builder, minDistance):
    builder.PrependFloat32Slot(4, minDistance, 0.0)

def AddMinDistance(builder, minDistance):
    DistanceConstraintAddMinDistance(builder, minDistance)

def DistanceConstraintAddMaxDistance(builder, maxDistance):
    builder.PrependFloat32Slot(5, maxDistance, 0.0)

def AddMaxDistance(builder, maxDistance):
    DistanceConstraintAddMaxDistance(builder, maxDistance)

def DistanceConstraintAddLimitStiffness(builder, limitStiffness):
    builder.PrependFloat32Slot(6, limitStiffness, 0.0)

def AddLimitStiffness(builder, limitStiffness):
    DistanceConstraintAddLimitStiffness(builder, limitStiffness)

def DistanceConstraintAddLimitDamping(builder, limitDamping):
    builder.PrependFloat32Slot(7, limitDamping, 0.0)

def AddLimitDamping(builder, limitDamping):
    DistanceConstraintAddLimitDamping(builder, limitDamping)

def DistanceConstraintEnd(builder):
    return builder.EndObject()

def End(builder):
    return DistanceConstraintEnd(builder)
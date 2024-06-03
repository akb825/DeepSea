# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaPhysics

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class GenericConstraint(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = GenericConstraint()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsGenericConstraint(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # GenericConstraint
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # GenericConstraint
    def FirstActor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # GenericConstraint
    def FirstPosition(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # GenericConstraint
    def FirstRotation(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Quaternion4f import Quaternion4f
            obj = Quaternion4f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # GenericConstraint
    def SecondActor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # GenericConstraint
    def SecondPosition(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # GenericConstraint
    def SecondRotation(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Quaternion4f import Quaternion4f
            obj = Quaternion4f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # GenericConstraint
    def Limits(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 20
            from DeepSeaPhysics.GenericConstraintLimit import GenericConstraintLimit
            obj = GenericConstraintLimit()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # GenericConstraint
    def LimitsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # GenericConstraint
    def LimitsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        return o == 0

    # GenericConstraint
    def Motors(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 12
            from DeepSeaPhysics.GenericConstraintMotor import GenericConstraintMotor
            obj = GenericConstraintMotor()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # GenericConstraint
    def MotorsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # GenericConstraint
    def MotorsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        return o == 0

    # GenericConstraint
    def CombineSwingTwistMotors(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

def GenericConstraintStart(builder):
    builder.StartObject(9)

def Start(builder):
    GenericConstraintStart(builder)

def GenericConstraintAddFirstActor(builder, firstActor):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(firstActor), 0)

def AddFirstActor(builder, firstActor):
    GenericConstraintAddFirstActor(builder, firstActor)

def GenericConstraintAddFirstPosition(builder, firstPosition):
    builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(firstPosition), 0)

def AddFirstPosition(builder, firstPosition):
    GenericConstraintAddFirstPosition(builder, firstPosition)

def GenericConstraintAddFirstRotation(builder, firstRotation):
    builder.PrependStructSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(firstRotation), 0)

def AddFirstRotation(builder, firstRotation):
    GenericConstraintAddFirstRotation(builder, firstRotation)

def GenericConstraintAddSecondActor(builder, secondActor):
    builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(secondActor), 0)

def AddSecondActor(builder, secondActor):
    GenericConstraintAddSecondActor(builder, secondActor)

def GenericConstraintAddSecondPosition(builder, secondPosition):
    builder.PrependStructSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(secondPosition), 0)

def AddSecondPosition(builder, secondPosition):
    GenericConstraintAddSecondPosition(builder, secondPosition)

def GenericConstraintAddSecondRotation(builder, secondRotation):
    builder.PrependStructSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(secondRotation), 0)

def AddSecondRotation(builder, secondRotation):
    GenericConstraintAddSecondRotation(builder, secondRotation)

def GenericConstraintAddLimits(builder, limits):
    builder.PrependUOffsetTRelativeSlot(6, flatbuffers.number_types.UOffsetTFlags.py_type(limits), 0)

def AddLimits(builder, limits):
    GenericConstraintAddLimits(builder, limits)

def GenericConstraintStartLimitsVector(builder, numElems):
    return builder.StartVector(20, numElems, 4)

def StartLimitsVector(builder, numElems):
    return GenericConstraintStartLimitsVector(builder, numElems)

def GenericConstraintAddMotors(builder, motors):
    builder.PrependUOffsetTRelativeSlot(7, flatbuffers.number_types.UOffsetTFlags.py_type(motors), 0)

def AddMotors(builder, motors):
    GenericConstraintAddMotors(builder, motors)

def GenericConstraintStartMotorsVector(builder, numElems):
    return builder.StartVector(12, numElems, 4)

def StartMotorsVector(builder, numElems):
    return GenericConstraintStartMotorsVector(builder, numElems)

def GenericConstraintAddCombineSwingTwistMotors(builder, combineSwingTwistMotors):
    builder.PrependBoolSlot(8, combineSwingTwistMotors, 0)

def AddCombineSwingTwistMotors(builder, combineSwingTwistMotors):
    GenericConstraintAddCombineSwingTwistMotors(builder, combineSwingTwistMotors)

def GenericConstraintEnd(builder):
    return builder.EndObject()

def End(builder):
    return GenericConstraintEnd(builder)

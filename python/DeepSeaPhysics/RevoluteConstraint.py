# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaPhysics

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class RevoluteConstraint(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = RevoluteConstraint()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsRevoluteConstraint(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # RevoluteConstraint
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # RevoluteConstraint
    def FirstActor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # RevoluteConstraint
    def FirstPosition(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RevoluteConstraint
    def FirstOrientation(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Quaternion4f import Quaternion4f
            obj = Quaternion4f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RevoluteConstraint
    def SecondActor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # RevoluteConstraint
    def SecondPosition(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RevoluteConstraint
    def SecondOrientation(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Quaternion4f import Quaternion4f
            obj = Quaternion4f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RevoluteConstraint
    def LimitEnabled(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

    # RevoluteConstraint
    def MinAngle(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # RevoluteConstraint
    def MaxAngle(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # RevoluteConstraint
    def LimitStiffness(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # RevoluteConstraint
    def LimitDamping(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # RevoluteConstraint
    def MotorType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # RevoluteConstraint
    def MotorTarget(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(28))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # RevoluteConstraint
    def MaxMotorTorque(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(30))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def RevoluteConstraintStart(builder):
    builder.StartObject(14)

def Start(builder):
    RevoluteConstraintStart(builder)

def RevoluteConstraintAddFirstActor(builder, firstActor):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(firstActor), 0)

def AddFirstActor(builder, firstActor):
    RevoluteConstraintAddFirstActor(builder, firstActor)

def RevoluteConstraintAddFirstPosition(builder, firstPosition):
    builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(firstPosition), 0)

def AddFirstPosition(builder, firstPosition):
    RevoluteConstraintAddFirstPosition(builder, firstPosition)

def RevoluteConstraintAddFirstOrientation(builder, firstOrientation):
    builder.PrependStructSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(firstOrientation), 0)

def AddFirstOrientation(builder, firstOrientation):
    RevoluteConstraintAddFirstOrientation(builder, firstOrientation)

def RevoluteConstraintAddSecondActor(builder, secondActor):
    builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(secondActor), 0)

def AddSecondActor(builder, secondActor):
    RevoluteConstraintAddSecondActor(builder, secondActor)

def RevoluteConstraintAddSecondPosition(builder, secondPosition):
    builder.PrependStructSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(secondPosition), 0)

def AddSecondPosition(builder, secondPosition):
    RevoluteConstraintAddSecondPosition(builder, secondPosition)

def RevoluteConstraintAddSecondOrientation(builder, secondOrientation):
    builder.PrependStructSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(secondOrientation), 0)

def AddSecondOrientation(builder, secondOrientation):
    RevoluteConstraintAddSecondOrientation(builder, secondOrientation)

def RevoluteConstraintAddLimitEnabled(builder, limitEnabled):
    builder.PrependBoolSlot(6, limitEnabled, 0)

def AddLimitEnabled(builder, limitEnabled):
    RevoluteConstraintAddLimitEnabled(builder, limitEnabled)

def RevoluteConstraintAddMinAngle(builder, minAngle):
    builder.PrependFloat32Slot(7, minAngle, 0.0)

def AddMinAngle(builder, minAngle):
    RevoluteConstraintAddMinAngle(builder, minAngle)

def RevoluteConstraintAddMaxAngle(builder, maxAngle):
    builder.PrependFloat32Slot(8, maxAngle, 0.0)

def AddMaxAngle(builder, maxAngle):
    RevoluteConstraintAddMaxAngle(builder, maxAngle)

def RevoluteConstraintAddLimitStiffness(builder, limitStiffness):
    builder.PrependFloat32Slot(9, limitStiffness, 0.0)

def AddLimitStiffness(builder, limitStiffness):
    RevoluteConstraintAddLimitStiffness(builder, limitStiffness)

def RevoluteConstraintAddLimitDamping(builder, limitDamping):
    builder.PrependFloat32Slot(10, limitDamping, 0.0)

def AddLimitDamping(builder, limitDamping):
    RevoluteConstraintAddLimitDamping(builder, limitDamping)

def RevoluteConstraintAddMotorType(builder, motorType):
    builder.PrependUint8Slot(11, motorType, 0)

def AddMotorType(builder, motorType):
    RevoluteConstraintAddMotorType(builder, motorType)

def RevoluteConstraintAddMotorTarget(builder, motorTarget):
    builder.PrependFloat32Slot(12, motorTarget, 0.0)

def AddMotorTarget(builder, motorTarget):
    RevoluteConstraintAddMotorTarget(builder, motorTarget)

def RevoluteConstraintAddMaxMotorTorque(builder, maxMotorTorque):
    builder.PrependFloat32Slot(13, maxMotorTorque, 0.0)

def AddMaxMotorTorque(builder, maxMotorTorque):
    RevoluteConstraintAddMaxMotorTorque(builder, maxMotorTorque)

def RevoluteConstraintEnd(builder):
    return builder.EndObject()

def End(builder):
    return RevoluteConstraintEnd(builder)

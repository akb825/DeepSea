# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaPhysics

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class SliderConstraint(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SliderConstraint()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsSliderConstraint(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # SliderConstraint
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SliderConstraint
    def FirstActor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # SliderConstraint
    def FirstPosition(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SliderConstraint
    def FirstOrientation(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Quaternion4f import Quaternion4f
            obj = Quaternion4f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SliderConstraint
    def SecondActor(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # SliderConstraint
    def SecondPosition(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SliderConstraint
    def SecondOrientation(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Quaternion4f import Quaternion4f
            obj = Quaternion4f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # SliderConstraint
    def LimitEnabled(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

    # SliderConstraint
    def MinDistance(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SliderConstraint
    def MaxDistance(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SliderConstraint
    def LimitStiffness(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SliderConstraint
    def LimitDamping(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SliderConstraint
    def MotorType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # SliderConstraint
    def MotorTarget(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(28))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # SliderConstraint
    def MaxMotorForce(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(30))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def SliderConstraintStart(builder):
    builder.StartObject(14)

def Start(builder):
    SliderConstraintStart(builder)

def SliderConstraintAddFirstActor(builder, firstActor):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(firstActor), 0)

def AddFirstActor(builder, firstActor):
    SliderConstraintAddFirstActor(builder, firstActor)

def SliderConstraintAddFirstPosition(builder, firstPosition):
    builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(firstPosition), 0)

def AddFirstPosition(builder, firstPosition):
    SliderConstraintAddFirstPosition(builder, firstPosition)

def SliderConstraintAddFirstOrientation(builder, firstOrientation):
    builder.PrependStructSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(firstOrientation), 0)

def AddFirstOrientation(builder, firstOrientation):
    SliderConstraintAddFirstOrientation(builder, firstOrientation)

def SliderConstraintAddSecondActor(builder, secondActor):
    builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(secondActor), 0)

def AddSecondActor(builder, secondActor):
    SliderConstraintAddSecondActor(builder, secondActor)

def SliderConstraintAddSecondPosition(builder, secondPosition):
    builder.PrependStructSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(secondPosition), 0)

def AddSecondPosition(builder, secondPosition):
    SliderConstraintAddSecondPosition(builder, secondPosition)

def SliderConstraintAddSecondOrientation(builder, secondOrientation):
    builder.PrependStructSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(secondOrientation), 0)

def AddSecondOrientation(builder, secondOrientation):
    SliderConstraintAddSecondOrientation(builder, secondOrientation)

def SliderConstraintAddLimitEnabled(builder, limitEnabled):
    builder.PrependBoolSlot(6, limitEnabled, 0)

def AddLimitEnabled(builder, limitEnabled):
    SliderConstraintAddLimitEnabled(builder, limitEnabled)

def SliderConstraintAddMinDistance(builder, minDistance):
    builder.PrependFloat32Slot(7, minDistance, 0.0)

def AddMinDistance(builder, minDistance):
    SliderConstraintAddMinDistance(builder, minDistance)

def SliderConstraintAddMaxDistance(builder, maxDistance):
    builder.PrependFloat32Slot(8, maxDistance, 0.0)

def AddMaxDistance(builder, maxDistance):
    SliderConstraintAddMaxDistance(builder, maxDistance)

def SliderConstraintAddLimitStiffness(builder, limitStiffness):
    builder.PrependFloat32Slot(9, limitStiffness, 0.0)

def AddLimitStiffness(builder, limitStiffness):
    SliderConstraintAddLimitStiffness(builder, limitStiffness)

def SliderConstraintAddLimitDamping(builder, limitDamping):
    builder.PrependFloat32Slot(10, limitDamping, 0.0)

def AddLimitDamping(builder, limitDamping):
    SliderConstraintAddLimitDamping(builder, limitDamping)

def SliderConstraintAddMotorType(builder, motorType):
    builder.PrependUint8Slot(11, motorType, 0)

def AddMotorType(builder, motorType):
    SliderConstraintAddMotorType(builder, motorType)

def SliderConstraintAddMotorTarget(builder, motorTarget):
    builder.PrependFloat32Slot(12, motorTarget, 0.0)

def AddMotorTarget(builder, motorTarget):
    SliderConstraintAddMotorTarget(builder, motorTarget)

def SliderConstraintAddMaxMotorForce(builder, maxMotorForce):
    builder.PrependFloat32Slot(13, maxMotorForce, 0.0)

def AddMaxMotorForce(builder, maxMotorForce):
    SliderConstraintAddMaxMotorForce(builder, maxMotorForce)

def SliderConstraintEnd(builder):
    return builder.EndObject()

def End(builder):
    return SliderConstraintEnd(builder)

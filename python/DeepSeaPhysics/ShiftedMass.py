# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaPhysics

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ShiftedMass(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ShiftedMass()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsShiftedMass(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ShiftedMass
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ShiftedMass
    def Mass(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return -1.0

    # ShiftedMass
    def RotationPointShift(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaPhysics.Vector3f import Vector3f
            obj = Vector3f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def ShiftedMassStart(builder):
    builder.StartObject(2)

def Start(builder):
    ShiftedMassStart(builder)

def ShiftedMassAddMass(builder, mass):
    builder.PrependFloat32Slot(0, mass, -1.0)

def AddMass(builder, mass):
    ShiftedMassAddMass(builder, mass)

def ShiftedMassAddRotationPointShift(builder, rotationPointShift):
    builder.PrependStructSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(rotationPointShift), 0)

def AddRotationPointShift(builder, rotationPointShift):
    ShiftedMassAddRotationPointShift(builder, rotationPointShift)

def ShiftedMassEnd(builder):
    return builder.EndObject()

def End(builder):
    return ShiftedMassEnd(builder)
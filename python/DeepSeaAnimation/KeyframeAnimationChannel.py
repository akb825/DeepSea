# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaAnimation

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class KeyframeAnimationChannel(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = KeyframeAnimationChannel()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsKeyframeAnimationChannel(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # KeyframeAnimationChannel
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # KeyframeAnimationChannel
    def Node(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # KeyframeAnimationChannel
    def Component(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # KeyframeAnimationChannel
    def Interpolation(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # KeyframeAnimationChannel
    def Values(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 16
            from DeepSeaAnimation.Vector4f import Vector4f
            obj = Vector4f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # KeyframeAnimationChannel
    def ValuesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # KeyframeAnimationChannel
    def ValuesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        return o == 0

def KeyframeAnimationChannelStart(builder):
    builder.StartObject(4)

def Start(builder):
    KeyframeAnimationChannelStart(builder)

def KeyframeAnimationChannelAddNode(builder, node):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(node), 0)

def AddNode(builder, node):
    KeyframeAnimationChannelAddNode(builder, node)

def KeyframeAnimationChannelAddComponent(builder, component):
    builder.PrependUint8Slot(1, component, 0)

def AddComponent(builder, component):
    KeyframeAnimationChannelAddComponent(builder, component)

def KeyframeAnimationChannelAddInterpolation(builder, interpolation):
    builder.PrependUint8Slot(2, interpolation, 0)

def AddInterpolation(builder, interpolation):
    KeyframeAnimationChannelAddInterpolation(builder, interpolation)

def KeyframeAnimationChannelAddValues(builder, values):
    builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(values), 0)

def AddValues(builder, values):
    KeyframeAnimationChannelAddValues(builder, values)

def KeyframeAnimationChannelStartValuesVector(builder, numElems):
    return builder.StartVector(16, numElems, 4)

def StartValuesVector(builder, numElems: int) -> int:
    return KeyframeAnimationChannelStartValuesVector(builder, numElems)

def KeyframeAnimationChannelEnd(builder):
    return builder.EndObject()

def End(builder):
    return KeyframeAnimationChannelEnd(builder)

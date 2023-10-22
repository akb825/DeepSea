# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaAnimation

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class AnimationKeyframes(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = AnimationKeyframes()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsAnimationKeyframes(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # AnimationKeyframes
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # AnimationKeyframes
    def KeyframeTimes(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Float32Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return 0

    # AnimationKeyframes
    def KeyframeTimesAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Float32Flags, o)
        return 0

    # AnimationKeyframes
    def KeyframeTimesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # AnimationKeyframes
    def KeyframeTimesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # AnimationKeyframes
    def Channels(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaAnimation.KeyframeAnimationChannel import KeyframeAnimationChannel
            obj = KeyframeAnimationChannel()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # AnimationKeyframes
    def ChannelsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # AnimationKeyframes
    def ChannelsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def AnimationKeyframesStart(builder):
    builder.StartObject(2)

def Start(builder):
    AnimationKeyframesStart(builder)

def AnimationKeyframesAddKeyframeTimes(builder, keyframeTimes):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(keyframeTimes), 0)

def AddKeyframeTimes(builder, keyframeTimes):
    AnimationKeyframesAddKeyframeTimes(builder, keyframeTimes)

def AnimationKeyframesStartKeyframeTimesVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartKeyframeTimesVector(builder, numElems: int) -> int:
    return AnimationKeyframesStartKeyframeTimesVector(builder, numElems)

def AnimationKeyframesAddChannels(builder, channels):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(channels), 0)

def AddChannels(builder, channels):
    AnimationKeyframesAddChannels(builder, channels)

def AnimationKeyframesStartChannelsVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartChannelsVector(builder, numElems: int) -> int:
    return AnimationKeyframesStartChannelsVector(builder, numElems)

def AnimationKeyframesEnd(builder):
    return builder.EndObject()

def End(builder):
    return AnimationKeyframesEnd(builder)

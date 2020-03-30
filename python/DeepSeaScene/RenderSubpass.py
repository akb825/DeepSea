# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class RenderSubpass(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsRenderSubpass(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = RenderSubpass()
        x.Init(buf, n + offset)
        return x

    # RenderSubpass
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # RenderSubpass
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # RenderSubpass
    def InputAttachments(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return 0

    # RenderSubpass
    def InputAttachmentsAsNumpy(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.GetVectorAsNumpy(flatbuffers.number_types.Uint32Flags, o)
        return 0

    # RenderSubpass
    def InputAttachmentsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # RenderSubpass
    def InputAttachmentsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

    # RenderSubpass
    def ColorAttachments(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 8
            from DeepSeaScene.AttachmentRef import AttachmentRef
            obj = AttachmentRef()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RenderSubpass
    def ColorAttachmentsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # RenderSubpass
    def ColorAttachmentsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        return o == 0

    # RenderSubpass
    def DepthStencilAttachment(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            x = o + self._tab.Pos
            from DeepSeaScene.AttachmentRef import AttachmentRef
            obj = AttachmentRef()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RenderSubpass
    def DrawLists(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.SceneItemList import SceneItemList
            obj = SceneItemList()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # RenderSubpass
    def DrawListsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # RenderSubpass
    def DrawListsIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        return o == 0

def RenderSubpassStart(builder): builder.StartObject(5)
def RenderSubpassAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def RenderSubpassAddInputAttachments(builder, inputAttachments): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(inputAttachments), 0)
def RenderSubpassStartInputAttachmentsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def RenderSubpassAddColorAttachments(builder, colorAttachments): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(colorAttachments), 0)
def RenderSubpassStartColorAttachmentsVector(builder, numElems): return builder.StartVector(8, numElems, 4)
def RenderSubpassAddDepthStencilAttachment(builder, depthStencilAttachment): builder.PrependStructSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(depthStencilAttachment), 0)
def RenderSubpassAddDrawLists(builder, drawLists): builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(drawLists), 0)
def RenderSubpassStartDrawListsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def RenderSubpassEnd(builder): return builder.EndObject()

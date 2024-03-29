# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class View(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = View()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsView(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # View
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # View
    def Surfaces(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.Surface import Surface
            obj = Surface()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # View
    def SurfacesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # View
    def SurfacesIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # View
    def Framebuffers(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.Framebuffer import Framebuffer
            obj = Framebuffer()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # View
    def FramebuffersLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # View
    def FramebuffersIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        return o == 0

def ViewStart(builder):
    builder.StartObject(2)

def Start(builder):
    ViewStart(builder)

def ViewAddSurfaces(builder, surfaces):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(surfaces), 0)

def AddSurfaces(builder, surfaces):
    ViewAddSurfaces(builder, surfaces)

def ViewStartSurfacesVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartSurfacesVector(builder, numElems):
    return ViewStartSurfacesVector(builder, numElems)

def ViewAddFramebuffers(builder, framebuffers):
    builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(framebuffers), 0)

def AddFramebuffers(builder, framebuffers):
    ViewAddFramebuffers(builder, framebuffers)

def ViewStartFramebuffersVector(builder, numElems):
    return builder.StartVector(4, numElems, 4)

def StartFramebuffersVector(builder, numElems):
    return ViewStartFramebuffersVector(builder, numElems)

def ViewEnd(builder):
    return builder.EndObject()

def End(builder):
    return ViewEnd(builder)

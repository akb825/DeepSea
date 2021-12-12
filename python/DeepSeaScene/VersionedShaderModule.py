# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class VersionedShaderModule(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = VersionedShaderModule()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsVersionedShaderModule(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # VersionedShaderModule
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # VersionedShaderModule
    def Version(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # VersionedShaderModule
    def DataType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # VersionedShaderModule
    def Data(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            from flatbuffers.table import Table
            obj = Table(bytearray(), 0)
            self._tab.Union(obj, o)
            return obj
        return None

def VersionedShaderModuleStart(builder): builder.StartObject(3)
def Start(builder):
    return VersionedShaderModuleStart(builder)
def VersionedShaderModuleAddVersion(builder, version): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(version), 0)
def AddVersion(builder, version):
    return VersionedShaderModuleAddVersion(builder, version)
def VersionedShaderModuleAddDataType(builder, dataType): builder.PrependUint8Slot(1, dataType, 0)
def AddDataType(builder, dataType):
    return VersionedShaderModuleAddDataType(builder, dataType)
def VersionedShaderModuleAddData(builder, data): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(data), 0)
def AddData(builder, data):
    return VersionedShaderModuleAddData(builder, data)
def VersionedShaderModuleEnd(builder): return builder.EndObject()
def End(builder):
    return VersionedShaderModuleEnd(builder)
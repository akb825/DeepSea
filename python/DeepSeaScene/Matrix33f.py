# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Matrix33f(object):
    __slots__ = ['_tab']

    # Matrix33f
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Matrix33f
    def Column0(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 0)
        return obj

    # Matrix33f
    def Column1(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 12)
        return obj

    # Matrix33f
    def Column2(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 24)
        return obj


def CreateMatrix33f(builder, column0_x, column0_y, column0_z, column1_x, column1_y, column1_z, column2_x, column2_y, column2_z):
    builder.Prep(4, 36)
    builder.Prep(4, 12)
    builder.PrependFloat32(column2_z)
    builder.PrependFloat32(column2_y)
    builder.PrependFloat32(column2_x)
    builder.Prep(4, 12)
    builder.PrependFloat32(column1_z)
    builder.PrependFloat32(column1_y)
    builder.PrependFloat32(column1_x)
    builder.Prep(4, 12)
    builder.PrependFloat32(column0_z)
    builder.PrependFloat32(column0_y)
    builder.PrependFloat32(column0_x)
    return builder.Offset()

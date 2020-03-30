# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Matrix44f(object):
    __slots__ = ['_tab']

    # Matrix44f
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Matrix44f
    def Column0(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 0)
        return obj

    # Matrix44f
    def Column1(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 16)
        return obj

    # Matrix44f
    def Column2(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 32)
        return obj

    # Matrix44f
    def Column3(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 48)
        return obj


def CreateMatrix44f(builder, column0_x, column0_y, column0_z, column0_w, column1_x, column1_y, column1_z, column1_w, column2_x, column2_y, column2_z, column2_w, column3_x, column3_y, column3_z, column3_w):
    builder.Prep(4, 64)
    builder.Prep(4, 16)
    builder.PrependFloat32(column3_w)
    builder.PrependFloat32(column3_z)
    builder.PrependFloat32(column3_y)
    builder.PrependFloat32(column3_x)
    builder.Prep(4, 16)
    builder.PrependFloat32(column2_w)
    builder.PrependFloat32(column2_z)
    builder.PrependFloat32(column2_y)
    builder.PrependFloat32(column2_x)
    builder.Prep(4, 16)
    builder.PrependFloat32(column1_w)
    builder.PrependFloat32(column1_z)
    builder.PrependFloat32(column1_y)
    builder.PrependFloat32(column1_x)
    builder.Prep(4, 16)
    builder.PrependFloat32(column0_w)
    builder.PrependFloat32(column0_z)
    builder.PrependFloat32(column0_y)
    builder.PrependFloat32(column0_x)
    return builder.Offset()

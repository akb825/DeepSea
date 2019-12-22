# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class OrientedBox3f(object):
    __slots__ = ['_tab']

    # OrientedBox3f
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # OrientedBox3f
    def Orientation(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 0)
        return obj

    # OrientedBox3f
    def Center(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 36)
        return obj

    # OrientedBox3f
    def HalfExtents(self, obj):
        obj.Init(self._tab.Bytes, self._tab.Pos + 48)
        return obj


def CreateOrientedBox3f(builder, orientation_column0_x, orientation_column0_y, orientation_column0_z, orientation_column1_x, orientation_column1_y, orientation_column1_z, orientation_column2_x, orientation_column2_y, orientation_column2_z, center_x, center_y, center_z, halfExtents_x, halfExtents_y, halfExtents_z):
    builder.Prep(4, 60)
    builder.Prep(4, 12)
    builder.PrependFloat32(halfExtents_z)
    builder.PrependFloat32(halfExtents_y)
    builder.PrependFloat32(halfExtents_x)
    builder.Prep(4, 12)
    builder.PrependFloat32(center_z)
    builder.PrependFloat32(center_y)
    builder.PrependFloat32(center_x)
    builder.Prep(4, 36)
    builder.Prep(4, 12)
    builder.PrependFloat32(orientation_column2_z)
    builder.PrependFloat32(orientation_column2_y)
    builder.PrependFloat32(orientation_column2_x)
    builder.Prep(4, 12)
    builder.PrependFloat32(orientation_column1_z)
    builder.PrependFloat32(orientation_column1_y)
    builder.PrependFloat32(orientation_column1_x)
    builder.Prep(4, 12)
    builder.PrependFloat32(orientation_column0_z)
    builder.PrependFloat32(orientation_column0_y)
    builder.PrependFloat32(orientation_column0_x)
    return builder.Offset()

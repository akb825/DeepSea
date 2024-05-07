# Physics

DeepSea Physics contains an interface that can be implemented for a physics engine. At least one implementation is provided to integrate with a 3rd party physics engine, and other implementations may be added either with a different 3rd party physics engine or a custom implementation.

# Compatibility Considerations

The design of the interface took multiple open-source physics engines into consideration, namely [Jolt](https://github.com/jrouwe/JoltPhysics), [Bullet](https://github.com/bulletphysics/bullet3), and [PhysX](https://github.com/NVIDIA-Omniverse/PhysX), to ensure as much compatibility as possible for integration. Some of the key highlights are mentioned here.

## Optimizations are exposed when possible

In some situations, optimizations are available for some physics libraries but won't change behavior if not present. Examples include:

* `dsRigidBodyGroup` can group together rigid bodies that are associated with each-other, exposing `PxAggregate` for PhysX. A dummy implementation is used for other implementations to make it easy to add/remove groups of rigid bodies from the physics scene.
* A rigid body flag can choose between reporting all contact points vs. allowing similar contacts to be combined, which is an option for Jolt.
* Flags can control whether certain aspects of a rigid body are mutable, which may use optimized structures on some implementations.

## Moments of inertia are computed by the base library

Different underlying physics libraries have differing utilities for computing the moment of inertia (or how easy it is to apply torque across each axis) for shapes and differing levels of correctness. For example, Bullet only uses proper moments of inertia for about half the shapes, and PhysX has some limited utilities provided as extensions.

To ensure consistent behavior, all moments of inertia are computed via `dsPhysicsMassProperties` in the base Physics library. Functions are provided to compute and manipulate the inertia tensor, and the default inertia for rigid bodies is computed based on these functions.

## Some features may be limited compared to specific libraries

Some of the features are limited based on the least-common denominator when compared to specific physics libraries.

One notable omission is a heightmap class. While each of the physics libraries used as reference have a heightfield class, they have extremely different levels of support. For example, Jolt only supports square heightfields of floats, while PhysX only supports 16-bit integer samples. Jolt only supports one material per quad with no control over the edge direction, but PhysX has material indices per triangle and allow you to change edge direction on a per-quad bases, while Bullet only allows you to choose the edge direction for the heightfield as a whole. Based on these limitations, a mesh should be used in place of a heightfield.

## Implementation Notes

Here are some notes for specific implementations as a reminder if or when they get added in the future.

### Bullet

Constraints will need to have force limits converted to impulses each step. For btConeTwistConstraint, the maximum stopping force will need to be converted to a damping value each step, while for other constraints the target velocity will need to be adjusted when using position based motors. Maximum force will need to be adjusted based on whether the motor is enabled or disabled to use the maximum motor force or stopping force.

At least in the case of btHingeConstraint, and likely for others as well, the `BT_CONSTRAINT_STOP_ERP` parameter appears to correspond to stiffness and relaxation factor corresponds to damping.

### Jolt

SwingTwistConstraint will need to have its maximum force values set per axis based on the separating axis between the current rotation and the target rotation. Same will need to be done with SixDOFConstraint when choosing between swing/twist motors (where the swing is two axes) and slerp constraint.

### PhysX

Constraint motors will need to either have a large stiffness for position motors or high damping for velocity motors. The force limit should be used to determine the motor limits, with the force limit adjusted based on if it's enabled or disabled, in which case it will use the stopping force limit.

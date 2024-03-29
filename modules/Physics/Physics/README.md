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

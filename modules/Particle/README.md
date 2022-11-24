# Particle

The DeepSea Particle library contains interfaces to display particles.

Particles are created with dsParticleEmitter. This is customizable, allowing for extra space for each particle to store state and an update function to create and update particles. Particles created by emitters can be drawn with dsParticleDraw.

Common parameters for all particles are stored in dsParticle. Helper functions are available for randomizing the parameters of particles for custom emitters. When a larger particle type is used for an emitter, it must lead with dsParticle for the common parameters.

dsStandardParticleEmitter is a dsParticleEmitter implementation that can handle many simple particle emitter cases. Particles are emitted from a volume in a random direction. A separate transform may be used for the spawn volume and the particles themselves, allowing for the spawn volume to move.
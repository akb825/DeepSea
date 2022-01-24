# Render Metal

DeepSea Render Metal provides the Metal implementation of the renderer.

If you wish to get optimal performance for Apple silicon, you may want to consider supporting [fragment inputs](https://github.com/akb825/ModularShaderLanguage/blob/master/doc/Language.md#fragment-inputs). This requires a separate set of shaders and render passes, but can reduce memory bandwidth and improve overall performance. It is roughly equivalent to using render subpasses on Vulkan, though is unfortunately requires a significantly different interface to utilize. It's also advisable to use `dsRenderbuffer` rather than `dsTexture` for any buffers you don't need to use outside of the render pass, which can reduce the amount of memory the driver needs to allocate. You may check the `hasFragmentInputs` member of `dsRenderer` to determine whether you should load the fragment input render passes and shaders or subpass versions.

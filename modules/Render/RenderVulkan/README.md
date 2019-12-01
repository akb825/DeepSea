# Render Vulkan

DeepSea Render Vulkan provides the Vulkan implementation of the renderer.

# Running with graphics debugging tools

Graphics debuggers, such as [RenderDoc](https://renderdoc.org/) and [NVIDIA Nsight Graphics](https://developer.nvidia.com/nsight-graphics) can be very useful tools for tracking down rendering problems or performance bottlenecks. However, you will sometimes run into problems when running with a debug build with validation enabled. For example, NVIDIA Nsight will hang indefinitely when capturing a frame.

DeepSea will automatically detect when RenderDoc is attached and disable validation. For other tools, you should set the `DS_DISABLE_VULKAN_VALIDATIONS=1` environment variable in the application launch options to disable the validation layers.

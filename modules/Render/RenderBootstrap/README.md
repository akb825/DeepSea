# Render Bootstrap

DeepSea Render Bootstrap provides a standard way to create a `dsRenderer` instance for various implementations based on what's compiled in and supported by the host system.

Calling `dsRenderBootstrap_createRenderer()` can create a `dsRenderer` instance for any standard renderer. The one exception is the mock renderer from the RenderMock library, as it doesn't perform actual rendering. Using type `dsRendererType_Default` will create the best renderer for the current system and hardware. Other values of the `dsRendererType` enum can be used to create a renderer of a specific type.

#include "AnyGLConfig.h"

#if ANYGL_LOAD == ANYGL_LOAD_WGL
#define WIN32_LEAN_AND_MEAN
#undef APIENTRY
#include <Windows.h>
#include <GL/gl.h>

#define ANYGL_NO_DEFINES
#include "wgl.h"

/* Generated by AnyGL. */

HMODULE AnyGL_gllib;

ANYGL_EXPORT PFNANYWGLCREATEBUFFERREGIONARBPROC AnyGL_wglCreateBufferRegionARB;
ANYGL_EXPORT PFNANYWGLDELETEBUFFERREGIONARBPROC AnyGL_wglDeleteBufferRegionARB;
ANYGL_EXPORT PFNANYWGLSAVEBUFFERREGIONARBPROC AnyGL_wglSaveBufferRegionARB;
ANYGL_EXPORT PFNANYWGLRESTOREBUFFERREGIONARBPROC AnyGL_wglRestoreBufferRegionARB;
ANYGL_EXPORT PFNANYWGLCREATECONTEXTATTRIBSARBPROC AnyGL_wglCreateContextAttribsARB;
ANYGL_EXPORT PFNANYWGLGETEXTENSIONSSTRINGARBPROC AnyGL_wglGetExtensionsStringARB;
ANYGL_EXPORT PFNANYWGLMAKECONTEXTCURRENTARBPROC AnyGL_wglMakeContextCurrentARB;
ANYGL_EXPORT PFNANYWGLGETCURRENTREADDCARBPROC AnyGL_wglGetCurrentReadDCARB;
ANYGL_EXPORT PFNANYWGLCREATEPBUFFERARBPROC AnyGL_wglCreatePbufferARB;
ANYGL_EXPORT PFNANYWGLGETPBUFFERDCARBPROC AnyGL_wglGetPbufferDCARB;
ANYGL_EXPORT PFNANYWGLRELEASEPBUFFERDCARBPROC AnyGL_wglReleasePbufferDCARB;
ANYGL_EXPORT PFNANYWGLDESTROYPBUFFERARBPROC AnyGL_wglDestroyPbufferARB;
ANYGL_EXPORT PFNANYWGLQUERYPBUFFERARBPROC AnyGL_wglQueryPbufferARB;
ANYGL_EXPORT PFNANYWGLGETPIXELFORMATATTRIBIVARBPROC AnyGL_wglGetPixelFormatAttribivARB;
ANYGL_EXPORT PFNANYWGLGETPIXELFORMATATTRIBFVARBPROC AnyGL_wglGetPixelFormatAttribfvARB;
ANYGL_EXPORT PFNANYWGLCHOOSEPIXELFORMATARBPROC AnyGL_wglChoosePixelFormatARB;
ANYGL_EXPORT PFNANYWGLBINDTEXIMAGEARBPROC AnyGL_wglBindTexImageARB;
ANYGL_EXPORT PFNANYWGLRELEASETEXIMAGEARBPROC AnyGL_wglReleaseTexImageARB;
ANYGL_EXPORT PFNANYWGLSETPBUFFERATTRIBARBPROC AnyGL_wglSetPbufferAttribARB;
ANYGL_EXPORT PFNANYWGLSETSTEREOEMITTERSTATE3DLPROC AnyGL_wglSetStereoEmitterState3DL;
ANYGL_EXPORT PFNANYWGLGETGPUIDSAMDPROC AnyGL_wglGetGPUIDsAMD;
ANYGL_EXPORT PFNANYWGLGETGPUINFOAMDPROC AnyGL_wglGetGPUInfoAMD;
ANYGL_EXPORT PFNANYWGLGETCONTEXTGPUIDAMDPROC AnyGL_wglGetContextGPUIDAMD;
ANYGL_EXPORT PFNANYWGLCREATEASSOCIATEDCONTEXTAMDPROC AnyGL_wglCreateAssociatedContextAMD;
ANYGL_EXPORT PFNANYWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC AnyGL_wglCreateAssociatedContextAttribsAMD;
ANYGL_EXPORT PFNANYWGLDELETEASSOCIATEDCONTEXTAMDPROC AnyGL_wglDeleteAssociatedContextAMD;
ANYGL_EXPORT PFNANYWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC AnyGL_wglMakeAssociatedContextCurrentAMD;
ANYGL_EXPORT PFNANYWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC AnyGL_wglGetCurrentAssociatedContextAMD;
ANYGL_EXPORT PFNANYWGLBLITCONTEXTFRAMEBUFFERAMDPROC AnyGL_wglBlitContextFramebufferAMD;
ANYGL_EXPORT PFNANYWGLCREATEDISPLAYCOLORTABLEEXTPROC AnyGL_wglCreateDisplayColorTableEXT;
ANYGL_EXPORT PFNANYWGLLOADDISPLAYCOLORTABLEEXTPROC AnyGL_wglLoadDisplayColorTableEXT;
ANYGL_EXPORT PFNANYWGLBINDDISPLAYCOLORTABLEEXTPROC AnyGL_wglBindDisplayColorTableEXT;
ANYGL_EXPORT PFNANYWGLDESTROYDISPLAYCOLORTABLEEXTPROC AnyGL_wglDestroyDisplayColorTableEXT;
ANYGL_EXPORT PFNANYWGLGETEXTENSIONSSTRINGEXTPROC AnyGL_wglGetExtensionsStringEXT;
ANYGL_EXPORT PFNANYWGLMAKECONTEXTCURRENTEXTPROC AnyGL_wglMakeContextCurrentEXT;
ANYGL_EXPORT PFNANYWGLGETCURRENTREADDCEXTPROC AnyGL_wglGetCurrentReadDCEXT;
ANYGL_EXPORT PFNANYWGLCREATEPBUFFEREXTPROC AnyGL_wglCreatePbufferEXT;
ANYGL_EXPORT PFNANYWGLGETPBUFFERDCEXTPROC AnyGL_wglGetPbufferDCEXT;
ANYGL_EXPORT PFNANYWGLRELEASEPBUFFERDCEXTPROC AnyGL_wglReleasePbufferDCEXT;
ANYGL_EXPORT PFNANYWGLDESTROYPBUFFEREXTPROC AnyGL_wglDestroyPbufferEXT;
ANYGL_EXPORT PFNANYWGLQUERYPBUFFEREXTPROC AnyGL_wglQueryPbufferEXT;
ANYGL_EXPORT PFNANYWGLGETPIXELFORMATATTRIBIVEXTPROC AnyGL_wglGetPixelFormatAttribivEXT;
ANYGL_EXPORT PFNANYWGLGETPIXELFORMATATTRIBFVEXTPROC AnyGL_wglGetPixelFormatAttribfvEXT;
ANYGL_EXPORT PFNANYWGLCHOOSEPIXELFORMATEXTPROC AnyGL_wglChoosePixelFormatEXT;
ANYGL_EXPORT PFNANYWGLSWAPINTERVALEXTPROC AnyGL_wglSwapIntervalEXT;
ANYGL_EXPORT PFNANYWGLGETSWAPINTERVALEXTPROC AnyGL_wglGetSwapIntervalEXT;
ANYGL_EXPORT PFNANYWGLGETDIGITALVIDEOPARAMETERSI3DPROC AnyGL_wglGetDigitalVideoParametersI3D;
ANYGL_EXPORT PFNANYWGLSETDIGITALVIDEOPARAMETERSI3DPROC AnyGL_wglSetDigitalVideoParametersI3D;
ANYGL_EXPORT PFNANYWGLGETGAMMATABLEPARAMETERSI3DPROC AnyGL_wglGetGammaTableParametersI3D;
ANYGL_EXPORT PFNANYWGLSETGAMMATABLEPARAMETERSI3DPROC AnyGL_wglSetGammaTableParametersI3D;
ANYGL_EXPORT PFNANYWGLGETGAMMATABLEI3DPROC AnyGL_wglGetGammaTableI3D;
ANYGL_EXPORT PFNANYWGLSETGAMMATABLEI3DPROC AnyGL_wglSetGammaTableI3D;
ANYGL_EXPORT PFNANYWGLENABLEGENLOCKI3DPROC AnyGL_wglEnableGenlockI3D;
ANYGL_EXPORT PFNANYWGLDISABLEGENLOCKI3DPROC AnyGL_wglDisableGenlockI3D;
ANYGL_EXPORT PFNANYWGLISENABLEDGENLOCKI3DPROC AnyGL_wglIsEnabledGenlockI3D;
ANYGL_EXPORT PFNANYWGLGENLOCKSOURCEI3DPROC AnyGL_wglGenlockSourceI3D;
ANYGL_EXPORT PFNANYWGLGETGENLOCKSOURCEI3DPROC AnyGL_wglGetGenlockSourceI3D;
ANYGL_EXPORT PFNANYWGLGENLOCKSOURCEEDGEI3DPROC AnyGL_wglGenlockSourceEdgeI3D;
ANYGL_EXPORT PFNANYWGLGETGENLOCKSOURCEEDGEI3DPROC AnyGL_wglGetGenlockSourceEdgeI3D;
ANYGL_EXPORT PFNANYWGLGENLOCKSAMPLERATEI3DPROC AnyGL_wglGenlockSampleRateI3D;
ANYGL_EXPORT PFNANYWGLGETGENLOCKSAMPLERATEI3DPROC AnyGL_wglGetGenlockSampleRateI3D;
ANYGL_EXPORT PFNANYWGLGENLOCKSOURCEDELAYI3DPROC AnyGL_wglGenlockSourceDelayI3D;
ANYGL_EXPORT PFNANYWGLGETGENLOCKSOURCEDELAYI3DPROC AnyGL_wglGetGenlockSourceDelayI3D;
ANYGL_EXPORT PFNANYWGLQUERYGENLOCKMAXSOURCEDELAYI3DPROC AnyGL_wglQueryGenlockMaxSourceDelayI3D;
ANYGL_EXPORT PFNANYWGLCREATEIMAGEBUFFERI3DPROC AnyGL_wglCreateImageBufferI3D;
ANYGL_EXPORT PFNANYWGLDESTROYIMAGEBUFFERI3DPROC AnyGL_wglDestroyImageBufferI3D;
ANYGL_EXPORT PFNANYWGLASSOCIATEIMAGEBUFFEREVENTSI3DPROC AnyGL_wglAssociateImageBufferEventsI3D;
ANYGL_EXPORT PFNANYWGLRELEASEIMAGEBUFFEREVENTSI3DPROC AnyGL_wglReleaseImageBufferEventsI3D;
ANYGL_EXPORT PFNANYWGLENABLEFRAMELOCKI3DPROC AnyGL_wglEnableFrameLockI3D;
ANYGL_EXPORT PFNANYWGLDISABLEFRAMELOCKI3DPROC AnyGL_wglDisableFrameLockI3D;
ANYGL_EXPORT PFNANYWGLISENABLEDFRAMELOCKI3DPROC AnyGL_wglIsEnabledFrameLockI3D;
ANYGL_EXPORT PFNANYWGLQUERYFRAMELOCKMASTERI3DPROC AnyGL_wglQueryFrameLockMasterI3D;
ANYGL_EXPORT PFNANYWGLGETFRAMEUSAGEI3DPROC AnyGL_wglGetFrameUsageI3D;
ANYGL_EXPORT PFNANYWGLBEGINFRAMETRACKINGI3DPROC AnyGL_wglBeginFrameTrackingI3D;
ANYGL_EXPORT PFNANYWGLENDFRAMETRACKINGI3DPROC AnyGL_wglEndFrameTrackingI3D;
ANYGL_EXPORT PFNANYWGLQUERYFRAMETRACKINGI3DPROC AnyGL_wglQueryFrameTrackingI3D;
ANYGL_EXPORT PFNANYWGLDXSETRESOURCESHAREHANDLENVPROC AnyGL_wglDXSetResourceShareHandleNV;
ANYGL_EXPORT PFNANYWGLDXOPENDEVICENVPROC AnyGL_wglDXOpenDeviceNV;
ANYGL_EXPORT PFNANYWGLDXCLOSEDEVICENVPROC AnyGL_wglDXCloseDeviceNV;
ANYGL_EXPORT PFNANYWGLDXREGISTEROBJECTNVPROC AnyGL_wglDXRegisterObjectNV;
ANYGL_EXPORT PFNANYWGLDXUNREGISTEROBJECTNVPROC AnyGL_wglDXUnregisterObjectNV;
ANYGL_EXPORT PFNANYWGLDXOBJECTACCESSNVPROC AnyGL_wglDXObjectAccessNV;
ANYGL_EXPORT PFNANYWGLDXLOCKOBJECTSNVPROC AnyGL_wglDXLockObjectsNV;
ANYGL_EXPORT PFNANYWGLDXUNLOCKOBJECTSNVPROC AnyGL_wglDXUnlockObjectsNV;
ANYGL_EXPORT PFNANYWGLCOPYIMAGESUBDATANVPROC AnyGL_wglCopyImageSubDataNV;
ANYGL_EXPORT PFNANYWGLDELAYBEFORESWAPNVPROC AnyGL_wglDelayBeforeSwapNV;
ANYGL_EXPORT PFNANYWGLENUMGPUSNVPROC AnyGL_wglEnumGpusNV;
ANYGL_EXPORT PFNANYWGLENUMGPUDEVICESNVPROC AnyGL_wglEnumGpuDevicesNV;
ANYGL_EXPORT PFNANYWGLCREATEAFFINITYDCNVPROC AnyGL_wglCreateAffinityDCNV;
ANYGL_EXPORT PFNANYWGLENUMGPUSFROMAFFINITYDCNVPROC AnyGL_wglEnumGpusFromAffinityDCNV;
ANYGL_EXPORT PFNANYWGLDELETEDCNVPROC AnyGL_wglDeleteDCNV;
ANYGL_EXPORT PFNANYWGLENUMERATEVIDEODEVICESNVPROC AnyGL_wglEnumerateVideoDevicesNV;
ANYGL_EXPORT PFNANYWGLBINDVIDEODEVICENVPROC AnyGL_wglBindVideoDeviceNV;
ANYGL_EXPORT PFNANYWGLQUERYCURRENTCONTEXTNVPROC AnyGL_wglQueryCurrentContextNV;
ANYGL_EXPORT PFNANYWGLJOINSWAPGROUPNVPROC AnyGL_wglJoinSwapGroupNV;
ANYGL_EXPORT PFNANYWGLBINDSWAPBARRIERNVPROC AnyGL_wglBindSwapBarrierNV;
ANYGL_EXPORT PFNANYWGLQUERYSWAPGROUPNVPROC AnyGL_wglQuerySwapGroupNV;
ANYGL_EXPORT PFNANYWGLQUERYMAXSWAPGROUPSNVPROC AnyGL_wglQueryMaxSwapGroupsNV;
ANYGL_EXPORT PFNANYWGLQUERYFRAMECOUNTNVPROC AnyGL_wglQueryFrameCountNV;
ANYGL_EXPORT PFNANYWGLRESETFRAMECOUNTNVPROC AnyGL_wglResetFrameCountNV;
ANYGL_EXPORT PFNANYWGLALLOCATEMEMORYNVPROC AnyGL_wglAllocateMemoryNV;
ANYGL_EXPORT PFNANYWGLFREEMEMORYNVPROC AnyGL_wglFreeMemoryNV;
ANYGL_EXPORT PFNANYWGLBINDVIDEOCAPTUREDEVICENVPROC AnyGL_wglBindVideoCaptureDeviceNV;
ANYGL_EXPORT PFNANYWGLENUMERATEVIDEOCAPTUREDEVICESNVPROC AnyGL_wglEnumerateVideoCaptureDevicesNV;
ANYGL_EXPORT PFNANYWGLLOCKVIDEOCAPTUREDEVICENVPROC AnyGL_wglLockVideoCaptureDeviceNV;
ANYGL_EXPORT PFNANYWGLQUERYVIDEOCAPTUREDEVICENVPROC AnyGL_wglQueryVideoCaptureDeviceNV;
ANYGL_EXPORT PFNANYWGLRELEASEVIDEOCAPTUREDEVICENVPROC AnyGL_wglReleaseVideoCaptureDeviceNV;
ANYGL_EXPORT PFNANYWGLGETVIDEODEVICENVPROC AnyGL_wglGetVideoDeviceNV;
ANYGL_EXPORT PFNANYWGLRELEASEVIDEODEVICENVPROC AnyGL_wglReleaseVideoDeviceNV;
ANYGL_EXPORT PFNANYWGLBINDVIDEOIMAGENVPROC AnyGL_wglBindVideoImageNV;
ANYGL_EXPORT PFNANYWGLRELEASEVIDEOIMAGENVPROC AnyGL_wglReleaseVideoImageNV;
ANYGL_EXPORT PFNANYWGLSENDPBUFFERTOVIDEONVPROC AnyGL_wglSendPbufferToVideoNV;
ANYGL_EXPORT PFNANYWGLGETVIDEOINFONVPROC AnyGL_wglGetVideoInfoNV;
ANYGL_EXPORT PFNANYWGLGETSYNCVALUESOMLPROC AnyGL_wglGetSyncValuesOML;
ANYGL_EXPORT PFNANYWGLGETMSCRATEOMLPROC AnyGL_wglGetMscRateOML;
ANYGL_EXPORT PFNANYWGLSWAPBUFFERSMSCOMLPROC AnyGL_wglSwapBuffersMscOML;
ANYGL_EXPORT PFNANYWGLSWAPLAYERBUFFERSMSCOMLPROC AnyGL_wglSwapLayerBuffersMscOML;
ANYGL_EXPORT PFNANYWGLWAITFORMSCOMLPROC AnyGL_wglWaitForMscOML;
ANYGL_EXPORT PFNANYWGLWAITFORSBCOMLPROC AnyGL_wglWaitForSbcOML;

int AnyGL_initialize(void)
{
	HWND window = NULL;
	HDC dc = NULL;
	HGLRC context = NULL;
	static int initialized;
	if (!AnyGL_gllib)
	{
		AnyGL_gllib = LoadLibraryA("opengl32.dll");
		if (!AnyGL_gllib)
			return 0;
	}
	if (initialized)
		return 1;

	if (!wglGetCurrentContext())
	{
		HINSTANCE hinst = GetModuleHandle(NULL);
		WNDCLASSA windowClass = {0};
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			32,
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			0,
			0,
			0,
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};
		unsigned int pixelFormat;

		windowClass.style = CS_OWNDC;
		windowClass.lpfnWndProc = &DefWindowProc;
		windowClass.hInstance = hinst;
		windowClass.lpszClassName = "AnyGLDummyWindow";
		if (!RegisterClassA(&windowClass))
			return 0;

		window = CreateWindowA(windowClass.lpszClassName, "Dummy", 0, 0, 0, 0, 0, NULL, NULL, hinst, NULL);
		if (!window)
			return 0;

		dc = GetDC(window);
		pixelFormat = ChoosePixelFormat(dc, &pfd);
		if (!pixelFormat || !SetPixelFormat(dc, pixelFormat, &pfd))
		{
			ReleaseDC(window, dc);
			DestroyWindow(window);
			return 0;
		}

		context = wglCreateContext(dc);
		if (!context || !wglMakeCurrent(dc, context))
		{
			ReleaseDC(window, dc);
			DestroyWindow(window);
			return 0;
		}
	}

	/* WGL_ARB_buffer_region */
	AnyGL_wglCreateBufferRegionARB = (PFNANYWGLCREATEBUFFERREGIONARBPROC)wglGetProcAddress("wglCreateBufferRegionARB");
	AnyGL_wglDeleteBufferRegionARB = (PFNANYWGLDELETEBUFFERREGIONARBPROC)wglGetProcAddress("wglDeleteBufferRegionARB");
	AnyGL_wglSaveBufferRegionARB = (PFNANYWGLSAVEBUFFERREGIONARBPROC)wglGetProcAddress("wglSaveBufferRegionARB");
	AnyGL_wglRestoreBufferRegionARB = (PFNANYWGLRESTOREBUFFERREGIONARBPROC)wglGetProcAddress("wglRestoreBufferRegionARB");

	/* WGL_ARB_context_flush_control */

	/* WGL_ARB_create_context */
	AnyGL_wglCreateContextAttribsARB = (PFNANYWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

	/* WGL_ARB_create_context_no_error */

	/* WGL_ARB_create_context_profile */

	/* WGL_ARB_create_context_robustness */

	/* WGL_ARB_extensions_string */
	AnyGL_wglGetExtensionsStringARB = (PFNANYWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");

	/* WGL_ARB_framebuffer_sRGB */

	/* WGL_ARB_make_current_read */
	AnyGL_wglMakeContextCurrentARB = (PFNANYWGLMAKECONTEXTCURRENTARBPROC)wglGetProcAddress("wglMakeContextCurrentARB");
	AnyGL_wglGetCurrentReadDCARB = (PFNANYWGLGETCURRENTREADDCARBPROC)wglGetProcAddress("wglGetCurrentReadDCARB");

	/* WGL_ARB_multisample */

	/* WGL_ARB_pbuffer */
	AnyGL_wglCreatePbufferARB = (PFNANYWGLCREATEPBUFFERARBPROC)wglGetProcAddress("wglCreatePbufferARB");
	AnyGL_wglGetPbufferDCARB = (PFNANYWGLGETPBUFFERDCARBPROC)wglGetProcAddress("wglGetPbufferDCARB");
	AnyGL_wglReleasePbufferDCARB = (PFNANYWGLRELEASEPBUFFERDCARBPROC)wglGetProcAddress("wglReleasePbufferDCARB");
	AnyGL_wglDestroyPbufferARB = (PFNANYWGLDESTROYPBUFFERARBPROC)wglGetProcAddress("wglDestroyPbufferARB");
	AnyGL_wglQueryPbufferARB = (PFNANYWGLQUERYPBUFFERARBPROC)wglGetProcAddress("wglQueryPbufferARB");

	/* WGL_ARB_pixel_format */
	AnyGL_wglGetPixelFormatAttribivARB = (PFNANYWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
	AnyGL_wglGetPixelFormatAttribfvARB = (PFNANYWGLGETPIXELFORMATATTRIBFVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
	AnyGL_wglChoosePixelFormatARB = (PFNANYWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

	/* WGL_ARB_pixel_format_float */

	/* WGL_ARB_render_texture */
	AnyGL_wglBindTexImageARB = (PFNANYWGLBINDTEXIMAGEARBPROC)wglGetProcAddress("wglBindTexImageARB");
	AnyGL_wglReleaseTexImageARB = (PFNANYWGLRELEASETEXIMAGEARBPROC)wglGetProcAddress("wglReleaseTexImageARB");
	AnyGL_wglSetPbufferAttribARB = (PFNANYWGLSETPBUFFERATTRIBARBPROC)wglGetProcAddress("wglSetPbufferAttribARB");

	/* WGL_ARB_robustness_application_isolation */

	/* WGL_ARB_robustness_share_group_isolation */

	/* WGL_3DFX_multisample */

	/* WGL_3DL_stereo_control */
	AnyGL_wglSetStereoEmitterState3DL = (PFNANYWGLSETSTEREOEMITTERSTATE3DLPROC)wglGetProcAddress("wglSetStereoEmitterState3DL");

	/* WGL_AMD_gpu_association */
	AnyGL_wglGetGPUIDsAMD = (PFNANYWGLGETGPUIDSAMDPROC)wglGetProcAddress("wglGetGPUIDsAMD");
	AnyGL_wglGetGPUInfoAMD = (PFNANYWGLGETGPUINFOAMDPROC)wglGetProcAddress("wglGetGPUInfoAMD");
	AnyGL_wglGetContextGPUIDAMD = (PFNANYWGLGETCONTEXTGPUIDAMDPROC)wglGetProcAddress("wglGetContextGPUIDAMD");
	AnyGL_wglCreateAssociatedContextAMD = (PFNANYWGLCREATEASSOCIATEDCONTEXTAMDPROC)wglGetProcAddress("wglCreateAssociatedContextAMD");
	AnyGL_wglCreateAssociatedContextAttribsAMD = (PFNANYWGLCREATEASSOCIATEDCONTEXTATTRIBSAMDPROC)wglGetProcAddress("wglCreateAssociatedContextAttribsAMD");
	AnyGL_wglDeleteAssociatedContextAMD = (PFNANYWGLDELETEASSOCIATEDCONTEXTAMDPROC)wglGetProcAddress("wglDeleteAssociatedContextAMD");
	AnyGL_wglMakeAssociatedContextCurrentAMD = (PFNANYWGLMAKEASSOCIATEDCONTEXTCURRENTAMDPROC)wglGetProcAddress("wglMakeAssociatedContextCurrentAMD");
	AnyGL_wglGetCurrentAssociatedContextAMD = (PFNANYWGLGETCURRENTASSOCIATEDCONTEXTAMDPROC)wglGetProcAddress("wglGetCurrentAssociatedContextAMD");
	AnyGL_wglBlitContextFramebufferAMD = (PFNANYWGLBLITCONTEXTFRAMEBUFFERAMDPROC)wglGetProcAddress("wglBlitContextFramebufferAMD");

	/* WGL_ATI_pixel_format_float */

	/* WGL_EXT_colorspace */

	/* WGL_EXT_create_context_es2_profile */

	/* WGL_EXT_create_context_es_profile */

	/* WGL_EXT_depth_float */

	/* WGL_EXT_display_color_table */
	AnyGL_wglCreateDisplayColorTableEXT = (PFNANYWGLCREATEDISPLAYCOLORTABLEEXTPROC)wglGetProcAddress("wglCreateDisplayColorTableEXT");
	AnyGL_wglLoadDisplayColorTableEXT = (PFNANYWGLLOADDISPLAYCOLORTABLEEXTPROC)wglGetProcAddress("wglLoadDisplayColorTableEXT");
	AnyGL_wglBindDisplayColorTableEXT = (PFNANYWGLBINDDISPLAYCOLORTABLEEXTPROC)wglGetProcAddress("wglBindDisplayColorTableEXT");
	AnyGL_wglDestroyDisplayColorTableEXT = (PFNANYWGLDESTROYDISPLAYCOLORTABLEEXTPROC)wglGetProcAddress("wglDestroyDisplayColorTableEXT");

	/* WGL_EXT_extensions_string */
	AnyGL_wglGetExtensionsStringEXT = (PFNANYWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

	/* WGL_EXT_framebuffer_sRGB */

	/* WGL_EXT_make_current_read */
	AnyGL_wglMakeContextCurrentEXT = (PFNANYWGLMAKECONTEXTCURRENTEXTPROC)wglGetProcAddress("wglMakeContextCurrentEXT");
	AnyGL_wglGetCurrentReadDCEXT = (PFNANYWGLGETCURRENTREADDCEXTPROC)wglGetProcAddress("wglGetCurrentReadDCEXT");

	/* WGL_EXT_multisample */

	/* WGL_EXT_pbuffer */
	AnyGL_wglCreatePbufferEXT = (PFNANYWGLCREATEPBUFFEREXTPROC)wglGetProcAddress("wglCreatePbufferEXT");
	AnyGL_wglGetPbufferDCEXT = (PFNANYWGLGETPBUFFERDCEXTPROC)wglGetProcAddress("wglGetPbufferDCEXT");
	AnyGL_wglReleasePbufferDCEXT = (PFNANYWGLRELEASEPBUFFERDCEXTPROC)wglGetProcAddress("wglReleasePbufferDCEXT");
	AnyGL_wglDestroyPbufferEXT = (PFNANYWGLDESTROYPBUFFEREXTPROC)wglGetProcAddress("wglDestroyPbufferEXT");
	AnyGL_wglQueryPbufferEXT = (PFNANYWGLQUERYPBUFFEREXTPROC)wglGetProcAddress("wglQueryPbufferEXT");

	/* WGL_EXT_pixel_format */
	AnyGL_wglGetPixelFormatAttribivEXT = (PFNANYWGLGETPIXELFORMATATTRIBIVEXTPROC)wglGetProcAddress("wglGetPixelFormatAttribivEXT");
	AnyGL_wglGetPixelFormatAttribfvEXT = (PFNANYWGLGETPIXELFORMATATTRIBFVEXTPROC)wglGetProcAddress("wglGetPixelFormatAttribfvEXT");
	AnyGL_wglChoosePixelFormatEXT = (PFNANYWGLCHOOSEPIXELFORMATEXTPROC)wglGetProcAddress("wglChoosePixelFormatEXT");

	/* WGL_EXT_pixel_format_packed_float */

	/* WGL_EXT_swap_control */
	AnyGL_wglSwapIntervalEXT = (PFNANYWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	AnyGL_wglGetSwapIntervalEXT = (PFNANYWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");

	/* WGL_EXT_swap_control_tear */

	/* WGL_I3D_digital_video_control */
	AnyGL_wglGetDigitalVideoParametersI3D = (PFNANYWGLGETDIGITALVIDEOPARAMETERSI3DPROC)wglGetProcAddress("wglGetDigitalVideoParametersI3D");
	AnyGL_wglSetDigitalVideoParametersI3D = (PFNANYWGLSETDIGITALVIDEOPARAMETERSI3DPROC)wglGetProcAddress("wglSetDigitalVideoParametersI3D");

	/* WGL_I3D_gamma */
	AnyGL_wglGetGammaTableParametersI3D = (PFNANYWGLGETGAMMATABLEPARAMETERSI3DPROC)wglGetProcAddress("wglGetGammaTableParametersI3D");
	AnyGL_wglSetGammaTableParametersI3D = (PFNANYWGLSETGAMMATABLEPARAMETERSI3DPROC)wglGetProcAddress("wglSetGammaTableParametersI3D");
	AnyGL_wglGetGammaTableI3D = (PFNANYWGLGETGAMMATABLEI3DPROC)wglGetProcAddress("wglGetGammaTableI3D");
	AnyGL_wglSetGammaTableI3D = (PFNANYWGLSETGAMMATABLEI3DPROC)wglGetProcAddress("wglSetGammaTableI3D");

	/* WGL_I3D_genlock */
	AnyGL_wglEnableGenlockI3D = (PFNANYWGLENABLEGENLOCKI3DPROC)wglGetProcAddress("wglEnableGenlockI3D");
	AnyGL_wglDisableGenlockI3D = (PFNANYWGLDISABLEGENLOCKI3DPROC)wglGetProcAddress("wglDisableGenlockI3D");
	AnyGL_wglIsEnabledGenlockI3D = (PFNANYWGLISENABLEDGENLOCKI3DPROC)wglGetProcAddress("wglIsEnabledGenlockI3D");
	AnyGL_wglGenlockSourceI3D = (PFNANYWGLGENLOCKSOURCEI3DPROC)wglGetProcAddress("wglGenlockSourceI3D");
	AnyGL_wglGetGenlockSourceI3D = (PFNANYWGLGETGENLOCKSOURCEI3DPROC)wglGetProcAddress("wglGetGenlockSourceI3D");
	AnyGL_wglGenlockSourceEdgeI3D = (PFNANYWGLGENLOCKSOURCEEDGEI3DPROC)wglGetProcAddress("wglGenlockSourceEdgeI3D");
	AnyGL_wglGetGenlockSourceEdgeI3D = (PFNANYWGLGETGENLOCKSOURCEEDGEI3DPROC)wglGetProcAddress("wglGetGenlockSourceEdgeI3D");
	AnyGL_wglGenlockSampleRateI3D = (PFNANYWGLGENLOCKSAMPLERATEI3DPROC)wglGetProcAddress("wglGenlockSampleRateI3D");
	AnyGL_wglGetGenlockSampleRateI3D = (PFNANYWGLGETGENLOCKSAMPLERATEI3DPROC)wglGetProcAddress("wglGetGenlockSampleRateI3D");
	AnyGL_wglGenlockSourceDelayI3D = (PFNANYWGLGENLOCKSOURCEDELAYI3DPROC)wglGetProcAddress("wglGenlockSourceDelayI3D");
	AnyGL_wglGetGenlockSourceDelayI3D = (PFNANYWGLGETGENLOCKSOURCEDELAYI3DPROC)wglGetProcAddress("wglGetGenlockSourceDelayI3D");
	AnyGL_wglQueryGenlockMaxSourceDelayI3D = (PFNANYWGLQUERYGENLOCKMAXSOURCEDELAYI3DPROC)wglGetProcAddress("wglQueryGenlockMaxSourceDelayI3D");

	/* WGL_I3D_image_buffer */
	AnyGL_wglCreateImageBufferI3D = (PFNANYWGLCREATEIMAGEBUFFERI3DPROC)wglGetProcAddress("wglCreateImageBufferI3D");
	AnyGL_wglDestroyImageBufferI3D = (PFNANYWGLDESTROYIMAGEBUFFERI3DPROC)wglGetProcAddress("wglDestroyImageBufferI3D");
	AnyGL_wglAssociateImageBufferEventsI3D = (PFNANYWGLASSOCIATEIMAGEBUFFEREVENTSI3DPROC)wglGetProcAddress("wglAssociateImageBufferEventsI3D");
	AnyGL_wglReleaseImageBufferEventsI3D = (PFNANYWGLRELEASEIMAGEBUFFEREVENTSI3DPROC)wglGetProcAddress("wglReleaseImageBufferEventsI3D");

	/* WGL_I3D_swap_frame_lock */
	AnyGL_wglEnableFrameLockI3D = (PFNANYWGLENABLEFRAMELOCKI3DPROC)wglGetProcAddress("wglEnableFrameLockI3D");
	AnyGL_wglDisableFrameLockI3D = (PFNANYWGLDISABLEFRAMELOCKI3DPROC)wglGetProcAddress("wglDisableFrameLockI3D");
	AnyGL_wglIsEnabledFrameLockI3D = (PFNANYWGLISENABLEDFRAMELOCKI3DPROC)wglGetProcAddress("wglIsEnabledFrameLockI3D");
	AnyGL_wglQueryFrameLockMasterI3D = (PFNANYWGLQUERYFRAMELOCKMASTERI3DPROC)wglGetProcAddress("wglQueryFrameLockMasterI3D");

	/* WGL_I3D_swap_frame_usage */
	AnyGL_wglGetFrameUsageI3D = (PFNANYWGLGETFRAMEUSAGEI3DPROC)wglGetProcAddress("wglGetFrameUsageI3D");
	AnyGL_wglBeginFrameTrackingI3D = (PFNANYWGLBEGINFRAMETRACKINGI3DPROC)wglGetProcAddress("wglBeginFrameTrackingI3D");
	AnyGL_wglEndFrameTrackingI3D = (PFNANYWGLENDFRAMETRACKINGI3DPROC)wglGetProcAddress("wglEndFrameTrackingI3D");
	AnyGL_wglQueryFrameTrackingI3D = (PFNANYWGLQUERYFRAMETRACKINGI3DPROC)wglGetProcAddress("wglQueryFrameTrackingI3D");

	/* WGL_NV_DX_interop */
	AnyGL_wglDXSetResourceShareHandleNV = (PFNANYWGLDXSETRESOURCESHAREHANDLENVPROC)wglGetProcAddress("wglDXSetResourceShareHandleNV");
	AnyGL_wglDXOpenDeviceNV = (PFNANYWGLDXOPENDEVICENVPROC)wglGetProcAddress("wglDXOpenDeviceNV");
	AnyGL_wglDXCloseDeviceNV = (PFNANYWGLDXCLOSEDEVICENVPROC)wglGetProcAddress("wglDXCloseDeviceNV");
	AnyGL_wglDXRegisterObjectNV = (PFNANYWGLDXREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXRegisterObjectNV");
	AnyGL_wglDXUnregisterObjectNV = (PFNANYWGLDXUNREGISTEROBJECTNVPROC)wglGetProcAddress("wglDXUnregisterObjectNV");
	AnyGL_wglDXObjectAccessNV = (PFNANYWGLDXOBJECTACCESSNVPROC)wglGetProcAddress("wglDXObjectAccessNV");
	AnyGL_wglDXLockObjectsNV = (PFNANYWGLDXLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXLockObjectsNV");
	AnyGL_wglDXUnlockObjectsNV = (PFNANYWGLDXUNLOCKOBJECTSNVPROC)wglGetProcAddress("wglDXUnlockObjectsNV");

	/* WGL_NV_DX_interop2 */

	/* WGL_NV_copy_image */
	AnyGL_wglCopyImageSubDataNV = (PFNANYWGLCOPYIMAGESUBDATANVPROC)wglGetProcAddress("wglCopyImageSubDataNV");

	/* WGL_NV_delay_before_swap */
	AnyGL_wglDelayBeforeSwapNV = (PFNANYWGLDELAYBEFORESWAPNVPROC)wglGetProcAddress("wglDelayBeforeSwapNV");

	/* WGL_NV_float_buffer */

	/* WGL_NV_gpu_affinity */
	AnyGL_wglEnumGpusNV = (PFNANYWGLENUMGPUSNVPROC)wglGetProcAddress("wglEnumGpusNV");
	AnyGL_wglEnumGpuDevicesNV = (PFNANYWGLENUMGPUDEVICESNVPROC)wglGetProcAddress("wglEnumGpuDevicesNV");
	AnyGL_wglCreateAffinityDCNV = (PFNANYWGLCREATEAFFINITYDCNVPROC)wglGetProcAddress("wglCreateAffinityDCNV");
	AnyGL_wglEnumGpusFromAffinityDCNV = (PFNANYWGLENUMGPUSFROMAFFINITYDCNVPROC)wglGetProcAddress("wglEnumGpusFromAffinityDCNV");
	AnyGL_wglDeleteDCNV = (PFNANYWGLDELETEDCNVPROC)wglGetProcAddress("wglDeleteDCNV");

	/* WGL_NV_multisample_coverage */

	/* WGL_NV_present_video */
	AnyGL_wglEnumerateVideoDevicesNV = (PFNANYWGLENUMERATEVIDEODEVICESNVPROC)wglGetProcAddress("wglEnumerateVideoDevicesNV");
	AnyGL_wglBindVideoDeviceNV = (PFNANYWGLBINDVIDEODEVICENVPROC)wglGetProcAddress("wglBindVideoDeviceNV");
	AnyGL_wglQueryCurrentContextNV = (PFNANYWGLQUERYCURRENTCONTEXTNVPROC)wglGetProcAddress("wglQueryCurrentContextNV");

	/* WGL_NV_render_depth_texture */

	/* WGL_NV_render_texture_rectangle */

	/* WGL_NV_swap_group */
	AnyGL_wglJoinSwapGroupNV = (PFNANYWGLJOINSWAPGROUPNVPROC)wglGetProcAddress("wglJoinSwapGroupNV");
	AnyGL_wglBindSwapBarrierNV = (PFNANYWGLBINDSWAPBARRIERNVPROC)wglGetProcAddress("wglBindSwapBarrierNV");
	AnyGL_wglQuerySwapGroupNV = (PFNANYWGLQUERYSWAPGROUPNVPROC)wglGetProcAddress("wglQuerySwapGroupNV");
	AnyGL_wglQueryMaxSwapGroupsNV = (PFNANYWGLQUERYMAXSWAPGROUPSNVPROC)wglGetProcAddress("wglQueryMaxSwapGroupsNV");
	AnyGL_wglQueryFrameCountNV = (PFNANYWGLQUERYFRAMECOUNTNVPROC)wglGetProcAddress("wglQueryFrameCountNV");
	AnyGL_wglResetFrameCountNV = (PFNANYWGLRESETFRAMECOUNTNVPROC)wglGetProcAddress("wglResetFrameCountNV");

	/* WGL_NV_vertex_array_range */
	AnyGL_wglAllocateMemoryNV = (PFNANYWGLALLOCATEMEMORYNVPROC)wglGetProcAddress("wglAllocateMemoryNV");
	AnyGL_wglFreeMemoryNV = (PFNANYWGLFREEMEMORYNVPROC)wglGetProcAddress("wglFreeMemoryNV");

	/* WGL_NV_video_capture */
	AnyGL_wglBindVideoCaptureDeviceNV = (PFNANYWGLBINDVIDEOCAPTUREDEVICENVPROC)wglGetProcAddress("wglBindVideoCaptureDeviceNV");
	AnyGL_wglEnumerateVideoCaptureDevicesNV = (PFNANYWGLENUMERATEVIDEOCAPTUREDEVICESNVPROC)wglGetProcAddress("wglEnumerateVideoCaptureDevicesNV");
	AnyGL_wglLockVideoCaptureDeviceNV = (PFNANYWGLLOCKVIDEOCAPTUREDEVICENVPROC)wglGetProcAddress("wglLockVideoCaptureDeviceNV");
	AnyGL_wglQueryVideoCaptureDeviceNV = (PFNANYWGLQUERYVIDEOCAPTUREDEVICENVPROC)wglGetProcAddress("wglQueryVideoCaptureDeviceNV");
	AnyGL_wglReleaseVideoCaptureDeviceNV = (PFNANYWGLRELEASEVIDEOCAPTUREDEVICENVPROC)wglGetProcAddress("wglReleaseVideoCaptureDeviceNV");

	/* WGL_NV_video_output */
	AnyGL_wglGetVideoDeviceNV = (PFNANYWGLGETVIDEODEVICENVPROC)wglGetProcAddress("wglGetVideoDeviceNV");
	AnyGL_wglReleaseVideoDeviceNV = (PFNANYWGLRELEASEVIDEODEVICENVPROC)wglGetProcAddress("wglReleaseVideoDeviceNV");
	AnyGL_wglBindVideoImageNV = (PFNANYWGLBINDVIDEOIMAGENVPROC)wglGetProcAddress("wglBindVideoImageNV");
	AnyGL_wglReleaseVideoImageNV = (PFNANYWGLRELEASEVIDEOIMAGENVPROC)wglGetProcAddress("wglReleaseVideoImageNV");
	AnyGL_wglSendPbufferToVideoNV = (PFNANYWGLSENDPBUFFERTOVIDEONVPROC)wglGetProcAddress("wglSendPbufferToVideoNV");
	AnyGL_wglGetVideoInfoNV = (PFNANYWGLGETVIDEOINFONVPROC)wglGetProcAddress("wglGetVideoInfoNV");

	/* WGL_OML_sync_control */
	AnyGL_wglGetSyncValuesOML = (PFNANYWGLGETSYNCVALUESOMLPROC)wglGetProcAddress("wglGetSyncValuesOML");
	AnyGL_wglGetMscRateOML = (PFNANYWGLGETMSCRATEOMLPROC)wglGetProcAddress("wglGetMscRateOML");
	AnyGL_wglSwapBuffersMscOML = (PFNANYWGLSWAPBUFFERSMSCOMLPROC)wglGetProcAddress("wglSwapBuffersMscOML");
	AnyGL_wglSwapLayerBuffersMscOML = (PFNANYWGLSWAPLAYERBUFFERSMSCOMLPROC)wglGetProcAddress("wglSwapLayerBuffersMscOML");
	AnyGL_wglWaitForMscOML = (PFNANYWGLWAITFORMSCOMLPROC)wglGetProcAddress("wglWaitForMscOML");
	AnyGL_wglWaitForSbcOML = (PFNANYWGLWAITFORSBCOMLPROC)wglGetProcAddress("wglWaitForSbcOML");

	if (context)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(context);
		ReleaseDC(window, dc);
		DestroyWindow(window);
	}

	initialized = 1;
	return 1;
}

void AnyGL_shutdown(void)
{
	if (AnyGL_gllib)
	{
		FreeLibrary(AnyGL_gllib);
		AnyGL_gllib = NULL;
	}
}

#endif /* ANYGL_LOAD */

#include "stdafx.h"
#include "Display/Device/GL/PCGLExtensions.h"

PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;
PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = NULL;

GLboolean GLExtensionSupported(const char* pExtension)
{
	const char* pExtensions = (char*)glGetString(GL_EXTENSIONS);

	if (strstr(pExtensions, pExtension))
	{
		return GL_TRUE;
	}
	else
	{
		return GL_FALSE;
	}
}

GLboolean WGLExtensionSupportedARB(const char* pExtension)
{
	if (!wglGetExtensionsStringARB)
	{
		wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	}

	if (wglGetExtensionsStringARB)
	{
		const char* pExtensions = (char*)wglGetExtensionsStringARB(wglGetCurrentDC());

		if (strstr(pExtensions, pExtension))
		{
			return GL_TRUE;
		}
	}

	return GL_FALSE;
}

GLboolean WGLExtensionSupportedEXT(const char* pExtension)
{
	if (!wglGetExtensionsStringEXT)
	{
		wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
	}

	if (wglGetExtensionsStringEXT)
	{
		const char* pExtensions = (char*)wglGetExtensionsStringEXT();

		if (strstr(pExtensions, pExtension))
		{
			return GL_TRUE;
		}
	}

	return GL_FALSE;
}

const char* GLMakeEntryFromFuncAndExt(const char* pFunc, const char* pExt)
{
	static char entry[256];
	sprintf_s(entry, 256, "%s%s", pFunc, pExt);
	return entry;
}

#define GLFunctionSupported(_func, _type, _ext) (_func = (_type)wglGetProcAddress(GLMakeEntryFromFuncAndExt(#_func, #_ext)))

// ARB_texture_compression

GLboolean ARB_texture_compression_supported = GL_FALSE;

PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2D = NULL;

GLboolean GLInitialiseTextureCompression()
{
	ARB_texture_compression_supported =
	(
		GLExtensionSupported("ARB_texture_compression") &&
		GLFunctionSupported(glCompressedTexImage2D, PFNGLCOMPRESSEDTEXIMAGE2DARBPROC, ARB)
	) ? GL_TRUE : GL_FALSE;

	if (ARB_texture_compression_supported)
	{
		LOGINFO(NewDisplay::g_Log, "Initialising extension ARB_texture_compression\n");
	}
	else
	{
		LOGERROR(NewDisplay::g_Log, "Initialising extension ARB_texture_compression\n");
	}

	return ARB_texture_compression_supported;
}

// GL_ARB_multitexture

GLint		GL_ARB_multitexture_units		= 0;
GLboolean	GL_ARB_multitexture_supported	= GL_FALSE;

PFNGLACTIVETEXTUREARBPROC		glActiveTexture			= NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTexture	= NULL;
PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1f		= NULL;
PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2f		= NULL;
PFNGLMULTITEXCOORD2FVARBPROC	glMultiTexCoord2fv		= NULL;
PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3f		= NULL;
PFNGLMULTITEXCOORD3FVARBPROC	glMultiTexCoord3fv		= NULL;

GLboolean GLInitialiseMultitexture()
{
	GL_ARB_multitexture_supported = 
	(
		GLExtensionSupported("GL_ARB_multitexture") &&
		GLFunctionSupported(glActiveTexture,		PFNGLACTIVETEXTUREARBPROC,			ARB) &&
		GLFunctionSupported(glClientActiveTexture,	PFNGLCLIENTACTIVETEXTUREARBPROC,	ARB) &&
		GLFunctionSupported(glMultiTexCoord1f,		PFNGLMULTITEXCOORD1FARBPROC,		ARB) &&
		GLFunctionSupported(glMultiTexCoord2f,		PFNGLMULTITEXCOORD2FARBPROC,		ARB) &&
		GLFunctionSupported(glMultiTexCoord2fv,		PFNGLMULTITEXCOORD2FVARBPROC,		ARB) &&
		GLFunctionSupported(glMultiTexCoord2f,		PFNGLMULTITEXCOORD2FARBPROC,		ARB) &&
		GLFunctionSupported(glMultiTexCoord3f,		PFNGLMULTITEXCOORD3FARBPROC,		ARB) &&
		GLFunctionSupported(glMultiTexCoord3fv,		PFNGLMULTITEXCOORD3FVARBPROC,		ARB) &&
		GLFunctionSupported(glMultiTexCoord2fv,		PFNGLMULTITEXCOORD2FVARBPROC,		ARB)
	) ? GL_TRUE : GL_FALSE;

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &GL_ARB_multitexture_units);

	if (GL_ARB_multitexture_supported)
	{
		LOGINFO(NewDisplay::g_Log, "Initialising extension GL_ARB_multitexture with %i units\n", GL_ARB_multitexture_units);
	}
	else
	{
		LOGERROR(NewDisplay::g_Log, "Initialising extension GL_ARB_multitexture with %i units\n", GL_ARB_multitexture_units);
	}

	return GL_ARB_multitexture_supported;
}

// GL_ARB_vertex_buffer_object

GLboolean GL_ARB_vertex_buffer_object_supported = GL_FALSE;
GLboolean GL_ARB_vertex_buffer_object_enabled	= GL_FALSE;

PFNGLBINDBUFFERARBPROC				glBindBuffer			= NULL;
PFNGLDELETEBUFFERSARBPROC			glDeleteBuffers			= NULL;
PFNGLGENBUFFERSARBPROC				glGenBuffers			= NULL;
PFNGLISBUFFERARBPROC				glIsBuffer				= NULL;
PFNGLBUFFERDATAARBPROC				glBufferData			= NULL;
PFNGLBUFFERSUBDATAARBPROC			glBufferSubData			= NULL;
PFNGLGETBUFFERSUBDATAARBPROC		glGetBufferSubData		= NULL;
PFNGLMAPBUFFERARBPROC				glMapBuffer				= NULL;
PFNGLUNMAPBUFFERARBPROC				glUnmapBuffer			= NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC	glGetBufferParameteriv	= NULL;
PFNGLGETBUFFERPOINTERVARBPROC		glGetBufferPointerv		= NULL;

GLboolean GLInitialiseVertexBuffer()
{
	GL_ARB_vertex_buffer_object_supported = 
	(
		GLExtensionSupported("GL_ARB_vertex_buffer_object") &&
		GLFunctionSupported(glBindBuffer,			PFNGLBINDBUFFERARBPROC,				ARB) &&
		GLFunctionSupported(glDeleteBuffers,		PFNGLDELETEBUFFERSARBPROC,			ARB) &&
		GLFunctionSupported(glGenBuffers,			PFNGLGENBUFFERSARBPROC,				ARB) &&
		GLFunctionSupported(glIsBuffer,				PFNGLISBUFFERARBPROC,				ARB) &&
		GLFunctionSupported(glBufferData,			PFNGLBUFFERDATAARBPROC,				ARB) &&
		GLFunctionSupported(glGetBufferSubData,		PFNGLGETBUFFERSUBDATAARBPROC,		ARB) &&
		GLFunctionSupported(glMapBuffer,			PFNGLMAPBUFFERARBPROC,				ARB) &&
		GLFunctionSupported(glUnmapBuffer,			PFNGLUNMAPBUFFERARBPROC,			ARB) &&
		GLFunctionSupported(glGetBufferParameteriv,	PFNGLGETBUFFERPARAMETERIVARBPROC,	ARB) &&
		GLFunctionSupported(glGetBufferPointerv,	PFNGLGETBUFFERPOINTERVARBPROC,		ARB)
	) ? GL_TRUE : GL_FALSE;

	if (GL_ARB_vertex_buffer_object_supported)
	{
		LOGINFO(NewDisplay::g_Log, "Initialising extension GL_ARB_vertex_buffer_object\n");
	}
	else
	{
		LOGERROR(NewDisplay::g_Log, "Initialising extension GL_ARB_vertex_buffer_object\n");
	}

	GL_ARB_vertex_buffer_object_enabled = GL_ARB_vertex_buffer_object_supported;

	return GL_ARB_vertex_buffer_object_supported;
}

// WGL_ARB_pbuffer, WGL_ARB_pixel_format, WGL_ARB_render_texture

GLboolean WGL_ARB_pbuffer_supported = GL_FALSE;

PFNWGLCREATEPBUFFERARBPROC			wglCreatePbuffer			= NULL;
PFNWGLGETPBUFFERDCARBPROC			wglGetPbufferDC				= NULL;
PFNWGLRELEASEPBUFFERDCARBPROC		wglReleasePbufferDC			= NULL;
PFNWGLDESTROYPBUFFERARBPROC			wglDestroyPbuffer			= NULL;
PFNWGLQUERYPBUFFERARBPROC			wglQueryPbuffer				= NULL;
PFNWGLGETPIXELFORMATATTRIBIVARBPROC	wglGetPixelFormatAttribiv	= NULL;
PFNWGLGETPIXELFORMATATTRIBFVARBPROC	wglGetPixelFormatAttribfv	= NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormat		= NULL;
PFNWGLBINDTEXIMAGEARBPROC			wglBindTexImage				= NULL;
PFNWGLRELEASETEXIMAGEARBPROC		wglReleaseTexImage			= NULL;
PFNWGLSETPBUFFERATTRIBARBPROC		wglSetPbufferAttrib			= NULL;

GLboolean GLInitialisePbuffer()
{
	WGL_ARB_pbuffer_supported = 
	(
		WGLExtensionSupportedARB("WGL_ARB_pbuffer") &&
		WGLExtensionSupportedARB("WGL_ARB_pixel_format") &&
		WGLExtensionSupportedARB("WGL_ARB_render_texture") &&
		GLFunctionSupported(wglGetPixelFormatAttribiv,	PFNWGLGETPIXELFORMATATTRIBIVARBPROC,	ARB) &&
		GLFunctionSupported(wglGetPixelFormatAttribfv,	PFNWGLGETPIXELFORMATATTRIBFVARBPROC,	ARB) &&
		GLFunctionSupported(wglChoosePixelFormat,		PFNWGLCHOOSEPIXELFORMATARBPROC,			ARB) &&
		GLFunctionSupported(wglCreatePbuffer,			PFNWGLCREATEPBUFFERARBPROC,				ARB) &&
		GLFunctionSupported(wglGetPbufferDC,			PFNWGLGETPBUFFERDCARBPROC,				ARB) &&
		GLFunctionSupported(wglReleasePbufferDC,		PFNWGLRELEASEPBUFFERDCARBPROC,			ARB) &&
		GLFunctionSupported(wglDestroyPbuffer,			PFNWGLDESTROYPBUFFERARBPROC,			ARB) &&
		GLFunctionSupported(wglQueryPbuffer,			PFNWGLQUERYPBUFFERARBPROC,				ARB) &&
		GLFunctionSupported(wglBindTexImage,			PFNWGLBINDTEXIMAGEARBPROC,				ARB) &&
		GLFunctionSupported(wglReleaseTexImage,			PFNWGLRELEASETEXIMAGEARBPROC,			ARB) &&
		GLFunctionSupported(wglSetPbufferAttrib,		PFNWGLSETPBUFFERATTRIBARBPROC,			ARB)
	) ? GL_TRUE : GL_FALSE;

	if (WGL_ARB_pbuffer_supported)
	{
		LOGINFO(NewDisplay::g_Log, "Initialising extensions WGL_ARB_pbuffer, WGL_ARB_pixel_format and WGL_ARB_render_texture\n");
	}
	else
	{
		LOGERROR(NewDisplay::g_Log, "Initialising extensions WGL_ARB_pbuffer, WGL_ARB_pixel_format and WGL_ARB_render_texture\n");
	}

	return WGL_ARB_pbuffer_supported;
}

// GL_EXT_blend_minmax

GLboolean GL_EXT_blend_minmax_supported =false;

PFNGLBLENDEQUATIONEXTPROC glBlendEquation = NULL;

GLboolean GLInitialiseBlendMinMax()
{
	GL_EXT_blend_minmax_supported = 
	(
		GLFunctionSupported(glBlendEquation, PFNGLBLENDEQUATIONEXTPROC, EXT)
	) ? GL_TRUE : GL_FALSE;

	if (GL_EXT_blend_minmax_supported)
	{
		LOGINFO(NewDisplay::g_Log, "Initialising extension GL_EXT_blend_minmax\n");
	}
	else
	{
		LOGERROR(NewDisplay::g_Log, "Initialising extension GL_EXT_blend_minmax\n");
	}

	return GL_EXT_blend_minmax_supported;
}

// GL_EXT_stencil_two_side

GLboolean GL_EXT_stencil_two_side_supported = false;

PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFace = NULL;

GLboolean GLInitialiseStencilTwoSide()
{
	GL_EXT_stencil_two_side_supported = 
	(
		GLFunctionSupported(glActiveStencilFace, PFNGLACTIVESTENCILFACEEXTPROC, EXT)
	) ? GL_TRUE : GL_FALSE;

	if (GL_EXT_stencil_two_side_supported)
	{
		LOGINFO(NewDisplay::g_Log, "Initialising extension GL_EXT_stencil_two_side\n");
	}
	else
	{
		LOGERROR(NewDisplay::g_Log, "Initialising extension GL_EXT_stencil_two_side\n");
	}

	return GL_EXT_stencil_two_side_supported;
}

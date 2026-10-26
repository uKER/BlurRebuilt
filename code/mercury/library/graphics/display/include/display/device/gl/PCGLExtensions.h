#ifndef DISPLAY_PCGLEXTENSIONS_H
#define DISPLAY_PCGLEXTENSIONS_H

#ifdef _MSC_VER
#pragma once
#endif

extern PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
extern PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT;

GLboolean GLExtensionSupported(const char* pExtension);
GLboolean WGLExtensionSupportedARB(const char* pExtension);
GLboolean WGLExtensionSupportedEXT(const char* pExtension);

// ARB_texture_compression

extern GLboolean ARB_texture_compression_supported;

extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2D;

GLboolean GLInitialiseTextureCompression();

// GL_ARB_multitexture

extern GLint		GL_ARB_multitexture_units;
extern GLboolean	GL_ARB_multitexture_supported;

extern PFNGLACTIVETEXTUREARBPROC		glActiveTexture;
extern PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTexture;
extern PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1f;
extern PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2f;
extern PFNGLMULTITEXCOORD2FVARBPROC		glMultiTexCoord2fv;
extern PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3f;
extern PFNGLMULTITEXCOORD3FVARBPROC		glMultiTexCoord3fv;

GLboolean GLInitialiseMultitexture();

// GL_ARB_vertex_buffer_object

extern GLboolean GL_ARB_vertex_buffer_object_supported;
extern GLboolean GL_ARB_vertex_buffer_object_enabled;

extern PFNGLBINDBUFFERARBPROC			glBindBuffer;
extern PFNGLDELETEBUFFERSARBPROC		glDeleteBuffers;
extern PFNGLGENBUFFERSARBPROC			glGenBuffers;
extern PFNGLISBUFFERARBPROC				glIsBuffer;
extern PFNGLBUFFERDATAARBPROC			glBufferData;
extern PFNGLBUFFERSUBDATAARBPROC		glBufferSubData;
extern PFNGLGETBUFFERSUBDATAARBPROC		glGetBufferSubData;
extern PFNGLMAPBUFFERARBPROC			glMapBuffer;
extern PFNGLUNMAPBUFFERARBPROC			glUnmapBuffer;
extern PFNGLGETBUFFERPARAMETERIVARBPROC	glGetBufferParameteriv;
extern PFNGLGETBUFFERPOINTERVARBPROC	glGetBufferPointerv;

GLboolean GLInitialiseVertexBuffer();

// WGL_ARB_pbuffer, WGL_ARB_pixel_format, WGL_ARB_render_texture

extern GLboolean WGL_ARB_pbuffer_supported;

extern PFNWGLGETEXTENSIONSSTRINGARBPROC		wglGetExtensionsString;
extern PFNWGLCREATEPBUFFERARBPROC			wglCreatePbuffer;
extern PFNWGLGETPBUFFERDCARBPROC			wglGetPbufferDC;
extern PFNWGLRELEASEPBUFFERDCARBPROC		wglReleasePbufferDC;
extern PFNWGLDESTROYPBUFFERARBPROC			wglDestroyPbuffer;
extern PFNWGLQUERYPBUFFERARBPROC			wglQueryPbuffer;
extern PFNWGLGETPIXELFORMATATTRIBIVARBPROC	wglGetPixelFormatAttribiv;
extern PFNWGLGETPIXELFORMATATTRIBFVARBPROC	wglGetPixelFormatAttribfv;
extern PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormat;
extern PFNWGLBINDTEXIMAGEARBPROC			wglBindTexImage;
extern PFNWGLRELEASETEXIMAGEARBPROC			wglReleaseTexImage;
extern PFNWGLSETPBUFFERATTRIBARBPROC		wglSetPbufferAttrib;

GLboolean GLInitialisePbuffer();

// GL_EXT_blend_minmax

#ifndef GL_EXT_blend_minmax
#define GL_EXT_blend_minmax
#define GL_FUNC_ADD_EXT			0x8006
#define GL_MIN_EXT				0x8007
#define GL_MAX_EXT				0x8008
#define GL_BLEND_EQUATION_EXT	0x8009
typedef void (APIENTRY * PFNGLBLENDEQUATIONEXTPROC)(GLenum mode);
#endif

extern GLboolean GL_EXT_blend_supported;

extern PFNGLBLENDEQUATIONEXTPROC glBlendEquation;

GLboolean GLInitialiseBlendMinMax();

// GL_EXT_stencil_two_side

#ifndef GL_EXT_stencil_two_side
#define GL_EXT_stencil_two_side
#define GL_STENCIL_TEST_TWO_SIDE_EXT	0x8910
#define GL_ACTIVE_STENCIL_FACE_EXT		0x8911
typedef void (APIENTRY * PFNGLACTIVESTENCILFACEEXTPROC)(GLenum face);
#endif

extern PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFace;

GLboolean GLInitialiseStencilTwoSide();

// GL_EXT_texture_edge_clamp

#ifndef GL_EXT_texture_edge_clamp
#define GL_EXT_texture_edge_clamp
#define GL_CLAMP_TO_EDGE_EXT 0x812F
#endif

#endif // DISPLAY_PCGLEXTENSIONS_H

#ifndef DISPLAY_GL_H
#define DISPLAY_GL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GL/GLDeviceTypes.h"

#ifdef PLATFORM_PC
#include "Display/Device/GL/PCGLExtensions.h"
#endif

namespace NewDisplay
{
namespace GL
{
	
INLINE void GLError()
{
	GLenum error = glGetError();	
	switch (error)
	{
	case GL_NO_ERROR:
		break;
	case GL_INVALID_ENUM:
		LOGERROR(g_Log, "GL_INVALID_ENUM\n");
		DebugBreak();
		break;
	case GL_INVALID_VALUE:
		LOGERROR(g_Log, "GL_INVALID_VALUE\n");
		DebugBreak();
		break;
	case GL_INVALID_OPERATION:
		LOGERROR(g_Log, "GL_INVALID_OPERATION\n");
		DebugBreak();
		break;
	case GL_STACK_OVERFLOW:
		LOGERROR(g_Log, "GL_STACK_OVERFLOW\n");
		DebugBreak();
		break;
	case GL_STACK_UNDERFLOW:
		LOGERROR(g_Log, "GL_STACK_UNDERFLOW\n");
		DebugBreak();
		break;
	case GL_OUT_OF_MEMORY:
		LOGERROR(g_Log, "GL_OUT_OF_MEMORY\n");
		DebugBreak();
		break;
#ifdef PLATFORM_PC
	case GL_TABLE_TOO_LARGE:
		LOGERROR(g_Log, "GL_TABLE_TOO_LARGE\n");
		DebugBreak();
		break;
#endif
	default:
		LOGERROR(g_Log, "GL_UNKNOWN\n");
		DebugBreak();
		break;
	}
}

}; // namespace GL

#ifdef GRAPHICS_GL
using namespace GL; // Make gl the global device.
#endif

}; // namespace NewDisplay

#endif // DISPLAY_GL_H

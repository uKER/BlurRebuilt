// fx_core.h common includes for .fx files
//
// this file provides cross-platform compatibility macros etc.
// Jan van Valburg 2008

#ifndef __FX_CORE_H__
#define __FX_CORE_H__

#if !(PLATFORM_PC + PLATFORM_XENON + PLATFORM_PS3)
#define MAYA
#endif

#ifndef FXFILE_PARSER
#define annotation(x)
#if PLATFORM_PS3
#ifndef shared
#define shared
#endif
#endif
#else
#define annotation(x) ___VERTEX_DECLARATION_ANNOTATION___ #x
#endif

#endif

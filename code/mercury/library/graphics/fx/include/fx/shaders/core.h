// fx_core.h common includes for .fx files
//
// this file provides cross-platform compatibility macros etc.
// Jan van Valburg 2008

#ifndef __FX_CORE_H__
#define __FX_CORE_H__

#if !(PLATFORM_PC + 0 + PLATFORM_PS3 + MAYA)
#define MAYA
#define XSI //This means we can't have shaders that work in Maya AND Xsi, as the Maya plugin #define's MAYA
#endif

#ifndef FXFILE_PARSER
	#define annotation(x)
	#if PLATFORM_PS3 && !defined(__VARIABLE_PARSING_STAGE__)
		#ifndef shared
			#define shared
		#endif
	#endif
#else
	#define annotation(x) ___VERTEX_DECLARATION_ANNOTATION___ #x
#endif

#define SRGB_TEXTURE SRGBTEXTURE = TRUE;

#endif

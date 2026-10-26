#ifndef SHADER_DEFINES_H
#define SHADER_DEFINES_H

// enabling and disabling

#if defined(ENABLE_BUMPMAP)
#	if defined(NO_BUMPMAP)
#		undef ENABLE_BUMPMAP
#	else
#		if !defined(ENABLE_TANGENTNORMAL)
#			define ENABLE_TANGENTNORMAL
#		endif
#	endif
#endif

#if defined(ENABLE_SPECULARMAP)
#	if defined(NO_SPECULARMAP)
#		undef ENABLE_SPECULARMAP
#	else
#		if !defined(ENABLE_SPECULAR)
#			define ENABLE_SPECULAR
#		endif
#	endif
#endif

#if defined(ENABLE_SPECULAR)
#	if !defined(ENABLE_VIEWVECTOR)
#		define ENABLE_VIEWVECTOR
#	endif
#endif

#if defined(ENABLE_EMISSIVE)
#	if defined(NO_EMISSIVE)
#		undef ENABLE_EMISSIVE
#	endif
#endif

#if !defined(DISABLE_FOG)
#	define ENABLE_FOG
#	if !defined(ENABLE_PROJPOS)
#		define ENABLE_PROJPOS
#	endif
#endif

#if !defined(NO_LIGHTMAP)
#	define ENABLE_LIGHTMAP
#endif

#if !defined(NO_VTXCOL)
#	if !defined(ENABLE_VERTEXCOLOURS)
#		define ENABLE_VERTEXCOLOURS
#	endif
#	if !defined(DISABLE_VERTEXLIGHTING) && !defined(ENABLE_VERTEXLIGHTING)
#		define ENABLE_VERTEXLIGHTING
#	endif
#endif

// define strings for optimisations and Maya plugin

#if defined(ENABLE_LAMBERT_SHADER)
#	define ALBEDO_NAME_STRING "<Diff>"
#	define SPECULAR_NAME_STRING "<Ipr0>"
#	define BUMP_NAME_STRING "<Bump>"
#	define EMISSIVE_NAME_STRING "<Neon>"
#else
#	define ALBEDO_NAME_STRING "Colour"
#	define SPECULAR_NAME_STRING "Specular"
#	define BUMP_NAME_STRING "Normal"
#	define EMISSIVE_NAME_STRING "Emissive"
#endif

#if defined(ENABLE_BUMPMAP_OPTIMISATION)
#	define BUMP_DEFINE_STRING string Define = "NB";
#else
#	define BUMP_DEFINE_STRING
#endif

#if defined(ENABLE_EMISSIVE_OPTIMISATION)
#	define EMISSIVE_DEFINE_STRING string Define = "NE";
#else
#	define EMISSIVE_DEFINE_STRING
#endif

#if defined(ENABLE_SPECULARMAP_OPTIMISATION)
#	define SPECULAR_DEFINE_STRING string Define = "NS";
#else
#	define SPECULAR_DEFINE_STRING
#endif

#if defined(ENABLE_LIGHTMAP_OPTIMISATION)
#	define LIGHTMAP_DEFINE_STRING string Define = "NL";
#else
#	define LIGHTMAP_DEFINE_STRING
#endif

// remove from Maya

#if defined(CFG_CONVERTER)

#if defined(ENABLE_SPECULAR)
#undef ENABLE_SPECULAR
#endif

#if defined(ENABLE_LIGHTMAP)
#undef ENABLE_LIGHTMAP
#endif

#if defined(ENABLE_FOG)
#undef ENABLE_FOG
#endif

#define DISABLE_LODFADE

#endif

#if defined(DISABLE_ALPHA_DETECT)

#define ALPHA_DETECT

#else

#define ALPHA_DETECT bool AlphaDetect = true;

#endif

#if defined(ENABLE_MIPMAP_ALPHA_ENHANCE)
#	define MIPMAP_ALPHA_ENHANCE bool EnhanceMipMapAlpha = true;
#else
#	define MIPMAP_ALPHA_ENHANCE
#endif

#endif // SHADER_DEFINES_H

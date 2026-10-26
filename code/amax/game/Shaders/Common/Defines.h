#ifndef AMAX_SHADERS_COMMON_DEFINES_H
#define AMAX_SHADERS_COMMON_DEFINES_H

// enabling and disabling

#if defined(ENABLE_BUMPMAP)
#	if defined(NO_BUMPMAP)
#		undef ENABLE_BUMPMAP
#	else
#		if !defined(ENABLE_NORMAL)
#			define ENABLE_NORMAL
#		endif
#		if !defined(ENABLE_TANGENTNORMAL)
#			define ENABLE_TANGENTNORMAL
#		endif
#	endif
#endif

#if defined(ENABLE_MATERIALMAP)
#	if defined(NO_MATERIALMAP)
#		undef ENABLE_MATERIALMAP
#	else
#		if !defined(LOD)
#			define ENABLE_SPECULAR
#		endif
#		define ENABLE_EMISSIVE
#	endif
#endif

#if !defined(DISABLE_FOG)
#	define ENABLE_FOG
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

#if !defined(ENABLE_NORMAL)
#	if defined(DAY_LIGHTING) || defined(ENABLE_SPECULAR)
#		define ENABLE_NORMAL
#	endif
#endif
#if !defined(ENABLE_VIEW)
#	if defined(ENABLE_SPECULAR)
#		define ENABLE_VIEW
#	endif
#endif

#if defined(ALPHA)
#	if defined(ENABLE_LIGHTING_PREPASS)
#		undef ENABLE_LIGHTING_PREPASS
#	endif
#	if defined(ENABLE_NORMALS_PREPASS)
#		undef ENABLE_NORMALS_PREPASS
#	endif
#endif

// define strings for optimisations and Maya plugin

#if defined(ENABLE_LAMBERT_SHADER)
#	define ALBEDO_NAME_STRING "<Diff>"
#	define MATERIAL_NAME_STRING "<Ipr0>"
#	define BUMP_NAME_STRING "<Bump>"
#elif defined(ENABLE_TEXTURE_NAMING_CONVENTIONS)
#	define ALBEDO_NAME_STRING "Colour"
#	define MATERIAL_NAME_STRING "<Ipr0+Colour>"
#	define BUMP_NAME_STRING "<Bump+Colour>"
#else
#	if defined(ENABLE_MATERIAL_NAMING_CONVENTION)
#		define MATERIAL_NAME_STRING "<Ipr0+Colour>"
#	else
#		define MATERIAL_NAME_STRING "Material"
#	endif
#	if defined(ENABLE_NORMAL_NAMING_CONVENTION)
#		define BUMP_NAME_STRING "<Bump+Colour>"
#	else
#		define BUMP_NAME_STRING "Normal"
#	endif
#	define ALBEDO_NAME_STRING "Colour"
#endif

#if defined(ENABLE_BUMPMAP_OPTIMISATION)
#	define BUMP_DEFINE_STRING string Define = "NB";
#else
#	define BUMP_DEFINE_STRING
#endif

#if defined(ENABLE_MATERIALMAP_OPTIMISATION)
#	define MATERIAL_DEFINE_STRING string Define = "NM";
#else
#	define MATERIAL_DEFINE_STRING
#endif

#if defined(ENABLE_LIGHTMAP_OPTIMISATION)
#	define LIGHTMAP_DEFINE_STRING string Define = "NL";
#else
#	define LIGHTMAP_DEFINE_STRING
#endif

#ifdef NIGHT_LIGHTING
#	define SH_NO_CHANNEL_COEFFS
#endif

#if defined(ENABLE_PREPASS)
#	if !defined(ENABLE_NORMAL)
#		define ENABLE_NORMAL
#	endif
#endif

#if defined(ENABLE_SHNODE_IN_PIXEL_SHADER)
#	if !defined(ENABLE_NORMAL)
#		define ENABLE_NORMAL
#	endif
#endif

#if !defined(DISABLE_LODFADE)
#	if !defined(ENABLE_NORMAL)
#		define ENABLE_NORMAL
#	endif
#endif

#if !defined(ENABLE_SHNODE_IN_PIXEL_SHADER)
#	define SHNODE_IN_VERTEX_CONSTANT
#endif

#if defined(ENABLE_MIPMAP_ALPHA_ENHANCE)
#	define MIPMAP_ALPHA_ENHANCE bool EnhanceMipMapAlpha = true;
#else
#	define MIPMAP_ALPHA_ENHANCE
#endif

// remove from Maya

#if defined(CFG_CONVERTER)

#ifndef ENABLE_SHNODE_IN_PIXEL_SHADER
#if defined(ENABLE_SPECULAR)
#undef ENABLE_SPECULAR
#endif
#endif

#if defined(ENABLE_LIGHTMAP)
#undef ENABLE_LIGHTMAP
#endif

#if defined(ENABLE_FOG)
#undef ENABLE_FOG
#endif

#define DISABLE_LODFADE

#endif

#endif // AMAX_SHADERS_COMMON_DEFINES_H

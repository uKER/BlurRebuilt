#ifndef DISPLAY_SHADER_SHADER_UTILS_H
#define DISPLAY_SHADER_SHADER_UTILS_H

/*
 this file defines two macros:
 EXTERN_SHADER is used to extern a shader which has been compiled into the exe. 
	e.g. EXTERN_SHADER(vsFontRender)
 REF_SHADER is used to reference a shader (in order to create it)
  e.g. CreateVertexShaderFromFunc("Shaders/vsFontRender", 0, &m_VertexShader, REF_SHADER(vsFontRender))

 if OBJ_SHADERS is not defined, both macros will default to using #include style shaders
 this is for backwards compatibility
*/

#if 1 //def OBJ_SHADERS

#if defined(PLATFORM_PC)
#define EXTERN_SHADER(name) extern "C" { extern S32 *_binary_##name##_bin_start, *_binary_##name##_bin_end; }
#define REF_SHADER(name) 0, *(C_ShaderData*)&_binary_##name##_bin_start
#else
#define EXTERN_SHADER(name) extern "C" { extern S32 *_binary_##name##_bin_start, *_binary_##name##_bin_end; }
#define REF_SHADER(name) (int)&_binary_##name##_bin_end-(int)&_binary_##name##_bin_start, (S32*)&_binary_##name##_bin_start
#endif

#else

#define EXTERN_SHADER(name)
#define REF_SHADER(name) sizeof(##name), ##name

#endif

#endif

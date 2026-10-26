#ifndef __RENDERDATACONVERSION_COMMON_H__
#define __RENDERDATACONVERSION_COMMON_H__

namespace RenderDataConversion
{

#define MAX_MATERIAL_STREAMS 4
#define MAX_VERTEX_STREAMS 4

enum E_NodeType
{
E_NodeType_Mesh,
E_NodeType_Base,
E_NodeType_Instance,
E_NodeType_InstanceReset,
E_NodeType_Max,
};

extern Debug::Logger &g_Log;
void PrintWarning(const char *psz, ...);

extern Util::C_String g_szDefaultFXName;
extern Util::C_String g_szInvalidFXName;
extern Util::C_String g_szSkinningVariation;
extern Util::C_String g_szGlobalEffectVariation;
extern Util::C_String g_szTensionMapVariation;

}

#endif

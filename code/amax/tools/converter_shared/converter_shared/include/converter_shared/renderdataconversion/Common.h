#ifndef __RENDERDATACONVERSION_COMMON_H__
#define __RENDERDATACONVERSION_COMMON_H__

namespace RenderDataConversion
{

enum E_NodeType
{
E_NodeType_Mesh,
E_NodeType_Base,
E_NodeType_Instance,
E_NodeType_Max,
};

extern Debug::Logger &g_Log;
void PrintWarning(const char *psz, ...);

extern Util::C_String g_szDefaultFXName;
extern Util::C_String g_szGlobalEffectVariation;

}

#endif

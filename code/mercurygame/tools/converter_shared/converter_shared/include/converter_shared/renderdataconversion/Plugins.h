#ifndef RENDERDATACONVERSION_PLUGINS_H
#define RENDERDATACONVERSION_PLUGINS_H

namespace RenderDataConversion
{

void RegisterDefaultTextureAnnotationReader();
void RegisterDefaultVtxInputSemantics();
void RegisterSkinningPlugin();

}

namespace SkinningProcessor
{

void Register();
void SetSkinningProcessor(I_SkinningProcessor *p);

}

#endif

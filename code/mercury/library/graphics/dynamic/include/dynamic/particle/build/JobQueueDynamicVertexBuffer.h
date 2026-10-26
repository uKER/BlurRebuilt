#ifndef RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_JOBQUEUEDYNAMICVERTEXBUFFER_H
#define RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_JOBQUEUEDYNAMICVERTEXBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Build/DynamicVertexBuffer.h"

namespace DynamicEffect
{

extern Debug::Logger &g_Log;

namespace Particle
{
namespace BuildPolicy
{

template<class ShapePolicy>
class T_JobQueueDynamicVertexBuffer : public T_DynamicVertexBuffer<ShapePolicy>
{
public:
	T_JobQueueDynamicVertexBuffer() {}
	virtual ~T_JobQueueDynamicVertexBuffer() {};

	void	Build(const C_View& view, C_CompositeRenderNode& node) const
	{
		const I_ParticleEffectBase* pEffect = (const I_ParticleEffectBase*)node.pEffect;
		const RenderSettings& renderSettings = pEffect->GetRenderSettings();

		U32 vertices = node.nNumElements * renderSettings.vertices;
		U32 vertexSize = renderSettings.vertexSize;
		
#ifndef CFG_SPEED
	#if defined(PLATFORM_PS3)
		bool ok = ((renderSettings.vertices * vertexSize) % 16 == 0);
		if(!ok)
			LOGWARN(g_Log, "DynamicEffect: ShapePolicy does not do 16 byte stores per particle, skipping");
		// check that store sizes are going to be multiples of 16
		if(vertices > 0 && ok)
	#else
		if(vertices > 0)
	#endif
#endif
		{
			// try and grab a vertex buffer block from the dynamic vertex buffer
			C_DynamicVertexBufferBlock block = pEffect->GetRenderer()->GetDynamicVertexBuffer()->Begin("Emitter", vertexSize, vertices);
	
			if(block.pVertices)
			{
				view.GetJobQueueDynamicVertexBufferBatcher().AddNode(view, &node.blocks, &renderSettings.vertexBuildSettings, block.pVertices); 
				// tell renderer we have added a dynamic vertex buffer block
				// it will need to say this is completed later
				view.AddDynamicVertexBufferBlock(block);
			}
			
			// store index to the vertex buffer block for later
			node.iVertexBuffer = block.blockIndex;
			return;
		}
		
		// scheduler not active or couldn't get VertexBuildData so specifiy invalid vertex buffer index
		node.iVertexBuffer = 0xFFFFFFFF;
	}
};

} // BuildPolicy
} // Particles
} // DynamicEffect

#endif

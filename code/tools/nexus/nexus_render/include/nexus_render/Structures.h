#ifndef NEXUS_RENDER_STRUCTURES_H
#define NEXUS_RENDER_STRUCTURES_H

namespace NexusRender
{
	//Describe the usage of our vertex buffer group.
	struct VertexBuffer
	{
		IDirect3DVertexBuffer9* pBuffer;
		void*					pContents;
		//The number of polygon vertices within the vertex buffer.
		U32						size;
	};

	struct IndexBuffer
	{
		IDirect3DIndexBuffer9*	pIndexBuffer;
		U32						size;
	};
};

#endif

#ifndef PRIMITIVESTEAM_H
#define PRIMITIVESTEAM_H

namespace Display2
{
	struct C_VertexBuffer;
};

namespace RenderingData
{
namespace Target
{
	//! Vertex buffer data -- moved out of RenderListNode.
	struct VertexBufferData
	{
		const Display2::C_VertexBuffer* pBuffer;
		S32 vertexSize;
	};
	
	//! Primitive stream structure.
	// replicated from RenderingData::RenderListNode::PrimitiveStream, as that version can't be forward-declared or included.
	struct PrimitiveStream
	{
		S16 vertexBuffer;		//!< Index of the vertex buffer in the render list.
		S16 streamIndex;		//!< Index of the stream.
		S32 vertexOffset;		//!< Offset.
	};
}
}

#endif // #ifdef PRIMITIVESTEAM_H

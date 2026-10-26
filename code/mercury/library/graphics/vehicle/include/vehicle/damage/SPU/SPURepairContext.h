#ifndef SPUREPAIRCONTEXT_H_INCLUDED
#define SPUREPAIRCONTEXT_H_INCLUDED

namespace SpuDamage
{
	static const U16						maxNumberOfVertexBuffers = 128;
	static const U16						maxControlPoints = 18;
	
	//! Repair context -- managed SPU tasks for car offset stream deformation.
	struct ALIGN128 C_RepairContext
	{
		U32									vertexBuffers[maxNumberOfVertexBuffers];			//!< EA of input vb.
		U32									vertexBufferSize[maxNumberOfVertexBuffers];			//!< Size of the input vertex buffers in bytes.
		U16									vertexBufferCount;									//!< Number of vertex buffers to repair.
	};
};

#endif // #ifndef SPUREPAIRCONTEXT_H_INCLUDED


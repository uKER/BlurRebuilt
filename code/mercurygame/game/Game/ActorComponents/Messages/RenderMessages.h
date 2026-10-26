#ifndef RENDER_MESSAGES_H
#define RENDER_MESSAGES_H

#include <misc_shared/uid/FourCC.h>

// render component messages
enum E_RenderMsg
{
	E_RenderMsg_HiBits = FOURCC_HI20('RNDR'), 

	E_RenderMsg_SetMatrix,
	E_RenderMsg_SetRenderOffset,
	E_RenderMsg_Show,
	E_RenderMsg_Hide,
	E_RenderMsg_DisablePortalCulling,

	E_RenderMsg_End, 
	E_RenderMsg_Begin = E_RenderMsg_HiBits + 1, 
	E_RenderMsg_Count = E_RenderMsg_End - E_RenderMsg_Begin
};

class C_SetRenderMatrixMsg: public Messaging::C_MessageDerived<C_SetRenderMatrixMsg, E_RenderMsg_SetMatrix>
{
public:
	explicit C_SetRenderMatrixMsg(const Maths::HMat43& matrix) {Maths::Store(matrix, &m_RenderMatrix);}

	const Maths::HMat43	GetMatrix() const { return Maths::Load<Maths::HMat43>(&m_RenderMatrix);}

	MathsStorage::Float43	m_RenderMatrix;	
};

class C_SetRenderOffsetMsg: public Messaging::C_MessageDerived<C_SetRenderOffsetMsg, E_RenderMsg_SetRenderOffset>
{
public:
	explicit C_SetRenderOffsetMsg(const Maths::C_Vector& offset) : m_Offset(offset) {}

	const Maths::HVec3	GetOffset() const { return Maths::Load<Maths::HVec3>(&m_Offset);}

	MathsStorage::Float3	m_Offset;	// use maths storage? perf issue?
};

class C_DisablePortalCullingMsg: public Messaging::C_MessageDerived<C_DisablePortalCullingMsg, E_RenderMsg_DisablePortalCulling> {};

#endif
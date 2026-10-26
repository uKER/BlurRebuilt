#ifndef RENDER_MESSAGES_H
#define RENDER_MESSAGES_H

#include <misc_shared/messaging/MessageValue.h>
#include <misc_shared/uid/FourCC.h>

#include <world_render/MaterialParameter.h>

#include <actor_system\ActorDeclare.h>

namespace DbUtil
{
	class I_DebugRender;
}

namespace Debug
{
	class Logger;
}

namespace Sys
{
	class I_File;
}



// render component messages
enum E_RenderMsg
{
	E_RenderMsg_HiBits = FOURCC_HI20('RNDR'), 

	E_RenderMsg_SetMatrix,
	E_RenderMsg_SetRenderOffset,
	E_RenderMsg_Show,
	E_RenderMsg_Hide,
	E_RenderMsg_DisablePortalCulling,
	E_RenderMsg_ShowElement,
	E_RenderMsg_HideElement,
	E_RenderMsg_SetElementMatrix,
	E_RenderMsg_EnableMotionBlur,
	E_RenderMsg_SetSubtechnique,
	E_RenderMsg_UpdateAnimatedShaderParams,
	E_RenderMsg_SetMaterialFloatParam,
	E_RenderMsg_EnableVisibilityFlags,
	E_RenderMsg_DisableVisibilityFlags,

	E_RenderMsg_RequestResourceUsage,
	E_RenderMsg_RequestResourceTotal,
	E_RenderMsg_NotifyResourceUsage,
	E_RenderMsg_NotifyResourceTotal,
	E_RenderMsg_DrawResourceUsage,
	E_RenderMsg_DumpResourceUsage,

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

// Element Messages
typedef Messaging::C_MessageValue<S32,E_RenderMsg_ShowElement>	C_RenderShowElementMsg;
typedef Messaging::C_MessageValue<S32,E_RenderMsg_HideElement>	C_RenderHideElementMsg;

class C_RenderSetElementMatrixMsg: public Messaging::C_MessageDerived<C_RenderSetElementMatrixMsg, E_RenderMsg_SetElementMatrix>
{
public:
	explicit C_RenderSetElementMatrixMsg(const S32 index, const Maths::HMat43& matrix):m_ElementIndex(index),m_RenderMatrix(matrix) {}

	const S32				GetIndex() const { return m_ElementIndex;}
	const Maths::HMat43	&	GetMatrix() const { return m_RenderMatrix;}

	S32				m_ElementIndex;
	Maths::HMat43	m_RenderMatrix;	// TODO: naughty! shouldn't be using aligned types here
};

typedef Messaging::C_MessageValue<bool, E_RenderMsg_EnableMotionBlur>	C_RenderEnableMotionBlurMsg;

typedef Messaging::C_MessageValue<U32, E_RenderMsg_SetSubtechnique>	C_RenderSetSubtechniqueMsg;


class C_RenderSetMaterialFloatParamMsg : public Messaging::C_MessageDerived<C_RenderSetMaterialFloatParamMsg, E_RenderMsg_SetMaterialFloatParam>
{
public:
	C_RenderSetMaterialFloatParamMsg(const WorldRender::MaterialParameter& param) : m_Param(param) {}
	WorldRender::MaterialParameter m_Param;
	MathsStorage::Float4 m_Storage;
};


typedef Messaging::C_MessageValue<U32, E_RenderMsg_EnableVisibilityFlags> C_RenderEnableVisibilityFlagsMsg;

typedef Messaging::C_MessageValue<U32, E_RenderMsg_DisableVisibilityFlags> C_RenderDisableVisibilityFlagsMsg;


class C_RenderNotifyResourceUsage : public Messaging::C_MessageDerived<C_RenderNotifyResourceUsage, E_RenderMsg_NotifyResourceUsage>
{
public:
	C_RenderNotifyResourceUsage();
	C_RenderNotifyResourceUsage(const Maths::C_Sphere& worldBoundingSphere, const bool bHasDetails, const Actor::C_ActorHandle& actorHandle, const Component::C_ComponentInterfaceId componentInterface = -1);

	~C_RenderNotifyResourceUsage();

	Maths::C_Sphere						GetWorldBoundingSphere() const;
	bool								HasDetails() const {return m_bHasDetails;}
	const Actor::C_ActorHandle&			GetActorHandle() const {return m_ActorHandle;}
	Component::C_ComponentInterfaceId	GetComponentInterface() const {return m_ComponentInterface;}

	void								SetWorldBoundingSphere(const Maths::C_Sphere& worldBoundingSphere);
	void								SetHasDetails(const bool bHasDetails) {m_bHasDetails = bHasDetails;}
	void								SetActorHandle(const Actor::C_ActorHandle& actorHandle) {m_ActorHandle = actorHandle;}
	void								SetComponentInterface(const Component::C_ComponentInterfaceId componentInterface) {m_ComponentInterface = componentInterface;}

private:
	MathsStorage::Float3				m_WorldCentre;
	F32									m_WorldRadius;
	bool								m_bHasDetails;
	Actor::C_ActorHandle				m_ActorHandle;
	Component::C_ComponentInterfaceId	m_ComponentInterface;

};

class C_RenderNotifyResourceTotalMsg : public Messaging::C_MessageDerived<C_RenderNotifyResourceTotalMsg, E_RenderMsg_NotifyResourceTotal>
{
public:
	C_RenderNotifyResourceTotalMsg();
	explicit C_RenderNotifyResourceTotalMsg(const Maths::HVec3& worldPosition, const U32 uGraphicsBytes, const U32 uPhysicsBytes);
	
	virtual ~C_RenderNotifyResourceTotalMsg();

	Maths::HVec3						GetWorldPosition() const;
	U32									GetGraphicsBytes() const {return m_uGraphicsBytes;}
	U32									GetPhysicsBytes() const {return m_uPhysicsBytes;}

	void								SetWorldPosition(const Maths::HVec3& worldPosition);
	void								SetGraphicsBytes(const U32 uGraphicsBytes) {m_uGraphicsBytes = uGraphicsBytes;}
	void								SetPhysicsBytes(const U32 uPhysicsBytes) {m_uPhysicsBytes = uPhysicsBytes;}

private:
	MathsStorage::Float3				m_WorldPosition;
	U32									m_uGraphicsBytes;
	U32									m_uPhysicsBytes;

};

class C_RenderDrawResourceUsageMsg : public Messaging::C_MessageDerived<C_RenderDrawResourceUsageMsg, E_RenderMsg_DrawResourceUsage>
{
public:
	C_RenderDrawResourceUsageMsg() : m_pDebugRender(NULL) {}
	explicit C_RenderDrawResourceUsageMsg(DbUtil::I_DebugRender* pDebugRender) : m_pDebugRender(pDebugRender) {}

	DbUtil::I_DebugRender*	GetDebugRender() const {return m_pDebugRender;}
	void					SetDebugRender(DbUtil::I_DebugRender* pDebugRender) {m_pDebugRender = pDebugRender;}

private:
	DbUtil::I_DebugRender*	m_pDebugRender;

};

class C_RenderDumpResourceUsageMsg : public Messaging::C_MessageDerived<C_RenderDumpResourceUsageMsg, E_RenderMsg_DumpResourceUsage>
{
public:
	explicit C_RenderDumpResourceUsageMsg(Debug::Logger* pDebugLogger = NULL, Sys::I_File* pFile = NULL) : m_pDebugLogger(pDebugLogger), m_pFile(pFile) {}

	Debug::Logger*	GetDebugLogger() const {return m_pDebugLogger;}
	Sys::I_File*	GetFile() const {return m_pFile;}

	void			SetDebugLogger(Debug::Logger* pDebugLogger) {m_pDebugLogger = pDebugLogger;}
	void			SetFile(Sys::I_File* pFile) {m_pFile = pFile;}

private:
	Debug::Logger*	m_pDebugLogger;
	Sys::I_File*	m_pFile;

};


#endif

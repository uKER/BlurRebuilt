#ifndef RENDER_COMPONENTS_H
#define RENDER_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <game_resource/ModelResource.h>
#include <resource/Resource.h>
#include <resource/TypeRegistry.h>
#include <shared_actors/Components/Creation/RenderComponentsCreate.h>
#include <shared_actors/Components/Interfaces/I_RenderComponents.h>


// forward decs
class C_ModelResourceUsage;

namespace Components
{
	class C_ElementState;
	class I_ElementManager;
}

namespace GameResource
{
	class C_Model;
	class C_ModelResource;
}

namespace Portal
{
	class BlockerHandle;
}

namespace WorldRender
{
	class C_Model;
	class C_ModelInstance;
	class C_EnvironmentSampler;
}


namespace Components
{

// Render component
class C_RenderComponent : public I_RenderComponent
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
						C_RenderComponent():m_bHidden(false){}
	//virtual			~C_RenderComponent(){}
	bool				OnMessage(const Messaging::C_MessageBase *pMessage );			// message handler

	virtual void		Show(void){}	// do nothing
	virtual void		Hide(void){}	// do nothing


	virtual void		SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void		SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSync(Sys::C_ContextualStream& stream);

	// I_RenderComponent interface
	const Maths::HMat43	&	GetRenderMatrix() const { return m_WorldMatrix; }
	const Maths::C_Sphere &	GetLocalBoundingSphere() const {return	m_LocalBoundingSphere;}

protected:
	void				SetRenderMatrix(const Maths::HMat43	&mat) {m_WorldMatrix = mat;}
	virtual bool		InitialiseImpl(); // from Component::I_GameComponent
	virtual bool		CreateImpl(); // from Component::I_GameComponent

private:
	void				Serialise(Sys::C_ContextualStream& stream) const;
	void				Deserialise(Sys::C_ContextualStream& stream);

protected:
	bool				m_bHidden;
	Maths::HMat43		m_WorldMatrix;
	Maths::C_Sphere		m_LocalBoundingSphere;

};


// Model Render Base Component Class
// ---------------------------------

// base class
class C_RenderModelBase : public C_RenderComponent
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
									C_RenderModelBase();
	virtual							~C_RenderModelBase();

	bool							OnMessage(const Messaging::C_MessageBase *pMessage );			// message handler

	virtual void					Show(void);
	virtual void					Hide(void);

	const C_RenderModelBaseCreate	*GetCreationData(void) const {return static_cast<const C_RenderModelBaseCreate *>(GetCreator().GetData());}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

	virtual F32						GetDrawDistance() const {return GetCreationData()->m_DrawDistance;}

	WorldRender::MaterialParameter	GetMaterialParameter(const Util::C_String& paramName) const;

	WorldRender::C_ModelInstance*	GetModelInstance() const;
	// Rendering
	bool							m_HasShadow;
	WorldRender::C_ModelInstance*	m_pModelInstance;
	WorldRender::C_EnvironmentSampler*	m_pEnvSampler;

protected:
	virtual bool					CreateImpl(); // from Component::I_GameComponent
	virtual void					KillImpl(); // from Component::I_GameComponent

	void							CustomiseModel(WorldRender::C_ModelInstance* pModelInstance, const GameResource::C_Model* pModelData, const WorldRender::C_Model* pRenderData) const;
	void							SetWorldSphere(const Maths::C_Sphere& worldSphere);

private:
	void							Serialise(Sys::C_ContextualStream& stream) const;
	void							Deserialise(Sys::C_ContextualStream& stream);

#ifndef CFG_GOLD
	virtual void					AccumulateModelResourceUsage(C_ModelResourceUsage& modelResourceUsage, const WorldRender::C_RenderModelData& renderModelData) const = 0;
#endif

};


// Simple Model Render Component
// -----------------------------

// component
class C_ModelSingleElementRender : public C_RenderModelBase
{
public:
								C_ModelSingleElementRender();
								~C_ModelSingleElementRender(){}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_SingleElementRenderModel;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_SingleElementRenderModel;} // from Component::I_GameComponent


	bool							OnMessage(const Messaging::C_MessageBase *pMessage );			// message handler
	const C_ModelSingleElementRenderCreate	*GetCreationData(void) const {return static_cast<const C_ModelSingleElementRenderCreate *>(GetCreator().GetData());}
	//void							Render(const U32 renderPass, const U32 renderStates);	// render function

	void							UpdateElementMatrix(void);
	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);
#ifndef CFG_GOLD
	virtual void					DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

protected:
	//virtual bool					InitialiseImpl(); // from Component::I_GameComponent
	//virtual bool					PrimeImpl(); // from Component::I_GameComponent
	virtual bool					CreateImpl(); // from Component::I_GameComponent
	virtual void					KillImpl(); // from Component::I_GameComponent

private:

	Maths::HVec3					m_RenderOffset;

	void							Serialise(Sys::C_ContextualStream& stream) const;
	void							Deserialise(Sys::C_ContextualStream& stream);

#ifndef CFG_GOLD
	virtual void					AccumulateModelResourceUsage(C_ModelResourceUsage& modelResourceUsage, const WorldRender::C_RenderModelData& renderModelData) const;
#endif

};


struct C_ModelElementRender
{
	C_ModelElementRender(){}
	C_ModelElementRender(Maths::HMat43 &mat, S32 index):m_Matrix(mat),m_ElementIndex(index){}

	Maths::HMat43	m_Matrix;
	S32				m_ElementIndex;

	typedef Btl::Vector<C_ModelElementRender> C_List;
};


class C_ModelSimpleMultiElementRender : public C_RenderModelBase
{
public:
	C_ModelSimpleMultiElementRender();
	~C_ModelSimpleMultiElementRender();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_SimpleMultiElementModelRender;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_SimpleMultiElementModelRender;} // from Component::I_GameComponent

	bool						OnMessage(const Messaging::C_MessageBase *pMessage );			// message handler
	const C_ModelSimpleMultiElementRenderCreate	*GetCreationData(void) const {return static_cast<const C_ModelSimpleMultiElementRenderCreate *>(GetCreator().GetData());}

	void				UpdateModelInstance();
protected:
	//void				BuildElementChildMatrices(const C_ElementState &element,Maths::HMat43 &mat);
	virtual void		KillImpl(); // from Component::I_GameComponent
	virtual bool		CreateImpl(); // from Component::I_GameComponent

	Btl::Vector<Maths::HMat43>	m_ElementMatrices;

private:
#ifndef CFG_GOLD
	virtual void		AccumulateModelResourceUsage(C_ModelResourceUsage& modelResourceUsage, const WorldRender::C_RenderModelData& renderModelData) const;
#endif

};


// Multi Element Render
// class C_ModelMultiElementRender : public C_RenderModelBase
// {
// public:
// 	C_ModelMultiElementRender();
// 	~C_ModelMultiElementRender();
// 
// 	static Component::C_ComponentTypeId		ClassType() {return Components::E_MultiElementRenderModel;}
// 	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MultiElementRenderModel;} // from Component::I_GameComponent
// 
// 	bool						OnMessage(const Messaging::C_MessageBase *pMessage );			// message handler
// 	const C_ModelMultiElementRenderCreate	*GetCreationData(void) const {return static_cast<const C_ModelMultiElementRenderCreate *>(GetCreator().GetData());}
// 	void						Render(const U32 renderPass, const U32 renderStates);	// render function
// 
// protected:
// 	void				BuildElementChildMatrices(const C_ElementState &element,Maths::HMat43 &mat);
// 	virtual void		KillImpl(); // from Component::I_GameComponent
// 	virtual bool		CreateImpl(); // from Component::I_GameComponent
// 
// private:
// };


//////////////////////////////////////////////////////////////////////////
// Element manager render component.
class C_ElementManagerObjectRender : public C_RenderModelBase
{
public:
	C_ElementManagerObjectRender();
	~C_ElementManagerObjectRender();

	static Component::C_ComponentTypeId ClassType() {return Components::E_ElementManagerObjectRender;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return Components::E_ElementManagerObjectRender;}

	bool OnMessage(const Messaging::C_MessageBase *pMessage );
	const C_ElementManagerObjectRenderCreate* GetCreationData(void) const {return static_cast<const C_ElementManagerObjectRenderCreate *>(GetCreator().GetData());}

protected:
	void BuildElementChildMatrices(const C_ElementState &element,Maths::HMat43 &mat);

	// From Component::I_GameComponent
 	virtual void KillImpl();
	virtual bool CreateImpl();

private:
#ifndef CFG_GOLD
	virtual void AccumulateModelResourceUsage(C_ModelResourceUsage& modelResourceUsage, const WorldRender::C_RenderModelData& renderModelData) const;
#endif

};



#if 0
// Text Renderer Component
// -----------------------

class C_TextRender : public C_RenderComponent
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_TextRender;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_TextRender;} // from Component::I_GameComponent

	const C_TextRenderCreate	*GetCreationData(void) const {return static_cast<const C_TextRenderCreate *>(GetCreator().GetData());}

	void						Render(const U32 renderPass, const U32 renderStates);	// render function

protected:
	virtual bool		CreateImpl(); // from Component::I_GameComponent

private:
};
#endif

// Portal Blocker Component
// ------------------------

// Serialised

class C_PortalBlocker : public I_PortalBlocker
{
public:
	C_PortalBlocker();
	virtual ~C_PortalBlocker();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_PortalBlocker;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_PortalBlocker;}

	const C_PortalBlockerCreate* GetCreationData(void) const {return static_cast<const C_PortalBlockerCreate*>(GetCreator().GetData());}

	virtual bool	OnMessage(const Messaging::C_MessageBase* pMessage);

	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	virtual bool	InitialiseImpl();
	virtual bool	CreateImpl();
	virtual void	KillImpl();

private:
	Portal::BlockerHandle*	m_pPortalBlockerHandle;
	bool					m_bOpen;

	void			Serialise(Sys::C_ContextualStream& stream) const;
	void			Deserialise(Sys::C_ContextualStream& stream);

};

} //namespace Components

#endif
#ifndef MG_RENDER_COMPONENTS_H
#define MG_RENDER_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource/TypeRegistry.h>
#include <resource/Resource.h>
#include <game_resource/ModelResource.h>

#include "Game/ActorComponents/GameComponents.h"
//#include "GameActors/GameActor.h"	// for C_Renderable

#include "shared_Actors/Components/Impl/RenderComponents.h"
#include "Game/ActorComponents/Creation/RenderComponentsCreate.h"
#include "shared_Actors/Components/Interfaces/I_RenderComponents.h"

// forward decs
//namespace Portal { class C_Blocker;}
namespace WorldRender
{
	class C_ModelInstance;
	class C_LightSampler;
}

namespace GameResource
{
	class C_ModelResource;
}
namespace Components
{
	class C_ElementState;
}

namespace Components
{
#if 0
// Render component
class C_RenderComponent : public I_RenderComponent//, public C_Renderable
{
public:
	//virtual			~C_RenderComponent(){}
	bool				OnMessage(const Messaging::C_MessageBase *pMessage );			// message handler

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
	Maths::HMat43		m_WorldMatrix;
	Maths::C_Sphere		m_LocalBoundingSphere;

};

// Model Render Base Component Class
// ---------------------------------

// base class
class C_RenderModelBase : public C_RenderComponent
{
public:
									C_RenderModelBase();
	virtual							~C_RenderModelBase();

	bool							OnMessage(const Messaging::C_MessageBase *pMessage );			// message handler
	const C_RenderModelBaseCreate	*GetCreationData(void) const {return static_cast<const C_RenderModelBaseCreate *>(GetCreator().GetData());}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

	// Rendering
	bool							m_HasShadow;
	WorldRender::C_ModelInstance*	m_pModelInstance;
	WorldRender::C_LightSampler*	m_pLightSampler;

protected:
	virtual bool					CreateImpl(); // from Component::I_GameComponent
	virtual void					KillImpl(); // from Component::I_GameComponent

private:
	void							Serialise(Sys::C_ContextualStream& stream) const;
	void							Deserialise(Sys::C_ContextualStream& stream);

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

protected:
	//virtual bool					InitialiseImpl(); // from Component::I_GameComponent
	//virtual bool					PrimeImpl(); // from Component::I_GameComponent
	virtual bool					CreateImpl(); // from Component::I_GameComponent
	virtual void					KillImpl(); // from Component::I_GameComponent

private:

	Maths::HVec3					m_RenderOffset;

	void							Serialise(Sys::C_ContextualStream& stream) const;
	void							Deserialise(Sys::C_ContextualStream& stream);

};

struct C_ModelElementRender
{
	C_ModelElementRender(){}
	C_ModelElementRender(Maths::HMat43 &mat, S32 index):m_Matrix(mat),m_ElementIndex(index){}

	Maths::HMat43	m_Matrix;
	S32				m_ElementIndex;

	typedef Btl::Vector<C_ModelElementRender> C_List;
};

// Multi Element Render
class C_ModelMultiElementRender : public C_RenderModelBase
{
public:
	C_ModelMultiElementRender();
	~C_ModelMultiElementRender();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_MultiElementRenderModel;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MultiElementRenderModel;} // from Component::I_GameComponent

	bool						OnMessage(const Messaging::C_MessageBase *pMessage );			// message handler
	const C_ModelMultiElementRenderCreate	*GetCreationData(void) const {return static_cast<const C_ModelMultiElementRenderCreate *>(GetCreator().GetData());}
	void						Render(const U32 renderPass, const U32 renderStates);	// render function

protected:
	void				BuildElementChildMatrices(const C_ElementState &element,Maths::HMat43 &mat);
	virtual void		KillImpl(); // from Component::I_GameComponent
	virtual bool		CreateImpl(); // from Component::I_GameComponent

private:
};
#endif


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

// Portal Blocker Component
// ------------------------

// Serialised

class C_PortalBlocker : public I_PortalBlocker
{
public:
											C_PortalBlocker();
											~C_PortalBlocker();
	static Component::C_ComponentTypeId		ClassType() {return Components::E_PortalBlocker;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_PortalBlocker;}
	virtual bool							CreateImpl();
	const C_PortalBlockerCreate	*GetCreationData(void) const {return static_cast<const C_PortalBlockerCreate *>(GetCreator().GetData());}

	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage );

	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

private:
	Portal::C_Blocker *m_pBlocker;
	bool	m_bOpen;

	void			Serialise(Sys::C_ContextualStream& stream) const;
	void			Deserialise(Sys::C_ContextualStream& stream);

};
#endif
} //namespace Components

#endif
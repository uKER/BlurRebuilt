#ifndef MG_RENDER_COMPONENTS_CREATE_H
#define MG_RENDER_COMPONENTS_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource_utils/ResourceUtilsDeclare.h>
#include "Game/ActorComponents/GameComponents.h"
#include "shared_actors/Components/Creation/RenderComponentsCreate.h"

namespace Components
{

#if 0
// Render component create base class
class C_RenderComponentCreateInfo : public I_GameComponentCreator
{
public:
	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);

	Maths::HMat43					m_WorldMatrix;
	Maths::C_Sphere					m_BoundingSphere;
	bool							m_bStartHidden;
};

// Model Render Base Component Class
// ---------------------------------

// create info
class C_RenderModelBaseCreate : public C_RenderComponentCreateInfo
{
public:
	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);

	bool							m_HasShadow;
};

// Model Render Component
// -----------------------------

// create info
class C_ModelSingleElementRenderCreate : public C_RenderModelBaseCreate
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_SingleElementRenderModel;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_SingleElementRenderModel;}

	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);
public:

	S32								m_ModelIndex;
	S32								m_ModelElementIndex;
	MathsStorage::Float3			m_RenderOffset;

};
#endif

// Multi Element Render Component
// ------------------------------

class C_ModelMultiElementRenderCreate : public C_RenderModelBaseCreate
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MultiElementRenderModel;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MultiElementRenderModel;}
	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);
public:
	S32							m_ModelIndex;
	bool						m_Pivoted;
};


#if 0

// Text Renderer Component
// -----------------------

class C_TextRenderCreate : public C_RenderComponentCreateInfo
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_TextRender;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_TextRender;}
public:
	Util::C_String		m_Text;
	F32					m_TextSize;
	Maths::RGBA			m_TextColour;
};

// Portal Blocker
// --------------

class C_PortalBlockerCreate : public I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_PortalBlocker;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_PortalBlocker;}

public:
	XtBoundingBox		m_BoundingBox;
	bool				m_bStartClosed;
};
#endif

} //namespace Components

#endif
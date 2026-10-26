#ifndef RENDER_COMPONENTS_CREATE_H
#define RENDER_COMPONENTS_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource_utils/ResourceUtilsDeclare.h>
#include <shared_actors/Components/SharedComponents.h>

namespace Components
{

// Render component create base class
class C_RenderComponentCreateInfo : public I_GameComponentCreator
{
public:
	C_RenderComponentCreateInfo();
	virtual ~C_RenderComponentCreateInfo();

	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);

	Maths::HMat43				m_WorldMatrix;
	Maths::C_Sphere				m_BoundingSphere;
	bool						m_bStartHidden;
};

// Model Render Base Component Class
// ---------------------------------

// create info
class C_RenderModelBaseCreate : public C_RenderComponentCreateInfo
{
public:
	C_RenderModelBaseCreate();
	virtual ~C_RenderModelBaseCreate();

	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);

	bool						m_HasShadow;
	bool						m_IgnorePortals;
	F32							m_DrawDistance;
};

// Model Render Component
// -----------------------------

// create info
class C_ModelSingleElementRenderCreate : public C_RenderModelBaseCreate
{
public:
	C_ModelSingleElementRenderCreate();
	virtual ~C_ModelSingleElementRenderCreate();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_SingleElementRenderModel;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_SingleElementRenderModel;}

	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);
public:

	S32								m_ModelIndex;
	S32								m_ModelElementIndex;
	MathsStorage::Float3			m_RenderOffset;

};

// Multi Element Render Component
// ------------------------------

class C_ModelSimpleMultiElementRenderCreate : public C_RenderModelBaseCreate
{
public:
	C_ModelSimpleMultiElementRenderCreate();
	virtual ~C_ModelSimpleMultiElementRenderCreate();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_SimpleMultiElementModelRender;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_SimpleMultiElementModelRender;}
	virtual void				Serialise(Sys::C_ContextualStream& stream) const;
	virtual void				Deserialise(Sys::C_ContextualStream& stream);
public:
	S32							m_ModelIndex;
};


// Element manager render
class C_ElementManagerObjectRenderCreate : public C_RenderModelBaseCreate
{
public:
	C_ElementManagerObjectRenderCreate();
	virtual ~C_ElementManagerObjectRenderCreate();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_ElementManagerObjectRender;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_ElementManagerObjectRender;}
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
#endif

// Portal Blocker
// --------------

class C_PortalBlockerCreate : public I_GameComponentCreator
{
public:
	C_PortalBlockerCreate();
	virtual ~C_PortalBlockerCreate();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_PortalBlocker;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_PortalBlocker;}

public:
	XtBoundingBox		m_BoundingBox;
	bool				m_bStartClosed;
};

} //namespace Components

#endif
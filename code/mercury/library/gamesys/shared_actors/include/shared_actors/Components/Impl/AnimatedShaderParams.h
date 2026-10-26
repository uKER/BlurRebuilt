#ifndef ANIMATED_SHADER_PARAMS_H
#define ANIMATED_SHADER_PARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/Interfaces/I_AnimatedShaderParams.h>
#include <shared_actors/Components/Creation/AnimatedShaderParamsCreate.h>
#include <world_render/MaterialParameter.h>

namespace Components
{

class I_AnimationController;

class C_AnimatedShaderParams : public I_AnimatedShaderParams
{
public:
	struct C_CurveShaderMapping
	{
		S32	m_CurveID;
		WorldRender::MaterialParameter m_MaterialParam; 
	};

	C_AnimatedShaderParams();
	virtual ~C_AnimatedShaderParams();


	static Component::C_ComponentTypeId		ClassType() {return Components::E_AnimatedShaderParams;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_AnimatedShaderParams;} // from Component::I_GameComponent

	const C_AnimatedShaderParamsCreator	*GetCreationData(void) const {return static_cast<const C_AnimatedShaderParamsCreator *>(GetCreator().GetData());}

	bool				OnMessage(const Messaging::C_MessageBase* pMessage);
#ifndef CFG_GOLD
	void				DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const OVERRIDE;
#endif

	// interface methods
	virtual void SetMaterialParameters(WorldRender::C_ModelInstance* pModelInstance) const;

protected:
	virtual bool		CreateImpl(); // from Component::I_GameComponent
	virtual void		KillImpl(); // from Component::I_GameComponent
private:
	const Components::I_AnimationController *GetAnimationControllerIF(void) const;

	Btl::Vector<C_CurveShaderMapping>		m_CurveShaderMapping;	// Vector mapping curves to shader params
	Actor::C_ActorHandle					m_AnimationController;	// Handle of animation controller actor
	S32										m_AnimationObjectID;	// Object ID of which animation object we a bound to

};

}//namespace Components

#endif
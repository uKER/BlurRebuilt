#include "SharedActorsPch.h"
#include <shared_actors/Components/Impl/AnimatedShaderParams.h>

#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <world_render/RenderModelData.h>
#include <world_render/ModelInstance.h>
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/Messages/AnimationMessages.h>
#include <shared_actors/Components/Messages/RenderMessages.h>
#include <shared_actors/Components/Interfaces/I_AnimationController.h>
#include <shared_actors/Components/Interfaces/I_ModelResource.h>


namespace Components
{

C_AnimatedShaderParams::C_AnimatedShaderParams()
{

}

C_AnimatedShaderParams::~C_AnimatedShaderParams()
{

}

bool C_AnimatedShaderParams::CreateImpl()
{

	return true;
}

void C_AnimatedShaderParams::KillImpl()
{

}


bool C_AnimatedShaderParams::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	// we are being controlled by an animation controller
		case ActorMsg::E_AnimationMsg_SetAnimationController:
		{
			m_AnimationObjectID = Messaging::MessageCast<ActorMsg::C_AnimationSetControllerMsg>(pMessage)->GetValue();
			m_AnimationController = Messaging::MessageCast<ActorMsg::C_AnimationSetControllerMsg>(pMessage)->GetActorHandle();

			const Components::I_AnimationController *pAnimationControllerIF = GetAnimationControllerIF();
			const I_ModelResource *pModelResource = (const I_ModelResource *)GetActor()->QueryInterface(Components::E_IF_ModelResource);

			if (pModelResource)
			{
				const WorldRender::C_Model* pModel = &pModelResource->GetRenderModelData()->GetModel(GetCreationData()->m_ModelIndex);
				if (pModel)
				{
					m_CurveShaderMapping.Clear();	// clear mapping list

					if(pAnimationControllerIF)
					{
						bool found = false;
						U32 noNamedFloatCurves = pAnimationControllerIF->GetNumberOfNamedCurves(m_AnimationObjectID);
				
						for(U32 curveNo = 0; curveNo < noNamedFloatCurves; curveNo++)
						{
							C_CurveShaderMapping shaderParam;
							const char *curveName = pAnimationControllerIF->GetNamedCurveName(m_AnimationObjectID,curveNo);
							shaderParam.m_CurveID = pAnimationControllerIF->GetNamedFloatCurveChannelID(m_AnimationObjectID, Util::C_String(curveName));
							shaderParam.m_MaterialParam = pModel->GetParameter(curveName);
							// Check if parameter was found, then add
							if(shaderParam.m_MaterialParam.index != -1)
							{
								found = true;
								m_CurveShaderMapping.PushBack(shaderParam);
							}
						}

						if(found)
							AddSubscriber(GameMessaging::E_GameMsg_Update);
					}
				}
			}
		}
		break;

		// Update
		case GameMessaging::E_GameMsg_Update:
		{
			// tell render component to update its shader params
			SendMessageToComponent(E_IF_Render, E_RenderMsg_UpdateAnimatedShaderParams);
		}
		break;

		default:
			return I_AnimatedShaderParams::OnMessage(pMessage);
	}
	return true;

}

// I_AnimatedShaderParams Interface implementation
void C_AnimatedShaderParams::SetMaterialParameters(WorldRender::C_ModelInstance* pModelInstance) const
{
	const Components::I_AnimationController *pAnimationControllerIF = GetAnimationControllerIF();

	for (S32 i=0; i<m_CurveShaderMapping.Size(); i++)	
	{
		F32 fValue = 0;
		pAnimationControllerIF->GetNamedFloatCurveValue(m_AnimationObjectID,m_CurveShaderMapping[i].m_CurveID,fValue);
		pModelInstance->SetParameter(m_CurveShaderMapping[i].m_MaterialParam, &fValue);
	}

}

// get a const pointer to the animation controller interface
const Components::I_AnimationController *C_AnimatedShaderParams::GetAnimationControllerIF(void) const
{
	const Actor::C_Actor *pActor = GetActorList()->GetActorPtr(m_AnimationController);
	if(pActor)
		return (const Components::I_AnimationController *)pActor->QueryInterface(Components::E_IF_AnimationController);
	else
		return NULL;
}


#ifndef CFG_GOLD
void C_AnimatedShaderParams::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{

}
#endif



}//namespace Components

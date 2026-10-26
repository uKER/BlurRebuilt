#include "stdafx.h"
#include <Game/GameActors/Drivers/DriverAnimationParameters.h>
#include <Game/GameActors/Drivers/AnimationTree.h>

namespace Game
{

	//
// C_AnimationAction
//

C_AnimationAction::C_AnimationAction():
	m_index(-1),
	m_requested(false),
	m_completed(false)
{
}

void C_AnimationAction::Bind(AnimationTree::C_Tree& tree, const CHAR8* name)
{
	m_index = tree.AddAction(name);
}

void C_AnimationAction::Request(F32 requestLifetime)
{
	m_requested = true;
	m_requestLifetime = requestLifetime;
}

void C_AnimationAction::Cancel()
{
	m_requested = false;
	m_completed = true;
}

inline void C_AnimationAction::Update(AnimationTree::C_Tree& tree, F32 deltaTime)
{
	m_completed = tree.IsActionComplete(m_index);
	if (m_completed)
	{
		tree.AcknowledgeActionComplete(m_index);
		m_requested = false;
	}
	else if (m_requested)
	{
		tree.RequestAction(m_index);

		m_requestLifetime -= deltaTime;

		if (m_requestLifetime <= 0.f)
		{
			Cancel();
		}
	}
}

//
// C_DriverAnimationParameters
//

struct C_ActionDef
{
	E_DriverAction	action;
	const CHAR8*	name;
};


struct C_ParamDef
{
	E_DriverParam	param;
	const CHAR8*	name;
};

#define BIND_ANIM_PARAM(x) x.Bind(tree, #x)

void C_DriverAnimationParameters::Bind(AnimationTree::C_Tree& tree)
{
	static const C_ParamDef kParamDefs[] =
	{
		{ E_DriverParam_Steering,			"steering" },
		{ E_DriverParam_SpeedMph,			"speedMph" },
		{ E_DriverParam_CollisionImpulse,	"collisionImpulse" },
		{ E_DriverParam_RandomSeed,			"randomSeed" },
		{ E_DriverParam_VehicleIndex,		"vehicleIndex" },
		{ E_DriverParam_GForceX,			"gForceX" },
		{ E_DriverParam_GForceY,			"gForceY" },
		{ E_DriverParam_GForceZ,			"gForceZ" },
		{ E_DriverParam_JoltX,				"joltX" },
		{ E_DriverParam_JoltZ,				"joltZ" },
		{ E_DriverParam_PredictedTurn,		"predictedTurn" },
		{ E_DriverParam_Roll,				"roll" },
		{ E_DriverParam_Airbourne,			"airbourne" }, 
		{ E_DriverParam_GravityDirX,		"gravityDirX" },
		{ E_DriverParam_GravityDirZ,		"gravityDirZ" },
		{ E_DriverParam_Count, NULL }
	};

	for (const C_ParamDef* p = kParamDefs; p->name != NULL; ++p)
	{
		parameterIndices[p->param] = tree.AddParameter(p->name);
		parameters[p->param] = 0.f;
	}

	static const C_ActionDef kActionDefs[] =
	{
		{ E_DriverAction_Celebrate,				"celebrate",				},
		{ E_DriverAction_ShowFrustration,		"show_frustration",			},
		{ E_DriverAction_LookLeft,				"look_left"					},
		{ E_DriverAction_LookRight,				"look_right"				},
		{ E_DriverAction_GlanceOutWindowLeft,	"glance_out_window_left"	},
		{ E_DriverAction_GlanceOutWindowRight,	"glance_out_window_right"	},
		{ E_DriverAction_Jolt,					"jolt",						},
		{ E_DriverAction_OvertakeOnLeft,		"overtake_on_left",			},
		{ E_DriverAction_OvertakeOnRight,		"overtake_on_right"			},
		{ E_DriverAction_PassedOnLeft,			"passed_on_left"			},
		{ E_DriverAction_PassedOnRight,			"passed_on_right"			},
		{ E_DriverAction_ActivatedPickup,		"activated_pickup"			},
		{ E_DriverAction_PlayCutSceneAnim,		"play_cutscene_anim"		},
		{ E_DriverAction_ChangeGear,			"change_gear"				},
		{ E_DriverAction_EngageHandbrake,		"engage_handbrake"			},
		{ E_DriverAction_ReleaseHandbrake,		"release_handbrake"			},
		{ E_DriverAction_Count, NULL }
	};
	
	for (const C_ActionDef* a = kActionDefs; a->name != NULL; ++a)
	{
		actions[a->action].Bind(tree, a->name);
	}
}

#undef BIND_ANIM_PARAM

void C_DriverAnimationParameters::CancelActions(AnimationTree::C_Tree& tree)
{
	for (U32 i = 0; i < E_DriverAction_Count; ++i)
	{
		actions[i].Cancel();
	}
	tree.ClearAllActionRequests();
}

void C_DriverAnimationParameters::Update(AnimationTree::C_Tree& tree, F32 deltaTime)
{
	PERF_EVENT("DriverActor.UpdateAnimParameters");
	for (U32 i = 0; i < E_DriverParam_Count; ++i)
	{
		tree.SetParameter(parameterIndices[i], parameters[i]);
	}
	
	for (U32 i = 0; i < E_DriverAction_Count; ++i)
	{
		actions[i].Update(tree, deltaTime);
	}
}

} // end Game

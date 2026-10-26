#ifndef DRIVERMANAGER_H
#define DRIVERMANAGER_H

#include <misc_shared/messaging/MessageHandler.h>
#include <btl/containers/vector.h>

namespace Game
{
	class C_ImpactMsg;

	class C_DriverActorCreate;
	class C_DriverActor;
}

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace Vehicle
{
	class C_ModelInput;
}

namespace WorldRender
{
	class C_LightSampler;
}

namespace Messaging
{
	class C_MessageBase;
}

namespace DbUtil
{
	class I_DebugRender;
}

namespace ActorMsg
{
	class C_SetDriverAnimMsg;
}

class C_DriverManager: public Messaging::C_MessageHandler
{
public:
	C_DriverManager();
	virtual ~C_DriverManager();

	static C_DriverManager* Instance()		{ BIZ_ASSERT(s_pInstance); return s_pInstance; }

	void 				SetSceneGraph(SceneGraph::C_SceneGraph* pSceneGraph);

	void 				Clear();
	void 				CreateDriver(U32 driverIndex, const Game::C_DriverActorCreate& createInfo);
	void 				DestroyDriver(U32 driverIndex);

	void 				ResetDriver(U32 driverIndex);

	// Vehicle inputs.
	void 				SetVehicleModelInput(U32 driverIndex, const Vehicle::C_ModelInput& modelInput);
	void 				SetVehicleLightSampler(U32 driverIndex, const WorldRender::C_LightSampler* pVehicleLightSampler);
	void 				SetVehicleCubeMapTexture(const void* pCubeMapTexture);
	void 				SetDynamicSHLightShaderParameters(U32 driverIndex, const F32 additiveSHLightData[16]);

	// Visibility.
	void 				SetDriverVisibility(U32 driverIndex, bool isVisible, U32 viewportIndex = 0);
	void 				SetDriverVisibilityDetectionFlags(U32 driverIndex, U32 flags);

	// Photo mode/cameras.
	void 				PhotoMoveDriver(U32 driverIndex, const Maths::HVec3& offset);
	Maths::C_QuatTrans	GetDriverHeadTransform(U32 driverIndex) const;

	// Update.
	void				UpdateDrivers(F32 deltaTime);

	// Events.
	void 				OnCollision(U32 driverIndex, const Game::C_ImpactMsg* pImpactMsg);
	void 				OnOvertake(U32 overtakerIndex, U32 overtakeeIndex, U32 overtakerRacePosition);
	void 				OnPlayDriverAnimation(U32 driverIndex, const ActorMsg::C_SetDriverAnimMsg* pSetDriverAnimMsg);
	void 				OnFatallyDamaged(U32 driverIndex);
	
	// Interaction with Actor messaging system.
	void				SubscribeToMessages(Messaging::C_MessageSubscriberSystem* pSubscriberSystem);
	virtual bool		MessageHandler(const Messaging::C_MessageBase* pMessage);

#ifndef CFG_GOLD
	void				DebugRender(DbUtil::I_DebugRender* debugRenderPtr) const;
#endif

private:
	Game::C_DriverActor*		GetDriverActor(U32 driverIndex)			{ return driverIndex < (U32)m_driverActors.Size() ? m_driverActors[driverIndex] : NULL; }
	const Game::C_DriverActor*	GetDriverActor(U32 driverIndex) const	{ return driverIndex < (U32)m_driverActors.Size() ? m_driverActors[driverIndex] : NULL; }

	Btl::Vector<Game::C_DriverActor*>	m_driverActors;
	SceneGraph::C_SceneGraph*			m_pSceneGraph;
	const void*							m_pVehicleCubeMapTexture;
	static C_DriverManager*				s_pInstance;
};

#endif


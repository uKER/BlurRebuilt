#ifndef VEHICLE_TEST_ACTOR_H
#define VEHICLE_TEST_ACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <game_resource/ModelResource.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>

#include "racingphysics/racingphysics.h"
#include "racingphysics/debug/draw.h"
#include "racingphysics/debug/colour.h"

#include <Game/GameControls/GameControls.h>

#include <framework/application/ApplicationBase.h>

#include <GameApp.h>

namespace GameResource { class C_ModelElement; }
namespace WorldRender { class C_ModelInstance; class C_LightSampler; }
namespace MGCam { class C_VehicleCamera; }

namespace Game
{
namespace Actors
{

void RegisterCarModelCallback();

class C_VehicleTestActorCreate : public ::Actors::C_PosActorCreate
{
public:
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_VehicleTest;}
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_VehicleTest;}

	Util::C_String m_ModelFilename;
};

// Actor Info
class C_VehicleTestActor : public C_GameActor
{
public:
	C_VehicleTestActor();
	virtual ~C_VehicleTestActor();
	const Maths::HMat43 &GetMatrix() const {return m_WorldMatrix;}
	static const Actor::C_ActorTypeId TypeId()	{ return E_ActorTypeId_VehicleTest; }
	static const Btl::String32 TypeName() { return "VehicleTest"; }

	virtual bool OnMessage(const Messaging::C_MessageBase *pMessage);

	void CreateGravityCar(RacingPhysics::Car& car, Gravity::C_ImportShape* pImportShape, const Maths::HF32 PCREF mass, const Maths::HVec3 PCREF cOfMOffset);
	void CreateModel();
	void DestroyModel();
	void InitCamera();
	void ExitCamera();
	const Maths::HMat43 GetWheelMatrix( const S32 wheelNo ) const;

#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const; // from Actor::C_Actor
#endif

protected:
	virtual bool InitialiseImpl();
	virtual bool CreateImpl();

private:
	Maths::HMat43	m_WorldMatrix;

	GameResource::C_ModelHandle			m_ModelHandle;
	GameResource::C_ModelResource*		m_pModelResource;
	WorldRender::C_ModelInstance*		m_pModelInstance;
	WorldRender::C_ModelInstance*		m_pInteriorModelInstance;
	Btl::Vector<WorldRender::C_ModelInstance*>		m_pWheelInstanceList;
	
	const GameResource::C_ModelElement*	m_pModelElement;

	Gravity::C_Body*					m_pBody;
	WorldRender::C_LightSampler*		m_pLightSampler;

	RacingPhysics::Car*					m_pCar;
	MGCam::C_VehicleCamera*				m_pChaseCamera;
};

} // namespace Actors

} // namespace Game

#endif //  VEHICLE_TEST_ACTOR_H

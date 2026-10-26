#include "stdafx.h"

#include <actor_system/GameComponentFactory.h>

#include "GameComponents.h"
//#include "Creation/RenderComponentsCreate.h"
#include "Impl/RenderComponents.h"
#include "Impl/PhysicsBodyComponent.h"
#include "Game/MGCam/MovementDualSpline.h"

XT_LIBRARY(ComponentCreation)
XT_DEPENDS_ON(SharedComponentCreation)
XT_END_LIBRARY

namespace Components
{


void RegisterGameComponents(void)
{
	Component::C_GameComponentFactory *factory = Component::C_GameComponentFactory::GetInstance();
	assert(factory);

	RegisterSharedComponents();

	// Register Game components here
	
	// Render components
	//factory->Register<C_ModelSingleElementRender>();
	//factory->Register<C_ModelMultiElementRender>();

	factory->Register<C_ModelPhysicsBody>();

	factory->Register<MGCam::C_MovementDualSpline>();

	// dynamic creation data
	//Component::C_ComponentCreatorFactory::Register<C_ModelSingleElementRenderCreate>();
	//Component::C_ComponentCreatorFactory::Register<C_ModelMultiElementRenderCreate>();

	Component::C_ComponentCreatorFactory::Register<C_ModelPhysicsBodyCreateInfo>();

	Component::C_ComponentCreatorFactory::Register<MGCam::C_MovementDualSplineCreate>();

	// Register Components Interface Debug
	factory->RegisterComponentIFName(E_IF_Render, "E_IF_Render");
//	factory->RegisterComponentIFName(E_IF_ElementManager, "E_IF_ElementManager");
}

}
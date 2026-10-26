#ifndef GAME_RESOURCE_PHYSICS_LEVEL_H
#define GAME_RESOURCE_PHYSICS_LEVEL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <gravity/importer/Importer.h>

namespace Gravity
{
	class C_Scene;
	class C_ImportScene;
	class C_ImportShape;
	class C_ImportSceneStaticInstance;
};

namespace GameResource
{

//! C_PhysicsLevel
class C_PhysicsLevel
{
public:
	C_PhysicsLevel();
	~C_PhysicsLevel();

	bool Load( Sys::I_File* pFile );

	Gravity::C_ImportScene* GetMainImportScene() const;
	S32 GetRouteCount() const;
	Gravity::C_ImportScene* GetRouteImportScene( const S32 routeNo ) const;

private:
	Gravity::C_ImportScene*						m_pMainImportScene;
	Btl::Vector<Gravity::C_ImportScene*>		m_pRouteImportScenes;
};

} // namespace GameResource

#endif // GAME_RESOURCE_PHYSICS_LEVEL_H

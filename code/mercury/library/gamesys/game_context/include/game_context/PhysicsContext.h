#ifndef SHARED_ACTORS_PHYSICSCONTEXT_H
#define SHARED_ACTORS_PHYSICSCONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/context/I_Anything.h>
#include <misc_shared/uid/FourCC.h>


namespace Gravity 
{ 
	class C_Scene; 
	struct C_ExternalConstants;
}


namespace GameContext
{

class I_PhysicsContext : public Sys::C_Anything<FOURCC_U32('IAPC')>
{
public:
	virtual Gravity::C_Scene* GetPhysicsScene() const = 0;
	virtual Gravity::C_ExternalConstants* GetDefaultExternalConstants() const = 0;

protected:
	explicit I_PhysicsContext(const Sys::C_AnythingUid typeUid) : C_Super(typeUid) {}

};

class C_PhysicsContext : public Sys::C_Anything<FOURCC_U32('CAPC'), I_PhysicsContext>
{
public:
	explicit C_PhysicsContext(	Gravity::C_Scene* pPhysicsScene, 
								Gravity::C_ExternalConstants* pDefaultExternalConstants ) 
		: m_pPhysicsScene(pPhysicsScene)
		, m_pDefaultExternalConstants(pDefaultExternalConstants) {}

	virtual Gravity::C_Scene* GetPhysicsScene() const {return m_pPhysicsScene;} // from I_PhysicsContext
	virtual Gravity::C_ExternalConstants* GetDefaultExternalConstants() const { return m_pDefaultExternalConstants; } // from I_PhysicsContext

	void SetPhysicsScene(Gravity::C_Scene* p_pPhysicsScene) { m_pPhysicsScene = p_pPhysicsScene; }
	void SetDefaultExternalConstants(Gravity::C_ExternalConstants* p_pDefaultExternalConstants) { m_pDefaultExternalConstants = p_pDefaultExternalConstants; }

private:
	Gravity::C_Scene* m_pPhysicsScene;
	Gravity::C_ExternalConstants* m_pDefaultExternalConstants;
};

} // namespace GameContext


#endif // SHARED_ACTORS_PHYSICSCONTEXT_H

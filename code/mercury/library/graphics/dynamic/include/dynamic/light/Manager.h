#ifndef RENDER_DYNAMICEFFECT_LIGHT_MANAGER_H
#define RENDER_DYNAMICEFFECT_LIGHT_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Light/Enums.h"
#include "Dynamic/Light/Light.h"
#include "Dynamic/ManagerInterface.h"
#include "Render/Occlusion/Occlusion.h"
#include "Render/Utils/ListPool.h"

namespace DynamicEffect
{

class C_Node;
class I_DynamicEffect;

namespace Light
{

class Light;
struct LightDefinition;

class C_Manager : public I_DynamicEffectManager
{
public:

	C_Manager(C_Renderer* pRenderer);
	virtual ~C_Manager();

	LightHandle	CreateLight(const LightDefinition* pDefinition, const Maths::C_QuatTrans& transform, const C_Node *pNode, const Maths::RGBA& colour, const F32 fIntensity = 1.0f, const F32 fDefaultOcclusion = 0.0f);
	void		DestroyLight(Light* pLight);
	void		ReserveLights(const U32 maxLights);
	void		CleanLights();
	
	void		GetExtents(const LightDefinition* pDefinition, const Maths::C_QuatTrans& transform, MathsStorage::Float4A16* pvCentre, MathsStorage::Float4A16* pvSizeCull);

	void		AddLightDefinition(LightDefinition* pDefinition);
	void		FreeLightDefinition(LightDefinition* pDefinition);
	void		FreeLightDefinitions();

	void		SetVertexShader(const E_VertexShader eShader);
	void		SetPixelShader(const E_PixelShader eShader);
	void		Render(const U32 visibilityFlags);

protected:

	void		CreateInstances(Light* pLight);
	void		DestroyInstances(Light* pLight);

	Light*		NewLight();
	void		RemoveLight(Light* pLight);
	void		FreeLights();

	void		GatherQueries(const U32 visibilityFlags);
	void		ProcessQueries();
	void		RemoveQueries(Light* pLight);
	void		ReceiveVisibility(Light* pLight);
	void		RequestVisibility(Light* pLight);

protected:

	virtual void				CleanNode(const C_Node* pNode);
	virtual void				CleanEffect(const I_DynamicEffect* pEffect);

	virtual bool				Create();
	virtual bool				CreateVolatile();
	virtual bool				Initialise();
	virtual void				Destroy();
	virtual void				DestroyVolatile();
	virtual void				Shutdown();
	virtual void				Reset();
	virtual void				SetShaderConstants() const;

	// 3D debug graphics
	virtual void				DisplayDebugGraphics() const;
	// 2D overlays etc
	virtual MathsStorage::Int2	DisplayDebugInfo(MathsStorage::Int2 position) const;

private:

	typedef Ut::UtSet<LightDefinition*> LightDefinitions;
	LightDefinitions				m_Definitions;

	typedef Ut::ListPool<Light>		LightList;
	LightList						m_LightList;
	
	Lights							m_Lights;

	LightArray						m_QueryArray;
	LightSorter						m_QuerySorter;

	Display::C_VertexShader			m_VertexShader[E_VertexShader_Count];
	Display::C_PixelShader			m_PixelShader[E_PixelShader_Count];
	Display::C_PixelShaderParameter	m_LuminanceParam[E_PixelShader_Count];

	typedef Render::Occlusion::C_Pool<Render::Occlusion::C_PercentageQuery> OcclusionQueries;
	OcclusionQueries				m_OcclusionQueries;

};

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_MANAGER_H

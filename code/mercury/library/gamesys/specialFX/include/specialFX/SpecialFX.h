#ifndef SPECIALFX_H
#define SPECIALFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>
#include <resource_utils/ResourceUtilsDeclare.h>

namespace SceneGraph
{
	class C_VisibleDetect;
}

namespace Sys
{
	class I_ContextProvider;
}

namespace SpecialFX
{
//Parameter ID Enum
enum E_ParamID
{
	E_ParamID_SetCameraViewport,
	E_ParamID_SetMeshViewFlags, // U32[2] Bits values & mask
	E_ParamID_KillMeshEffect, //No param, it needs to be call before update
	E_ParamID_CalculateMeshAxisBox, //Param: A zero size pointer to an Maths::C_AxisBox
	E_ParamID_EnableTransformUpdate, // bool. True to enable, false to disable
	E_ParamID_ImmediateUpdate, // No param, update the matrix immediately
};


//////////////////////////////////////////////////////////////////////////
// Special Effect Interface Instance
//////////////////////////////////////////////////////////////////////////
/*class I_SpecialFXInstance
{
public:
	virtual ~I_SpecialFXInstance() {};
	virtual void Start() = 0;
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix) = 0;
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity) = 0;
	virtual void Stop() = 0;
	virtual bool Update(F32 time) = 0;
	virtual F32	 GetDuration() = 0;
	virtual void SetParameter(U32 paramID, void* pData) = 0;
	virtual void UpdateIntensity(F32 intensity) = 0;
};*/

struct C_ComponentDataSpecialFX
{
	bool	m_Started;
	float	m_SpeedFactor;
	float	m_StartOffset;
};

class I_SpecialFXComponentInstance;
class C_SpecialFX;

class I_SpecialFXInstance
{
	//Vector with the component instance
	Btl::Vector<I_SpecialFXComponentInstance*>	m_ComponentInstanceVector;
	//data component vector 
	Btl::Vector<C_ComponentDataSpecialFX>		m_ComponentDataInstanceVector;
	//Started 
	bool	m_Started;
	//Create internal component instances
	void CreateComponentInstances(const C_SpecialFX& specialFXRecord, const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag, SceneGraph::C_VisibleDetect* pVisibilityDetect = NULL);

	friend class C_SpecialFXManager;

public:
	I_SpecialFXInstance();
	~I_SpecialFXInstance();
	//SpecialFXInstance interface
	void Start();
	void UpdateWorldMatrix(const Maths::HMat43& worldMatrix);
	void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal);
	void Stop();
	bool Update(F32 timeStep);
	F32	 GetDuration();
	void SetParameter(U32 paramID, void* pData);
	void UpdateIntensity(F32 intensity);
};

INLINE void I_SpecialFXInstance::UpdateWorldMatrix(const Maths::HMat43& worldMatrix)
{
	UpdateWorldMatrix(worldMatrix, Maths::HVec3(), Maths::HVec3(), Maths::C_Normal());
}

class I_SpecialFXComponentInstance
{
public:
	virtual ~I_SpecialFXComponentInstance() {};
	virtual void Start() = 0;
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal) = 0;
	virtual void Stop() {};
	virtual bool Update(F32 time) = 0;
	virtual F32	 GetDuration() = 0;
	virtual void SetParameter(U32 /*paramID*/, void* /* pData*/) {};
	virtual void UpdateIntensity(F32 /*intensity*/) {};
	virtual void SetVisibilityDetect(SceneGraph::C_VisibleDetect* /*pVisibilityDetect*/) {};
};

//////////////////////////////////////////////////////////////////////////
// Sorcery Classes
//////////////////////////////////////////////////////////////////////////

struct C_HermiteKeyFrame
{
	XtF32										mTime,
												mValue,
												mInTangent,
												mOutTangent;
};

struct C_HermiteCurve
{
	XtArray<C_HermiteKeyFrame>					mKeyFrames;
};


class C_BaseSpecialFXComponent
{
public:
	XtF32											m_SpeedFactor;
	XtF32											m_StartOffset;

	// Enumerate required resources
	virtual void AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const;
private:
	virtual I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const { return NULL;};

	friend class C_SpecialFXManager;
	friend class I_SpecialFXInstance;
};

struct C_SpecialFXComponentData
{
	XtF32											m_SpeedFactor;
	XtF32											m_StartOffset;
	XtHandle<C_BaseSpecialFXComponent>				m_Component;
};

class C_SpecialFX
{
public:
	// Enumerate required resources
	void AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const;
	XtArray< C_SpecialFXComponentData >			m_Components;
};


//Init Param ID Enum
enum E_InitParamID
{
	E_InitParamID_PostResolveRenderPass,
	E_InitParamID_CubemapViewFlag,
	E_InitParamID_ReflectionViewFlag,
	E_InitParamID_MirrorViewFlag,
	E_InitParamID_Count
};
//////////////////////////////////////////////////////////////////////////
//	Special Effects Manager
//////////////////////////////////////////////////////////////////////////
class C_SpecialFXManager
{
	static void* ms_InitParamVector[E_InitParamID_Count];

public:
	static void SetInitParam(E_InitParamID initParamID, void* pInitParam);
	static void* GetInitParam(E_InitParamID initParamID);

	static I_SpecialFXInstance* CreateSpecialFXInstance(const C_SpecialFX& specialFXRecord, const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag, SceneGraph::C_VisibleDetect* pVisibilityDetect = NULL);
};

//extern C_SpecialFXManager* g_SpecialFXManager;

//C_SpecialFXManager* SpecialFXManager();
//void InitSpecialFXManager(C_SpecialFXManager* pSpecialFXManager);

bool SfxIsLanguageCode(const char* szCode);
bool SfxIgnoreLanguageCode(const char* szCode);
void SfxSetLanguageCode(const char* szCode);

}
#endif

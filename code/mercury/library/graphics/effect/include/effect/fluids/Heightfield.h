#ifndef EFFECT_FLUIDS_HEIGHTFIELD_H
#define EFFECT_FLUIDS_HEIGHTFIELD_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Fluids/HeightfieldTasks.h"
#include "System/Scheduler/JobQueue.h"

namespace Effect
{

static const U32 c_HeightfieldIterations = 2;

class C_HeightfieldCpuSolver;
class C_HeightfieldGpuSolver;
class C_HeightfieldSurface;

class C_HeightfieldSurface
{
	friend class C_HeightfieldCpuSolver;
	friend class C_HeightfieldGpuSolver;
public:

	C_HeightfieldSurface();
	~C_HeightfieldSurface();

	bool CreateVolatile();
	void DestroyVolatile();

	const Display::C_Texture2D*	GetTexture() const { return &m_pCurrentTexture->GetTexture(); }

private:

	Display::C_RenderTexture2D	m_HeightTextures[c_HeightfieldIterations + 1];

	mutable Display::C_RenderTexture2D*	m_pPreviousTexture;
	mutable Display::C_RenderTexture2D*	m_pCurrentTexture;
	mutable Display::C_RenderTexture2D*	m_pNextTexture;

	F32							m_fSurfaceSize; // size of the surface, in metres
	U32							m_iTextureSize; // size of the texture, in pixels

};

class C_HeightfieldCpuSolver
{
public:

	C_HeightfieldCpuSolver();
	~C_HeightfieldCpuSolver();

	bool Create();
	void Destroy();

	void SetPolicy(const Scheduler::PolicyHandle& hPolicy) { m_hPolicy = hPolicy; }

	void ClearSurface(C_HeightfieldSurface* pSurface) const;
	void ModifySurface(C_HeightfieldSurface* pSurface, Display::C_Texture2D* pTexture, const Display::C_Float4& vTransformU, const Display::C_Float4& vTransformV, const F32 fHeight = 1.0f);
	void SolveSurface(C_HeightfieldSurface* pSurface, const F32 fTimeStep);
	void SolveSurface(C_HeightfieldSurface* pSurface, Display::C_Texture2D* pTop, Display::C_Texture2D* pBottom, Display::C_Texture2D* pLeft, Display::C_Texture2D* pRight, const F32 fTimeStep);

	void ModifyWait() const;
	void SolveWait() const;

private:

	Scheduler::GroupHandle		m_hModifyJob;
	Scheduler::GroupHandle		m_hSolveJob;
	Scheduler::PolicyHandle		m_hPolicy;
	
	HeightfieldModifyData		m_ModifyData;
	HeightfieldSolveData		m_SolveData;

};

class C_HeightfieldGpuSolver : public Display::I_Renderer
{
public:

	enum E_Shader
	{
		E_Shader_Inside = 0,
		E_Shader_Top,
		E_Shader_Bottom,
		E_Shader_Left,
		E_Shader_Right,
		E_Shader_Count,
	};

public:

	C_HeightfieldGpuSolver();
	~C_HeightfieldGpuSolver();

	bool OnCreate();
	void OnDestroy();

	void ClearSurface(const C_HeightfieldSurface* pSurface) const;

	void ModifySurface(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pTexture, const Display::C_Float4& vTransformU, const Display::C_Float4& vTransformV, const F32 fHeight = 1.0f) const;
	void SolveSurface(const C_HeightfieldSurface* pSurface, const F32 fTimeStep);
	void SolveSurface(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pTop, const Display::C_Texture2D* pBottom, const Display::C_Texture2D* pLeft, const Display::C_Texture2D* pRight, const F32 fTimeStep);
	
	void SolveInterior(const C_HeightfieldSurface* pSurface, const F32 fTimeStep) const;
	void SolveTop(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pTop, const F32 fTimeStep) const;
	void SolveBottom(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pBottom, const F32 fTimeStep) const;
	void SolveLeft(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pLeft, const F32 fTimeStep) const;
	void SolveRight(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pRight, const F32 fTimeStep) const;
	void SolveEdge(const C_HeightfieldSurface* pSurface, const Display::C_Texture2D* pEdge, const E_Shader eShader, const F32 x1, const F32 y1, const F32 x2, const F32 y2, const F32 du, const F32 dv, const F32 fTimeStep) const;

	void BindSolver(const C_HeightfieldSurface* pSurface) const;
	void UnbindSolver(const C_HeightfieldSurface* pSurface) const;

private:

	Display::C_PixelShader			m_ModifyPixelShader;
	Display::C_PixelShader			m_SolvePixelShader[E_Shader_Count];
	Display::C_PixelShaderParameter	m_ModifyParameter;
	Display::C_PixelShaderParameter	m_ModifyUParameter;
	Display::C_PixelShaderParameter	m_ModifyVParameter;
	Display::C_PixelShaderParameter	m_SolveParameter[E_Shader_Count];
	mutable Display::C_RenderTarget	m_SavedRenderTarget;

};

}; // namespace Effect

#endif // EFFECT_FLUIDS_HEIGHTFIELD_H

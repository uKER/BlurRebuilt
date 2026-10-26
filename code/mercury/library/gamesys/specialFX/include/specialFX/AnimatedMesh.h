#ifndef ANIMATEDMESH_H
#define ANIMATEDMESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>
#include <SpecialFX/SpecialFX.h>

namespace SpecialFX
{

class C_PostUpdateCallBack
{
public:
	virtual void PostUpdate() = 0;
};

class C_InternalSpecialFX
{
public:
	XtSz8					m_AnimationNodeName;
	XtHandle<C_SpecialFX>	m_SpecialFX;
};

class C_AnimatedMeshSpecialFX : public C_BaseSpecialFXComponent
{
public:
	C_AnimatedMeshSpecialFX();

	//XT Data
	XtSz8										m_AnimationFileName;
	XtSz8										m_AnimatedMeshFileName;
	XtSz8										m_SkeletonFileName;
	XtF32										m_FarDistance;
	XtArray<C_InternalSpecialFX>				m_InternalSpecialFXVector;
	bool										m_ExcludeFromCubemap;
	bool										m_ExcludeFromReflection;
	bool										m_ExcludeFromMirror;


	// inherited from C_BaseSpecialFXComponent
	virtual void AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const; 

	static void CreatePostUpdateSystem();
	static void DestroyPostUpdateSystem();
	static void PostUpdate();
	static S32 PushPostUpdate(C_PostUpdateCallBack* pPostUpdate);	// returns index
	static void ClearPostUpdate(S32 index);

	static void RegisterModelType();

private:
	virtual I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;
	bool HasSkeleton() const;

	//Post Update System
	static Btl::Vector<C_PostUpdateCallBack* > s_PostUpdateCallBackVector;
};
}

#endif

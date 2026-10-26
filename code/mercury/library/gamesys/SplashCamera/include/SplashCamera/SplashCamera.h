#ifndef SPLASHCAMERA_H
#define SPLASHCAMERA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include <Btl/Containers/List.h>
#include <btl/Allocators/pool.h>
#include <display/utils/Renderer.h>
#include <display/buffer/VertexDeclarations.h>
#include <TextureResource/TextureResource.h>
#include <SceneGraph/RenderNode.h>
#include <SceneGraph/GatherNode.h>
#include <SceneGraph/RendererView.h>
#include <SceneGraph/RendererInterface.h>
#include <Display/context/SamplerState.h>
#include <Xt/XtGame.h>
#include "misc_shared/Curve/HermiteCurve.h"
#include <resource_utils/ResourceBagUtils.h>
#include <System/misc/Random.h>

namespace SceneGraph
{
	class C_SceneGraph;
}

namespace SplashCamera
{
	//Sorcery record
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

	struct C_SplashType
	{
		XtF32										m_LifeTime;
		XtF32										m_DistributionLifeTime;
		XtF32										m_InitScreenCentrePosition;
		XtF32										m_InitScreenVerticalPosition;
		XtF32										m_InitOverCarFactor;
		XtF32										m_Size;
		XtF32										m_DistributionSize;
		XtF32										m_Rotate;
		XtF32										m_DistributionRotate;
		XtArray<C_HermiteKeyFrame>					m_FadeGraph;	
		XtArray<C_HermiteKeyFrame>					m_DistributionFadeGraph;
		XtArray<C_HermiteKeyFrame>					m_SpeedGraph;	
		XtArray<C_HermiteKeyFrame>					m_DistributionSpeedGraph;
		XtF32										m_SpeedFactor;
		XtSz8										m_SplashTextureName;
		TextureResource::C_TextureHandle			m_SplashTextureHandle;
		U32											m_TextureArraySize;

		XtSz8										m_DistorsionTextureName;
		TextureResource::C_TextureHandle			m_DistorsionTextureHandle;
		XtF32										m_RefractionIntensity;
		XtF32										m_DistorsionIntensity;

		XtSz8										m_NormalTextureName;
		TextureResource::C_TextureHandle			m_NormalTextureHandle;
		XtF32										m_AmbientIntensity;
		XtF32										m_LightIntensity;

		Curve::C_HermiteCurve						m_FadeCurve;
		Curve::C_HermiteCurve						m_DistributionFadeCurve;
		Curve::C_HermiteCurve						m_SpeedCurve;
		Curve::C_HermiteCurve						m_DistributionSpeedCurve;

		virtual void AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const; 
	};


	

	// we integrate this into the scenegraph using both a gather node and a renderer
	class C_SplashCamera : public Display::I_Renderer, public SceneGraph::I_RenderNode, public SceneGraph::I_GatherNode
	{
		class C_Splash
		{
			//Update
			Maths::HVec2			m_Position;
			F32						m_Time;
			F32						m_Fade;
			Maths::HVec2			m_Direction;
			F32						m_LifeTime;
			F32						m_DistanceFade;
			F32						m_DistributionSpeedDeviation;
			F32						m_DistributionFadeDeviation;

			//Render
			F32			m_Size;
			U32			m_TextureArrayIndex;
			F32			m_Rotate;

			friend class C_SplashCamera;
		};

		typedef Btl::List<C_Splash, Btl::ListParameters_Pool<> > SplashList;

		struct C_FastAccessSplash
		{
			C_SplashType*					m_pSplashType;
			SplashList						m_SplashList;
		};
		
		struct C_ViewSettings
		{
			C_ViewSettings()
			{
				m_pLightCubeTexture = NULL;
			}
			Maths::HMat43					m_CameratoWorldMatrix;
			Maths::HVec3					m_CameraSpeed;
			Maths::HMat43					m_WorldtoCameraMatrix;
			F32								m_CameraSpeedFactor;
			Maths::HVec2					m_CameraSlideFactor;
			Btl::Vector<C_FastAccessSplash>	m_FastAccessSplash;
			const Display::C_TextureCube*	m_pLightCubeTexture;
		};

		Btl::Vector<C_ViewSettings>				m_ViewSettings;

		Display::C_VertexShader					m_SplashCameraVertexShader;
		Display::C_PixelShader					m_SplashCameraPixelShader;
		Display::C_VertexShader					m_SplashCameraRefractVertexShader;
		Display::C_PixelShader					m_SplashCameraRefractPixelShader;
		Display::C_PixelShader					m_SplashCameraLightPixelShader;

		Display::C_PixelShaderParameter			m_SplashCameraRefractParameter;
		Display::C_PixelShaderParameter			m_SplashCameraLightParameter;
		Display::C_PixelShaderParameter			m_SplashCameraLightCameraMatrixXParameter;
		Display::C_PixelShaderParameter			m_SplashCameraLightCameraMatrixYParameter;
		Display::C_PixelShaderParameter			m_SplashCameraLightCameraMatrixZParameter;

		U32										m_RenderPass;
		U32										m_VisibilityFlags;
		bool									m_bCreated;
		volatile bool							m_resourcesPending;

		Display::C_SamplerState					m_SamplerState;
		Display::C_SamplerState					m_MirrorSamplerState;

		Util::C_Random							m_RandomGenerator;

		const Display::C_Texture2D*				m_pRefractionTexture;
		
		bool UpdateSplash(const C_ViewSettings& viewSettings, const F32& timeStep, C_Splash* pSplash, C_SplashType* pSplashType);

	public:
		C_SplashCamera();
		virtual ~C_SplashCamera();

		bool	OnCreate();
		void	OnDestroy();

		// setup
		void	SetRenderPass(const U32 renderPass);
		void	SetNumViewSettings(const S32 numViews);
		void	SetVisibilityFlags(const U32 viewFlags);
		
		// scene graph I_GatherNode implementation
		void	Accept(SceneGraph::I_Visitor& visitor) const;
		void	OnGather(const SceneGraph::C_View& view) const;
		
		//I_RenderNode
		void	OnRender(SceneGraph::C_Context& context) const;

		void	Render (SceneGraph::C_Context& context) const;

		bool	IsCreated() { return m_bCreated; }

		void	Update(const U32 viewIndex, const F32 timeStep, const Maths::HMat43& cameraMatrix, const Maths::HVec3& cameraSpeed);

		void	AddSplash (U32 viewIndex, C_SplashType* pSplashType, U32 count, const Maths::HVec3& position, const Maths::HVec3& velocity, F32 spread, const ResourceUtils::I_ResourceBag* pResourceBag, bool cameraFadeDisabled = false);

		void	Clear();

		void	SetRefractionTexture(const Display::C_Texture2D* pRefractionTexture) {m_pRefractionTexture = pRefractionTexture;};
		void	SetLightCubeTexture(U32 viewIndex, const Display::C_Texture2D* pLightCubeTexture);
	};

	INLINE void C_SplashCamera::SetRenderPass(const U32 renderPass)
	{
		m_RenderPass = renderPass;
	}

	INLINE void C_SplashCamera::SetNumViewSettings(const S32 numViews)
	{
		 m_ViewSettings.Resize(numViews);
	}

	INLINE void C_SplashCamera::SetVisibilityFlags(const U32 visibilityFlags)
	{
		m_VisibilityFlags = visibilityFlags;
	}
}

#endif

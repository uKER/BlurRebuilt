#ifndef FRAMEWORK_TESTSECTION_ANIMATION_H
#define FRAMEWORK_TESTSECTION_ANIMATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <testsections/TestSection.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/cycle/Cycle.h>
#include <render/debug/DebugRender.h>
#include <display/buffer/DynamicVertexBuffer.h>
#include "ActorList.h"

namespace Framework
{
	class C_TestSectionAnimation : public C_TestSection
	{
	public:

		C_TestSectionAnimation(const Util::C_String& name);
		~C_TestSectionAnimation();

		virtual bool OnCreate();
		virtual void OnDestroy();
		virtual void OnUpdate();
		virtual void OnRender3DViewWithAA(const S32 viewIndex);

	protected:
	
		void StartRenderTasks();
		void FinishRenderTasks();
		void RenderSkeletons();
	
	
		static const U32 MAX_CYCLES = 8;
	
		C_ActorList m_actorList;
		
		Display::C_Texture2D m_gridTexture;
		Display::C_Texture2D m_boneTexture;
		Display::C_DynamicVertexBuffer m_dynamicVertexBuffer;
		
		U32 m_vertexBufferOffset;
		bool m_renderTaskPending;
		bool m_vertexBufferLocked;
		
		U32 m_cameraTypeIndex;
		Scheduler::GroupHandle m_skeletonRenderGroup;
		
		Display::C_VertexDeclaration* m_pSkeletonVertexDecl;
		
		
	};

}

#endif

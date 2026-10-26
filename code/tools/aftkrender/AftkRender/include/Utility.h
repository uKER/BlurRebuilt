#ifndef AFTK_RENDER_UTILS_INCLUDE
#define AFTK_RENDER_UTILS_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

#include "camera/Camera.h"

namespace AftkRender
{

	// Material IDs from AftkSpeedtree.h)
	const int CGFX_BASE = 0x486a53f1;
	const int CGFX_SPEEDTREE_BRANCH = CGFX_BASE + 10;
	const int CGFX_SPEEDTREE_FROND = CGFX_BASE + 11;
	const int CGFX_SPEEDTREE_LEAF = CGFX_BASE + 12;
	const int CGFX_SPEEDTREE_BILLBOARD = CGFX_BASE + 13;
	const int CGFX_SPEEDTREE_BRANCH_ALPHA = CGFX_BASE + 14;
	const int CGFX_GOTHAM_FLARE = CGFX_BASE + 15;

	namespace Utility
	{

		enum E_ImagePostProcess
		{
			E_ImagePostProcess_None,
			E_ImagePostProcess_FloodFill
		};


		bool CaptureRenderTarget(const Util::C_String& fileName, E_ImagePostProcess postProcess = E_ImagePostProcess_None);

		bool CaptureRenderTarget(Attk::C_Image& image);

		// Frames an object in the viewport with a perspective projection
		void FrameObjectPerspective(
			C_Camera& camera,
			const Maths::C_AxisBox& aabbox,
			const Maths::HVec3& viewVector);

		void FrameObjectPerspective(
			C_Camera& camera,
			const Aftk::C_SceneNode* pSceneNode,
			const Maths::HVec3& viewVector);

		// Frames an object in the viewport with an orthographic projection
		void FrameObjectOrthographic(
			C_Camera& camera,
			const Maths::C_AxisBox& aabbox,
			const Maths::HVec3& viewVector);

		void FrameObjectOrthographic(
			C_Camera& camera,
			const Aftk::C_SceneNode* pSceneNode,
			const Maths::HVec3& viewVector);

		void GetVertexColourChannelAndType(
			const Aftk::C_SceneMeshNode *pAftkMesh,
			S32& channel,
			Aftk::E_GenericAttributeType& type);

		U32 GetLightMapUVSet(const Aftk::C_SceneMeshNode *pAftkSceneMeshNode);
	}
}

#endif
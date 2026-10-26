
#include "stdafx.h"
#include <camera2/export/AftkToAnimData.h>
#include <camera2/tv/AnimData.h>

#pragma warning (push,3)
#include <aftk/Aftk.h>
#pragma warning (pop)
#include <core/maths/hvec/new/IncludeGeneral.h>
#include <core/maths/hvec/new/IncludeGeneral.inl>

namespace Cam2
{

static void PosAndOrient(const Aftk::Matrix44& amat, Maths::HVec3& position, Maths::C_Quaternion& orient)
{
	// Convert to HMat43:
	Maths::HMat43 mat43;
	Aftk::TemplateToNightmareMatrix43(mat43, amat);
	position = mat43.AW();

	// Decompose to get rid of any scaling or shearing:
	Maths::ScaleMat3 scale;
	Maths::ShearMat3 shear;
	Maths::Mat33 pureRotation;
	Maths::DecomposeScaleShearRotate( mat43.Rotation(), &scale, &shear, &pureRotation);
	orient = Maths::C_Quaternion(pureRotation);
}

static bool EqualFrames(const C_AnimFrame& fa, const C_AnimFrame& fb)
{
	if (
		fa.m_position.X() != fb.m_position.X() ||
		fa.m_position.Y() != fb.m_position.Y() ||
		fa.m_position.Z() != fb.m_position.Z()
	)
	return false;

	if (
		fa.m_orient.Real().W() != fb.m_orient.Real().W() ||
		fa.m_orient.Imag().X() != fb.m_orient.Imag().X() ||
		fa.m_orient.Imag().Y() != fb.m_orient.Imag().Y() ||
		fa.m_orient.Imag().Z() != fb.m_orient.Imag().Z()
	)
	return false;

	// See if any attributes changed:	
	if (fa.m_attr.Size() != fb.m_attr.Size())
		return false;
	S32 i = 0;
	for (i = 0; i < fa.m_attr.Size(); i++)
		if (fa.m_attr[i] != fb.m_attr[i])
			return false;

	return true;
}

static bool GetAttributeValue(const Aftk::C_SceneNode* node, const char* name, F32& value)
{
	const Aftk::I_Plug* plug = node->GetPlugPtr(name);
	if (plug == 0)
		return false;

	if (!plug->GetValue(value))
	{
		F64 val64 = 0.f;
		if (!plug->GetValue(val64))
			return false;

		value = (F32)val64;
	}
	return true;
}

static void ReadAnimationData(Aftk::C_Scene* scene, const Aftk::C_SceneNode* source, const Aftk::C_SceneNode* instance, C_AnimData* panim)
{
	if (source->GetType() != Aftk::E_ObjType_SceneCamera)
		return;

	Aftk::C_SceneCameraNode* camera = (Aftk::C_SceneCameraNode*)source;
	
	// Find out seconds per frame:
	F32 spf = scene->GetFrameRate();
	if (spf > 0.0001f)
		spf = 1.f / spf;

	F32 start = scene->GetStartTime();
	F32 end   = scene->GetEndTime();
	
	C_AnimFrame frame, prevFrame;
	F32 lastPush = 0.f;

	F32 t = 0.f;
	for (t = start; t <= end; t += 1.f)
	{
		scene->SetCurrentTime(t);

		// Find out information about frame:
		frame.m_time = t * spf;
		PosAndOrient(instance->GetWorldMatrix(), frame.m_position, frame.m_orient);
		
		frame.SetAttr(E_Attr_Fov, camera->GetVerticalFieldOfView());
		
		F32 value = 0.f;
		if (GetAttributeValue(camera, "objectLook", value))
			frame.SetAttr(E_Attr_ObjectLook, value);

		if (GetAttributeValue(camera, "viewRange", value))
			frame.SetAttr(E_Attr_ViewRange, value);
		
		if (!EqualFrames(prevFrame, frame))
		{
			// Position/orient is changing - push previous frame if we need it 
			if (t > start && prevFrame.m_time > lastPush)
				panim->m_frame.PushBack(prevFrame);
			
			panim->m_frame.PushBack(frame);
			lastPush = frame.m_time;
		}

		prevFrame = frame;
	}

	scene->SetCurrentTime(0.f);
}

// Read the fcurves (or static x,y,z values) from a camera SceneNode and convert into animation data:
void AftkToAnimData(Aftk::C_Scene* scene, const Aftk::C_SceneNode* source, const Aftk::C_SceneNode* instance, C_AnimData* panim)
{
	panim->m_name = instance->GetName().c_str();

	ReadAnimationData(scene, source, instance, panim);
}

} // end of namespace

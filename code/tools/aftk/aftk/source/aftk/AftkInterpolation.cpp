#include <stdafx.h>
#include <Aftk/AftkEuler.h>
#include "AftkInterpolation.h"

using namespace Aftk;

bool Aftk::DoWeSlerpThisNode(
	const C_SceneNode* pSceneNode,
	const I_Object* pRotXSourceConnection,
	const I_Object* pRotYSourceConnection,
	const I_Object* pRotZSourceConnection)
{
	if (!pSceneNode->GetRotationInterpolation() == E_RotationInterpolation_Quaternion)
		return false;
	const I_Plug& rotatePlug = pSceneNode->GetRotatePlug();

	if ((pRotXSourceConnection && pRotXSourceConnection->HasType(Aftk::E_ObjType_IEvaluatable)) &&
		(pRotYSourceConnection && pRotYSourceConnection->HasType(Aftk::E_ObjType_IEvaluatable)) &&
		(pRotZSourceConnection && pRotZSourceConnection->HasType(Aftk::E_ObjType_IEvaluatable)))
		return true;

	return false;
}

void Aftk::BuildRotationMatrix(
	Matrix44& rotationMatrix,
	const C_SceneNode* pSceneNode,
	const I_Object* pRotXSourceConnection, //Optional: x animation curve/blend weighted
	const I_Object* pRotYSourceConnection, //Optional: y animation curve/blend weighted
	const I_Object* pRotZSourceConnection) //Optional: z animation curve/blend weighted
{
	if (!DoWeSlerpThisNode(
		pSceneNode,
		pRotXSourceConnection,
		pRotYSourceConnection,
		pRotZSourceConnection))
	{
		Vector3F32 euler;
		if (pRotXSourceConnection && pRotXSourceConnection->HasType(Aftk::E_ObjType_IEvaluatable))
			((Aftk::I_EvaluatableNode*)pRotXSourceConnection)->Evaluate(euler[0]);
		else 
			pSceneNode->GetRotatePlug().GetChildPtr(0)->GetValue(euler[0]);

		if (pRotYSourceConnection && pRotYSourceConnection->HasType(Aftk::E_ObjType_IEvaluatable))
			((Aftk::I_EvaluatableNode*)pRotYSourceConnection)->Evaluate(euler[1]);
		else 
			pSceneNode->GetRotatePlug().GetChildPtr(1)->GetValue(euler[1]);

		if (pRotZSourceConnection && pRotZSourceConnection->HasType(Aftk::E_ObjType_IEvaluatable))
			((Aftk::I_EvaluatableNode*)pRotZSourceConnection)->Evaluate(euler[2]);
		else 
			pSceneNode->GetRotatePlug().GetChildPtr(2)->GetValue(euler[2]);

		EulerToMatrix(
			rotationMatrix,
			euler, 
			pSceneNode->GetRotationOrder());
		return;
	}

	Vector3F32 euler1;
	Vector3F32 euler2;

	//Const-cast here as we're not going to be perminantly modifying the state of the scene, just
	//while in this function.
	C_Scene* pScene = const_cast<C_Scene*>(pSceneNode->GetScenePtr());

	F32 currentTime = pSceneNode->GetScenePtr()->GetCurrentTime();

	F32 distance;
	F32 startX;
	F32 endX;

	bool abortDueToUnevenKeySpacing = false;
	//X
	{
		C_BaseKey key1;
		C_BaseKey key2;
		I_EvaluatableNode* pEvaluatableNode = 
			(I_EvaluatableNode*)pRotXSourceConnection;
		pEvaluatableNode->GetAdjacentKeys(key1, key2);

		distance = Maths::Abs(key1.GetX() - currentTime) + Maths::Abs(key2.GetX() - currentTime);
		startX = key1.GetX();
		endX = key2.GetX();
		euler1[0] = key1.GetValue();
		euler2[0] = key2.GetValue();
	}

	//Y
	{
		C_BaseKey key1;
		C_BaseKey key2;
		I_EvaluatableNode* pEvaluatableNode = 
			(I_EvaluatableNode*)pRotYSourceConnection;
		pEvaluatableNode->GetAdjacentKeys(key1, key2);

		if (!Aftk::EqualityTestFScalar(startX, key1.GetX()))
			abortDueToUnevenKeySpacing = true;

		endX = key2.GetX();
		euler1[1] = key1.GetValue();
		euler2[1] = key2.GetValue();
	}

	//Z
	if (!abortDueToUnevenKeySpacing)
	{
		C_BaseKey key1;
		C_BaseKey key2;
		I_EvaluatableNode* pEvaluatableNode = 
			(I_EvaluatableNode*)pRotZSourceConnection;
		pEvaluatableNode->GetAdjacentKeys(key1, key2);

		if (!Aftk::EqualityTestFScalar(startX, key1.GetX()))
			abortDueToUnevenKeySpacing = true;

		euler1[2] = key1.GetValue();
		euler2[2] = key2.GetValue();
	}

	if (abortDueToUnevenKeySpacing)
	{
		Vector3F32 euler;
		pSceneNode->GetRotatePlug().GetChildPtr(0)->GetValue(euler[0]);
		pSceneNode->GetRotatePlug().GetChildPtr(1)->GetValue(euler[1]);
		pSceneNode->GetRotatePlug().GetChildPtr(2)->GetValue(euler[2]);

		EulerToMatrix(
			rotationMatrix,
			euler, 
			pSceneNode->GetRotationOrder());
		return;
	}

	F32 dx = endX - startX;
	F32 t = 0;
	if (dx > AFTK_EPSILON)
	{
		t = (currentTime - startX) / dx;
		Maths::HMat33 m1;
		Aftk::EulerToMatrix(m1, Aftk::TemplateToNightmare(euler1), pSceneNode->GetRotationOrder());
		Maths::HMat33 m2;
		Aftk::EulerToMatrix(m2, Aftk::TemplateToNightmare(euler2), pSceneNode->GetRotationOrder());
		Maths::C_Quaternion q1(m1);
		Maths::C_Quaternion q2(m2);

		Maths::C_Quaternion q;
		q = q1.ShortestSlerp(q2, t);
		q.Normalise();
		Maths::HMat33 slerped = (Maths::HMat33)q;
		NightmareToTemplateMatrix33(rotationMatrix, slerped);
		return;
	}

	//Don't bother doing quaternion interpolation.
	EulerToMatrix(
		rotationMatrix,
		euler1, 
		pSceneNode->GetRotationOrder());
}

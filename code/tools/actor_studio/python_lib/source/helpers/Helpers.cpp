#include "stdafx.h"

#include <EditorLib/Core.h>
#include <EditorLib/Collision/Ray.h>
#include <EditorLib/Collision/CollisionStructs.h>
#include <EditorLib/Collision/CollisionResult.h>
#include <EditorLib/Collision/CollisionManager.h>
#include <EditorLib/Collision/CollisionScene.h>
#include <EditorLib/Collision/CollisionHelpers.h>

namespace EditorLib
{

C_CollisionManager* GetCollisionManager()
{
	return &C_Core::Instance().GetCollisionManager();
}

void Vector3_SetX(Vector3* v, float x)
{
	v->X(x);
}

void Vector3_SetY(Vector3* v, float y)
{
	v->Y(y);
}

void Vector3_SetZ(Vector3* v, float z)
{
	v->Z(z);
}

float Vector3_GetX(Vector3* v)
{
	return v->X();
}

float Vector3_GetY(Vector3* v)
{
	return v->Y();
}

float Vector3_GetZ(Vector3* v)
{
	return v->Z();
}

}
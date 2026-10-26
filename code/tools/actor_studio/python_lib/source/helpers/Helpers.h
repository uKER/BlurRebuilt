#ifndef PYTHON_LIB_HELPERS_H
#define PYTHON_LIB_HELPERS_H

#include <EditorLib/Misc/Definitions.h>

namespace EditorLib
{
	class C_CollisionManager;

	C_CollisionManager* GetCollisionManager();
	void Vector3_SetX(Vector3* v, float x);
	void Vector3_SetY(Vector3* v, float y);
	void Vector3_SetZ(Vector3* v, float z);
	float Vector3_GetX(Vector3* v);
	float Vector3_GetY(Vector3* v);
	float Vector3_GetZ(Vector3* v);
}

#endif
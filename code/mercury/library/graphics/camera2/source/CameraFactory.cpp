
#include "stdafx.h"
#include <camera2/CameraFactory.h>
#include <camera2/CameraType.h>

#include <camera2/stock/AnimCamera.h>
#include <camera2/stock/BlendCamera.h>
#include <camera2/stock/ChaseCamera2.h>
#include <camera2/stock/CharacterCamera.h>
#include <camera2/stock/FixedCamera.h>
#include <camera2/stock/FlyCamera.h>
#include <camera2/stock/OrbitCamera.h>
#include <camera2/stock/RelativeCamera.h>
#include <camera2/stock/RelativeFlyCamera.h>
#include <camera2/stock/ScriptCamera.h>
#include <camera2/stock/SimpleChaseCamera.h>
#include <camera2/stock/TvCamera.h>

using namespace Maths;

namespace Cam2
{

C_CameraFactory::C_CameraFactory()
{
	// Register mercury types:
	Register<C_AnimCamera>       (E_AnimCamera);
	Register<C_BlendCamera>      (E_BlendCamera);
	Register<C_ChaseCamera2>     (E_ChaseCamera2);
	Register<C_CharacterCamera>  (E_CharacterCamera);
	Register<C_FixedCamera>      (E_FixedCamera);
	Register<C_FlyCamera>        (E_FlyCamera);
	Register<C_OrbitCamera>      (E_OrbitCamera);
	Register<C_RelativeCamera>   (E_RelativeCamera);
	Register<C_RelativeFlyCamera>(E_RelativeFlyCamera);
	Register<C_ScriptCamera>     (E_ScriptCamera);
	Register<C_SimpleChaseCamera>(E_SimpleChaseCamera);
	Register<C_TvCamera>         (E_TvCamera);
}

bool C_CameraFactory::RegisterPrivate(S32 type, CreateFn createFn)
{
	Cinfo& info = m_classMap[type]; // Allocate a new entry at 'type' and return a reference to it
	
	info.m_createFn = createFn;

	return true;
}

C_Camera* C_CameraFactory::Create(S32 type)
{
	ClassMap::Iterator i = m_classMap.Find(type);

	if (i == m_classMap.End())
	{
		return 0;
	}
	
	// Call create function:
	CreateFn cf = i->GetValue().m_createFn;
	C_Camera * cam = cf();

	return cam;
}

// ---------------------------------------------------------------------------------------------

C_CameraFactory g_CameraFactory;

} // End of namespace Cam2

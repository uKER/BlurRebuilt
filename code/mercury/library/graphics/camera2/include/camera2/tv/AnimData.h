
#ifndef CAMERA2_ANIMDATA_H
#define CAMERA2_ANIMDATA_H

#pragma warning (push, 3)
#include <misc_shared/archive/archive.h>
#include <misc_shared/archive/vector.h>
#include <system/serialise/btl.h>
#pragma warning (pop)

namespace Cam2
{

enum E_Attr
{
	E_Attr_Fov = 0,
	E_Attr_ObjectLook,
	E_Attr_ViewRange,
	E_Attr_Count,
};

class C_AnimFrame
{
public:
	F32 m_time; // Time of frame in seconds
	Maths::HVec3 m_position;
	Maths::C_Quaternion m_orient;
	Btl::Vector<F32> m_attr; // List of attributes
	
	C_AnimFrame();
	void SetAttr(E_Attr type, F32 value);
};

class C_AnimData
{
public:
	Util::C_String m_name;
	Btl::Vector<C_AnimFrame> m_frame;

	void EvaluatePosition(F32 counter, Maths::HVec3& pos, Maths::C_Quaternion& orient) const;
	F32 EvaluateAttribute(F32 counter, E_Attr attribute) const;
	F32 CalcLength() const;
};

} // End of namespace

#endif

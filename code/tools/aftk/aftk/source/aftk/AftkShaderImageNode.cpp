#include <stdafx.h>

using namespace Aftk;

#define AFTK_SHADER_IMAGE_FILE_NAME_STATIC_PLUG_INDEX 0
#define AFTK_SHADER_IMAGE_TYPE_STATIC_PLUG_INDEX 1
#define AFTK_SHADER_IMAGE_NUM_STATIC_PLUGS 2
#define AFTK_SHADER_IMAGE_R_INDEX 0
#define AFTK_SHADER_IMAGE_G_INDEX 1
#define AFTK_SHADER_IMAGE_B_INDEX 2
#define AFTK_SHADER_IMAGE_A_INDEX 3

C_ShaderImageNode::C_ShaderImageNode(C_Status* pStatus) : 
	C_ShaderNode(),
	m_filePlug(Aftk::E_GenericAttributeType_String, 0), 
	m_imageTypePlug(Aftk::E_GenericAttributeType_String, 0)
{
	C_Status status;
	status = OneTimeInit();
	if (pStatus)
		*pStatus = status;
}

C_Node* C_ShaderImageNode::Clone(C_Scene* pScene) const
{
	return new C_ShaderNode(*this);
}

C_ShaderImageNode::~C_ShaderImageNode() 
{
}

C_ShaderImageNode::C_ShaderImageNode(const C_ShaderImageNode &rhs) : 
	C_ShaderNode(),
	m_filePlug(Aftk::E_GenericAttributeType_String, 0), 
	m_imageTypePlug(Aftk::E_GenericAttributeType_String, 0)
{
	OneTimeInit();
	*this = rhs;
}

const C_ShaderImageNode& C_ShaderImageNode::operator=(const C_ShaderImageNode &rhs) 
{
	C_ShaderNode::operator =(rhs);

	m_filePlug = rhs.m_filePlug;
	m_imageTypePlug = rhs.m_imageTypePlug;

	return *this;
}

bool C_ShaderImageNode::operator==(const C_ShaderImageNode &rhs) const 
{
	return IsEqualToAll(rhs);
}

bool C_ShaderImageNode::operator!= (const C_ShaderImageNode &rhs) const 
{
	return !((*this) == rhs);
}

bool C_ShaderImageNode::operator<(const C_ShaderImageNode &rhs) const
{
	if (m_filePlug 
		< 
		rhs.m_filePlug)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

C_StaticPlug<std::string>& C_ShaderImageNode::GetFilePlug()
{
	return m_filePlug;
}

const C_StaticPlug<std::string>& C_ShaderImageNode::GetFilePlug() const
{
	return m_filePlug;
}

C_StaticPlug<std::string>& C_ShaderImageNode::GetImageTypePlug()
{
	return m_imageTypePlug;
}

const C_StaticPlug<std::string>& C_ShaderImageNode::GetImageTypePlug() const
{
	return m_imageTypePlug;
}

E_ObjType C_ShaderImageNode::GetType() const 
{
	return E_ObjType_ShaderImage;
}

bool C_ShaderImageNode::HasType(E_ObjType type) const
{
	if (C_ShaderNode::HasType(type))
		return true;
	if (type == C_ShaderImageNode::GetType()) 
		return true;

	return false;
}

U32 C_ShaderImageNode::GetNumStaticPlugs() const
{
	return C_ShaderNode::GetNumStaticPlugs() + AFTK_SHADER_IMAGE_NUM_STATIC_PLUGS;
}

const I_Plug* C_ShaderImageNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = C_ShaderNode::GetNumStaticPlugs();
	if (instance < staticOffset)
		return C_ShaderNode::GetStaticPlugPtr(instance);
	
	instance -= staticOffset;
	switch(instance)
	{
	case AFTK_SHADER_IMAGE_FILE_NAME_STATIC_PLUG_INDEX:
		return &m_filePlug;
	case AFTK_SHADER_IMAGE_TYPE_STATIC_PLUG_INDEX:
		return &m_imageTypePlug;
	}

	return 0;
}

I_Plug* C_ShaderImageNode::GetStaticPlugPtr(U32 instance)
{
	return const_cast<I_Plug*>(((const C_ShaderImageNode*)this)->GetStaticPlugPtr(instance));
}

C_ShaderNode* C_ShaderImageNode::Clone() const
{
	return new C_ShaderImageNode(*this);
}

bool C_ShaderImageNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_ShaderImageNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (classToAbortTest == E_ObjType_ShaderImage)
		return true;
	return (IsEqualToThis(rhs, flags) && C_ShaderNode::IsEqualToAll(rhs, classToAbortTest, flags));
}

bool C_ShaderImageNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_ShaderImage))
	{
		return false;
	}
	const C_ShaderImageNode& rhs = (const C_ShaderImageNode&)_rhs;

	if (m_filePlug != rhs.m_filePlug)
		return false;

	if (m_imageTypePlug != rhs.m_imageTypePlug)
		return false;

	return true;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

C_Status C_ShaderImageNode::OneTimeInit()
{
	m_filePlug.SetLogicalIndex(0);
	m_filePlug.SetParentPtr(this);
	m_filePlug.SetName("file");
	m_filePlug.SetValue("");
	{
		C_NullPlug* pRPlug = new C_NullPlug(0);
		if (!pRPlug) return C_Status::E_Status_OutOfMemory;
		
		C_NullPlug* pGPlug = new C_NullPlug(0);
		if (!pGPlug) return C_Status::E_Status_OutOfMemory;
		
		C_NullPlug* pBPlug = new C_NullPlug(0);
		if (!pBPlug) return C_Status::E_Status_OutOfMemory;
		
		C_NullPlug* pAPlug = new C_NullPlug(0);
		if (!pAPlug) return C_Status::E_Status_OutOfMemory;

		m_filePlug.m_children.Reserve(4);
		m_filePlug.AddChildPtr(pRPlug);
		m_filePlug.AddChildPtr(pGPlug);
		m_filePlug.AddChildPtr(pBPlug);
		m_filePlug.AddChildPtr(pAPlug);
		m_filePlug.SetChildrenLocked(true);
		
		pRPlug->SetName("r");
		pGPlug->SetName("g");
		pBPlug->SetName("b");
		pAPlug->SetName("a");
	}
	m_imageTypePlug.SetLogicalIndex(1);
	m_imageTypePlug.SetParentPtr(this);
	m_imageTypePlug.SetName("imageType");
	m_imageTypePlug.SetValue("");
	m_imageTypePlug.SetChildrenLocked(true);

	return C_Status::E_Status_Success;
}

void C_ShaderImageNode::Clear()
{
	ClearThis();
	C_ShaderNode::Clear();
}

void C_ShaderImageNode::ClearThis() 
{
	m_filePlug.SetValue("");

	m_imageTypePlug.SetValue("");
}

#endif


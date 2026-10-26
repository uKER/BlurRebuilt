#include <stdafx.h>

using namespace Aftk;

C_NullPlug::C_NullPlug(void(*pOnChangedCallback)(void* pThis)) : I_Plug(pOnChangedCallback)
{
	OneTimeInit();
}

C_NullPlug::~C_NullPlug()
{
	for (U32 i = 0; i < (U32)m_children.Size(); i++)
		delete m_children[i];
}

const C_NullPlug& C_NullPlug::operator =(const C_NullPlug& rhs)
{
	I_Plug::operator =(rhs);

	m_childrenLocked = rhs.m_childrenLocked;
	m_name = rhs.m_name;

	AFTK_ASSERT(m_children.Size() == rhs.m_children.Size());
	for (U32 i = 0; i < (U32)rhs.m_children.Size(); i++)
	{
		delete m_children[i];
		m_children[i] = rhs.m_children[i]->Clone();
		//Associate our cloned child with this plug.
		m_children[i]->m_logicalIndex = i;
		m_children[i]->m_pParent = this;
	}

	return *this;
}

bool C_NullPlug::operator==(const C_NullPlug &rhs) const
{
	return IsEqualToAll(rhs);	
}

bool C_NullPlug::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_NullPlug::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_NullPlug))
		return false;

	const C_NullPlug& rhs = (const C_NullPlug&)_rhs;

	if (m_name != rhs.m_name)
		return false;

	return true;
}

bool C_NullPlug::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (!IsEqualToThis(rhs, flags))
		return false;

	if (!I_Plug::IsEqualToAll(rhs, classToAbortTest, flags))
		return false;

	return true;
}

bool C_NullPlug::operator!= (const C_NullPlug &rhs) const
{
	return !((*this) == rhs);
}

bool C_NullPlug::operator< (const C_NullPlug &rhs) const
{
	if (I_Plug::operator <(rhs))
		return true;

	return false;
}

U32	C_NullPlug::GetNumChildren() const
{
	return (U32)m_children.Size();
}

const I_Plug* C_NullPlug::GetChildPtr(U32 instance) const
{
	return m_children[instance];
}

I_Plug* C_NullPlug::GetChildPtr(U32 instance)
{
	return m_children[instance];
}

bool C_NullPlug::GetValue(U8& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(U16& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(U32& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;
	return false;
}

bool C_NullPlug::GetValue(U64& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(S8& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(S16& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(S32& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(S64& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(F32& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(F64& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(bool& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(std::string& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

bool C_NullPlug::GetValue(Matrix44& value, bool evaluate) const
{
	bool evalResult;
	if (!EvaluateConnectivity(value, evalResult))
	{
		//Evaluation failed.
		if (!evalResult)
			return false;
	}
	//Else evaluation was successful.
	else if (evalResult)
		return true;

	return false;
}

U32 C_NullPlug::GetHash() const
{
	return 0;
}
	
bool C_NullPlug::IsNull() const
{
	return true;
}

E_GenericAttributeType C_NullPlug::GetDataType() const
{
	return Aftk::E_GenericAttributeType_Null;
}

E_ObjType C_NullPlug::GetType() const
{
	return E_ObjType_NullPlug;
}

bool C_NullPlug::HasType(E_ObjType type) const
{
	if (I_Plug::HasType(type))
		return true;
	if (type == C_NullPlug::GetType()) 
		return true;

	return false;
}

const std::string& C_NullPlug::GetName() const
{
	return m_name;
}

I_Plug* C_NullPlug::Clone() const
{
	C_NullPlug* p = new C_NullPlug(m_pOnChangedCallback);
	if (p)
	{
		*p = *this;
	}

	return p;
}

void C_NullPlug::Clear()
{
	ClearThis();
	I_Plug::Clear();
}

void* C_NullPlug::operator new (size_t size)
{
	return C_MemoryManager::PoolAllocate((U32)size);
}

void C_NullPlug::operator delete(void* p)
{
	C_MemoryManager::PoolFree(sizeof(C_NullPlug), p);
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_NullPlug::SetName(const std::string& name)
{
	m_name = name;
}

void C_NullPlug::OneTimeInit()
{
	m_childrenLocked = false;
}

void C_NullPlug::ClearThis()
{
	DeleteChildren();
}

void C_NullPlug::SetChildrenLocked(bool lock)
{
	m_childrenLocked = lock;
}

void C_NullPlug::DeleteChildren()
{
	for (U32 i = 0; i < (U32)m_children.Size(); i++)
		delete m_children[i];
	m_children.Clear();
}

S32 C_NullPlug::AddChildPtr(I_Plug* pChild)
{
	AFTK_ASSERT(!m_childrenLocked);
	if (m_childrenLocked)
	{
		delete pChild;
		return -1;
	}
	m_children.PushBack(pChild);
	pChild->SetLogicalIndex(m_children.Size()-1);
	pChild->SetParentPtr(this);
	return m_children.Size() -1;
}

#endif

#include <stdafx.h>

using namespace Aftk;

C_DynamicPlug::C_DynamicPlug()
: 
I_Plug(0)
{
	OneTimeInit();
}

C_DynamicPlug::C_DynamicPlug(const C_DynamicPlug &rhs)
: 
I_Plug(0)
{
	OneTimeInit();
	*this = rhs;
}

C_DynamicPlug::~C_DynamicPlug()
{
	for (U32 i = 0; i < (U32)m_children.Size(); i++)
	{
		delete m_children[i];
	}
}

const C_DynamicPlug& C_DynamicPlug::operator =(const C_DynamicPlug& rhs)
{
	ClearThis();

	I_Plug::operator =(rhs);
	
	m_attribute = rhs.m_attribute;

	m_children.Resize(rhs.m_children.Size());
	for (U32 i = 0; i < (U32)rhs.m_children.Size(); i++)
	{
		m_children[i] = rhs.m_children[i]->Clone();
		//Correct compound plug pointers
		m_children[i]->m_pParent = this;
		m_children[i]->m_logicalIndex = i;
	}

	return *this;
}

bool C_DynamicPlug::operator==(const C_DynamicPlug &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_DynamicPlug::IsEqualTo(const I_Object& rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_DynamicPlug::IsEqualToThis(const I_Object& _rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_DynamicPlug))
		return false;

	C_DynamicPlug& rhs = (C_DynamicPlug&) _rhs;

	//Perform this quick rejection test first.
	if ((m_children.Size() != rhs.m_children.Size())
		||
		(m_logicalIndex != rhs.m_logicalIndex)
		)
		return false;

	//Perform the base-class equality test
	if (m_attribute != rhs.m_attribute)
		return false;

	//Recurse: are our child plugs equal?
	for (U32 i = 0; i < (U32)m_children.Size(); i++)
		if ((*m_children[i]) != (*rhs.m_children[i]))
			return false;

	return true;
}

bool C_DynamicPlug::IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (classToAbortTest == E_ObjType_DynamicPlug)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && I_Plug::IsEqualToAll(rhs, classToAbortTest, flags));
}
	
bool C_DynamicPlug::operator!= (const C_DynamicPlug &rhs) const
{
	return !((*this) == rhs);
}
	
bool C_DynamicPlug::IsNull() const
{
	if ((m_children.Size() == 0)
		&&
		(!m_pSourceConnection)
		&&
		(m_attribute.GetDataType() == E_GenericAttributeType_Null == 0)
		)
	{
		//This plug doesn't contain any data!
		return true;
	}

	return false;
}

S32	C_DynamicPlug::AddChildPtr(C_DynamicPlug* pPlug)
{
	m_children.PushBack(pPlug);
	U32 plugIndex = (U32)m_children.Size() -1;

	//Associate this child with this plug
	pPlug->SetLogicalIndex(plugIndex);
	pPlug->SetParentPtr(this);

	return plugIndex;
}

void C_DynamicPlug::SetValue(const U8& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const U16& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const U32& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const U64& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const S8& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const S16& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const S32& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const S64& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const F32& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const F64& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const bool& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const std::string& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const Matrix44& value, bool evaluateDependants)
{
	SetValueT(value, evaluateDependants);
}

void C_DynamicPlug::SetValue(const Aftk::C_DynamicPlug& value, bool evaluateDependants)
{
	m_attribute = value.m_attribute;
	if (evaluateDependants)
		OnChanged();
}

bool C_DynamicPlug::GetValue(U8& value, bool evaluate) const
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
    
	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_U8)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(U16& value, bool evaluate) const
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

	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_U16)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(U32& value, bool evaluate) const
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

	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_U32)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(U64& value, bool evaluate) const
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

	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_U64)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(S8& value, bool evaluate) const
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

	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_S8)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(S16& value, bool evaluate) const
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


	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_S16)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(S32& value, bool evaluate) const
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


	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_S32)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(S64& value, bool evaluate) const
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


	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_S64)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(F32& value, bool evaluate) const
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

	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_F32)
		return false;
	
	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(F64& value, bool evaluate) const
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

	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_F64)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(bool& value, bool evaluate) const
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

	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_Boolean)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(std::string& value, bool evaluate) const
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

	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_String)
		return false;

	m_attribute.GetValue(value);
	return true;
}

bool C_DynamicPlug::GetValue(Matrix44& value, bool evaluate) const
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

	if (m_attribute.GetDataType() != Aftk::E_GenericAttributeType_Matrix44F32)
		return false;

	m_attribute.GetValue(value);
	return true;
}

const C_GenericAttribute& C_DynamicPlug::GetAttribute() const
{
	return m_attribute;
}

const std::string& C_DynamicPlug::GetName() const
{
	return m_attribute.GetName();
}

void C_DynamicPlug::SetName(const std::string& name)
{
	m_attribute.SetName(name);
}

E_GenericAttributeType C_DynamicPlug::GetDataType() const
{
	return m_attribute.GetDataType();
}

const I_Plug* C_DynamicPlug::GetChildPtr(U32 instance) const
{
	return m_children[instance];
}

U32 C_DynamicPlug::GetNumChildren() const
{
	return (U32)m_children.Size();
}

E_ObjType C_DynamicPlug::GetType() const
{
	return E_ObjType_DynamicPlug;
}

bool C_DynamicPlug::HasType(E_ObjType type) const
{
	if (I_Plug::HasType(type))
		return true;
	if (type == C_DynamicPlug::GetType()) 
		return true;

	return false;
}

I_Plug* C_DynamicPlug::Clone() const
{
	return new C_DynamicPlug(*this);
}

void C_DynamicPlug::Clear()
{
	ClearThis();
	I_Plug::Clear();
}

void C_DynamicPlug::DeleteChildren()
{
	for (U32 i = 0; i < (U32)m_children.Size(); i++)
		delete m_children[i];
	m_children.Clear();
}

void* C_DynamicPlug::operator new (size_t size)
{
	return C_MemoryManager::PoolAllocate((U32)size);
}

void C_DynamicPlug::operator delete(void* p)
{
	C_MemoryManager::PoolFree(sizeof(C_DynamicPlug), p);
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_DynamicPlug::OneTimeInit()
{
}

void C_DynamicPlug::ClearThis()
{
	m_attribute.Clear();
	DeleteChildren();
}

#endif

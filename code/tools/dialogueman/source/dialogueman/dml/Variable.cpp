#include "stdafx.h"

#include <DialogueMan/Dml/Variable.h>
#include <wx/tokenzr.h>

namespace DMan
{
namespace Dml
{

/////////////////////////// C_Variable \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_SimpleVariable C_Variable::NULL_VAR("__NULLVAR", wxString(""));
C_SimpleVariable C_Variable::END_VAR("__ENDVAR", wxString("__END__"));

C_Variable::C_Variable(const wxString& name) :
	m_name(name),
	m_pContext(NULL),
	m_pIteratorToThis(NULL)
{}

C_Variable::~C_Variable()
{
	if(m_pIteratorToThis)
		delete m_pIteratorToThis;
}

wxString C_Variable::GetName() const
{
	return m_name;
}

C_MutantValue C_Variable::GetValue(bool solveRef) const
{
	return "";
}

void C_Variable::SetValue(const C_MutantValue& value)
{}

C_Variable& C_Variable::FindChildVariable(const wxString& name)
{
	return *this;
}

int C_Variable::FindChildVariableIndex(const wxString& name)
{
	return 0;
}

C_Variable& C_Variable::operator[](int index)
{
	return *this;
}

C_Variable& C_Variable::operator[](const wxString& name)
{
	return *this;
}

void C_Variable::Add(C_Variable* pVariable, bool takeOwnership)
{}

void C_Variable::Delete(const wxString& name)
{}

int C_Variable::GetChildCount()
{
	return 0;
}

void C_Variable::SetContext(C_Variable* pContext)
{
	m_pContext = pContext;
}

C_Variable* C_Variable::GetContext()
{
	return m_pContext;
}

C_Variable::operator C_MutantValue() const
{ 
	return GetValue(); 
}

C_Variable::operator wxString() const
{ 
	return wxString(GetValue()); 
}

C_Variable::operator void*() const
{
	return (void*)GetValue();
}

C_Variable& C_Variable::SolveReference(const wxString& refToSolve, C_Variable* pContext, bool stripMarkers)
{
	wxString refClean = refToSolve;
	
	if(stripMarkers)
	{
		refClean = refToSolve.Mid(2, refToSolve.Length() - 3); //get rid of sign and brackets
	}
	
	wxStringTokenizer tkz(refClean, "."); //break into contexts
	C_Variable* pCurrContext = pContext;

	while ( tkz.HasMoreTokens() )
	{
		wxString name = tkz.GetNextToken();
		if(pCurrContext)
		{
			pCurrContext = &(*pCurrContext)[name];
		}
	}
	
	return *pCurrContext;
}

/////////////////////////// C_SimpleVariable \\\\\\\\\\\\\\\\\\\\\\\\\\\

C_SimpleVariable::C_SimpleVariable(const wxString& name, const C_MutantValue& value) :
	C_Variable(name),
	m_value(value)
{}

C_SimpleVariable::~C_SimpleVariable()
{}

C_MutantValue C_SimpleVariable::GetValue(bool solveRef) const 
{	
	if(solveRef && m_value.GetType() == E_DataType_String)
	{
		wxString lookIn = wxString(m_value);
		wxStringList varRefs = GetVariableReferences(lookIn);
		for(size_t i = 0; i < varRefs.size(); i++)
		{
			wxString varRef = varRefs[i];
			C_MutantValue solved = SolveReference(varRef, m_pContext);
			if(solved.GetType() == E_DataType_String)
				lookIn.Replace(varRef, wxString(solved));
			else
				return solved;
		}
		return lookIn;
	}
	
	return m_value; 
}

void C_SimpleVariable::SetValue(const C_MutantValue& value)
{
	m_value = value;
}

wxStringList C_SimpleVariable::GetVariableReferences(const wxString& _lookIn) const
{
	wxStringList varRefs;

	wxString lookIn = _lookIn;
	int signPos = lookIn.Find('$');
	while (signPos != wxNOT_FOUND)
	{
		//Sample:
		//X${varRef}
		//0123456789
		int backetEnd = lookIn.Find('}'); //9 
		wxString ref = lookIn.Mid(signPos, backetEnd - signPos + 1); 
		varRefs.Add(ref);

		lookIn = lookIn.Mid(backetEnd + 1);
		signPos = lookIn.Find('$');
	}

	return varRefs;
}

/////////////////////////// C_CompositeVariable \\\\\\\\\\\\\\\\\\\\\\\\\\\
 
C_CompositeVariable::C_CompositeVariable(const wxString& name) :
	C_Variable(name),
	m_pIteratorToBegin(NULL),
	m_pIteratorToEnd(NULL)
{}

C_CompositeVariable::~C_CompositeVariable()
{
	for(int i = 0; i < m_variables.Size(); i++)
	{
		if(m_varToTakeOwnership[ m_variables[i] ])
			delete m_variables[i];
	}

	if(m_pIteratorToBegin)
		delete m_pIteratorToBegin;

	if(m_pIteratorToEnd)
		delete m_pIteratorToEnd;
}

void C_CompositeVariable::Add(C_Variable* pVariable, bool takeOwnership)
{
	m_varToTakeOwnership[pVariable] = takeOwnership;
	Delete(pVariable->GetName()); //delete if previously added
	m_variables.PushBack(pVariable);
}

void C_CompositeVariable::Delete(const wxString& name)
{
	int varPos = FindChildVariableIndex(name);
	if(varPos >= 0 && varPos < m_variables.Size())
	{
		if(m_varToTakeOwnership[ m_variables[varPos] ])
			delete m_variables[varPos]; 
		
		m_variables.EraseByIndex(varPos);
	}
}

int C_CompositeVariable::GetChildCount()
{
	return m_variables.Size();
}

C_Variable& C_CompositeVariable::FindChildVariable(const wxString& name)
{
	for(int i = 0; i < m_variables.Size(); i++)
	{
		if(m_variables[i]->GetName() == name)
		{
			return *m_variables[i];
		}
	}
	return C_Variable::END_VAR;
}

int C_CompositeVariable::FindChildVariableIndex(const wxString& name)
{
	for(int i = 0; i < m_variables.Size(); i++)
	{
		if(m_variables[i]->GetName() == name)
		{
			return i;
		}
	}
	return -1;
}


C_Variable& C_CompositeVariable::operator[](int index)
{
	if(index >= 0 && index < m_variables.Size())
	{
		C_Variable* pVarAtIndex = m_variables[index];
		
		if(GetContext()) //give our context 
			pVarAtIndex->SetContext(GetContext());
		
		return *pVarAtIndex;
	}
	return C_Variable::END_VAR;
}

C_Variable& C_CompositeVariable::operator[](const wxString& name)
{
	C_Variable& found = FindChildVariable(name);
	if(GetContext()) //give our context
		found.SetContext(GetContext());
	
	return found;
}

} //namespace Dml
} //namespace DMan
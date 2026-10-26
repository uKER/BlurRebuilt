#include "stdafx.h"

#include <cctype>

#include <core/maths/mathsutil.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>

#include <EditorLib/Core.h>


namespace EditorLib
{

C_ArgumentDatabase::C_ArgumentDatabase()
{
}

C_ArgumentDatabase::~C_ArgumentDatabase()
{
}

C_ArgumentDatabase::C_ArgumentDatabase(const C_ArgumentDatabase& rhs)
{
	*this = rhs;
}

const C_ArgumentDatabase& C_ArgumentDatabase::operator=(const C_ArgumentDatabase& rhs)
{
	m_argumentGroups = rhs.m_argumentGroups;
	return *this;
}

bool C_ArgumentDatabase::AddArgGroup(
	const C_Argument& flagArg, 
	const Btl::Vector<Btl::String>& commandArgs)
{
	for (Btl::Vector<C_ArgumentGroup>::ConstIterator it = m_argumentGroups.Begin(); 
		it != m_argumentGroups.End(); ++it)
	{
		const C_ArgumentGroup& arg = *it;
		if (arg.shortName == flagArg.GetShortName())
			return false;
	}

	//Store new group.
	m_argumentGroups.PushBack();
	C_ArgumentGroup& result = m_argumentGroups.Back();
	result.shortName = flagArg.GetShortName();

	const VarTypeArray& dataTypes = flagArg.GetDataTypes();
	if (dataTypes.Size() != commandArgs.Size())
	{
		LOGERROR(C_Core::Instance().GetLogger(), "Datatype array and command array arg mismatch!");
		return false;
	}
	S32 nData = commandArgs.Size();
	result.data.Resize(nData);

	//Parse data.
	for (S32 i = 0; i < nData; i++)
	{
		Util::C_KeyValue& attr = result.data[i];
		attr.FromString(commandArgs[i], dataTypes[i]);
	}

	return true;
}

const C_ArgumentDatabase::C_ArgumentGroup* C_ArgumentDatabase::GetArgGroup(
	const char shortName) const
{
	//Examine short names first.
	for (Btl::Vector<C_ArgumentGroup>::ConstIterator it = m_argumentGroups.Begin(); 
		it != m_argumentGroups.End(); ++it)
	{
		const C_ArgumentGroup& arg = *it;
		if (std::tolower(arg.shortName) == std::tolower(shortName))
			return &arg;
	}

	//Not found.
	return 0;
}

bool C_ArgumentDatabase::IsSet(const char shortName) const
{
	return GetArgGroup(shortName) ? true : false;
}

const ArgumentDataArray* C_ArgumentDatabase::GetArgData(const char shortName) const
{
	const C_ArgumentGroup* pArg = GetArgGroup(shortName);
	if (!pArg)
		return 0;

	return (ArgumentDataArray*)&pArg->data;
}

}

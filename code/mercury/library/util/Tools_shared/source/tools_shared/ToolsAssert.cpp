#include "stdafx.h"

#include <Core\Containers.h>
#include <System\DebugLog.h>
#include "tools_shared\ToolsAssert.h"
#include "tools_shared\MiniDumper.h"
#include "tools_shared\ToolsApplication.h"

Debug::Logger& Tools::g_log = Debug::GetLogger("tools.shared");

namespace Tools
{

// assert info for the Ut::UtSet
struct AssertID
{
	Util::C_String szFunction;
	Util::C_String szFilename;
	int iLineNumber;
};

bool operator <(const AssertID &t, const AssertID &other)
{
	return (t.szFunction < other.szFunction) || (t.szFilename < other.szFilename) || (t.iLineNumber < other.iLineNumber);
}


Ut::UtSet<AssertID> g_UniqueAsserts;

// replace < and > with &; (to display properly in HTML)
Util::C_String ConvertAngleBracketsToXML(const char *pszExpression)
{
	Util::C_String sExpression;
	while (*pszExpression)
	{
		if (*pszExpression == '<')
			sExpression = sExpression + "&#60;";
		else if (*pszExpression == '>')
			sExpression = sExpression + "&#62;";
		else
			sExpression = sExpression + Util::C_String(*pszExpression);

		pszExpression++;
	}

	return sExpression;
}

bool BizToolsAssert(BizAssertKind kind, const char* expr, const char* msg, const char* file, int line)
{
	bool retVal = ToolsAssert(msg, expr, file, line);
	
	// BIZ_ASSERT will cause the debugger to stop again if we return True
	return false;
}

/// override for core assert
bool ToolsAssert(const char *pszFunction, const char *pszExpression, const char *pszFilename, int iLineNumber)
{
	// have we had this assert before?
	AssertID id;
	id.iLineNumber = iLineNumber;
	id.szFilename = pszFilename;
	id.szFunction = pszFunction;

	Util::C_String sExpression = ConvertAngleBracketsToXML(pszExpression);
	Util::C_String sFunction = ConvertAngleBracketsToXML(pszFunction);

	if (g_UniqueAsserts.Find(id) != g_UniqueAsserts.End())
	{
		// display a logWARN for the next time an assert happens
		LOGWARN( g_pMyApplication->GetLogger(), "Assert(%s) failed in %s(%d) [%s]",
			sExpression.Chars(), pszFilename, iLineNumber, sFunction.Chars());
		return true;
	}
	g_UniqueAsserts.Insert(id);

	// display a logfatal for the first time an assert happens
	LOGFATAL( g_pMyApplication->GetLogger(), "Assert(%s) failed in %s(%d) [%s]",
		sExpression.Chars(), pszFilename, iLineNumber, sFunction.Chars());

#ifdef COMPILER_VC
	// write out a .dmp file the first time a particular assert happens
	if (IsDebuggerPresent())
	{
		DebugBreak();
	}
	else
	{
		Util::C_String sAssert;
		sAssert = Util::C_String::Format("Assert(%s)", sExpression.Chars());

		Util::C_String sFilePos;
		sFilePos = Util::C_String::Format("%s(%d)", pszFilename, iLineNumber);

		ULONG_PTR params[] = {
								(ULONG_PTR)sAssert.Chars(),
								(ULONG_PTR)sFunction.Chars(),
								(ULONG_PTR)sFilePos.Chars() };

		RaiseException(TOOLS_ASSERT_EXCEPTION_CODE, 0, _countof(params), params);
	}
#endif
	
	return true;
}

}; // namespace Tools

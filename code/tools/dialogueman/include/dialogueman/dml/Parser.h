#ifndef DMAN_DML_PARSER_H
#define DMAN_DML_PARSER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class C_XmlDomElement;
}

namespace DMan
{

namespace Dml
{
	class C_Variable;
	class C_Task;
	
	class C_Parser
	{
	public:
		C_Parser();
		bool Parse(const wxString& dmlFilePath, C_Task** pTask, C_Variable** pVariable);

	public:
		static const wxString GLOBALS_NAME;
		static const wxString MAIN_NAME;

	private:
		void ProcessTaskChildren(Sys::C_XmlDomElement* pParent, C_Task* pParentTask, C_Variable* pGlobals);
		void ProcessVariableChildren(Sys::C_XmlDomElement* pParent, C_Variable* pParentVariable);
	};

} //namespace Dml
} //namespace DMan

#endif //DMAN_DML_PARSER_H
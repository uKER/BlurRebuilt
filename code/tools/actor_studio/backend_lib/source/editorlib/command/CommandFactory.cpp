#include "stdafx.h"
#include <EditorLib/Command/CommandFactory.h>

namespace EditorLib
{
	C_CommandFactory::C_CommandFactory(C_Status (*f)(const char* pName))
	{
		m_nextId = 0;
		m_pPostRegFunc = f;
	}

	C_CommandFactory::~C_CommandFactory()
	{
	}

	bool C_CommandFactory::Create(
		I_Command*& pCommand,
		C_Syntax*& pSyntax,
		const Btl::String& command)
	{
		CommandHashMap::Iterator it = m_commandMapping.Find(command);
		if (it == m_commandMapping.End())
			//Command not found!
			return false;
		
		pCommand = it->GetValue().create(it->GetValue().id);
		assert(pCommand);
		pSyntax = it->GetValue().createSyntax();
		return true;
	}

	const C_CommandFactory::CommandHashMap& C_CommandFactory::GetCommands() const
	{
		return m_commandMapping;
	}
}

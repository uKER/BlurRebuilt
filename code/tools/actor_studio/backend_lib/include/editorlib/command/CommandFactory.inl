template<class COMMAND>
C_Status C_CommandFactory::Register()
{
	Btl::String commandName = COMMAND::GetName();

	CommandHashMap::Iterator it = m_commandMapping.Find(commandName);
	if (it != m_commandMapping.End())
	{
		//Duplicate command in registry!
		return E_Status_Failure;
	}

	//Add to registry.
	m_commandMapping[commandName] = CreateInfo(
		COMMAND::Create, 
		COMMAND::CreateSyntax,
		++m_nextId);

	if (m_pPostRegFunc)
		return m_pPostRegFunc(commandName.Chars());

	return E_Status_Success;
}

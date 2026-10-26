template <typename FILE_SYSTEM>
void C_LogFsManager::OnLogFsCreated(C_LogFs<FILE_SYSTEM>* pLogFs)
{
	m_mutex.Wait();
	m_regFsList.PushBack(new C_RegFs(pLogFs->GetName(), pLogFs));
	m_mutex.Release();
}

template <typename FILE_SYSTEM>
void C_LogFsManager::OnLogFsDestroy(C_LogFs<FILE_SYSTEM>* pLogFs)
{
	m_mutex.Wait();
	for (RegFsList::Iterator it = m_regFsList.Begin(); it != m_regFsList.End(); ++it)
	{
		if ((*it)->pFileSystem == pLogFs)
		{
			delete *it;
			m_regFsList.Erase(it);
			break;
		}
	}
	m_mutex.Release();
}


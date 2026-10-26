namespace LogicalFs
{

template<typename FILE_SYSTEM>
C_LogFs<FILE_SYSTEM>::~C_LogFs()
{
	C_LogFsManager::Instance().OnLogFsDestroy(this);
}

template<typename FILE_SYSTEM>
Sys::I_File* C_LogFs<FILE_SYSTEM>::Open(
	const Sys::C_FilePath& filePath,
	const Sys::E_FileAccess fileAccess,
	const Sys::C_OpenParams& params)
{
	Sys::I_File *pFile = FILE_SYSTEM::Open(filePath, fileAccess, params);
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		pFile ? DebugLevel::Info : DebugLevel::Error, 
		"Open(\"%s\", %u)\treturned 0x%x", filePath.Chars(), fileAccess, (U32)(INTPTR)pFile);	// [Blur2PC][x64] log only
	C_LogFsManager::Instance().LogFileAccess(m_name.Chars(),
		filePath.Chars());
	return pFile;
}

template<typename FILE_SYSTEM>
bool C_LogFs<FILE_SYSTEM>::FileExists(const Sys::C_FilePath& filePath)
{
	bool result = FILE_SYSTEM::FileExists(filePath);
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		DebugLevel::Info, 
		"FileExists(\"%s\")\treturned %s", filePath.Chars(), result ? "true" : "false");
	return result;
}

template<typename FILE_SYSTEM>
bool C_LogFs<FILE_SYSTEM>::FileInfo(const Sys::C_FilePath& filePath, Sys::C_FileInfo* pFileInfo)
{
	bool result = FILE_SYSTEM::FileInfo(filePath, pFileInfo);
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		DebugLevel::Info, 
		"FileInfo(\"%s\")\treturned %s", filePath.Chars(), result ? "true" : "false");
	return result;
}

template<typename FILE_SYSTEM>
bool C_LogFs<FILE_SYSTEM>::FileAccess(const Sys::C_FilePath& filePath, U32 fileAccess)
{
	bool result = FILE_SYSTEM::FileAccess(filePath, fileAccess);
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		result ? DebugLevel::Info : DebugLevel::Warn, 
		"FileAccess(\"%s\", %u)\treturned %s", filePath.Chars(), fileAccess, result ? "true" : "false");
	return result;
}

template<typename FILE_SYSTEM>
bool C_LogFs<FILE_SYSTEM>::FileTime(const Sys::C_FilePath& filePath, const Time::C_Time &fileTime)
{
	bool result = FILE_SYSTEM::FileTime(filePath, fileTime);
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		result ? DebugLevel::Info : DebugLevel::Warn, 
		#ifdef COMPILER_VC
		"FileTime(\"%s\", %I64u)\treturned %s",
		#else
		"FileTime(\"%s\", %llu)\treturned %s", 
		#endif
		filePath.Chars(), (U64)fileTime, result ? "true" : "false");
	return result;
}

template<typename FILE_SYSTEM>
bool C_LogFs<FILE_SYSTEM>::DeleteFile(const Sys::C_FilePath& filePath)
{
	bool result = FILE_SYSTEM::DeleteFile(filePath);
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		result ? DebugLevel::Info : DebugLevel::Error, 
		"DeleteFile(\"%s\")\treturned %s", filePath.Chars(), result ? "true" : "false");
	return result;
}

template<typename FILE_SYSTEM>
bool C_LogFs<FILE_SYSTEM>::DirectoryInfo(const Sys::C_FilePath& filePath, Sys::C_DirectoryInfo* pDirInfo)
{
	bool result = FILE_SYSTEM::DirectoryInfo(filePath, pDirInfo);
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		result ? DebugLevel::Info : DebugLevel::Warn, 
		"DirectoryInfo(\"%s\")\treturned %s", filePath.Chars(), result ? "true" : "false");
	return result;
}

template<typename FILE_SYSTEM>
bool C_LogFs<FILE_SYSTEM>::CreateSingleDirectory(const Sys::C_FilePath& path)
{
	bool result = FILE_SYSTEM::CreateSingleDirectory(path);
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		result ? DebugLevel::Info : DebugLevel::Warn, 
		"CreateSingleDirectory(\"%s\")\treturned %s", path.Chars(), result ? "true" : "false");
	return result;
}

template<typename FILE_SYSTEM>
bool C_LogFs<FILE_SYSTEM>::SetWorkingDirectory(const Sys::C_FilePath& path)
{
	bool result = FILE_SYSTEM::SetWorkingDirectory(path);
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		result ? DebugLevel::Info : DebugLevel::Error, 
		"SetWorkingDirectory(\"%s\")\treturned %s", path.Chars(), result ? "true" : "false");
	return result;
}

template<typename FILE_SYSTEM>
Sys::C_FilePath C_LogFs<FILE_SYSTEM>::GetWorkingDirectory() const
{
	Sys::C_FilePath filePath = FILE_SYSTEM::GetWorkingDirectory();
	C_LogFsManager::Instance().LogMessage(m_name.Chars(),
		DebugLevel::Info, 
		"GetWorkingDirectory()\treturned \"%s\"", filePath.Chars());
	return filePath;
}

template<typename FILE_SYSTEM>
C_LogFs<FILE_SYSTEM>::C_LogFs(const char* name) : 
	FILE_SYSTEM(), 
	m_name(name)
{
	C_LogFsManager::Instance().OnLogFsCreated(this, name);
}

template<typename FILE_SYSTEM>
const Btl::String& C_LogFs<FILE_SYSTEM>::GetName() const
{
	return m_name;
}

}


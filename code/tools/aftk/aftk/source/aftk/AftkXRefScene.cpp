#include <stdafx.h>
#include <aftk/AftkXRefScene.h>
#include <aftk/AftkStringTable.h>

using namespace Aftk;

C_XRefScene::C_XRefScene(C_Scene* pRootScene)
{
	OneTimeInit(pRootScene);
}

C_XRefScene::C_XRefScene(const C_XRefScene& rhs)
{
	*this = rhs;
}

C_XRefScene::~C_XRefScene()
{
	Clear();
}

const C_XRefScene& C_XRefScene::operator =(const C_XRefScene& rhs)
{
	m_fileName = rhs.m_fileName;
	m_pScene = rhs.m_pScene;
	return *this;
}

C_Status C_XRefScene::SetSourceScene(const std::string& fileName)
{
	C_Status status;
	//Return failure if we're unable to read the type of file provided.

	//Calculate CRC value for the given file.
	m_crc = CalculateCRC(fileName, status);
	if (!status)
	{
		m_crc = 0;
		return status;
	}

	m_fileName = fileName;
	return C_Status::E_Status_Success;
}

C_Status C_XRefScene::SetSourceScene(const std::string& fileName, U32 crc)
{
	C_Status status;
	U32 fileCRC = CalculateCRC(fileName, status);
	if (fileCRC != crc)
	{
		return C_Status(C_Status::E_Status_Failure, AFTK_E_XREF_CRC_FAIL);
	}
	m_crc = crc;
	m_fileName = fileName;
	return C_Status::E_Status_Success;
}

C_Status C_XRefScene::Load()
{
	C_Status status;
	AFTK_ASSERT(m_crc != 0);
	if (m_crc == 0)
	{
		return C_Status::E_Status_Failure;
	}

	if (m_pScene)
	{
		return C_Status::E_Status_Success;
	}

	m_pScene = new C_Scene(m_pRootScene);

	//Check file CRC before loading
	bool crcValid = CheckCRC(m_fileName, m_crc, status);
	if (!crcValid || !status)
	{
		return status;
	}
	
	status = m_pScene->Read(m_fileName);
	if (!status)
	{
		delete m_pScene;
		m_pScene = 0;
		return status;
	}

	//Update the scene graphs for all loaded scenes so that the x-ref placeholder nodes in the hierarchy know
	//that this scene has been loaded.
	status = m_pRootScene->OnLoadXRefScene(this);
	if (!status) return status;

	//Load all of the x-refs for the scene.
	status = m_pScene->LoadAllXRefScenes();

	return status;
}

void C_XRefScene::Unload()
{
	if (m_pScene)
	{
		//Update the scene graphs for all loaded scenes so that the x-ref placeholder nodes in the hierarchy know
		//that this scene has been loaded.
		m_pRootScene->OnUnloadXRefScene(this);

		delete m_pScene;
		m_pScene = 0;
	}
}

const std::string& C_XRefScene::GetFileName() const
{
	return m_fileName;
}

bool C_XRefScene::IsLoaded() const
{
	if (m_pScene)
	{
		return true;
	}
	return false;
}

U32 C_XRefScene::GetCRC() const
{
	return m_crc;
}

C_Scene* C_XRefScene::GetScenePtr()
{
	return m_pScene;
}

const C_Scene* C_XRefScene::GetScenePtr() const
{
	return m_pScene;
}

void C_XRefScene::Clear()
{
	delete m_pScene;
	m_pScene = 0;
	m_crc = 0;
	m_fileName.clear();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_XRefScene::OneTimeInit(C_Scene* pRootScene)
{
	AFTK_ASSERT(pRootScene);
	m_pRootScene = pRootScene;
	m_pScene = 0;
	m_crc = 0;
}

#endif

#include "stdafx.h"
#include "MultiplayerProfileStorage.h"
#include "system\serialise\C_MemoryStream.h"
#include "SystemProfile\SystemProfileManager.h"
#include "Game\Game.h"

#include "network\PerUserServerStorageWriter.h"
#include "network\PerUserServerStorageReader.h"

Debug::Float g_debugStorageWaitTime("Multiplayer.PrimaryProfile.StorageWaitTime", 15.0f);

extern Debug::Bool g_ForceProfileRebuild;

namespace Game
{

const U32 C_MultiplayerProfileStorage::s_id=0xB12A88EC;
const char* C_MultiplayerProfileStorage::s_profileName="MultiplayerProfile.save";
const int C_MultiplayerProfileStorage::s_maxDataSize=8192;
const int C_MultiplayerProfileStorage::s_headerSize=4*4;  // ID+Version+PackVersion+Size

C_MultiplayerProfileStorage::C_MultiplayerProfileStorage(int version):
	m_hasValidDownloadedData(false),
	m_version(version),
	m_packVersion(0),
	m_lastTransactionResult(E_MPS_Ok),
	m_storageWriter(new NetPlay::C_PerUserServerStorageWriter),
	m_storageReader(new NetPlay::C_PerUserServerStorageReader)
{

}

C_MultiplayerProfileStorage::~C_MultiplayerProfileStorage()
{
	SAFE_DELETE_PTR(m_storageReader);
	SAFE_DELETE_PTR(m_storageWriter);
}


void C_MultiplayerProfileStorage::SetPackVersion(int packVersion)
{
	m_packVersion=packVersion;
}

E_MPS_Result C_MultiplayerProfileStorage::Upload(const Sys::C_MemoryStream& stream, const SysProfile::FullProfile& player)
{
	m_hasValidDownloadedData=false;
	BIZ_ASSERT_SPEED(m_lastTransactionResult!=E_MPS_Pending);

	if (m_lastTransactionResult==E_MPS_Pending)
	{
		m_storageWriter->StopAndCleanup();
		m_storageReader->StopAndCleanup();

		m_lastTransactionResult=E_MPS_RequestFailed;
		return m_lastTransactionResult;
	}
	
	GenerateData(stream);

	bool res=m_storageWriter->Start(player, s_profileName, &m_buffer[0], m_buffer.Size() );
	m_lastTransactionResult=res?E_MPS_Pending:E_MPS_RequestFailed;

	m_storageTimer = 0.f;

	return m_lastTransactionResult;
}

bool C_MultiplayerProfileStorage::IsUploadFinished(F32 timeStep)
{
	BIZ_ASSERT_SPEED(m_lastTransactionResult==E_MPS_Pending);

	if (m_lastTransactionResult!=E_MPS_Pending)
		return true;

	m_storageTimer += timeStep;

	NetPlay::C_PerUserServerStorageWriter::ePerUserStorageWriterState state=m_storageWriter->Continue();

	if(m_storageTimer > g_debugStorageWaitTime)
	{
		m_storageWriter->StopAndCleanup();
		m_storageTimer = 0.f;
		m_lastTransactionResult=E_MPS_RequestFailed;
		return true;
	}

	switch (state)
	{
	case NetPlay::C_PerUserServerStorageWriter::eWriteSuccessful:
		m_lastTransactionResult=E_MPS_Ok;
		return true;
	case NetPlay::C_PerUserServerStorageWriter::eWriteInProgress:
		return false;
	}

	m_lastTransactionResult=E_MPS_RequestFailed;
	return true;
}


E_MPS_Result C_MultiplayerProfileStorage::Download(const SysProfile::FullProfile& localPlayer, const SysProfile::UserIdentifier& fileOwner)
{
	m_hasValidDownloadedData=false;

	BIZ_ASSERT_SPEED(m_lastTransactionResult!=E_MPS_Pending);

	if (m_lastTransactionResult==E_MPS_Pending)
	{
		m_storageWriter->StopAndCleanup();
		m_storageReader->StopAndCleanup();

		m_lastTransactionResult=E_MPS_RequestFailed;
		return m_lastTransactionResult;
	}

	m_buffer.Resize(s_maxDataSize);

	bool res=m_storageReader->Start(localPlayer, fileOwner, s_profileName, &m_buffer[0], m_buffer.Size() );
	m_lastTransactionResult=res?E_MPS_Pending:E_MPS_RequestFailed;

	m_storageTimer = 0.f;

	return m_lastTransactionResult;
}

bool C_MultiplayerProfileStorage::IsDownloadFinished(F32 timeStep)
{
	BIZ_ASSERT_SPEED(m_lastTransactionResult==E_MPS_Pending);

	if (m_lastTransactionResult!=E_MPS_Pending)
		return true;
	m_storageTimer += timeStep;

	U32 downloadedSize;
	NetPlay::C_PerUserServerStorageReader::ePerUserStorageReaderState state=m_storageReader->Continue(downloadedSize);

	if(m_storageTimer > g_debugStorageWaitTime)
	{
		m_storageReader->StopAndCleanup();
		m_storageTimer = 0.f;
		m_lastTransactionResult=E_MPS_RequestFailed;
		return true;
	}

	switch (state)
	{
	case NetPlay::C_PerUserServerStorageReader::eReadSuccessful:
		ProcessDownloadSuccess(downloadedSize);
		return true;
	case NetPlay::C_PerUserServerStorageReader::eReadInProgress:
		return false;
	case NetPlay::C_PerUserServerStorageReader::eReadFileDoesNotExist:
		m_lastTransactionResult=E_MPS_DoesNotExist;
		return true;
	}

	m_lastTransactionResult=E_MPS_RequestFailed;
	return true;

	return false;
}

E_MPS_Result C_MultiplayerProfileStorage::GetDownloaded(Btl::Vector<U8>& buffer)
{
	BIZ_ASSERT_SPEED(m_hasValidDownloadedData);

	E_MPS_Result m_lastTransactionResult=ConvertData(buffer);

	return m_lastTransactionResult;
}


E_MPS_Result C_MultiplayerProfileStorage::ConvertData(Btl::Vector<U8>& buffer)
{
	U8* pointer=&m_buffer[0];
	E_MPS_Result res=CheckSignature(pointer);
	if (res!=E_MPS_Ok)
		return res;

	int profileSize=GetProfileSize(pointer);
	if (profileSize+s_headerSize!=m_buffer.Size())
		return E_MPS_Corrupted;
	buffer.Resize(profileSize);
	memcpy(&buffer[0], pointer, profileSize);

	return res;
}

E_MPS_Result C_MultiplayerProfileStorage::CheckSignature(U8*& data) const
{
	U32* array=(U32*)data;
	if (array[0]!=s_id)
		return E_MPS_Corrupted;

	if (array[1]!=m_version)
		return E_MPS_VersionMismatch;

	if (array[2]!=m_packVersion)
		return E_MPS_VersionMismatch;

	data+=12;
	return E_MPS_Ok;
}

int C_MultiplayerProfileStorage::GetProfileSize(U8*& data)
{
	U32* array=(U32*)data;
	data+=4;
	return *array;
}

void C_MultiplayerProfileStorage::GenerateData(const Sys::C_MemoryStream& stream)
{
	BIZ_ASSERT_SPEED(stream.GetBufferSize()<s_maxDataSize-s_headerSize);
	int profileSize=(int)(stream.GetBufferSize());
	int dataSize=profileSize+s_headerSize;

	m_buffer.Resize(dataSize);

	U32* destData=(U32*)&m_buffer[0];
	destData[0]=s_id;
	destData[1]=m_version;
	destData[2]=m_packVersion;
	destData[3]=profileSize;

	U8* source=stream.GetBufferPtr();
	memcpy(destData+4, source, profileSize);
}



E_MPS_Result C_MultiplayerProfileStorage::GetLastTransactionResult() const
{
	return m_lastTransactionResult;
}

bool C_MultiplayerProfileStorage::HasValidDownloadedData() const
{
	return m_hasValidDownloadedData;
}

void C_MultiplayerProfileStorage::ResetDownloadedData()
{
	m_hasValidDownloadedData=false;
	m_buffer.Resize(0);
}

void C_MultiplayerProfileStorage::ProcessDownloadSuccess( U32 downloadedSize )
{
	m_buffer.Resize(downloadedSize);

	if (downloadedSize<s_headerSize)
	{
		m_lastTransactionResult=E_MPS_Corrupted;
		return;
	}

	if (g_ForceProfileRebuild)
	{
		m_lastTransactionResult=E_MPS_Corrupted;
		return;
	}

	U8* pointer=&m_buffer[0];

	m_lastTransactionResult=CheckSignature(pointer);

	if (m_lastTransactionResult==E_MPS_Ok)
	{
		int profileSize=GetProfileSize(pointer);

		if (profileSize+s_headerSize!=m_buffer.Size())
		{
			m_lastTransactionResult=E_MPS_Corrupted;
		}
		else
		{
			m_hasValidDownloadedData=true;
		}			
	}
}

};



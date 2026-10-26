#ifndef _MULTIPLAYER_PROFILE_STORAGE_H_
#define _MULTIPLAYER_PROFILE_STORAGE_H_

namespace NetPlay
{
	class C_PerUserServerStorageWriter;
	class C_PerUserServerStorageReader;
};

namespace SysProfile
{
	struct FullProfile;
	struct UserIdentifier;
}

namespace Sys
{
	class C_MemoryStream;
}

namespace Game
{
//	class C_KeyArchiveManager;

enum E_MPS_Result
{
	E_MPS_Pending,
	E_MPS_Ok,
	E_MPS_DoesNotExist,
	E_MPS_Corrupted,
	E_MPS_VersionMismatch,
	E_MPS_RequestFailed,
};

class C_MultiplayerProfileStorage
{
public:
	C_MultiplayerProfileStorage(int version);
	~C_MultiplayerProfileStorage();

	void SetPackVersion(int packVersion);

	E_MPS_Result Upload(const Sys::C_MemoryStream& stream, const SysProfile::FullProfile& player);
	bool IsUploadFinished(F32 timeStep);

	E_MPS_Result Download(const SysProfile::FullProfile& localPlayer, const SysProfile::UserIdentifier& fileOwner);
	bool IsDownloadFinished(F32 timeStep);
	E_MPS_Result GetDownloaded(Btl::Vector<U8>& buffer);

	E_MPS_Result GetLastTransactionResult() const;

	bool HasValidDownloadedData() const;
	void ResetDownloadedData();

private:
	void ProcessDownloadSuccess( U32 downloadedSize );

	void GetDownloadedData(Btl::Vector<U64>& newData) const;
	E_MPS_Result ConvertData(Btl::Vector<U8>& buffer);
	E_MPS_Result CheckSignature(U8*& data) const;
	static int GetProfileSize(U8*& data);

	void GenerateData(const Sys::C_MemoryStream& stream);

	bool m_hasValidDownloadedData;
	Btl::Vector<U8> m_buffer;
	int m_version;
	int m_packVersion;
	E_MPS_Result m_lastTransactionResult;

	NetPlay::C_PerUserServerStorageWriter* m_storageWriter;
	NetPlay::C_PerUserServerStorageReader* m_storageReader;
	F32 m_storageTimer;

	static const U32 s_id;
	static const char* s_profileName;
	static const int s_maxDataSize;
	static const int s_headerSize;

};

};

#endif // _MULTIPLAYER_PROFILE_STORAGE_H_


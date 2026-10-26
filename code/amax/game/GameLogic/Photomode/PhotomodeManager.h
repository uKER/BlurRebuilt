#ifndef PHOTOMODE_MANAGER_H
#define PHOTOMODE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <jpeghelper/JpegDecoder.h>
#include <jpeghelper/JpegEncoder.h>

#include <misc_shared/messaging/messagebase.h>

namespace Game
{

class C_PhotomodeManager
{
public:

	C_PhotomodeManager()
	{
		mInUI=true;
	};

	void Init();
	
	bool UploadPhoto();
	void DownloadPhoto(int index);
	void DecodePhotoBuffer();

	void SetJpegTagStr(const char *str){mJpegTagStr=str;};

	static C_PhotomodeManager& Instance(){return s_instance;};

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	bool InUI(){return mInUI;};
	void InUI(bool in_ui){mInUI=in_ui;};

private:
	static C_PhotomodeManager s_instance;

	JpegHelper::C_JpegEncoder mJpegEncoder;
	JpegHelper::C_JpegDecoder mJpegDecoder;

	BYTE *mPhotoDownloadBuffer;
	int mPhotoDownloadBufferSize;

	Util::C_String256 mJpegTagStr;

	bool mInUI;
};

} // namespace Game

#endif // PHOTOMODE_MANAGER_H
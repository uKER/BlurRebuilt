//extern __declspec(dllimport) int TestFunc();

#ifndef WAVEBUNDLERDLL_H
#define WAVEBUNDLERDLL_H

#ifdef _MSC_VER
#pragma once
#endif

typedef void (*FuncNewAsyncOpMessage)(int msgType, Util::C_String& str, void* pContext);
//typedef void (*FuncSetAsyncOpProgress)(F32 fProgress, void* pContext);

struct WaveFileInfo
{
	WaveFileInfo()
	{
		nChannels = -1;
		sampleRate = -1;
		bLoop = -1;
		bitDepth = -1;
		bWavReadError = -1;
	};

	// pass this in
	Util::C_String strFilename;

	// get these passed back
	int			nChannels;			
	int			sampleRate;
	int			bLoop;
	int			bitDepth;
	int			bWavReadError;
};

struct BuildBankParams
{
	BuildBankParams()
	{
		newMsgFunc = 0;
		pContext = 0;
		setProgressFunc = 0;
		fProgressMin = 0.0f;
		fProgressMax = 1.f;
		pStrXmlText = 0;
		bQuantize = true;
		eQuantizeMethod = 0;	// resample
		iResamplerQuality = 0;	// best quality

		bWriteBitDepthFiles = false;
		bWriteQuantizedFiles = false;
		bWriteResampledFiles = false;
		bWriteEncodedFiles = false;
		bDeleteWavFiles = false;
	};

	Util::C_String* pStrXmlText;
	Util::C_String strOutputDir;

	void* pContext;
	FuncNewAsyncOpMessage newMsgFunc;
	FuncSetAsyncOpProgress setProgressFunc;
	F32 fProgressMin;
	F32 fProgressMax;

	bool bQuantize;
	int eQuantizeMethod;
	int iResamplerQuality;

	bool bWriteBitDepthFiles;
	bool bWriteQuantizedFiles;
	bool bWriteResampledFiles;
	bool bWriteEncodedFiles;
	bool bDeleteWavFiles;
};

#endif // WAVEBUNDLERDLL_H
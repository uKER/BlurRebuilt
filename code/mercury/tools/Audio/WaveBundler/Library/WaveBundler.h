#pragma once

// flags/settings for wavebundler execution
struct WbSettings
{
	WbSettings()
	{
		gbVerboseMode = false;
		gbDoSystemPause = true;
		gbWriteIntermediateBitDepthFiles = false;
		gbWriteIntermediateQuantizedFiles = false;
		gbWriteIntermediateResampledFiles = false;
		gbWriteIntermediateEncodedFiles = false;
		gbDeleteIntermediateFiles = false;
		gbWriteBankToDest = false;
		gbQuantizeEnabled = true;
		giQuantizeMethod = 0;
		giResamplerQuality = 0;	// best quality
	};

	bool gbVerboseMode;
	bool gbDoSystemPause;
	bool gbWriteIntermediateBitDepthFiles;
	bool gbWriteIntermediateQuantizedFiles;
	bool gbWriteIntermediateResampledFiles;
	bool gbWriteIntermediateEncodedFiles;
	bool gbDeleteIntermediateFiles;
	bool gbWriteBankToDest;
	bool gbQuantizeEnabled;
	int giQuantizeMethod;	// 0 for resample, 1 for insert/delete
	int giResamplerQuality;	
	Util::C_String gstrOutputPath;
};

extern WbSettings gSettings;
#ifndef CONVERSION_OPTIONS_H
#define CONVERSION_OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

class C_Options : public Tools::Shim::C_Options
{
public:
	bool ParseCommandLine(
		Util::C_String& xmlFilePath,
		int argc, 
		char* argv[]);

	virtual bool ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom);

	const Util::C_String& GetTextureFilePath() const;

	const Util::C_String& GetInputAssetFilePath() const;
	//Dummy filepath for input file in our temp folder.
	//This is where our ubercopy xml files will live.
	const Util::C_String& GetInputTempFilePath() const;
	
	const Util::C_String& GetOutputNameOverride() const;
	
	//Where our copied file resides in the working folder.
	const Util::C_String& GetOutputPath() const;

	//Texture Format
	const Util::C_String& GetTextureFormat() const;

	//Gamma
	// TRUE: SRGB texture, it is degamma corrected when bind. lighting, 3D. Need postprocess gamma correction.
	// FALSE: Standard, with not post process gamma correction
	bool GetGamma() const { return m_Gamma;};

	//Generate Mipmaps
	bool GetGenerateMipmaps() const { return m_GenerateMipmaps;};

	//Colour Normalize
	bool GetColourNormalize() const { return m_ColourNormalize;};

	//Resize
	bool GetResize(U32& width, U32& height) const
	{
		width = m_ResizeWidth; height = m_ResizeHeight;
		return m_Resize;
	};

private:
	bool ParseOutputDirectory(const Sys::C_XmlDom& dom);

	bool ParseConfiguration(const Sys::C_XmlDom& dom);

	bool ParseInputAssetFilePath(
		const Sys::C_XmlDomBaseNode* pNode,
		const Util::C_String& partialDirectory);

	Util::C_String m_inputTextureFile;
	//Dummy filepath for input file in our temp folder.
	//This is where our ubercopy xml files will live.
	Util::C_String m_inputTempFilePath;

	Util::C_String m_outputNameOverride;

	//Where our copied file resides in the working folder.
	Util::C_String m_outputPath;
	//Format
	Util::C_String m_TextureFormat;
	// Gamma
	bool m_Gamma;
	//Generate Mipmaps
	bool m_GenerateMipmaps;
	//Colour Normalize
	bool m_ColourNormalize;
	//Resize
	bool m_Resize;
	U32 m_ResizeWidth;
	U32 m_ResizeHeight;

};

#endif

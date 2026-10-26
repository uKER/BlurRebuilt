#ifndef CONVERSION_OPTIONS_H
#define CONVERSION_OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/tools/CycleBuilder.h>		// for C_CycleParameters
#include <animation/tools/Scene.h>				// for C_SceneConfig

class C_Options : public Tools::Shim::C_Options
{
public:
	bool ParseCommandLine(
		Util::C_String& xmlFilePath,
		int argc, 
		char* argv[]);

	virtual bool ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom);

	const Util::C_String& GetInputAssetFilePath() const;
	
	
	//Dummy filepath for input file in our temp folder.
	//This is where our ubercopy xml files will live.
	const Util::C_String& GetInputTempFilePath() const;
	
	
	//Where our copied file resides in the working folder.
	const Util::C_String& GetOutputPath() const;

	/// Get output filename, without extension. Typically same as input asset filename, but can be changed.
	const Util::C_String& GetOutputFilename() const;
	
	
	/// Get parameters to use when importing data from animated scenes.
	const AnimationTools::C_SceneConfig& GetSceneConfig() const;

	/// Get parameters to use when exporting cycles.
	const AnimationTools::C_CycleParameters& GetCycleParameters() const;
	
	const Util::C_String& GetInputSkeletonFilePath() const				{ return m_inputSkeletonFilePath; }
	
	const Util::C_String& GetInputCycleFilePath() const				{ return m_inputCycleFilePath; }
	
	const Util::C_String& GetInputSubtractedCycleFilePath() const		{ return m_inputSubtractedCycleFilePath; }
	
	enum E_Mode
	{
		E_Mode_ExportSkeleton	= 0,
		E_Mode_ExportCycle
	};
	
	E_Mode GetMode() const
	{
		return m_mode;
	}
	
	/// Returns true if an additive cycle needs to be created.
	bool IsAdditive() const		{ assert( m_mode == E_Mode_ExportCycle ); return m_isAdditive; }
	

private:
	bool ParseOutputDirectory(const Sys::C_XmlDom& dom);

	bool ParseCycleParameters( const Sys::C_XmlDomBaseNode* pNode );

	bool ParseSceneConfig( const Sys::C_XmlDomBaseNode* pNode );

	bool ParseInputAssetFilePath(
		const Sys::C_XmlDomBaseNode* pNode,
		const char* attributeName,
		const Util::C_String& partialDirectory,
		Util::C_String& outAssetFilePath);
		
	bool ParseBoolOption(
		const Sys::C_XmlDomBaseNode* pNode,
		const char* attributeName,
		bool& outValue
	) const;

	bool ParseStringOption(
		const Sys::C_XmlDomBaseNode* pNode,
		const char* attributeName,
		Util::C_String& outValue
	) const;

	bool ParseFloatOption(
		const Sys::C_XmlDomBaseNode* pNode,
		const char* attributeName,
		F32& outValue
	) const;
	
	
	
	bool ParseOutputDirRules(
		const Sys::C_XmlDomBaseNode* pNode,
		const char* ruleListTagName,
		Btl::Vector<Util::C_String>& outPatterns,
		Btl::Vector<Util::C_String>& outDirs
	) const;
	
	void SetRelativeOutputPath( const Util::C_String& relativePath );
		
	Util::C_String GetParentNodeTypeName() const;
	
	AnimationTools::C_SceneConfig		m_sceneConfig;
	AnimationTools::C_CycleParameters	m_cycleParameters;
		
	Util::C_String m_inputSkeletonFilePath;
	Util::C_String m_inputCycleFilePath;
	Util::C_String m_inputSubtractedCycleFilePath;

	Util::C_String m_inputAssetFilePath;	// Filename of primary source asset (cycle or skeleton depending on mode).
	
	Util::C_String m_inputTempFilePath;		// Dummy filepath for input file in our temp folder (where ubercopy xml files will live)
	Util::C_String m_outputPath;			// Where our copied file resides in the working folder.
	Util::C_String m_outputFilename;		// What the copied filename will be.
	E_Mode	m_mode;							// Export mode.
			
	bool m_isAdditive;
	bool m_useOutputDirRules;
};

#endif

#include "stdafx.h"
#include "Options.h"

#define EXECUTER_XML_ANIMATION_CONVERTER_FILE				"export_animation_converter_file"
#define EXECUTER_XML_ANIMATION_SKELETON_FILE				"export_animation_skeleton_file"
#define EXECUTER_XML_ANIMATION_CYCLE_FILE					"export_animation_cycle_file"
#define EXECUTER_XML_ANIMATION_CONVERTER_OUTPUT_PATH		"export_animation_converter_file_output_path"
#define EXECUTER_XML_ANIMATION_CONVERTER_OUTPUT_FILENAME	"export_animation_converter_file_output_filename"
#define EXECUTER_XML_ANIMATION_OUTPUT_DIR_FROM_FILENAME		"export_animation_output_dir_from_filename"

#define EXECUTER_XML_ANIMATION_CYCLE_FILENAME				"export_animation_cycle_filename"
#define EXECUTER_XML_ANIMATION_SKELETON_FILENAME			"export_animation_skeleton_filename"
#define EXECUTER_XML_ANIMATION_SUBTRACTED_CYCLE_FILENAME	"export_animation_subtracted_cycle_filename"

#define EXECUTER_XML_ANIMATION_EXPORT_ADDITIVE_CYCLE		"export_animation_additive_cycle"

#define EXECUTER_XML_ANIMATION_CYCLE_ROTATION_ERROR_THRESHOLD				"export_animation_cycle_rotation_error_threshold"
#define EXECUTER_XML_ANIMATION_CYCLE_TRANSLATION_ERROR_THRESHOLD			"export_animation_cycle_translation_error_threshold"
//#define EXECUTER_XML_ANIMATION_CYCLE_OPTIMISE_CHANNELS_MATCHING_BASE_POSE	"export_animation_cycle_optimise_channels_matching_base_pose"
#define EXECUTER_XML_ANIMATION_CYCLE_REMOVE_TRANSLATION_CHANNELS_MATCHING_BASE_POSE	"export_animation_cycle_remove_translation_channels_matching_base_pose"
#define EXECUTER_XML_ANIMATION_CYCLE_REMOVE_CHANNELS_WITH_NO_KEYS			"export_animation_cycle_remove_channels_with_no_keys"
#define EXECUTER_XML_ANIMATION_CYCLE_STORE_IN_WORLD_SPACE					"export_animation_cycle_store_in_world_space"

#define MISSING_XML_ATTR "Could not find required XML attribute in %s.\n"

OPTIMISE_OFF

bool C_Options::ParseCommandLine(
	Util::C_String& xmlFilePath,
	int argc, 
	char* argv[])
{
	try
	{  
		TCLAP::CmdLine cmd( "Animation Converter.", ' ', "1.00" );

		// Executer XML file argument.
		TCLAP::ValueArg<std::string> xmlFileArg(
			"","xml","XML file",true,"","Executer XML file");
		cmd.add(xmlFileArg);

		// Export cycle or skeleton switches (one or other must be specified).
		TCLAP::SwitchArg exportSkeletonArg( "", "export_skeleton", "Export skeleton.", false );
		TCLAP::SwitchArg exportCycleArg( "", "export_cycle", "Export cycle.", false );
		
		cmd.xorAdd( exportSkeletonArg, exportCycleArg );

		// Parse the args.
		cmd.parse(argc, argv);

		// Get the values
		xmlFilePath = xmlFileArg.getValue().c_str();
		
		if ( exportSkeletonArg.getValue() )
		{
			m_mode = E_Mode_ExportSkeleton;
		}
		else if (exportCycleArg.getValue() )
		{
			m_mode = E_Mode_ExportCycle;
		}
		else
		{
			assert( false );
		}
	} 
	catch (TCLAP::CmdLineParseException &e)
	{
		std::cerr << "Failed to parse arguments: " << e.argId() << std::endl;
		return false;
	}
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		return false;
	}

	return true;
}

bool C_Options::ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom)
{
	if (!Tools::Shim::C_Options::ParseXml(xmlFilePath, dom))
		return false;
		
	m_useOutputDirRules = false;

	//Where to put it.
	Util::C_String outputDirectory;

	if (!ParseOutputDirectory(dom))
		return false;
		
	if ( m_mode == E_Mode_ExportSkeleton )
	{
		// Get skeleton file path.
		if ( !ParseInputAssetFilePath(
				dom.GetRootNodePtr(),
				EXECUTER_XML_ANIMATION_SKELETON_FILENAME,
				outputDirectory,
				m_inputSkeletonFilePath ))
		{
			return false;
		}
		
		// Set input asset file path and output filename.
		m_inputAssetFilePath = m_inputSkeletonFilePath;
		m_outputFilename = Util::RemoveFileExtension( Util::ExtractFilename( m_inputAssetFilePath ) );
	}
	else if ( m_mode == E_Mode_ExportCycle )
	{
		// Get cycle file path.
		if ( !ParseInputAssetFilePath(
			dom.GetRootNodePtr(),
			EXECUTER_XML_ANIMATION_CYCLE_FILENAME,
			outputDirectory,
			m_inputCycleFilePath ))
		{
			return false;
		}
		
		m_inputAssetFilePath = m_inputCycleFilePath;
		m_outputFilename = Util::RemoveFileExtension( Util::ExtractFilename( m_inputAssetFilePath ) );
		
		// Parse additive flag.
		m_isAdditive = false;
		if ( !ParseBoolOption( dom.GetRootNodePtr(), EXECUTER_XML_ANIMATION_EXPORT_ADDITIVE_CYCLE, m_isAdditive ) )
		{
			return false;
		}
		
		if ( m_isAdditive )
		{
			
			if ( !ParseInputAssetFilePath(
				dom.GetRootNodePtr(),
				EXECUTER_XML_ANIMATION_SUBTRACTED_CYCLE_FILENAME,
				outputDirectory,
				m_inputSubtractedCycleFilePath )
			)
			{
				return false;
			}
		}
		
		// Parse the cycle parameters.
		if ( !ParseCycleParameters( dom.GetRootNodePtr() ) )
		{
			return false;
		}
	}
	
	// Parse scene config.
	if ( !ParseSceneConfig( dom.GetRootNodePtr() ) )
	{
		return false;
	}
		
	// Apply outputDir rules to derive output directory from input filename.
	if ( m_useOutputDirRules )
	{
		// Parse rules from config file.
		Btl::Vector<Util::C_String> patterns;
		Btl::Vector<Util::C_String> outputDirs;
		
		if ( !ParseOutputDirRules( dom.GetRootNodePtr(), "output_dir_rules", patterns, outputDirs ) )
		{
			return false;
		}
		
		assert( patterns.Size() == outputDirs.Size() );
		
		// Apply rules.
		const Util::C_String inputFilename = Util::ExtractFilename( m_inputAssetFilePath );
		bool foundMatch = false;
		for ( S32 i = 0; i < patterns.Size() && !foundMatch; ++i )
		{
			if ( 0 == inputFilename.ISearch( patterns[i] ) )
			{
				// Match! Override the derived output path.
				SetRelativeOutputPath( outputDirs[i] );
				foundMatch = true;
				break;
			}
		}
		
		if ( !foundMatch )
		{
			// No match - naming convention must be wrong.
			EXECUTER_FATAL("Input file '%s' does not match known naming convention.", inputFilename.Chars());
			return false;
		}
	}
	
	m_inputTempFilePath = m_inputAssetFilePath.IReplace(m_shimRootAssetPath, m_shimRootTempPath);
	if (m_inputTempFilePath == "")
		return false;

	return true;
}

const Util::C_String& C_Options::GetInputAssetFilePath() const
{
	return m_inputAssetFilePath;
}

const Util::C_String& C_Options::GetInputTempFilePath() const
{
	return m_inputTempFilePath;
}

const Util::C_String& C_Options::GetOutputPath() const
{
	return m_outputPath;
}

const Util::C_String& C_Options::GetOutputFilename() const
{
	return m_outputFilename;
}

const AnimationTools::C_SceneConfig& C_Options::GetSceneConfig() const
{
	return m_sceneConfig;
}

const AnimationTools::C_CycleParameters& C_Options::GetCycleParameters() const
{
	return m_cycleParameters;
}

bool C_Options::ParseCycleParameters( const Sys::C_XmlDomBaseNode* pNode )
{
	// Set defaults.
	m_cycleParameters = AnimationTools::C_CycleParameters();
		
	// Parse rotation error threshold.
	if ( !ParseFloatOption(
			pNode,
			EXECUTER_XML_ANIMATION_CYCLE_ROTATION_ERROR_THRESHOLD,
			m_cycleParameters.maxGlobalRotationError ) )
	{
		return false;
	}
	
	// Parse translation error threshold.
	if ( !ParseFloatOption(
			pNode,
			EXECUTER_XML_ANIMATION_CYCLE_TRANSLATION_ERROR_THRESHOLD,
			m_cycleParameters.maxGlobalTranslationError ) )
	{
		return false;
	}
	
	// Parse "remove translation channels that match base pose" flag.
	if ( !ParseBoolOption(
			pNode,
			EXECUTER_XML_ANIMATION_CYCLE_REMOVE_TRANSLATION_CHANNELS_MATCHING_BASE_POSE,
			m_cycleParameters.removeTranslationChannelsMatchingBasePose ) )
	{
		return false;
	}	

	// Parse "remove channels without keys" flag.
	if ( !ParseBoolOption(
			pNode,
			EXECUTER_XML_ANIMATION_CYCLE_REMOVE_CHANNELS_WITH_NO_KEYS,
			m_cycleParameters.removeChannelsWithNoKeys ) )
	{
		return false;
	}
/*
	// Parse "store in world space" flag.
	if ( !ParseBoolOption(
			pNode,
			EXECUTER_XML_ANIMATION_CYCLE_STORE_IN_WORLD_SPACE,
			m_cycleParameters.inWorldSpace ) )
	{
		return false;
	}
*/

	// Read "use naming convention to decide output directory" flag.
	m_useOutputDirRules = false;
	if ( !ParseBoolOption( pNode, EXECUTER_XML_ANIMATION_OUTPUT_DIR_FROM_FILENAME, m_useOutputDirRules ) )
	{
		return false;
	}
		
	// Read replacement filename. If blank, use the input filename.
	Util::C_String outputFilename;
	if ( !ParseStringOption( pNode, EXECUTER_XML_ANIMATION_CONVERTER_OUTPUT_FILENAME, outputFilename ) )
	{
		return false;
	}
	
	if ( outputFilename.Length() > 0 )
	{
		m_outputFilename = outputFilename;
	}
	
	return true;
}

inline Util::C_String GetValueAttribute( const Sys::C_XmlDomBaseNode* pNode )
{
	Util::C_String value;
	if ( pNode != NULL && pNode->GetType() == Sys::E_XmlDomNodeType_Element )
	{
		value = static_cast<const Sys::C_XmlDomElement*>(pNode)->GetAttributes().GetAttributeByName("value")->value;
	}
	return value;
}

inline bool GetAttribute( const Sys::C_XmlDomBaseNode* pNode, const char* name, Util::C_String& outValue )
{
	if ( pNode != NULL && pNode->GetType() == Sys::E_XmlDomNodeType_Element )
	{
		outValue = static_cast<const Sys::C_XmlDomElement*>(pNode)->GetAttributes().GetAttributeByName(name)->value;
		return true;
	}
	return false;
}

bool C_Options::ParseSceneConfig( const Sys::C_XmlDomBaseNode* pNode )
{
	// Find <animation_converter_config> block.
	static const char* kAnimationConverterConfig = "animation_converter_config";
	const Sys::C_XmlDomBaseNode* animConverterConfig = Sys::C_XmlDomUtility::FindXmlNodeByName( pNode, kAnimationConverterConfig );
	
	if ( !animConverterConfig )
	{
		EXECUTER_FATAL("Could not find XML node %s.\n", kAnimationConverterConfig );
		return false;
		
	}
	
	AnimationTools::C_SceneConfig& sc = m_sceneConfig;
	
	// Reset scene config to its defaults.
	sc = AnimationTools::C_SceneConfig();
	
	// Extract scale factor.
	static const char* kSceneScale = "scene_scale";
	if ( const Sys::C_XmlDomBaseNode* sceneScaleNode = Sys::C_XmlDomUtility::FindXmlNodeByName( animConverterConfig, kSceneScale ) )
	{
		Util::C_String sceneScaleStr = GetValueAttribute( sceneScaleNode );
		
		F32 scale = static_cast<F32>(atof( sceneScaleStr.Chars() ));
		
		if ( scale == 0.f )
		{
			EXECUTER_WARN( "Invalid or missing scene scale parameter\n" );
			scale = 1.f;
		}
		sc.scaleFactor = scale;
	}
		
	static const char* kAnimationNamingConvention = "animation_naming_convention";
	if ( const Sys::C_XmlDomBaseNode* namingConventionNode = Sys::C_XmlDomUtility::FindXmlNodeByName( animConverterConfig, kAnimationNamingConvention ) )
	{
		for ( S32 i = 0; i < namingConventionNode->GetChildren().Size(); ++i )
		{
			const Sys::C_XmlDomBaseNode* child = namingConventionNode->GetChildren()[ i ];
			
			if ( child->GetName() == "skeleton_root" )
			{
				sc.rootNodeNames.Add( GetValueAttribute( child ) );
			}
			else if ( child->GetName() == "dynamic_aim_joint" )
			{
				sc.dynamicAimJointNames.Add( GetValueAttribute( child ) );
			}
			else if ( child->GetName() == "dynamic_slider_joint" )
			{
				sc.dynamicSliderJointNames.Add( GetValueAttribute( child ) );
			}
			else if ( child->GetName() == "exclude" )
			{
				sc.excludedNodeNames.Add( GetValueAttribute( child ) );
			}
			else if ( child->GetName() == "reference" )
			{
				sc.referenceNodeNames.Add( GetValueAttribute( child ) );
			}
		}
	}	

	/*
	m_skeletonParameters.Reset();
	
	m_skeletonParameters.AddExcludedNodeNamePattern( "*_end" );
	m_skeletonParameters.AddExcludedNodeNamePattern( "*_target" );
	m_skeletonParameters.AddExcludedNodeNamePattern( "*_null" );
	m_skeletonParameters.AddExcludedNodeNamePattern( "*_cust" );
	
	m_skeletonParameters.AddDynamicJointNamePattern( "*_dynamic" );
	
	// Exclude XSI things.
	
	m_skeletonParameters.AddExcludedNodeNamePattern( "root*" );
	m_skeletonParameters.AddExcludedNodeNamePattern( "eff*" );
	m_skeletonParameters.EnableRemovalOfRootNodeNamePrefixes( true );
	*/
	
	return true;
}

bool C_Options::ParseOutputDirectory(const Sys::C_XmlDom& dom)
{
	Util::C_String parentTypeName = GetParentNodeTypeName();
	
	const Sys::C_XmlDomBaseNode* pExportFile = 
		Sys::C_XmlDomUtility::FindXmlNodeByTypeNameAttributeSubStr(dom.GetRootNodePtr(), parentTypeName );
	if (!pExportFile)
	{
		EXECUTER_FATAL("Could not find XML node %s.\n", parentTypeName.Chars() );
		return false;
	}

	const Sys::C_XmlDomBaseNode* pSceneDirectoryNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pExportFile, EXECUTER_XML_ANIMATION_CONVERTER_OUTPUT_PATH);
	if (!pSceneDirectoryNode)
	{
		EXECUTER_FATAL("Could not find XML node %s.\n", EXECUTER_XML_ANIMATION_CONVERTER_OUTPUT_PATH);
		return false;
	}

	Util::C_String partialDirectory =
		((Sys::C_XmlDomElement*)pSceneDirectoryNode)->GetAttributes().GetAttributeByName("value")->value;
		
	Util::C_String test;
	bool ok = ParseStringOption( dom.GetRootNodePtr(), EXECUTER_XML_ANIMATION_CONVERTER_OUTPUT_PATH, test );
	assert( partialDirectory == test );
	assert( ok );
		
	SetRelativeOutputPath( partialDirectory );

	return true;
}

// Replace this with ParseStringOption?
bool C_Options::ParseInputAssetFilePath(
	const Sys::C_XmlDomBaseNode* pNode,
	const char* attributeName,
	const Util::C_String& partialDirectory,
	Util::C_String& outAssetFilePath)
{
	const Sys::C_XmlDomBaseNode* pExportFileNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(
		pNode,
		EXECUTER_XML_ANIMATION_CONVERTER_FILE );
	if (!pExportFileNode)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}

	const Sys::C_XmlDomBaseNode* pFileNameNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(
		pExportFileNode, attributeName );
	if (!pFileNameNode)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}

	const Sys::C_XmlAttribute* pFileName = 
		((Sys::C_XmlDomElement*)pFileNameNode)->GetAttributes().GetAttributeByName("value");
	if (!pFileName)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}

	outAssetFilePath = pFileName->value;

	return true;
}

bool C_Options::ParseBoolOption(
	const Sys::C_XmlDomBaseNode* pNode,
	const char* attributeName,
	bool& outValue
) const
{
	//const Sys::C_XmlDomBaseNode* pExportFileNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(
	//	pNode,
	//	EXECUTER_XML_ANIMATION_CONVERTER_FILE );
	//if (!pExportFileNode)
	//{
	//	EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
	//	return false;
	//}

	const Sys::C_XmlDomBaseNode* attrNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pNode, attributeName );
	if (!attrNode)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}

	const Sys::C_XmlAttribute* attr = 
		static_cast< const Sys::C_XmlDomElement* >( attrNode )->GetAttributes().GetAttributeByName("value");
	if (!attr)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}
	
	outValue = ( attr->value.ISearch( "true" ) ) >= 0;
	
	return true;
}

bool C_Options::ParseStringOption(
	const Sys::C_XmlDomBaseNode* pNode,
	const char* attributeName,
	Util::C_String& outValue
) const
{
	//const Sys::C_XmlDomBaseNode* pExportFileNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(
	//	pNode,
	//	EXECUTER_XML_ANIMATION_CONVERTER_FILE );
	//if (!pExportFileNode)
	//{
	//	EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
	//	return false;
	//}

	const Sys::C_XmlDomBaseNode* attrNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pNode, attributeName );
	if (!attrNode)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}

	const Sys::C_XmlAttribute* attr = 
		static_cast< const Sys::C_XmlDomElement* >( attrNode )->GetAttributes().GetAttributeByName("value");
	if (!attr)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}
	
	outValue = attr->value;
	
	return true;
}

bool C_Options::ParseFloatOption(
	const Sys::C_XmlDomBaseNode* pNode,
	const char* attributeName,
	F32& outValue
) const
{
	//const Sys::C_XmlDomBaseNode* pExportFileNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(
	//	pNode,
	//	EXECUTER_XML_ANIMATION_CONVERTER_FILE );
	//if (!pExportFileNode)
	//{
	//	EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
	//	return false;
	//}

	const Sys::C_XmlDomBaseNode* attrNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pNode, attributeName );
	if (!attrNode)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}

	const Sys::C_XmlAttribute* attr = 
		static_cast< const Sys::C_XmlDomElement* >( attrNode )->GetAttributes().GetAttributeByName("value");
	if (!attr)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}
	
	outValue = (F32) atof( attr->value.Chars() );
	
	return true;
}

Util::C_String C_Options::GetParentNodeTypeName() const
{
	Util::C_String parentTypeName;
	
	switch ( m_mode )
	{
		case E_Mode_ExportSkeleton: parentTypeName = EXECUTER_XML_ANIMATION_SKELETON_FILE; break;
		case E_Mode_ExportCycle: parentTypeName = EXECUTER_XML_ANIMATION_CYCLE_FILE; break;
		default: assert( false ); break;
	}
	return parentTypeName;
}

bool C_Options::ParseOutputDirRules(
	const Sys::C_XmlDomBaseNode* pNode,
	const char* ruleListTagName,
	Btl::Vector<Util::C_String>& outPatterns,
	Btl::Vector<Util::C_String>& outDirs
) const
{
	const char* kOutputDirRules = "output_dir_rules";
	const Sys::C_XmlDomBaseNode* pRuleListNode = Sys::C_XmlDomUtility::FindXmlNodeByName( pNode, ruleListTagName );
	if ( !pRuleListNode )
	{
		EXECUTER_FATAL( "Missing %s node\n", ruleListTagName );
		return false;
	}
	
	outPatterns.Clear();
	outDirs.Clear();
	
	U32 numChildren = pRuleListNode->GetChildren().Size();
	
	for ( U32 i = 0; i < numChildren; ++i )
	{
		const Sys::C_XmlDomBaseNode* pChild = pRuleListNode->GetChildren()[i];
		
		if ( pChild->GetName() == "output_dir_rule" )
		{
			Util::C_String pattern;
			Util::C_String outputDir;
			
			if ( GetAttribute( pChild, "pattern", pattern ) && GetAttribute( pChild, "outputDir", outputDir ) )
			{
				outPatterns.PushBack( pattern );
				outDirs.PushBack( outputDir );
			}
			else
			{
				EXECUTER_FATAL( "<output_dir_rule> tag is missing 'pattern' or 'outputDir' attributes\n" );
				return false;
			}
		}
	}
	
	return true;
}


void C_Options::SetRelativeOutputPath( const Util::C_String& relativePath )
{
	Util::C_String partialDirectory = Util::FormatWindowsFileString(relativePath);
	partialDirectory = Util::RemoveTrailingSlash(partialDirectory);
	partialDirectory = Util::RemoveLeadingSlash(partialDirectory);

	m_outputPath = 
		m_shimRootTempPath + 
		"\\" + 
		partialDirectory + 
		"\\";
}



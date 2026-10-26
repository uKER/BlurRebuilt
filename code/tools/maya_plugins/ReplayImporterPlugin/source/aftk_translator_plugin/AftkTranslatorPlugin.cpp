#include "stdafx.h"

#include <system\DebugLog.h>
#include "MayaSubscriber.h"

char g_versionString[512];

#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MPxLocatorNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MDgModifier.h>
#include <maya/MQuaternion.h>
#include <maya/MEulerRotation.h>
#include <maya/MArgList.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>

#include "misc_shared/archive/archive.h"
#include "misc_shared/archive/btl.h"
#include "system/serialise/C_FileSystem.h"
#include "system/serialise/C_FileBlock.h"

AftkMayaPlugin::AftkMayaPlugin()
{
/*	m_attrFlag = 0;
	m_createFlag = 0;
	m_connectionFlag = 0;
	m_mayaFlagsAllocated = false;
	*/
}

AftkMayaPlugin::~AftkMayaPlugin()
{
//	Clear();
}

/*inline MString AftkMayaPlugin::filter() const
{
	return m_filter;
}

inline MString AftkMayaPlugin::defaultExtension() const
{	
	return m_extension;
}


inline bool AftkMayaPlugin::haveReadMethod() const
{	
	return true;
}

inline bool AftkMayaPlugin::haveWriteMethod() const
{	
	return true;
}

inline void AftkMayaPlugin::setPluginName(const MString& name)
{	
	m_pluginName = name;	
}

inline MString AftkMayaPlugin::translatorName()
{	
	return m_translatorName;	
}*/

void* AftkMayaPlugin::creator()
{
	return new AftkMayaPlugin();
}

/*
#pragma warning(push)
#pragma warning(disable : 4018)
///
/// Maya calls this method to find out if this translator is capable of
/// handling the given file.
///
MPxFileTranslator::MFileKind AftkMayaPlugin::identifyFile(
		const MFileObject& file, const char* buffer, short bufferLen
) const
{
	MString	tagStr = m_translatorName;
	int		tagLen = tagStr.length();

	//
	// If the buffer contains enough info to positively identify the file,
	// then use it.  Otherwise we'll base the identification on the file
	// extension.
	//
	if (bufferLen >= tagLen)
	{
		MString	initialContents(buffer, bufferLen);
		MStringArray	initialLines;

		initialContents.split('\n', initialLines);

		if (initialLines.length() > 0)
		{
			if ((initialLines[0].length() >= tagLen)
			&&	(initialLines[0].substring(0, tagLen-1) == tagStr))
			{
				return kIsMyFileType;
			}
		}
	}
	else
	{
		MString	fileName(file.name());
		int		fileNameLen = fileName.length();
		int		startOfExtension = fileName.rindex('.') + 1;

		if ((startOfExtension > 0)
		&&	(startOfExtension < fileNameLen)
		&&	(fileName.substring(startOfExtension, fileNameLen) == m_extension))
		{
			return kIsMyFileType;
		}
	}

	return kNotMyFileType;
}

#pragma warning(pop)
*/


/*
MStatus AftkMayaPlugin::AllocateFlags()
{
	MStatus mStatus;
	// Get some node flags to keep track of those nodes for which we
	// have already done various stages of processing.
	m_createFlag = MFnDependencyNode::allocateFlag(m_pluginName, &mStatus);
	if (!mStatus) return mStatus;
	m_attrFlag = MFnDependencyNode::allocateFlag(m_pluginName, &mStatus);
	if (!mStatus) return mStatus;
	m_connectionFlag = MFnDependencyNode::allocateFlag(m_pluginName, &mStatus);
	if (!mStatus) return mStatus;
	m_mayaFlagsAllocated = true;
	return MS::kSuccess;
}
*/

/*MStatus AftkMayaPlugin::ClearFlags()
{
	// Run through all of the dg nodes in the scene and clear their flags.
	MItDependencyNodes	nodesIter;
	for (; !nodesIter.isDone(); nodesIter.next()) 
	{
		MObject				node = nodesIter.item();
		MFnDependencyNode	nodeFn(node);

		nodeFn.setFlag(m_createFlag, false);
		nodeFn.setFlag(m_attrFlag, false);
		nodeFn.setFlag(m_connectionFlag, false);
	}

	return MS::kSuccess;
}

void AftkMayaPlugin::ReleaseFlags()
{
	if (m_mayaFlagsAllocated)
	{
		MFnDependencyNode::deallocateFlag(m_pluginName, m_createFlag);
		MFnDependencyNode::deallocateFlag(m_pluginName, m_attrFlag);
		MFnDependencyNode::deallocateFlag(m_pluginName, m_connectionFlag);
		m_mayaFlagsAllocated = false;
	}
}
//
// Maya calls this method to have the translator write out a file.
//
MStatus AftkMayaPlugin::writer(
	const MFileObject& file,
	const MString& options,
	MPxFileTranslator::FileAccessMode mode
)
{
	MString processedOptions(options);

	GuiResetCounts();

	MStatus mStatus;

	mStatus = AllocateFlags();
	CHECK_ERROR_RETURN;
	mStatus = ClearFlags();
	CHECK_ERROR_RETURN;

	//Are we exporting the current selection only?
	if (mode == kExportActiveAccessMode)
		processedOptions += MString(";exportSelection=1");
	else
		processedOptions += MString(";exportSelection=0");

	// Let's see if we can open the output file.
	std::fstream output(file.fullName().asChar(), std::ios::out | std::ios::trunc);
	if (output.good() == false) 
	{
		Clear();
		return MS::kNotFound;
	}
	output.close();

	DWORD dwStart = GetTickCount();
	{
		GuiProgressCallback guiProgressCallback;
		guiProgressCallback.StartProgress();

		Aftk::C_Status aftkStatus;
		Aftk::C_Scene aftkScene;
		
		aftkStatus = aftkScene.ReadExternalScene(
			Aftk::E_ExternalScene_Maya,
			processedOptions.asChar(),
			&guiProgressCallback);

		guiProgressCallback.EndProgress();
		if (!aftkStatus)
		{
			Clear();
			GuiDisplayExportSummary();
			EmailReport(aftkStatus);
			return MS::kFailure;
		}

		guiProgressCallback.StartProgress();
		guiProgressCallback.OnProgress(0);
		guiProgressCallback.OnPrintProgress("Writing file...");

		std::string fileName = file.fullName().asChar();
		{
			std::string extension;
			U32 length = (U32)fileName.length();
			if (length < 5)
			{
				#ifdef XSI_FILE_TRANSLATOR
				fileName += ".xsi";
				#elif A3D_FILE_TRANSLATOR
				fileName += ".a3d";
				#endif
			}
			else
			{
				extension = fileName.substr(length-4, 4);
				std::transform(extension.begin(), extension.end(), extension.begin(), tolower); 

				#ifdef XSI_FILE_TRANSLATOR
				if (extension != std::string(".xsi")) 
				{
					fileName += ".xsi";
				}
				#elif A3D_FILE_TRANSLATOR
				if (extension != std::string(".a3d")) 
				{
					fileName += ".a3d";
				}
				#endif
			}
		}
		#ifdef XSI_FILE_TRANSLATOR
		aftkStatus = aftkScene.Write(
			fileName, 
			Aftk::E_FileType_Xsi,
			0, 
			0,
			&guiProgressCallback);
		#elif A3D_FILE_TRANSLATOR
		aftkStatus = aftkScene.Write(
			fileName, 
			Aftk::E_FileType_A3d, 
			0, 
			0,
			&guiProgressCallback);
		#endif
		if (!aftkStatus)
		{
			S_ERROR(AftkMaya::g_mainLogger, 
				"File writing failed. Aftk says [" 
				<< aftkStatus.GetErrorString() 
				<< "].");
			return MStatus::kFailure;
		}

		guiProgressCallback.EndProgress();
	}

	

	//If everything went okay, then output time statistics.
	if (mStatus)
	{
		DWORD dwEnd = GetTickCount();

		DWORD elapsedTime = dwEnd - dwStart;
		F32 time = (F32)elapsedTime;
		time *= 0.001f;
		std::string messageString = "Scene export completed in ";
		{
			char buffer[50];
			sprintf(buffer, "%3.2f", time);
			messageString += buffer;
		}
		messageString += " seconds.\n";
		S_INFO(AftkMaya::g_mainLogger, messageString);
	}

	//Post a message to all windows indicating that we're finished.
	{
		U32 message = RegisterWindowMessage("AftkMayaExportComplete");
		PostMessage(HWND_BROADCAST, message, 0, 0);
	}

	Clear();
	return mStatus;
}

void AftkMayaPlugin::GetAddAttrCmds(const MObject& node, MStringArray& cmds)
{
	//
	// Run through the node's attributes.
	//
	MFnDependencyNode	nodeFn(node);
	unsigned int		numAttrs = nodeFn.attributeCount();
	unsigned int		i;

	for (i = 0; i < numAttrs; i++)
	{
		//
		// Use the attribute ordering which Maya uses when doing I/O.
		//
		MObject	attr = nodeFn.reorderedAttribute(i);

		//
		// If this attribute has been added since the node was created,
		// then we may want to write out an addAttr statement for it.
		//
		if (nodeFn.isNewAttribute(attr))
		{
			MFnAttribute	attrFn(attr);

			//
			// If the attribute has a parent then ignore it because it will
			// be processed when we process the parent.
			//
			MStatus	status;

			attrFn.parent(&status);

			if (status == MS::kNotFound)
			{
				//
				// If the attribute is a compound, then we can do its entire
				// tree at once.
				//
				MFnCompoundAttribute	cAttrFn(attr, &status);

				if (status)
				{
					MStringArray	newCmds;

					cAttrFn.getAddAttrCmds(newCmds);

					unsigned int	numCommands = newCmds.length();
					unsigned int	c;

					for (c = 0; c < numCommands; c++)
					{
						if (newCmds[c] != "")
							cmds.append(newCmds[c]);
					}
				}
				else
				{
					MString	newCmd = attrFn.getAddAttrCmd();

					if (newCmd != "") cmds.append(newCmd);
				}
			}
		}
	}
}


void AftkMayaPlugin::GetSetAttrCmds(const MObject& node, MStringArray& cmds)
{
	//
	// Get rid of any garbage already in the array.
	//
	cmds.clear();

	//
	// Run through the node's attributes.
	//
	MFnDependencyNode	nodeFn(node);
	unsigned int		numAttrs = nodeFn.attributeCount();
	unsigned int		i;

	for (i = 0; i < numAttrs; i++)
	{
		//
		// Use the attribute ordering which Maya uses when doing I/O.
		//
		MObject			attr = nodeFn.reorderedAttribute(i);
		MFnAttribute	attrFn(attr);
		MStatus			status;

		attrFn.parent(&status);

		bool			isChild = (status != MS::kNotFound);

		//
		// We don't want attributes which are children of other attributes
		// because they will be processed when we process the parent.
		//
		// And we only want storable attributes which accept inputs.
		//
		if (!isChild && attrFn.isStorable() && attrFn.isWritable())
		{
			//
			// Get a plug for the attribute.
			//
			MPlug	plug(node, attr);

			//
			// Get setAttr commands for this attribute, and any of its
			// children, which have had their values changed by the scene.
			//
			MStringArray	newCmds;

			plug.getSetAttrCmds(newCmds, MPlug::kChanged, false);

			unsigned int	numCommands = newCmds.length();
			unsigned int	c;

			for (c = 0; c < numCommands; c++)
			{
				if (newCmds[c] != "")
				{
					cmds.append(newCmds[c]);
				}
			}
		}
	}
}


MStatus AftkMayaPlugin::reader(
	const MFileObject& file,
	const MString& options,
	FileAccessMode mode
)
{
	GuiResetCounts();

	MStatus mStatus;

	mStatus = AllocateFlags();
	CHECK_ERROR_RETURN;
	mStatus = ClearFlags();
	CHECK_ERROR_RETURN;

	DWORD dwStart = GetTickCount();
	{
		GuiProgressCallback guiProgressCallback;
		guiProgressCallback.StartProgress();
		guiProgressCallback.SetTitle("Reading file...");

		Aftk::E_FileType fileType;
		Aftk::C_Status aftkStatus = Aftk::C_Global::GetFileType(file.name().asChar(), fileType);
		if (!aftkStatus) 
		{
			S_ERROR(AftkMaya::g_mainLogger, aftkStatus.GetErrorString()); return MStatus::kFailure;
		}
		if (fileType == Aftk::E_FileType_Unknown)
		{
			S_ERROR(AftkMaya::g_mainLogger, "Failed to read file. Unknown file type."); return MStatus::kFailure;
		}

		#ifdef A3D_FILE_TRANSLATOR
		U32 highestVersion;
		aftkStatus = Aftk::C_Global::GetHighestSupportedFileVersion(fileType, highestVersion);
		if (!aftkStatus)
		{
			S_ERROR(AftkMaya::g_mainLogger, aftkStatus.GetErrorString()); return MStatus::kFailure;
		}
		U32 fileVersion;
		aftkStatus = Aftk::C_Global::GetFileVersion(file.name().asChar(), fileVersion);
		if (!aftkStatus)
		{
			S_ERROR(AftkMaya::g_mainLogger, aftkStatus.GetErrorString()); return MStatus::kFailure;
		}
		if (fileVersion > highestVersion)
		{
			S_ERROR(AftkMaya::g_mainLogger,
				"Could not read file. The file is of version " 
				<< fileVersion 
				<< " while the highest supported version is " 
				<< highestVersion);
			return MStatus::kFailure;
		}
		#endif

		Aftk::C_Scene aftkScene;
		//Read scene from disk
		aftkStatus = aftkScene.Read(
			file.name().asChar(),
			fileType,
			0,
			&guiProgressCallback);
		if (!aftkStatus)
		{
			S_ERROR(AftkMaya::g_mainLogger,
				"File reading failed. Aftk says [" 
				<< aftkStatus.GetErrorString() 
				<< "].");
			return MStatus::kFailure;
		}
		guiProgressCallback.EndProgress();

		guiProgressCallback.StartProgress();
		guiProgressCallback.SetTitle("Expanding poly-verts...");
		//Traverse the entire scene and expand polygon vertices
		Aftk::C_SceneNodeVisit::Traverse(
			&aftkScene,	
			Aftk::C_MeshExpandPolygonVertices::C_ExpandPolygonVerticesVisitor(),
			&guiProgressCallback);
		guiProgressCallback.EndProgress();

		guiProgressCallback.StartProgress();
		guiProgressCallback.SetTitle("Importing scene...");
		
		aftkStatus = aftkScene.WriteExternalScene(
			Aftk::E_ExternalScene_Maya,
			options.asChar(),
			&guiProgressCallback);

		guiProgressCallback.EndProgress();
		if (!aftkStatus)
		{
			Clear();
			GuiDisplayExportSummary();
			EmailReport(aftkStatus);
			return MS::kFailure;
		}
	}

	DWORD dwEnd = GetTickCount();

	DWORD elapsedTime = dwEnd - dwStart;
	float time = (float) elapsedTime;
	time *= 0.001f;
	std::string messageString = "Scene import completed in ";
	{
		char buffer[50];
		sprintf(buffer, "%3.2f", time);
		messageString += buffer;
	}
	messageString += " seconds.\n";

	S_INFO(AftkMaya::g_mainLogger, messageString);

	Clear();
	return mStatus;
}

void AftkMayaPlugin::Clear()
{
	ReleaseFlags();
}
*/

/*
const MString*	AftkMayaPlugin::GetPluginName()
{
	return &m_pluginName;
}
*/
struct C_GameObjectRecord
{
	Btl::String name;

	/// Write to stream
	template < typename T_Stream >
	FORCE_INLINE void Serialise( T_Stream& stream )
	{
		stream << name;
	}

	/// Read from stream
	template < typename T_Stream >
	FORCE_INLINE void Deserialise( T_Stream& stream )
	{
		stream >> name;
	}
};

struct C_GameObjectSample
{
	F32						time;
	MathsStorage::Float4*	pQuat;
	MathsStorage::Float3*	pPos;

	/// Write to stream
	template < typename T_Stream >
	FORCE_INLINE void Serialise( T_Stream& stream, U32 nActors)
	{
		stream << time;
		for (U32 i = 0; i < nActors; ++i)
		{
			pQuat[i].Serialise(stream);
			pPos[i].Serialise(stream);
		}
	}

	/// Read from stream
	template < typename T_Stream >
	FORCE_INLINE void Deserialise( T_Stream& stream, U32 nActors )
	{
		stream >> time;
		for (U32 i = 0; i < nActors; ++i)
		{
			pQuat[i].Deserialise(stream);
			pPos[i].Deserialise(stream);
		}
	}
};

MStatus	AftkMayaPlugin::doIt(const MArgList& args)
{
	Btl::Vector<C_GameObjectRecord> myActorTable;
	Btl::Vector<C_GameObjectSample> samples;

	Btl::Vector<Btl::Vector<MathsStorage::Float4> > actorQuats;
	Btl::Vector<Btl::Vector<MathsStorage::Float3> > actorTrans;

	MStatus status;
	MString filename = args.asString(0, &status);
	if (status != MS::kSuccess)
		return status;

	{
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(
			filename.asChar(), 
			Sys::E_FileAccess_Read);
		if (!pFile)
			return MS::kFailure;

		U32 endianDetector;
		pFile->Read(&endianDetector, 4);
		
		if (endianDetector != 1)
			pFile->SetEndianSwapFlag(true);

		//Header
		{
			Sys::C_FileBlock headerBlock(pFile);
			Util::C_String name = headerBlock.Name();

			const Sys::C_StreamPos pos0 = headerBlock.GetPosition();

			{
				Arch::BinaryInputArchive ar(headerBlock);
				U32 version;
				ar >> version;
			}

			const Sys::C_StreamPos pos1 = headerBlock.GetPosition();

			//Table describing actors.
			U32 nActors = 0;
			{
				Sys::C_FileBlock gameDescBlock(&headerBlock);
				Btl::String str = gameDescBlock.GetBlockHeader().m_Id;
				if (!strcmp(gameDescBlock.GetBlockHeader().m_Id, "objdesc"))
				{
					Arch::BinaryInputArchive ar(gameDescBlock);
					ar >> nActors;
					myActorTable.Resize(nActors);
					for (U32 i = 0; i < nActors; ++i)
					{
						const Sys::C_StreamPos pos2 = headerBlock.GetPosition();
						myActorTable[i].Deserialise(ar);
					}
				}
			}

			const Sys::C_StreamPos pos4 = headerBlock.GetPosition();

			//Data per frame.
			{
				Sys::C_FileBlock gameObjectBlock(&headerBlock);
				Btl::String str = gameObjectBlock.GetBlockHeader().m_Id;
				if (!strcmp(gameObjectBlock.GetBlockHeader().m_Id, "samples"))
				{
					U32 nFrames;

					Arch::BinaryInputArchive ar(gameObjectBlock);
					ar >> nFrames;
					samples.Resize(nFrames);
					actorQuats.Resize(nFrames);
					actorTrans.Resize(nFrames);

					for (U32 i = 0; i < nFrames; ++i)
					{
						actorQuats[i].Resize(nActors);
						actorTrans[i].Resize(nActors);
						samples[i].pQuat = &(actorQuats[i][0]);
						samples[i].pPos = &(actorTrans[i][0]);
						samples[i].Deserialise(ar, nActors);
					}
				}
			}
		}
		SAFE_DELETE_PTR(pFile);
	}


	int actorNo = 0;

	for (Btl::Vector<C_GameObjectRecord>::Iterator actor = myActorTable.Begin(); actor != myActorTable.End(); ++actor, ++actorNo)
	{
		MFnDagNode locatorNode;
		MFnMesh visNode;
			
		MStatus status;

		Btl::String actorName = actor->name;

		MObject locatorObject = locatorNode.create("locator", MObject::kNullObj, &status);

		if (status != MStatus::kSuccess)
			return status;

		locatorNode.setName(actorName.Chars());

		MFnTransform locatorTransform(locatorObject);
		locatorTransform.setRotationOrder(MTransformationMatrix::kZYX, true);

		MFloatPointArray visVertices;

		float xSize = 0.5f;
		float yMin = 0.0f;
		float yMax = 1.0f;
		float zSize = 1.0f;
		MFloatPoint vtx_1( -xSize, yMin, -zSize );
        MFloatPoint vtx_2(  xSize, yMin, -zSize );
        MFloatPoint vtx_3(  xSize, yMin,  zSize );
        MFloatPoint vtx_4( -xSize, yMin,  zSize );
        MFloatPoint vtx_5( -xSize, yMax, -zSize );
        MFloatPoint vtx_6( -xSize, yMax,  zSize );
        MFloatPoint vtx_7(  xSize, yMax,  zSize );
        MFloatPoint vtx_8(  xSize, yMax, -zSize );
        visVertices.append( vtx_1 );
        visVertices.append( vtx_2 );
        visVertices.append( vtx_3 );
        visVertices.append( vtx_4 );
        visVertices.append( vtx_5 );
        visVertices.append( vtx_6 );
        visVertices.append( vtx_7 );
        visVertices.append( vtx_8 );

		int polyCounts[] = {4,4,4,4,4,4};
		MIntArray visPolyCounts(polyCounts, 6);
		int polyConnects[] = { 0, 1, 2, 3, 4, 5, 6, 7, 3, 2, 6, 5, 0, 3, 5, 4, 0, 4, 7, 1, 1, 7, 6, 2 };
		MIntArray visPolyConnects(polyConnects, 24);

		MObject visObject = visNode.create(8, 6, visVertices, visPolyCounts, visPolyConnects, locatorObject, &status);

	    if (status != MStatus::kSuccess)
			return status;

		MFnTransform visTransform(visObject, &status);
		double visScale[3] = {1,1,2};
		visTransform.setScale(visScale);

		Btl::String keyFrameCommand;
		Btl::String moveCommand;
		Btl::String rotateCommand;

		MFnAnimCurve translateX, translateY, translateZ;
		MFnAnimCurve rotateX, rotateY, rotateZ, rotateW;
		
		translateX.create(locatorObject, locatorNode.attribute("translateX"));
		translateY.create(locatorObject, locatorNode.attribute("translateY"));
		translateZ.create(locatorObject, locatorNode.attribute("translateZ"));
		rotateX.create(locatorObject, locatorNode.attribute("rotateX"));
		rotateY.create(locatorObject, locatorNode.attribute("rotateY"));
		rotateZ.create(locatorObject, locatorNode.attribute("rotateZ"));

		for (Btl::Vector<C_GameObjectSample>::Iterator sample = samples.Begin(); sample != samples.End(); ++sample)
		{
			MQuaternion quat(sample->pQuat[actorNo].x, sample->pQuat[actorNo].y, sample->pQuat[actorNo].z, sample->pQuat[actorNo].w);
			MEulerRotation euler = quat.asEulerRotation();

			MTime time;
			time.setUnit(MTime::kSeconds);
			time.setValue(sample->time);

			translateX.addKeyframe(time, sample->pPos[actorNo].x);
			translateY.addKeyframe(time, sample->pPos[actorNo].y);
			translateZ.addKeyframe(time, -sample->pPos[actorNo].z); // flip z, due to differing handed-ness

			rotateX.addKeyframe(time, euler.x);
			rotateY.addKeyframe(time, euler.y+3.14); // rotate animation 180 degrees due to differing handed-ness
			rotateZ.addKeyframe(time, euler.z);
		}

		Util::C_String filterCommand;
		filterCommand = filterCommand.Format("filterCurve %s", actorName.Chars());
		if (MGlobal::executeCommand(filterCommand.Chars(), false, false) != MS::kSuccess)
			return MS::kFailure;
	}

	return MS::kSuccess;
}

MStatus AftkMayaPlugin::redoIt()
{
	return MS::kSuccess;
}

MStatus AftkMayaPlugin::undoIt()
{
	return MS::kSuccess;
}

bool AftkMayaPlugin::isUndoable() const
{
	return false;
}

MStatus initializePlugin(MObject obj)
{
	sprintf(g_versionString, "ReplayImporter %i Build %s", Aftk::C_Global::GetVersion(), Aftk::C_Global::GetBuildStamp().c_str());
	MFnPlugin plugin(obj, "Bizarre Creations", g_versionString, "Any");
	plugin.registerCommand("importReplay", AftkMayaPlugin::creator);
	
	/*
	sprintf(g_versionString, "Aftk %i Build %s", Aftk::C_Global::GetVersion(), Aftk::C_Global::GetBuildStamp().c_str());
	MFnPlugin plugin(obj, "Bizarre Creations", g_versionString, "Any");

	AftkMayaPlugin::setPluginName(plugin.name());

	Aftk::C_Global::Create();

	plugin.registerFileTranslator(
		AftkMayaPlugin::translatorName(),
		0,
		AftkMayaPlugin::creator,
		(char *)a3dOptionScript,
		(char *)a3dDefaultOptions,
		false
	);
	*/

	Debug::AutoConfigureDebugLog();

	//Subscribe to Jan's logging system.
	MayaSubscriber *p = new MayaSubscriber();
	p->SetXMLParameter("format", "%l: %c");
	Debug::GetRootLogger().AddSubscriber(p);

	p = new MayaSubscriber();
	p->SetXMLParameter("format", "%c");
	p->SetXMLParameter("filter", "Info");
	Debug::GetRootLogger().AddSubscriber(p);

	{
		/*U32 versionHigh;
		U32 versionLow;
		Aftk::C_Global::GetHighestSupportedFileVersion(Aftk::E_FileType_A3d, versionHigh);
		Aftk::C_Global::GetLowestSupportedFileVersion(Aftk::E_FileType_A3d, versionLow);
		*/

		S_INFO(AftkMaya::g_mainLogger, "*** Replay Import Plug-In Loaded ***");
		/*
		S_INFO(AftkMaya::g_mainLogger, "AFTK version [" << Aftk::C_Global::GetVersion() << "]");
		S_INFO(AftkMaya::g_mainLogger, "AFTK build stamp [" << Aftk::C_Global::GetBuildStamp() << "]");
		S_INFO(AftkMaya::g_mainLogger, "AFTK build PC [" << Aftk::C_Global::GetBuildPc() << "]");
		S_INFO(AftkMaya::g_mainLogger, "AFTK SVN revision(s) [" << Aftk::C_Global::GetSvnRevision() << "]");
		S_INFO(AftkMaya::g_mainLogger, "A3D exported file version [" << versionHigh << "]");
		S_INFO(AftkMaya::g_mainLogger, "A3D importable file versions [" << versionLow << "] to [" << versionHigh << "]");
		S_INFO(AftkMaya::g_mainLogger, "**************************************************");
		*/
	}

	return MS::kSuccess;
}


MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	plugin.deregisterCommand("importReplay");
	//plugin.deregisterFileTranslator(AftkMayaPlugin::translatorName());
	//Aftk::C_Global::Destroy();

	return MS::kSuccess;
}

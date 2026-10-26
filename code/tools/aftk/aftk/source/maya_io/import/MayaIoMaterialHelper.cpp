#include "stdafx.h"


//#pragma message("TODO:REMOVE OLD STRING PROC")
#include <core/utility/stringproc.h>
//#pragma message("TODO:REMOVE OLD FILE UTILS")
#include "../../../common/tools_common/FileUtilities.h"

using namespace MayaIo;

Aftk::C_Status C_MaterialHelper::ValidateTextures(
	C_Importer* pImporter,
	C_ImportOptions* pOptions,
	Aftk::I_Progress* pProgress)
{
	if (pProgress)
	{
		pProgress->OnPrintProgress("Checking for missing textures...");
		pProgress->OnProgress(0.f);
	}
	Aftk::C_Status status;
	MItDependencyNodes dgIt;

	U32 workToDo;
	U32 workDone = 0;

	bool missingTextures = false;
	Ut::UtVector<Ut::DynamicVector<MObject> > materialNodes;
	for (; !dgIt.isDone(); dgIt.next())
	{
		MFnDependencyNode node(dgIt.item());
		if (
			((node.typeId().id() >= MAYA_IO_CGFX_ID_RANGE_START) &&	(node.typeId().id() < MAYA_IO_CGFX_ID_RANGE_END)) 
			|| 
			node.typeId().id() == MAYA_IO_CGFX_2008_ID
			|| 
			node.typeId().id() == MAYA_IO_HLSL_2008_ID
			)
		{
			materialNodes.PushBack(dgIt.item());
			continue;
		}

		switch(dgIt.item().apiType()) 
		{
			case MFn::kPhong:
			case MFn::kLambert:
			case MFn::kBlinn:
				materialNodes.PushBack(dgIt.item());
				break;
		}
	}
	workToDo = (U32)materialNodes.Size();
	if (workToDo == 0)
	{
		return Aftk::C_Status::E_Status_Success;
	}

	for (U32 iMaterial = 0; iMaterial < (U32)materialNodes.Size(); iMaterial++)
	{
		if (HasMissingTextures(materialNodes[iMaterial]))
		{
			missingTextures = true;
		}
		workDone++;
		if (pProgress)
		{
			pProgress->OnProgress(((F32)workDone/(F32)workToDo)*100.f);
		}
	}

	if (missingTextures)
	{
		if (!pOptions->strict)
		{
			AFTK_DO_BEGIN 
				<< TRANS_MISSING_TEXTURES_WARN
			AFTK_DO_INFO_END;
			return Aftk::C_Status::E_Status_Success;
		}
		else
		{
			AFTK_DO_BEGIN 
				<< TRANS_MISSING_TEXTURES_FAIL
			AFTK_DO_INFO_END;
		}
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, TRANS_MISSING_TEXTURES_FAIL, 0);
	}
	return Aftk::C_Status::E_Status_Success;
}

void C_MaterialHelper::CleanUpFileName(const std::string& input, std::string& output) 
{
	if (input.length() < 2)
	{
		output = input;
		return;
	}
	U32 index;
	for (index = 0; index < input.length()-1; index++)
	{
		if ((input[index] == '\\' && input[index+1] == '\\')
			||
			(input[index] == '/' && input[index+1] == '/')
			)
		{
			index++;
		}
		output += input[index];
	}
	output += input[index];
}

bool C_MaterialHelper::HasMissingTextures(MObject& materialObj)
{
	MFnDependencyNode node(materialObj);

	Aftk::C_Status status;
	MStatus mStatus;

	MItDependencyGraph dgIt(materialObj,
		MFn::kInvalid,
		MItDependencyGraph::kUpstream, 
		MItDependencyGraph::kBreadthFirst,
		MItDependencyGraph::kNodeLevel, 
		&mStatus);

	bool texturesMissing = false;
	while( !dgIt.isDone() )
	{
		MObject dgNodeObject = dgIt.thisNode();
		if ( dgNodeObject.hasFn( MFn::kFileTexture ) )
		{
			MFnDependencyNode fnDependencyNode(dgNodeObject);
			// get the attribute for the full texture path
			MPlug ftn = fnDependencyNode.findPlug(MAYA_IO_TEXTURE_FILE_NAME_PLUG_NAME);

			// get the filename from the attribute
			MString fileName;
			ftn.getValue(fileName);

			std::string fileNameStd;
			CleanUpFileName(fileName.asChar(), fileNameStd);
			// write the file name
			#ifdef DEBUG_MATERIALS
			g_tcDebugMan <<	fileNameStd.asChar() << "\n";
			#endif

			Tu::SearchAndReplace(fileNameStd, "\\", "/");

			if (!Tu::FileExists(fileNameStd.c_str()))
			{
				MPlug outColour = fnDependencyNode.findPlug("outColor", &mStatus);

				MPlugArray connectedPlugArray;
				outColour.connectedTo(connectedPlugArray, false, true);
				if (connectedPlugArray.length() > 0)
				{
					#ifdef DEBUG_MATERIALS
					g_tcDebugMan << currentPlugName << " is the destination of ";
					#endif
				}

				std::string destinationPlugName;
				if (connectedPlugArray.length() > 0)
				{
					destinationPlugName = 
						connectedPlugArray[0].name().asChar();
				}

				AFTK_DO_BEGIN 
					<< "Missing texture [" 
					<< fileNameStd 
					<< "] on material [" 
					<< node.name().asChar()
					<< "] at plug [" 
					<< destinationPlugName 
					<< "]." 
				AFTK_DO_WARNING_END;
				texturesMissing	= true;
			}
		}
		dgIt.next();
	}

	return texturesMissing;
}

MObject C_MaterialHelper::FindShader(
	MObject& setNode, 
	Aftk::C_Status& status)
{
    MFnDependencyNode fnNode(setNode);
    MPlug shaderPlug = fnNode.findPlug("surfaceShader");
            
    if (!shaderPlug.isNull()) 
	{
        MPlugArray connectedPlugs;
        bool asSrc = false;
        bool asDst = true;
        shaderPlug.connectedTo( connectedPlugs, asDst, asSrc );

        if (connectedPlugs.length() != 1)
		{
			AFTK_DO_BEGIN << "Could not get shader." AFTK_DO_WARNING_END;
		}
        else 
		{
			status = Aftk::C_Status::E_Status_Success;
            return connectedPlugs[0].node();
		}
    }           
    
	status = Aftk::C_Status::E_Status_Success;
    return MObject::kNullObj;
}


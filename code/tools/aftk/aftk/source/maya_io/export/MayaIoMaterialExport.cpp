#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_MaterialExport::Export(	
	C_Exporter* pExporter,
	const Aftk::C_Scene* pAftkScene,
	const Aftk::C_MaterialNode* pAftkMaterialNode,
	MObject* pMayaMaterialObject,
	MObject* pMayaMaterialShadingGroupObject)
{
	Aftk::C_Status status;
	MStatus mStatus;

	//For the time being, don't parse the shader tree of the AFTK. Instead, create a lambertian shader
	//and, if appropriate, wire up the first texture map to the colour plug.

	//Create file node
	MFnDependencyNode mayaFileNode;
	{
		mayaFileNode.create(MString("file"), &mStatus);
		MCHECK_BEGIN << "Could not create file node" MCHECK_ERROR_RETURN_END_1;

		// get the attribute for the full texture path
		MPlug ftn = mayaFileNode.findPlug(MAYA_IO_TEXTURE_FILE_NAME_PLUG_NAME);

		// Set the file name for the file node
		MString fileName;
		if (pAftkMaterialNode->GetNumShaderImageNodes() > 0)
		{
			std::string temp;
			pAftkMaterialNode->GetShaderImageNodePtr(0)->GetFilePlug().GetValue(temp);
			fileName = temp.c_str();
		}
		else
		{
			fileName.clear();
		}
		ftn.setValue(fileName);
	}

	//Create 2d placement node
	MFnDependencyNode mayaPlace2DTextureNode;
	{
		mayaPlace2DTextureNode.create(MString("place2dTexture"), &mStatus);
		MCHECK_BEGIN << "Could not create place2dTexture node" MCHECK_ERROR_RETURN_END_1;

		status = ConnectPlugs(mayaPlace2DTextureNode, "coverage", mayaFileNode, "coverage");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "translateFrame", mayaFileNode, "translateFrame");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "mirrorU", mayaFileNode, "mirrorU");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "mirrorV", mayaFileNode, "mirrorV");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "stagger", mayaFileNode, "stagger");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "wrapU", mayaFileNode, "wrapU");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "wrapV", mayaFileNode, "wrapV");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "repeatUV", mayaFileNode, "repeatUV");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "offset", mayaFileNode, "offset");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "rotateUV", mayaFileNode, "rotateUV");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "noiseUV", mayaFileNode, "noiseUV");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "vertexUvOne", mayaFileNode, "vertexUvOne");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "vertexUvTwo", mayaFileNode, "vertexUvTwo");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "vertexUvThree", mayaFileNode, "vertexUvThree");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "vertexCameraOne", mayaFileNode, "vertexCameraOne");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "outUV", mayaFileNode, "uv");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
		status = ConnectPlugs(mayaPlace2DTextureNode, "outUvFilterSize", mayaFileNode, "uvFilterSize");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1;
	}

	//Create a lamber shader
	MFnLambertShader mayaLambertShader;
	{
		*pMayaMaterialObject = mayaLambertShader.create(true, &mStatus);
		MCHECK_BEGIN << "Could not create lambert shader" MCHECK_ERROR_RETURN_END_1

		//Connect lambert to file texture
		status = ConnectPlugs(mayaFileNode, "outColor", mayaLambertShader, "color");
		AFTK_CHECK_BEGIN << TRANS_CONNECT_PLUGS_FAIL AFTK_CHECK_ERROR_RETURN_END_1
	}
	mayaLambertShader.setName(MString(pAftkMaterialNode->GetName().c_str()));
	//Get the material name from maya, this is potential different to the name we just assigned.
	MString mayaMaterialName = mayaLambertShader.name();

	//Create shading group
	MString mayaMaterialSG;
	{
		MString command;
		command += "sets -renderable true -noSurfaceShader true -empty -name \"";
		command += mayaMaterialName;
		command += "SG";
		command += "\"";
		MStringArray resultArray;

		mStatus = MGlobal::executeCommand(command, resultArray, false, false);
		MCHECK_ERROR_RETURN_1;
		
		mayaMaterialSG = resultArray[0];
		// Connect the material to the Shading Group.
		command.clear();
		command += "connectAttr -f ";
		command += mayaMaterialName;
		command += ".outColor ";
		command += mayaMaterialSG;
		command += ".surfaceShader";

		mStatus = MGlobal::executeCommand(command);
		MCHECK_ERROR_RETURN_1;
	}
	{
		//Acquire shading group object
		MSelectionList selList;
		mStatus = MGlobal::getSelectionListByName(mayaMaterialSG, selList);
		MCHECK_ERROR_RETURN_1;
		mStatus = selList.getDependNode(0, *pMayaMaterialShadingGroupObject);
		MCHECK_ERROR_RETURN_1;
	}

	return Aftk::C_Status::E_Status_Success;
}


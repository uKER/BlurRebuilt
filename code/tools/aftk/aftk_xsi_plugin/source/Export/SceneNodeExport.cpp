#include "XSIA3DExportPCH.h"
#include "ExportOptions.h"
#include "ISceneNodeExport.h"
#include "SceneNodeExport.h"
#include "../XSIUtility.h"

namespace XSITrans
{

C_SceneNodeExport::~C_SceneNodeExport()
{
}

Aftk::C_SceneNode* C_SceneNodeExport::Export(
	Aftk::C_Scene& aftkScene,
	const C_ExportOptions& options, 
	XSI::X3DObject xsiNode)
{
	Aftk::C_SceneNode* pSceneNode = new Aftk::C_SceneNode(&aftkScene);
	if (!pSceneNode) return 0;

	if (!_Export(
		*pSceneNode,
		options,
		xsiNode))
		goto fail;
	
	return pSceneNode;

fail:
	delete pSceneNode;
	return 0;
}

bool C_SceneNodeExport::_Export(
	Aftk::C_SceneNode& aftkSceneNode,
	const C_ExportOptions& options, 
	XSI::X3DObject xsiNode)
{
	aftkSceneNode.SetName(xsiNode.GetName().GetAsciiString());

	XSI::MATH::CTransformation xsiTrans = 
		xsiNode.GetKinematics().GetLocal().GetTransform();

	XSI::MATH::CMatrix4 xsiM = xsiTrans.GetMatrix4();

	Aftk::Matrix44 aftkM;
	if (!XsiToAftkMatrix44(
		aftkM, xsiM))
	{
		//Corrupt matrix. TODO: warning here.
		UtMaths::SetIdentity(&aftkM);
	}

	aftkSceneNode.SetLocalMatrix(aftkM);

	return true;
}

}
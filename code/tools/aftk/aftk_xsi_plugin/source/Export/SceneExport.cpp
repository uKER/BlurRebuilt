#include "XSIA3DExportPCH.h"

#include "ExportOptions.h"
#include "SceneExport.h"
#include "SceneNodeExportHelper.h"
#include "../XSIUtility.h"

namespace XSITrans
{

C_SceneExport::C_SceneExport()
{
	m_interestingNodeCount = 0;
	m_nNodesProcessed = 0;
}

bool C_SceneExport::DoWeImportThisNode(const C_ExportOptions& options, XSI::X3DObject node) const
{
	if (options.exportSelection)
	{
		XSI::Application app;
		XSI::Selection selection = app.GetSelection();
		for (S32 i = 0; i < selection.GetCount(); i++)
		{
			XSI::CRef ref = selection.GetItem(i);
			if (node.GetRef() == ref)
				return true;
		}
		return false;
	}

	return true;
}

bool C_SceneExport::PreProcessScene(const C_ExportOptions& options, XSI::X3DObject node)
{
	if (DoWeImportThisNode(options, node))
	{
		m_interestingNodeCount++;
	}

	for (U32 i = 0; i < (U32)node.GetChildren().GetCount(); i++)
	{
		if (!PreProcessScene(options, node.GetChildren()[i]))
			return false;
	}

	return true;
}

bool C_SceneExport::ProcessScene(
	Aftk::C_SceneNode* pLastGoodAncestor,
	Aftk::Matrix44* pBridgeMatrix,
	const C_ExportOptions& options, 
	XSI::X3DObject node,
	XSI::ProgressBar& bar)
{
	Aftk::Matrix44 bridgeMatrix;
	UtMaths::SetIdentity(&bridgeMatrix);

	bool importThisNode = DoWeImportThisNode(options, node);
	if (importThisNode)
	{
		Aftk::C_SceneNode* p = C_SceneNodeExportHelper::Export(
			m_scene,
			options, 
			node);
		if (!p) 
			return false;

		if (!pLastGoodAncestor)
		{
			m_scene.GetRootSceneNodePtr()->AddChildPtr(p);
		}
		else
		{
			pLastGoodAncestor->AddChildPtr(p);
		}

		if (pBridgeMatrix)
		{
			p->SetLocalMatrix(p->GetLocalMatrix(false) * (*pBridgeMatrix));
		}

		pBridgeMatrix = 0;
		m_nNodesProcessed++;
		bar.Increment();
		pLastGoodAncestor = p;
	}
	else
	{
		//Add bridge matrix.
		XSI::MATH::CTransformation xsiTrans = 
			node.GetKinematics().GetLocal().GetTransform();

		XSI::MATH::CMatrix4 xsiM = xsiTrans.GetMatrix4();

		Aftk::Matrix44 aftkM;
		if (!XsiToAftkMatrix44(
			aftkM, xsiM))
		{
			//Corrupt matrix. TODO: warning here.
			UtMaths::SetIdentity(&aftkM);
		}

		if (pBridgeMatrix)
			bridgeMatrix = bridgeMatrix * aftkM;
		else
			bridgeMatrix = aftkM;
	}

	for (U32 i = 0; i < (U32)node.GetChildren().GetCount(); i++)
	{
		if (!ProcessScene(
			pLastGoodAncestor,
			&bridgeMatrix,
			options, 
			node.GetChildren()[i],
			bar))
			return false;
	}

	return true;
}

bool C_SceneExport::Export(const C_ExportOptions& options)
{
	XSI::Application app;

	if (!PreProcessScene(options, app.GetActiveSceneRoot()))
		return false;

	// Setup the progress bar
	XSI::UIToolkit kit = app.GetUIToolkit();
	XSI::ProgressBar bar = kit.GetProgressBar();

	bar.PutMaximum( m_interestingNodeCount );
	bar.PutStep( 1 );
	bar.PutVisible( true );
	bar.PutCaption( L"Exporting..." );
	bar.PutCancelEnabled(false);

	Aftk::C_SceneNode* pAftkRoot = new Aftk::C_SceneNode(&m_scene);
	pAftkRoot->SetName("AftkRoot");
	m_scene.SetRootSceneNodePtr(pAftkRoot);

	//Set scene transform
	if (m_scene.GetRootSceneNodePtr())
	{
		Aftk::Matrix44 sceneTransform;
		SetIdentity(&sceneTransform);
		sceneTransform[2] *= -1.f;
		m_scene.GetRootSceneNodePtr()->SetLocalMatrix(
			sceneTransform * m_scene.GetRootSceneNodePtr()->GetLocalMatrix(false));
	}

	if (!ProcessScene(
		0, 
		0,
		options,
		app.GetActiveSceneRoot(),
		bar))
		return false;

	bar.PutVisible( false );

	return true;
}

const Aftk::C_Scene& C_SceneExport::GetScene() const
{
	return m_scene;
}

}
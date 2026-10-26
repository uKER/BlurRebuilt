#include "ImposterPch.h"

#include "Imposter/ImposterImageHandle.h"
#include "Imposter/ImposterData.h"
#include "Imposter/ImposterProcess.h"
#include "Imposter/ImposterRenderer.h"
#include "Imposter/ImposterUtils.h"


#include "attk/PixelFormats.h"

#include "System/Misc/BtlStringUtil.h"
#include "TextureAtlas/ImageDescription.h"
#include "TextureAtlas/Atlas.h"
#include "TextureAtlas/Pack/BinaryTreePack.h"
#include "TextureAtlas/Pack/HorizonPack.h"


namespace Imposter
{

Debug::Logger& g_Log = Debug::GetLogger("tools.Imposter");

bool HasChildOfType(const Aftk::C_SceneNode *pNode, Aftk::E_ObjType type)
{
	for (U32 i=0; i<pNode->GetNumChildren(); ++i)
	{
		Aftk::C_SceneNodeConstIterator it(pNode->GetChildPtr(i));

		while (it)
		{
			if (it->HasType(type))
				return true;

			++it;
		}
	}

	return false;
}

bool IsAncestorOf(const Aftk::C_SceneNode *pNode, const Aftk::C_SceneNode *pRoot)
{
	while (pNode)
	{
		if (pNode == pRoot)
			return true;

		pNode = pNode->GetParentPtr();
	}
	return false;
}


void BuildImposterDataArray(Aftk::C_Scene& aftkScene, ImposterDataArray& imposterData)
{
	for (Aftk::C_SceneNodeIterator it(&aftkScene); it; ++it)
	{
		if (it->HasType(Aftk::E_ObjType_SceneMesh))
		{
			//ignore if not a high detail lod
			{
				Aftk::C_SceneNode *pParent = it->GetParentPtr();
				bool ignore = false;
				while (pParent)
				{
					if (pParent->HasType(Aftk::E_ObjType_SceneLodGroup))
					{
						Aftk::C_SceneLodGroupNode *pLodParent = static_cast<Aftk::C_SceneLodGroupNode*>(pParent);
						if (pLodParent->GetNumLodLevels()==0 || pLodParent->GetLodLevelInfo(0).GetSceneNodePtr() != it)
							ignore = true;

						break;
					}
					pParent = pParent->GetParentPtr();
				}
			}


			AftkUt::C_NodeFlagParser actorParser(*it, NULL, false);

			Btl::OpenHashSet<S32> imposterTypeSet;
			std::string lodType;
			if (actorParser.GetParameterValue("AutoLodGeneration", lodType) && lodType != "None" && lodType != "Inherit")
			{
				E_ImposterType type = Imposter::GetImposterType(lodType.c_str());
				if (type != Imposter::E_ImposterType_Count)
					imposterTypeSet.Insert((S32)type);
			}

			for (U32 i=0; i<it->GetNumInstancesOfThisNode(); ++i)
			{
				Aftk::C_SceneNode *pSceneInstanceNode = it->GetInstanceOfThisNode(i);
				AftkUt::C_NodeFlagParser instParser(*pSceneInstanceNode , NULL, false);
				std::string instLodType;
				if (instParser.GetParameterValue("AutoLodGeneration", instLodType) && instLodType != "None" && instLodType != "Inherit")			
				{
					E_ImposterType type = Imposter::GetImposterType(instLodType.c_str());
					if (type != E_ImposterType_Count)
						imposterTypeSet.Insert((S32)type);
				}
			}

			if (imposterTypeSet.Size() > 0)
			{
				const Aftk::C_SceneMeshNode *pMeshNode = (const Aftk::C_SceneMeshNode*) &(*it);				
				if (pMeshNode->GetNumPolygons(0) < 20)
					LOGWARN(g_Log, "Low polygon mesh flagged with AutoLodGeneration (%s %d polygons)", pMeshNode->GetPath().c_str(), pMeshNode->GetNumPolygons(0));

				imposterData.PushBack();
				imposterData.Back().pSceneNode = it;

				for (Btl::OpenHashSet<S32>::Iterator typeIt=imposterTypeSet.Begin(); typeIt != imposterTypeSet.End(); ++typeIt)
					imposterData.Back().types.PushBack((Imposter::E_ImposterType)typeIt->GetKey());

				//increment iterator to skip subtree ...
				{
					const Aftk::C_SceneNode *pNode = it;
					Aftk::C_SceneNodeIterator it2 = it;
					it2++;
					while (IsAncestorOf(it2, pNode))
					{
						it = it2;
						++it2;
					}
				}
			}
		}
	}
}

bool RenderImposters(ImposterDataArray& imposterDataArray, const Btl::OpenHashMap<S32 , Aftk::C_SceneNode*>& baseMeshOverrides)
{
	if (!Renderer::DeviceInit())
		return false;

	C_ImposterRenderer *pRenderer = new C_ImposterRenderer();
	if (!pRenderer || !pRenderer->Create() || !pRenderer->CreateVolatile())
		return false;

	AftkRender::PushSceneRenderer(pRenderer);
	pRenderer->LoadDefaultShaders();
	AftkRender::C_Scene& renderScene = Renderer::GetScene();

	for (S32 i=0; i<imposterDataArray.Size(); ++i)
	{	
		ImposterData& imposterData = imposterDataArray[i];
		BIZ_ASSERT(imposterData.types.Size());

		Aftk::C_SceneNode *pSceneNode = imposterData.pSceneNode;
		{
			Btl::OpenHashMap<S32 , Aftk::C_SceneNode*>::ConstIterator it = baseMeshOverrides.Find(pSceneNode->GetUniqueId());
			if (it != baseMeshOverrides.End())
				pSceneNode = it->GetValue();
		}

		if (renderScene.Load(pSceneNode, true) && renderScene.BuildMeshClusters())
		{
			renderScene.LoadTextures(0);
			pRenderer->AttachSceneGraphNode(renderScene.GetRootSceneGraphNodePtr());
			renderScene.GetRootSceneGraphNodePtr()->UpdateCache();

			AftkRender::C_ViewContext *pContext = Renderer::GetViewContext();
			pContext->GetView().camera.SetType(AftkRender::C_Camera::E_CameraType_Orthographic);

			F32 maxBoundsRadius = 0.f;

			//calculate bounding volume of subtree so we can frame the camera appropriately
			Aftk::C_Bounds bounds = GetWorldBounds(pSceneNode);
			for (U32 j=0; j<pSceneNode->GetNumChildren(); ++j)
				for (Aftk::C_SceneNodeIterator it(pSceneNode->GetChildPtr(j)); it; ++it)
					bounds += GetWorldBounds(it);

			maxBoundsRadius = Maths::Max(maxBoundsRadius, Maths::Sqrt(bounds.GetObjectRadiusSquared()));

			for (S32 j=0; j<imposterData.types.Size(); ++j)
			{
				ViewData viewData = GetViewData(imposterData.types[j]);

				for (U32 k=0; k<viewData.nImages; ++k)
				{
					//check if we've already rendered the object along this view vector before?
					{
						Aftk::Vector3F32 v0(viewData.pViewVectors[k].x, viewData.pViewVectors[k].y, viewData.pViewVectors[k].z);
						bool found = false;
						for (S32 l=0; l<imposterData.imageData.Size(); ++l)
						{
							const ImposterData::ImageData& imageData = imposterData.imageData[l];
							Aftk::Vector3F32 v1(imageData.viewvector[0], imageData.viewvector[2], imageData.viewvector[2]);

							//5 degree tolerance
							//NOTE: we're assuming the view vectors are normalised
							if ((v0 | v1) > Maths::Cos(Maths::DegToRad(5.f)))
								found = true;
						}
						if (found)
							continue;						
					}


					//position the camera appropriately
					AftkRender::Utility::FrameObjectOrthographic(
						pContext->GetView().camera, 
						Maths::C_AxisBox(
							Maths::HVec3(bounds.GetCentre().x-maxBoundsRadius, bounds.GetCentre().y-maxBoundsRadius, bounds.GetCentre().z-maxBoundsRadius),
							Maths::HVec3(bounds.GetCentre().x+maxBoundsRadius, bounds.GetCentre().y+maxBoundsRadius, bounds.GetCentre().z+maxBoundsRadius)),
							Maths::HVec3(viewData.pViewVectors[k].x, viewData.pViewVectors[k].y, viewData.pViewVectors[k].z));

					//push camera back a little (even though we're using an orthographic camera, 'screen aligned' geometry will not be aligned to the camera plane)
					pContext->GetView().camera.GetPosition()[0] -= 20.f*viewData.pViewVectors[k].x;
					pContext->GetView().camera.GetPosition()[1] -= 20.f*viewData.pViewVectors[k].y;
					pContext->GetView().camera.GetPosition()[2] -= 20.f*viewData.pViewVectors[k].z;


					//do the rendering
					Renderer::BeginRendering();		

					Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
					Display::Device()->Clear(Display::E_Clear_Colour|Display::E_Clear_Depth|Display::E_Clear_Stencil, Maths::RGBA(0, 0, 0, 0));
				
					pRenderer->Update(0);
					pRenderer->Render();

					Renderer::EndRendering();

					//capture image
					{
						C_ImageHandle imageHandle = C_ImageHandle::Create("", new Attk::C_Image);
						bool capture = AftkRender::Utility::CaptureRenderTarget(*(Attk::C_Image*)imageHandle);

						RECT r = GetBoundingRect(*(Attk::C_Image*)imageHandle);
						imposterData.imageData.PushBack();

						ImposterData::ImageData& imageData = imposterData.imageData.Back();

						if (capture && r.top != -1 && CropImage(*(Attk::C_Image*)imageHandle, r))
						{
							imageData.image = imageHandle;

							//The camera was framed on the object using the bounds previously calculated.
							//As we've now trimmed excess blank space from the image, we need to recalculate the quad size as well as any offset information

							const U32 w=pContext->PresentParams().BackBufferWidth, h=pContext->PresentParams().BackBufferHeight;
							F32 texelSize = pContext->GetView().camera.GetHeightExtent() / h;

							imageData.offset[0] = texelSize * (-0.5f*w + r.left);
							imageData.offset[1] = texelSize * (0.5f*h - r.bottom);
							imageData.offset[2] = texelSize * (-0.5f*w + r.right);
							imageData.offset[3] = texelSize * (0.5f*h - r.top);				
						}
						else
						{
							//we've got a blank image for some reason... camera framing bug? or maybe missing textures?
							LOGWARN(g_Log, "Failed to render %s [%f %f %f]", imposterData.pSceneNode->GetName().c_str(), viewData.pViewVectors[k].x, viewData.pViewVectors[k].y, viewData.pViewVectors[k].z);

							imageData.offset[0] = 0.f;
							imageData.offset[1] = 0.f;
							imageData.offset[2] = 0.f;
							imageData.offset[3] = 0.f;

							//keep this imageData around so we don't attempt to re-render the object from this view vector
						}

						imageData.viewvector[0] = viewData.pViewVectors[k].x;
						imageData.viewvector[1] = viewData.pViewVectors[k].y;
						imageData.viewvector[2] = viewData.pViewVectors[k].z;

						imageData.anchor[0] = pContext->GetView().camera.GetFocusPosition().X();
						imageData.anchor[1] = pContext->GetView().camera.GetFocusPosition().Y();
						imageData.anchor[2] = pContext->GetView().camera.GetFocusPosition().Z();

						if (pSceneNode != imposterData.pSceneNode)
						{
							//scene node override position != original scene node position
							//fudge focal position :(
							imageData.anchor[0] += imposterData.pSceneNode->GetWorldMatrix()[3][0] - pSceneNode->GetWorldMatrix()[3][0];
							imageData.anchor[1] += imposterData.pSceneNode->GetWorldMatrix()[3][1] - pSceneNode->GetWorldMatrix()[3][1];
							imageData.anchor[2] += imposterData.pSceneNode->GetWorldMatrix()[3][2] - pSceneNode->GetWorldMatrix()[3][2];
						}
					}
				}
			}
			pRenderer->DetachSceneGraphNode(renderScene.GetRootSceneGraphNodePtr());
		}
	}

	pRenderer->Destroy();
	pRenderer->DestroyVolatile();
	BIZ_ASSERT(AftkRender::PopSceneRenderer() == pRenderer);
	delete pRenderer;

	Renderer::DeviceDestroy();

	return true;
}

void ReplaceImposterLodGroup(Aftk::C_SceneNode *pSceneNode, Aftk::C_SceneNode *pLodMesh)
{
	//node has been flagged as a permanent low lod.
	//remove high lod and any lod groups

	Aftk::C_SceneNode *pAftkParentNode = pSceneNode->GetParentPtr();
	if (pAftkParentNode && pAftkParentNode->HasType(Aftk::E_ObjType_SceneLodGroup))
	{
		//inherit layer membership
		for (U32 i=0; i<pAftkParentNode->GetNumLayers(); ++i)
		{
			Aftk::C_LayerNode *pLayerNode = pAftkParentNode->GetLayerPtr(i);
			pLayerNode->AddMember(pLodMesh);
		}

		//copy node flags
		{
			AftkUt::C_NodeFlagParser parser(*pSceneNode);
			parser.StoreFlags(*pLodMesh);
		}

		//delete any children...
		Aftk::C_Scene *pScene = pAftkParentNode->GetScenePtr();
		pScene->DeleteChildren(pAftkParentNode);
		//replace lod group node
		Aftk::Matrix44 m = pAftkParentNode->GetWorldMatrix();
		pLodMesh->Replace(pAftkParentNode, true);
		pLodMesh->SetWorldMatrix(m);
	}
	else
	{
		//inherit layer membership
		for (U32 i=0; i<pSceneNode->GetNumLayers(); ++i)
		{
			Aftk::C_LayerNode *pLayerNode = pSceneNode->GetLayerPtr(i);
			pLayerNode->AddMember(pLodMesh);
		}

		//copy node flags
		{
			AftkUt::C_NodeFlagParser parser(*pSceneNode);
			parser.StoreFlags(*pLodMesh);
		}

		//no lod group. replace high detail mesh
		Aftk::Matrix44 m = pSceneNode->GetWorldMatrix();
		pLodMesh->Replace(pSceneNode, true);
		pLodMesh->SetWorldMatrix(m);
	}
}

void SetupImposterLodGroup(Aftk::C_SceneNode *pSceneNode, Aftk::C_SceneNode *pLodMesh)
{
	Aftk::C_SceneLodGroupNode *pParentLodGroup = 0;
	{
		Aftk::C_SceneNode *pAftkParentNode = pSceneNode->GetParentPtr();
		if (!pAftkParentNode ||! pAftkParentNode->HasType(Aftk::E_ObjType_SceneLodGroup))
		{
			//create a lod group
			pParentLodGroup = new Aftk::C_SceneLodGroupNode(pSceneNode->GetScenePtr());
			if (pAftkParentNode)
				pAftkParentNode->AddChildPtr(pParentLodGroup);
			else
			{
				//err...create lod group as child of root?
				pSceneNode->GetScenePtr()->GetRootSceneNodePtr()->AddChildPtr(pParentLodGroup);
			}

			pParentLodGroup->SetName(pSceneNode->GetName() + "_AutoLod");
			//attach scene node as a child of the new lod group
			pParentLodGroup->AddChildPtr(pSceneNode);

			Aftk::C_GroupLodLevelInfo info;
			info.SetSceneNodePtr(pSceneNode);

			//set default thresholds

			Aftk::C_SceneNode *pSourceMesh = (pSceneNode->HasType(Aftk::E_ObjType_SceneInstance)) 
												? static_cast<Aftk::C_SceneInstanceNode*>(pSceneNode)->GetSourceNodePtr() 
												: pSceneNode;
				
			Aftk::C_Bounds bounds =	GetWorldBounds(pSourceMesh);

			//magic numbers taken from afktspeedtree
			const F32 lodDistanceScale = 30.f, lodDistanceMin = 30.f, lodDistanceMax = 600.f;
			const F32 cutoff = 7.f, scaleFactor = 12.f, offset = 40.f;

			F32 size = Maths::Sqrt(bounds.GetBoundingBoxRadiusSquared());
			F32 scale = 10.f * Maths::Max(0.f, size-cutoff);

			info.SetThreshold(size*lodDistanceScale + scale + offset);

			LOGDEBUG(g_Log, "Creating Lod Group for %s [threshold %f]", pSceneNode->GetName().c_str(), info.GetThreshold());
			pParentLodGroup->AddLodLevelInfo(info);

			//inherit layer membership
			for (U32 i=0; i<pSceneNode->GetNumLayers(); ++i)
			{
				Aftk::C_LayerNode *pLayerNode = pSceneNode->GetLayerPtr(i);
				pLayerNode->AddMember(pAftkParentNode);
			}

		}
		else
			pParentLodGroup = static_cast<Aftk::C_SceneLodGroupNode*>(pAftkParentNode);
	}

	BIZ_ASSERT(pParentLodGroup);

	Aftk::C_GroupLodLevelInfo info;
	info.SetSceneNodePtr(pLodMesh);
	info.SetThreshold(0.f);
	pParentLodGroup->AddLodLevelInfo(info);

	//inherit layer membership
	for (U32 i=0; i<pParentLodGroup->GetNumLayers(); ++i)
	{
		Aftk::C_LayerNode *pLayerNode = pParentLodGroup->GetLayerPtr(i);
		pLayerNode->AddMember(pLodMesh);
	}

	pLodMesh->SetLocalMatrix(pSceneNode->GetLocalMatrix());
	pParentLodGroup->AddChildPtr(pLodMesh);

	//copy node flags
	{
		AftkUt::C_NodeFlagParser parser(*pSceneNode);
		parser.StoreFlags(*pLodMesh);
	}

	//remove "MaxDrawDistance" and replace UseSHLighting with UseVertexLighting
	{
		AftkUt::C_NodeFlagParser parser(*pLodMesh);
		F32 maxDrawDistance;
		if (parser.GetParameterValue("MaxDrawDistance", maxDrawDistance))
			parser.SetParameterValue("MaxDrawDistance", "", AftkUt::E_ParamType_F32);
/*
		bool b = false;
		if (parser.GetParameterValue("UseSHLighting", b))
			parser.SetParameterValue("UseSHLighting", false);

		parser.SetParameterValue("UseVertexLighting", true);
*/
	}
}


bool GenerateImposterGeometry(Aftk::C_Scene& aftkScene, const ImposterDataArray& imposterDataArray, const Btl::String& workingFolder)
{
	const Aftk::C_BlindDataResource* pPolyFlagsBlindData = 0;
	{
		Aftk::C_BlindDataNode blindData;
		
		Aftk::C_GenericAttribute attr;
		
		attr.SetName("polySet_doNotCastShadows");
		blindData.AddAttribute(attr);
		attr.Clear();

		attr.SetName("polySet_doNotLight");
		blindData.AddAttribute(attr);
		attr.Clear();

		pPolyFlagsBlindData = aftkScene.AddBlindDataNode(blindData);
	}

	for (S32 i=0; i<imposterDataArray.Size(); ++i)
	{
		const ImposterData& imposterData = imposterDataArray[i];
		if (!imposterData.types.Size())
			continue;

		//check we've rendered at least 1 image for this node
		{
			U32 imageCount = 0;
			for (S32 j=0; j<imposterData.imageData.Size(); ++j)
				if ((const Attk::C_Image*)imposterData.imageData[j].image)
					imageCount++;

			if (!imageCount)
				continue;
		}

		//create aftk mesh per imposter type, parented to the lod group
		Aftk::C_SceneMeshNode* pAftkMeshImposters[E_ImposterType_Count];
		memset(pAftkMeshImposters, 0, sizeof(E_ImposterType)*E_ImposterType_Count);

		for (S32 j=0; j<imposterData.types.Size(); ++j)
		{
			BIZ_ASSERT(pAftkMeshImposters[imposterData.types[j]] == 0);

			Aftk::C_SceneMeshNode *pAftkMeshNode = new Aftk::C_SceneMeshNode(&aftkScene);
			pAftkMeshImposters[imposterData.types[j]] = pAftkMeshNode;

			LOGDEBUG(g_Log, "Generating Imposter Mesh '%s' for '%s'", kImposterTypeParams[imposterData.types[j]], imposterData.pSceneNode->GetName().c_str());
			pAftkMeshNode->SetName(imposterData.pSceneNode->GetName() + "_AutoLod(" + kImposterTypeParams[imposterData.types[j]] + ")");

			//copy flags from original node
			{
				AftkUt::C_NodeFlagParser parser(*imposterData.pSceneNode);
				parser.StoreFlags(*pAftkMeshNode);
			}

			S16 blindDataMapping = pAftkMeshNode->AddPolygonBlindDataMapping(pPolyFlagsBlindData);
			ViewData viewData = GetViewData(imposterData.types[j]);

			for (U32 k=0; k<viewData.nImages; ++k)
			{
				const ImposterData::ImageData *pImageData = 0;
				{
					Aftk::Vector3F32 v0(viewData.pViewVectors[k].x, viewData.pViewVectors[k].y, viewData.pViewVectors[k].z);
					//find image previously rendered along this view vector
					for (S32 l=0; l<imposterData.imageData.Size(); ++l)
					{
						const ImposterData::ImageData& imageData = imposterData.imageData[l];
						Aftk::Vector3F32 v1(imageData.viewvector[0], imageData.viewvector[2], imageData.viewvector[2]);

						//5 degree tolerance
						//NOTE: we're assuming the view vectors are normalised
						if ((v0 | v1) > Maths::Cos(Maths::DegToRad(5.f)))
						{
							pImageData = &imposterData.imageData[l];
							break;
						}
					}

					if (!pImageData || !(const Attk::C_Image*)pImageData->image)
						continue;
				}

				S32 posIdx[4] = {-1, -1, -1, -1};
				S32 uvIdx[4] = {-1, -1, -1, -1};
				S32 pvIdx[4] = {-1, -1, -1, -1};
				S32 nIdx = -1;

				//vertex positions
				{
					Aftk::Position3 p0, p1, p2, p3;
					{
						Aftk::Vector3F32 camY(0.f, 1.f, 0.f);
						Aftk::Vector3F32 camZ(pImageData->viewvector[0], pImageData->viewvector[1], pImageData->viewvector[2]);
						Aftk::Vector3F32 camX = camY^camZ;
						UtMaths::SafeNormalise(&camX);						
						camY = camZ^camX;
						UtMaths::SafeNormalise(&camY);

						//Calculate offset as the bounding sphere centre != scene node position
						Aftk::Vector3F32 offset(imposterData.pSceneNode->GetWorldMatrix()[3][0]-pImageData->anchor[0],
												imposterData.pSceneNode->GetWorldMatrix()[3][1]-pImageData->anchor[1],
												imposterData.pSceneNode->GetWorldMatrix()[3][2]-pImageData->anchor[2]);

						offset = (offset|camZ)*camZ;

						p0.x = pImageData->anchor[0] + pImageData->offset[0]*camX.x + pImageData->offset[1]*camY.x + offset[0];
						p0.y = pImageData->anchor[1] + pImageData->offset[0]*camX.y + pImageData->offset[1]*camY.y + offset[1];
						p0.z = pImageData->anchor[2] + pImageData->offset[0]*camX.z + pImageData->offset[1]*camY.z + offset[2];

						p1.x = pImageData->anchor[0] + pImageData->offset[0]*camX.x + pImageData->offset[3]*camY.x + offset[0];
						p1.y = pImageData->anchor[1] + pImageData->offset[0]*camX.y + pImageData->offset[3]*camY.y + offset[1];
						p1.z = pImageData->anchor[2] + pImageData->offset[0]*camX.z + pImageData->offset[3]*camY.z + offset[2];

						p2.x = pImageData->anchor[0] + pImageData->offset[2]*camX.x + pImageData->offset[3]*camY.x + offset[0];
						p2.y = pImageData->anchor[1] + pImageData->offset[2]*camX.y + pImageData->offset[3]*camY.y + offset[1];
						p2.z = pImageData->anchor[2] + pImageData->offset[2]*camX.z + pImageData->offset[3]*camY.z + offset[2];

						p3.x = pImageData->anchor[0] + pImageData->offset[2]*camX.x + pImageData->offset[1]*camY.x + offset[0];
						p3.y = pImageData->anchor[1] + pImageData->offset[2]*camX.y + pImageData->offset[1]*camY.y + offset[1];
						p3.z = pImageData->anchor[2] + pImageData->offset[2]*camX.z + pImageData->offset[1]*camY.z + offset[2];

					}


					p0 = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverse(), p0);
					p1 = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverse(), p1);
					p2 = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverse(), p2);
					p3 = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverse(), p3);

					Aftk::Vector3F32 v01 = p1-p0;
					Aftk::Vector3F32 v12 = p2-p1;

					posIdx[0] = pAftkMeshNode->AddVertexPosition(p0);
					posIdx[1] = pAftkMeshNode->AddVertexPosition(p1);
					posIdx[2] = pAftkMeshNode->AddVertexPosition(p2);
					posIdx[3] = pAftkMeshNode->AddVertexPosition(p3);
				}

				//vertex normal
				{
					//Aftk::Vector3F32 n(-viewData.pViewVectors[k].x, -viewData.pViewVectors[k].y, -viewData.pViewVectors[k].z);
					Aftk::Vector3F32 n(0.f, 1.f, 0.f);
					n = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverseTranspose(), n);
					UtMaths::SafeNormalise(&n);
					nIdx = pAftkMeshNode->AddVertexNormal((const Aftk::Normal3&)n);
				}

				//texture coordinates
				{
					uvIdx[0] = pAftkMeshNode->AddTextureCoordinate(0, Aftk::TextureCoordinate4(pImageData->texcoord[0], pImageData->texcoord[3], 0.f, 0.f));
					uvIdx[1] = pAftkMeshNode->AddTextureCoordinate(0, Aftk::TextureCoordinate4(pImageData->texcoord[0], pImageData->texcoord[1], 0.f, 0.f));
					uvIdx[2] = pAftkMeshNode->AddTextureCoordinate(0, Aftk::TextureCoordinate4(pImageData->texcoord[2], pImageData->texcoord[1], 0.f, 0.f));
					uvIdx[3] = pAftkMeshNode->AddTextureCoordinate(0, Aftk::TextureCoordinate4(pImageData->texcoord[2], pImageData->texcoord[3], 0.f, 0.f));
				}


				//polygon vertices
				{

					S32 spriteOffsetChannel = -1;
					if (imposterData.types[j] == Imposter::E_ScreenAligned)
					{
						Aftk::C_VertexDataChannelInfo channelInfo;
						channelInfo.SetName("spriteoffset");
						channelInfo.SetDataType(Aftk::E_GenericAttributeType_Vector2F32);
						spriteOffsetChannel = pAftkMeshNode->AddVertexDataChannelInfo(channelInfo);
					}



					for (S32 l=0; l<4; ++l)
					{
						Aftk::C_PolygonVertex pv;
						pv.SetPositionIndex(posIdx[l]);
						pv.SetNormalIndex(nIdx);
						pv.SetTextureCoordinateIndex(0, uvIdx[l]);

						if (spriteOffsetChannel != -1)
						{
							const Aftk::Position3& p = pAftkMeshNode->GetVertexPosition(posIdx[l]);
							const Aftk::Matrix44 m = pAftkMeshNode->GetLocalMatrix();
							Aftk::Vector2F32 offset(p.x-m[3][0], p.y-m[3][1]);
							pv.SetData(spriteOffsetChannel, offset);
						}

						pvIdx[l] = pAftkMeshNode->AddPolygonVertex(pv);
					}
				}


				//aftk material
				S16 materialMapping = -1;
				{
					Aftk::C_MaterialNode material;
					material.SetOrigin(Aftk::E_MaterialOrigin_Maya);

					//add shader node
					Aftk::C_ShaderNode* pLodShader = new Aftk::C_ShaderNode;
					material.AddShaderNodePtr(pLodShader );
					pLodShader->SetName("LodShader");

					{
						Aftk::C_DynamicPlug* pNewPlug = new Aftk::C_DynamicPlug();
						material.AddPlugPtr(pNewPlug);
						pNewPlug->SetName("mayaSurfaceShader");
						pNewPlug->SetSourceConnectionPtr(pLodShader); 
					}

					std::string colourPlugName = "color";

					//set billboard shader
					if (imposterData.types[j] == Imposter::E_ScreenAligned)
					{
						//emulate maya hlsl shaders
						material.SetOrigin(Aftk::E_MaterialOrigin_Maya_Hlsl_2008);
						Aftk::C_DynamicPlug* pFXPlug = new Aftk::C_DynamicPlug();
						pLodShader->AddPlugPtr(pFXPlug);
						pFXPlug->SetValue(std::string("BillBoard.fx"));
						pFXPlug->SetName("shader");

						colourPlugName = "Colour";
					}



					//add shader image node(s)
					{
						Aftk::C_ShaderImageNode* pImageNode = new Aftk::C_ShaderImageNode;
						material.AddShaderNodePtr(pImageNode);
						pImageNode->SetName("file1");

						Btl::String name = imposterData.imageData[k].image.GetFileName();

						pImageNode->GetFilePlug().SetValue(std::string(name.Chars()));
						pImageNode->GetImageTypePlug().SetValue("");

						Aftk::C_DynamicPlug* pOutColourPlug = new Aftk::C_DynamicPlug();
						pImageNode->AddPlugPtr(pOutColourPlug);
						pOutColourPlug->SetName("outColor");

						Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
						pLodShader->AddPlugPtr(pPlug);
						pPlug->SetName(colourPlugName);
						pPlug->SetSourceConnectionPtr(pOutColourPlug);
					}

					const Aftk::C_MaterialResource *pMaterialResource = aftkScene.AddMaterialNode(material);
					materialMapping = pAftkMeshNode->AddPolygonMaterialMapping(pMaterialResource);
				}


				//now the actual polygons
				{
					S32 polyIdx = pAftkMeshNode->AddPolygon();
					Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(polyIdx);

					polygon.AddPolygonVertexIndex(pvIdx[0]);
					polygon.AddPolygonVertexIndex(pvIdx[1]);
					polygon.AddPolygonVertexIndex(pvIdx[2]);
					polygon.SetMaterialMappingIndex(materialMapping);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}
				{
					S32 polyIdx = pAftkMeshNode->AddPolygon();
					Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(polyIdx);

					polygon.AddPolygonVertexIndex(pvIdx[0]);
					polygon.AddPolygonVertexIndex(pvIdx[2]);
					polygon.AddPolygonVertexIndex(pvIdx[3]);
					polygon.SetMaterialMappingIndex(materialMapping);
					polygon.SetBlindDataMappingIndex(blindDataMapping);
				}

				if (viewData.mirror)
				{
					S32 nIdxBack = -1;
					{
						//vertex normal for back facing polygons
						//Aftk::Vector3F32 n(viewData.pViewVectors[k].x, viewData.pViewVectors[k].y, viewData.pViewVectors[k].z);
						Aftk::Vector3F32 n(0.f, 1.f, 0.f);
						n = Aftk::ApplyMatrix(aftkScene.GetRootSceneNodePtr()->GetWorldMatrixInverseTranspose(), n);
						UtMaths::SafeNormalise(&n);
						nIdxBack = pAftkMeshNode->AddVertexNormal((const Aftk::Normal3&)n);
					}

					//duplicate polygon vertices & invert normals
					S32 pvIdxBack[4] = {-1, -1, -1, -1};
					for (S32 l=0; l<4; ++l)
					{
						Aftk::C_PolygonVertex pv = pAftkMeshNode->GetPolygonVertex(pvIdx[l]);
						pv.SetNormalIndex(nIdxBack);
						pvIdxBack[l] = pAftkMeshNode->AddPolygonVertex(pv);
					}

					{
						S32 polyIdx = pAftkMeshNode->AddPolygon();
						Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(polyIdx);

						polygon.AddPolygonVertexIndex(pvIdxBack[0]);
						polygon.AddPolygonVertexIndex(pvIdxBack[2]);
						polygon.AddPolygonVertexIndex(pvIdxBack[1]);
						polygon.SetMaterialMappingIndex(materialMapping);
						polygon.SetBlindDataMappingIndex(blindDataMapping);
					}
					{
						S32 polyIdx = pAftkMeshNode->AddPolygon();
						Aftk::C_Polygon& polygon = pAftkMeshNode->GetPolygon(polyIdx);

						polygon.AddPolygonVertexIndex(pvIdxBack[0]);
						polygon.AddPolygonVertexIndex(pvIdxBack[3]);
						polygon.AddPolygonVertexIndex(pvIdxBack[2]);
						polygon.SetMaterialMappingIndex(materialMapping);
						polygon.SetBlindDataMappingIndex(blindDataMapping);
					}
				}
			}
		}

		//we've created the low lod imposter meshes... set up the lod group for the base mesh if needed
		Aftk::C_SceneNode *pBaseLod = 0;
		Imposter::E_ImposterType baseLodType = E_ImposterType_Count;
		{
			AftkUt::C_NodeFlagParser parser(*imposterData.pSceneNode, NULL, false);
			std::string imposterType;
			if (parser.GetParameterValue("AutoLodGeneration", imposterType))
			{
				if (imposterType != "None" && imposterType != "Inherit")
				{
					for (S32 k=0; k<E_ImposterType_Count; ++k)
					{
						if (imposterType == kImposterTypeParams[k])
						{
							if (pAftkMeshImposters[k])
							{
								Aftk::C_SceneMeshNode* pMesh = static_cast<Aftk::C_SceneMeshNode*>(pAftkMeshImposters[k]->Clone(&aftkScene));
								SetupImposterLodGroup(imposterData.pSceneNode, pMesh);
								if (k ==  Imposter::E_ScreenAligned)
								{
									//remove any rotation/scaling
									Aftk::Matrix44 m;
									UtMaths::SetIdentity(&m);
									m[3] = imposterData.pSceneNode->GetLocalMatrix()[3];
									pMesh->SetLocalMatrix(m);
								}

								//store base mesh
								pBaseLod = pAftkMeshImposters[k];
								baseLodType = (Imposter::E_ImposterType)k;
							}							
							break;
						}
					}
				}
			}
		}


		//now setup the lod groups for each instance
		{
			Btl::Vector<Aftk::C_SceneNode*> lodInstances;
			for (U32 j=0; j<imposterData.pSceneNode->GetNumInstancesOfThisNode(); ++j)
				lodInstances.PushBack(imposterData.pSceneNode->GetInstanceOfThisNode(j));


			for (U32 j=0; j<lodInstances.Size(); ++j)
			{
				Aftk::C_SceneNode *pSourceLod = pBaseLod;
				Imposter::E_ImposterType sourceLodType = baseLodType;

				AftkUt::C_NodeFlagParser parser(*lodInstances[j], NULL, false);			
				std::string imposterType;
				if (parser.GetParameterValue("AutoLodGeneration", imposterType))
				{
					if (imposterType == "None")
						pSourceLod = 0;
					else if (imposterType == "Inherit")
					{
						//use base instance type
					}
					else
					{
						for (S32 k=0; k<E_ImposterType_Count; ++k)
						{
							if (imposterType == kImposterTypeParams[k])
							{
								pSourceLod = pAftkMeshImposters[k];
								sourceLodType = (Imposter::E_ImposterType)k;
							}
						}
					}
				}

				if (pSourceLod)
				{
					bool replace = false;
					parser.GetParameterValue("ReplaceWithAutoLod", replace);

					Aftk::C_SceneMeshNode* pMesh = static_cast<Aftk::C_SceneMeshNode*>(pSourceLod->Clone(&aftkScene));

					if (!replace)
					{
						SetupImposterLodGroup(lodInstances[j], pMesh);
						if (sourceLodType ==  Imposter::E_ScreenAligned)
						{
							//remove any rotation/scaling
							Aftk::Matrix44 m;
							UtMaths::SetIdentity(&m);
							m[3] = lodInstances[j]->GetWorldMatrix()[3];
							pMesh->SetWorldMatrix(m);
						}
					}
					else
					{
						ReplaceImposterLodGroup(lodInstances[j], pMesh);

						//null out dangling pointers.
						lodInstances[j] = NULL;
						//NOTE: the AftkUt::C_NodeFlagParser instance now has an invalid ptr.
						//As we haven't modified it, it shouldn't try & commit it's changes on destruction

						if (sourceLodType ==  Imposter::E_ScreenAligned)
						{
							//remove any rotation/scaling
							Aftk::Matrix44 m;
							UtMaths::SetIdentity(&m);
							m[3] = pMesh->GetWorldMatrix()[3];
							pMesh->SetWorldMatrix(m);
						}
					}
				}
			}
		}

		for (U32 j=0; j<lengthof(pAftkMeshImposters); ++j)
			delete pAftkMeshImposters[j];
	}

	FileTag::C_TagDBManager::Instance().Write();

	return true;
}






IMPOSTER_DECL bool ProcessAftk(Aftk::C_Scene& aftkScene, const Btl::String& workingFolder, bool useTextureAtlas, Aftk::C_Scene* pAftkBaseScene, Progress::I_ProgressBar* pProgress)
{
	ImposterDataArray imposterDataArray;
	BuildImposterDataArray(aftkScene, imposterDataArray);

	Btl::OpenHashMap<S32 , Aftk::C_SceneNode*> baseMeshOverrides;

	if (pAftkBaseScene)
	{
		for (Aftk::C_SceneNodeIterator it(pAftkBaseScene); it; ++it)
		{
			AftkUt::C_NodeFlagParser actorParser(*it, NULL, false);
			S32 uid;
			if (actorParser.GetParameterValue("BaseMeshUID", uid))
				baseMeshOverrides.Insert(uid, it);
		}
		LOGDEBUG(g_Log, "%d lit mesh overrides found");
	}

	//resize & name the textures
	if (imposterDataArray.Size())
	{
		RenderImposters(imposterDataArray, baseMeshOverrides);

		Btl::Map<const Attk::C_Image*, TextureAtlas::ImageSize> imageResizeMap;

		for (S32 i=0; i<imposterDataArray.Size(); ++i)
		{
			ImposterData& imposterData = imposterDataArray[i];
			for (S32 j=0; j<imposterData.imageData.Size(); ++j)
			{
				if (!(Attk::C_Image*)imposterData.imageData[j].image)
					continue;

				U32 width = ((Attk::C_Image*)imposterData.imageData[j].image)->GetWidth();
				U32 height = ((Attk::C_Image*)imposterData.imageData[j].image)->GetHeight();

				F32 objectScale = imposterData.pSceneNode->GetWorldBounds().GetObjectRadiusSquared();
				objectScale = Maths::Sqrt(objectScale) / 40.f;

				const U32 imageClampSize = 96;
				if (width > height)
				{
					if (width > imageClampSize)
					{
						F32 s = (F32)imageClampSize/(F32)width;
						width = imageClampSize;
						height = (U32)(height*s);
					}					
				}
				else
				{
					if (height > imageClampSize)
					{
						F32 s = (F32)imageClampSize/(F32)height;
						height = imageClampSize;
						width = (U32)(width*s);
					}
				}

				imageResizeMap.Insert((Attk::C_Image*)imposterData.imageData[j].image, TextureAtlas::ImageSize(width, height));

				Btl::String name = GenerateTextureName(workingFolder, imposterData.pSceneNode, j);
				imposterData.imageData[j].image.SetFileName(name);

				//set the dirty flag so we can avoid repeat processing
				imposterData.imageData[j].image.SetDirty(true);

				//setup texture tags
				{
					FileTag::C_Tag* pTag = FileTag::C_TagDBManager::Instance().AddTag(name.Chars());
					pTag->GetAttributes().Clear();
					{
						FileTag::C_GenericAttribute attr;
						attr.GetName() = "address_mode_u";
						attr.SetValue(Util::C_String("Clamp"), FileTag::C_GenericAttribute::E_DataType_StringEnum);
						pTag->GetAttributes().PushBack(attr);
					}
					{
						FileTag::C_GenericAttribute attr;
						attr.GetName() = "address_mode_v";
						attr.SetValue(Util::C_String("Clamp"), FileTag::C_GenericAttribute::E_DataType_StringEnum);
						pTag->GetAttributes().PushBack(attr);
					}
					{
						FileTag::C_GenericAttribute attr;
						attr.GetName() = "scale_mip_alpha";
						attr.SetValue(true);
						pTag->GetAttributes().PushBack(attr);
					}
					{
						FileTag::C_GenericAttribute attr;
						attr.GetName() = "alpha_type";
						attr.SetValue(Util::C_String("Alpha Dither"), FileTag::C_GenericAttribute::E_DataType_StringEnum);
						pTag->GetAttributes().PushBack(attr);
					}
				}
			}
		}

		if (useTextureAtlas)
		{
			TextureAtlas::C_Atlas<TextureAtlas::C_HorizonPack> atlas;

			for (S32 i=0; i<imposterDataArray.Size(); ++i)
			{
				ImposterData& imposterData = imposterDataArray[i];
				for (S32 j=0; j<imposterData.imageData.Size(); ++j)
				{
					Attk::C_Image *pImage = (Attk::C_Image*)imposterData.imageData[j].image;
					if (!pImage)
						continue;

					if (imposterData.imageData[j].image.IsDirty())
					{
						Btl::Map<const Attk::C_Image*, TextureAtlas::ImageSize>::Iterator it = imageResizeMap.Find(pImage);
						BIZ_ASSERT(it != imageResizeMap.End());

						atlas.AddImage(imposterData.imageData[j].image.GetFileName(), it->GetValue());
						imposterData.imageData[j].image.SetDirty(false);
					}
				}
			}

			TextureAtlas::ImageSizeArray atlasSizes;
			if (atlas.Pack(atlasSizes, 2048, 2048, 0, 1))
			{
				Btl::Vector<C_ImageHandle> atlasImageArray;
				for (U32 i=0; i<(U32)atlasSizes.Size(); ++i)
				{
					Attk::C_Image *pAtlasImage = new Attk::C_Image();

					pAtlasImage->NewPixelData(atlasSizes[i].x, atlasSizes[i].y, 1, Attk::E_PixelFormat_R8G8B8A8U);
					memset(pAtlasImage->GetPixelDataPtr(), 0, pAtlasImage->GetBytesPerPixel(Attk::E_PixelFormat_R8G8B8A8U)*atlasSizes[i].x*atlasSizes[i].y);

					Btl::String name;
					name.Printf("%s\\texatlas_%d.tga", workingFolder.Chars(), i);
					C_ImageHandle handle = C_ImageHandle::Create(name, pAtlasImage);
					atlasImageArray.PushBack(handle);
				};

				const TextureAtlas::OutputImageDescArray& output = atlas.GetOutputImageDescArray();
				for (S32 i=0; i<output.Size(); ++i)
				{
					//search through all imposter data for the current image

					for (S32 j=0; j<imposterDataArray.Size(); ++j)
					{
						Attk::C_Image *pImage = 0;

						ImposterData& imposterData = imposterDataArray[j];
						for (S32 k=0; k<imposterData.imageData.Size(); ++k)
						{
							Btl::String name = imposterData.imageData[k].image.GetFileName();
							name = name.Lower();
							name = Misc::FormatFileString(name);

							if (name == output[i].fileName)
							{
								Attk::C_Image *pAtlasImage = (Attk::C_Image*) atlasImageArray[output[i].atlas];

								if (!pImage)
								{
									pImage = imposterData.imageData[k].image;
									//LOGDEBUG(g_Log, "%s [%d %d %d %d]", output[i].fileName.Chars(), output[i].rect.x,output[i].rect.y, output[i].rect.w, output[i].rect.h);

									U32 oldWidth = pImage->GetWidth();
									U32 oldHeight = pImage->GetHeight();

									if (oldWidth != output[i].rect.w || oldHeight != output[i].rect.h)
									{
										AttkUt::C_ImageResize::Resize(*pImage, output[i].rect.w, output[i].rect.h, 1);
										PostProcessImage(*pImage);
									}
						
									pAtlasImage->Insert(output[i].rect.x, output[i].rect.y, *pImage);
								}
								else
								{
									BIZ_ASSERT(pImage == (Attk::C_Image*)imposterData.imageData[k].image);
								}
								
								//remap image handle to the texture atlas
								imposterData.imageData[k].image = atlasImageArray[output[i].atlas];
								imposterData.imageData[k].texcoord[0] = (F32)output[i].rect.x/(F32)pAtlasImage->GetWidth();
								imposterData.imageData[k].texcoord[1] = (F32)output[i].rect.y/(F32)pAtlasImage->GetHeight();
								imposterData.imageData[k].texcoord[2] = (F32)(output[i].rect.x+output[i].rect.w)/(F32)pAtlasImage->GetWidth();
								imposterData.imageData[k].texcoord[3] = (F32)(output[i].rect.y+output[i].rect.h)/(F32)pAtlasImage->GetHeight();
							}
						}
					}
				}


				for (S32 i=0; i<atlasImageArray.Size(); ++i)
				{
					Attk::C_Image *pAtlasImage = (Attk::C_Image*) atlasImageArray[i];
					Attk::C_Image tmp;
					AttkUt::C_FloodFill::FloodFill(tmp, *pAtlasImage);
					*pAtlasImage = tmp;

					atlasImageArray[i].Write();

					//setup texture tags
					{
						FileTag::C_Tag* pTag = FileTag::C_TagDBManager::Instance().AddTag(atlasImageArray[i].GetFileName().Chars());
						pTag->GetAttributes().Clear();
						{
							FileTag::C_GenericAttribute attr;
							attr.GetName() = "address_mode_u";
							attr.SetValue(Util::C_String("Clamp"), FileTag::C_GenericAttribute::E_DataType_StringEnum);
							pTag->GetAttributes().PushBack(attr);
						}
						{
							FileTag::C_GenericAttribute attr;
							attr.GetName() = "address_mode_v";
							attr.SetValue(Util::C_String("Clamp"), FileTag::C_GenericAttribute::E_DataType_StringEnum);
							pTag->GetAttributes().PushBack(attr);
						}
						{
							FileTag::C_GenericAttribute attr;
							attr.GetName() = "NumMipMaps";
							attr.SetValue((S32)0);
							pTag->GetAttributes().PushBack(attr);
						}
						{
							FileTag::C_GenericAttribute attr;
							attr.GetName() = "alpha_type";
							attr.SetValue(Util::C_String("Alpha Dither"), FileTag::C_GenericAttribute::E_DataType_StringEnum);
							pTag->GetAttributes().PushBack(attr);
						}
					}
				}
			}
			else
			{
				LOGINFO(g_Log, "Failed to generate texture atlas.. reverting to individual textures");

				//dirty all images handles so we avoid writing out shared textures multiple times
				for (S32 i=0; i<imposterDataArray.Size(); ++i)
				{
					ImposterData& imposterData = imposterDataArray[i];
					for (S32 j=0; j<imposterData.imageData.Size(); ++j)
						imposterData.imageData[j].image.SetDirty(true);
				}

				//Ensure both dimensions of the textures are pow2
				for (S32 i=0; i<imposterDataArray.Size(); ++i)
				{
					ImposterData& imposterData = imposterDataArray[i];
					for (S32 j=0; j<imposterData.imageData.Size(); ++j)
					{
						Attk::C_Image* pImage = (Attk::C_Image*)imposterData.imageData[j].image;
						if (!pImage)
							continue;

						if (imposterData.imageData[j].image.IsDirty())
						{
							Btl::Map<const Attk::C_Image*, TextureAtlas::ImageSize>::Iterator it = imageResizeMap.Find(pImage);
							BIZ_ASSERT(it != imageResizeMap.End());

							U32 oldWidth = pImage->GetWidth();
							U32 oldHeight = pImage->GetHeight();
							U32 width = UnsignedRoundUpPow2(it->GetValue().x);
							U32 height = UnsignedRoundUpPow2(it->GetValue().y);

							if (oldWidth != width || oldHeight != height)
							{
								AttkUt::C_ImageResize::Resize(*pImage, width, height, 1);
								PostProcessImage(*pImage);
							}
							Attk::C_Image tmp;
							AttkUt::C_FloodFill::FloodFill(tmp, *pImage);
							*pImage = tmp;

							imposterData.imageData[j].image.Write();
							imposterData.imageData[j].image.SetDirty(false);
						}
					}
				}
			}
		}
		else
		{
			for (S32 i=0; i<imposterDataArray.Size(); ++i)
			{
				ImposterData& imposterData = imposterDataArray[i];
				for (S32 j=0; j<imposterData.imageData.Size(); ++j)
				{
					Attk::C_Image* pImage = (Attk::C_Image*)imposterData.imageData[j].image;
					if (!pImage)
						continue;

					if (imposterData.imageData[j].image.IsDirty())
					{
						Btl::Map<const Attk::C_Image*, TextureAtlas::ImageSize>::Iterator it = imageResizeMap.Find(pImage);
						BIZ_ASSERT(it != imageResizeMap.End());

						U32 oldWidth = pImage->GetWidth();
						U32 oldHeight = pImage->GetHeight();
						U32 width = UnsignedRoundUpPow2(it->GetValue().x);
						U32 height = UnsignedRoundUpPow2(it->GetValue().y);
						
						if (oldWidth != width || oldHeight != height)
						{
							AttkUt::C_ImageResize::Resize(*pImage, width, height, 1);
							PostProcessImage(*pImage);
						}

						Attk::C_Image tmp;
						AttkUt::C_FloodFill::FloodFill(tmp, *pImage);
						*pImage = tmp;

						imposterData.imageData[j].image.Write();
						imposterData.imageData[j].image.SetDirty(false);
					}
				}
			}
		}

		GenerateImposterGeometry(aftkScene, imposterDataArray, workingFolder);
	}

	return true;
}

}

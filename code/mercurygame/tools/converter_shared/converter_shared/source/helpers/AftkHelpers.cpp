
#include "stdafx.h"

#include <core/Maths.h>

#include <system/misc/StringUtil.h>
#include <system/memory/MemoryTracker.h>

#include <converter_shared/helpers/AftkHelpers.h>
#include <converter_shared/helpers/AftkProgress.h>
#include "AftkUt/AftkUt.h"

namespace AftkHelpers
{

bool ReadAftkScene( Aftk::C_Scene& aftkScene, const Util::C_String& aftkFilename, Progress::I_ProgressBar* pProgress )
{
	MEMORY_TRACKER_PUSH("converter_shared.AftkHelpers.ReadAftkScene");

	C_AftkProgress aftkProgress(pProgress); 
	Aftk::C_Status status = aftkScene.Read( aftkFilename.Chars(), Aftk::E_FileType_BestGuess, 0, &aftkProgress);

	MEMORY_TRACKER_POP("converter_shared.AftkHelpers.ReadAftkScene");

	if( !status )
	{
		LOGERROR( Tools::g_log, "Read Aftk Scene Failed : '%s'", aftkFilename.Chars() );
		return false;
	}

	if (!aftkScene.GetRootSceneNodePtr())
	{
		LOGERROR( Tools::g_log, "Aftk Scene has no RootSceneNodePtr : '%s'", aftkFilename.Chars() );
		return false;
	}

	return true;
}

bool WriteAftkScene( const Aftk::C_Scene& aftkScene, const Util::C_String& aftkFilename, Progress::I_ProgressBar* pProgress )
{
	Util::C_String outputDir = Util::ExtractDirectory(aftkFilename);

	Sys::C_FileSystem::Instance().CreateDirectory(outputDir);

	C_AftkProgress aftkProgress(pProgress); 
	Aftk::C_Status status = aftkScene.Write(aftkFilename.Chars(), Aftk::E_FileType_BestGuess, 0, 0, &aftkProgress);

	if( !status )
	{
		LOGERROR( Tools::g_log, "Write Aftk Scene Failed : '%s'", aftkFilename.Chars() );
		return false;
	}

	return true;
}

void ApplyGlobalSceneTransform( Aftk::C_SceneNode& aftkRootNode, const F32 globalScale, const bool rotateY180 )
{
	// Ensure root node has a Z flip.
	Aftk::Matrix44 zFlipMatrix;
	UtMaths::SetIdentity(&zFlipMatrix);
	zFlipMatrix[2][2] = -1.f;
	aftkRootNode.SetLocalMatrix(zFlipMatrix);
	
	// Apply scale.
	Maths::HMat44 sceneTransform = GetHMat44( aftkRootNode.GetLocalMatrix() );

	sceneTransform.AX( Maths::HVec4( sceneTransform.AX().X() * globalScale, sceneTransform.AX().Y() * globalScale, sceneTransform.AX().Z() * globalScale, sceneTransform.AX().W() ) );
	sceneTransform.AY( Maths::HVec4( sceneTransform.AY().X() * globalScale, sceneTransform.AY().Y() * globalScale, sceneTransform.AY().Z() * globalScale, sceneTransform.AY().W() ) );
	sceneTransform.AZ( Maths::HVec4( sceneTransform.AZ().X() * globalScale, sceneTransform.AZ().Y() * globalScale, sceneTransform.AZ().Z() * globalScale, sceneTransform.AZ().W() ) );
	sceneTransform.AW( Maths::HVec4( sceneTransform.AW().X() * globalScale, sceneTransform.AW().Y() * globalScale, sceneTransform.AW().Z() * globalScale, sceneTransform.AW().W() ) );
		
	if(rotateY180)
    {
		// Apply rotation. This effectively removes the Z flip introduced before and replaces it with a flip in X.
		Maths::HMat44 rotM;
        rotM.AX( Maths::HVec4( -1.0f,  0.0f,  0.0f, 0.0f) );
        rotM.AY( Maths::HVec4(  0.0f,  1.0f,  0.0f, 0.0f) );
        rotM.AZ( Maths::HVec4(  0.0f,  0.0f, -1.0f, 0.0f) );
        rotM.AW( Maths::HVec4(  0.0f,  0.0f,  0.0f, 1.0f) );
        
        sceneTransform = sceneTransform * rotM;
    }
    
    aftkRootNode.SetLocalMatrix( GetMatrix44(sceneTransform) );
}

bool ApplyGlobalSceneTransform( Aftk::C_Scene& aftkScene, const F32 globalScale, const bool rotateY180 )
{
	Aftk::Matrix44 identity;
	UtMaths::SetIdentity(&identity);
	
	Aftk::C_SceneNode* rootNode = aftkScene.GetRootSceneNodePtr();
	
	// Is this the root of a merged scene?
	if (rootNode->GetName() == "AftkUtImportRoot")
	{
		// Set root transforms for each merged scene to identity.
		for (U32 childIndex = 0; childIndex < rootNode->GetNumChildren(); ++childIndex)
		{
			Aftk::C_SceneNode* childRootNode = rootNode->GetChildPtr(childIndex);
			if (childRootNode == NULL) continue;
			if (childRootNode->GetName() != "AftkRoot") continue;
			
			ApplyGlobalSceneTransform( *childRootNode, globalScale, rotateY180 );
		}
	}
	else
	{
		ApplyGlobalSceneTransform( *rootNode, globalScale, rotateY180 );
	}
	return true;
}

Aftk::Matrix44 GetGlobalSceneTransformMatrix44( const Aftk::C_SceneNode& node )
{
	Aftk::Matrix44 globalSceneTransform;
	UtMaths::SetIdentity(&globalSceneTransform);
	
	// Since we might be in a merged scene (with multiple AftkRoots for each scene),
	// we need to search up the hierarchy for the first "AftkRoot" and read
	// the global scene transform from that.
	const Aftk::C_SceneNode* parent = &node;
	while (parent != NULL)
	{
		if (parent->GetName() == "AftkRoot")
		{
			globalSceneTransform = parent->GetLocalMatrix();
			break;
		}
		parent = parent->GetParentPtr();
	}
	return globalSceneTransform;
}

Util::C_String GetLayerName( const Aftk::C_SceneNode* pAftkSceneNode )
{
	for (U32 iLayer = 0; iLayer < pAftkSceneNode->GetNumLayers(); iLayer++)
	{
		const Aftk::C_LayerNode *pLayer = pAftkSceneNode->GetLayerPtr(iLayer);
		return pLayer->GetName().c_str();
	}
	return "";
}

bool IsNamedLayer( const Aftk::C_LayerNode *pLayer, const Util::C_String& searchName )
{
	if( !pLayer )
		return( false ); 
    
	Util::C_String myString( pLayer->GetName().c_str() );
	myString = myString.Upper();
	
	if( myString.Search( searchName.Upper() ) != -1 )
		return( true );

	return( false );
}

bool InNamedLayer( const Aftk::C_SceneNode* pAftkSceneNode, const Util::C_String& searchName )
{
	//[Ken] Scene nodes may now be in multiple layers.
	for (U32 iLayer = 0; iLayer < pAftkSceneNode->GetNumLayers(); iLayer++)
	{
		const Aftk::C_LayerNode *pLayer = pAftkSceneNode->GetLayerPtr(iLayer);
		return IsNamedLayer( pLayer, searchName );
	}
	return false;
}

bool IsInstance( const Aftk::C_SceneNode* pAftkSceneNode )
{
	switch( pAftkSceneNode->GetType() )
	{
		case Aftk::E_ObjType_SceneInstance:
		case Aftk::E_ObjType_SceneMeshRemappedInstance:
		return true;

		default:
		break;
	}

	return false;
}

bool IsConstraint( const Aftk::C_SceneNode* pAftkSceneNode )
{
	for (U32 i = 0; i < pAftkSceneNode->GetNumDynamicPlugs(); i++)
	{
		const Aftk::I_Plug& dynPlug = *pAftkSceneNode->GetDynamicPlugPtr(i);
		if (dynPlug.GetName() == "constraintType")
		{
			return true;
		}
	}
	return false;
}

const Aftk::C_SceneNode* InstanceGetSourceNode( const Aftk::C_SceneNode* pAftkSceneNode )
{
    const Aftk::C_SceneNode* pInstancedNode = pAftkSceneNode;
	const Aftk::E_ObjType nodeType = pAftkSceneNode->GetType();

	switch(nodeType)
	{
		default:
		break;
		
		case Aftk::E_ObjType_SceneInstance:
		{
			Aftk::C_SceneInstanceNode* pInstanceNode = (Aftk::C_SceneInstanceNode*)pAftkSceneNode;
			pInstancedNode = pInstanceNode->GetSourceNodePtr();
		}
		break; 

		case Aftk::E_ObjType_SceneMeshRemappedInstance:
		{
			Aftk::C_SceneMeshRemappedInstanceNode* pInstanceNode = (Aftk::C_SceneMeshRemappedInstanceNode*)pAftkSceneNode;
			pInstancedNode = pInstanceNode->GetSourceNodePtr();
		}
		break; 
	}
	
	return pInstancedNode;
}

Aftk::Matrix44 GetMatrix44( const Maths::HMat44& hMat )
{
	Aftk::Matrix44 aftkMatrix;

    aftkMatrix[0][0] = hMat.AX().X();
    aftkMatrix[0][1] = hMat.AX().Y();
    aftkMatrix[0][2] = hMat.AX().Z();
    aftkMatrix[0][3] = hMat.AX().W();

    aftkMatrix[1][0] = hMat.AY().X();
    aftkMatrix[1][1] = hMat.AY().Y();
    aftkMatrix[1][2] = hMat.AY().Z();
    aftkMatrix[1][3] = hMat.AY().W();

    aftkMatrix[2][0] = hMat.AZ().X();
    aftkMatrix[2][1] = hMat.AZ().Y();
    aftkMatrix[2][2] = hMat.AZ().Z();
    aftkMatrix[2][3] = hMat.AZ().W();

    aftkMatrix[3][0] = hMat.AW().X();
    aftkMatrix[3][1] = hMat.AW().Y();
    aftkMatrix[3][2] = hMat.AW().Z();
    aftkMatrix[3][3] = hMat.AW().W();
	return aftkMatrix;
}

Maths::HMat44 GetHMat44( const Aftk::Matrix44& aftkMatrix )
{
	Maths::HMat44 result;

	result.AX( Maths::HVec4(aftkMatrix[0][0], aftkMatrix[0][1], aftkMatrix[0][2], aftkMatrix[0][3]) );
	result.AY( Maths::HVec4(aftkMatrix[1][0], aftkMatrix[1][1], aftkMatrix[1][2], aftkMatrix[1][3]) );
	result.AZ( Maths::HVec4(aftkMatrix[2][0], aftkMatrix[2][1], aftkMatrix[2][2], aftkMatrix[2][3]) );
	result.AW( Maths::HVec4(aftkMatrix[3][0], aftkMatrix[3][1], aftkMatrix[3][2], aftkMatrix[3][3]) );

	return result;
}

Maths::HMat44 GetHMat44( const Aftk::C_SceneNode* pAftkSceneNode )
{
	return GetHMat44( pAftkSceneNode->GetWorldMatrix() );
}

Maths::HMat43 GetHMat43( const Maths::HMat44 origMatrix, const bool orthoNormal, const bool flipZ )
{
	Maths::HMat43 result;

	Maths::HVec3 aX, aY, aZ, aW;

	aX = Maths::HVec3( origMatrix.AX() );
	aY = Maths::HVec3( origMatrix.AY() );
	aZ = Maths::HVec3( origMatrix.AZ() );
	aW = Maths::HVec3( origMatrix.AW() );

	if( flipZ )
	{
		aZ = -aZ;
	}

	if(! (aX.Finite() && aY.Finite() && aZ.Finite()) )
	{
		aX = Maths::HVec3(1.0f, 0.0f, 0.0f);
		aY = Maths::HVec3(0.0f, 1.0f, 0.0f);
		aZ = Maths::HVec3(0.0f, 0.0f, 1.0f);
	}

	if(!aW.Finite())
	{
		aW = Maths::HVec3(0.0f, 0.0f, 0.0f);
	}
	
	if(orthoNormal)
	{
		// normalise and orthogonalise matrix

		aZ = aZ.Normalised().Normal();
		aY = aY.Normalised().Normal();

		Maths::HMat33 rotMatrix = Maths::HMat33( Maths::AxisZ(Maths::C_Normal(aZ)), Maths::AxisY(Maths::C_Normal(aY)) );

		result = Maths::HMat43(rotMatrix.AX(), rotMatrix.AY(), rotMatrix.AZ(), aW);
	}
	else
	{
		// Pass through matrices 
		result = Maths::HMat43(aX, aY, aZ, aW);
	}

	return result;
}

Maths::HMat43 GetHMat43( const Aftk::C_SceneNode* pAftkSceneNode, const bool orthoNormal, const bool flipZ )
{
	return GetHMat43( GetHMat44( pAftkSceneNode ), orthoNormal, flipZ );
}

const E_CollisionType GetCollisionType( const Aftk::C_SceneNode* pAftkSceneNode )
{	
	for (U32 i = 0; i < pAftkSceneNode->GetNumDynamicPlugs(); i++)
	{
		const Aftk::I_Plug& dynPlug = *pAftkSceneNode->GetDynamicPlugPtr(i);
		if (dynPlug.GetName() == "collisionPrimitive")
		{
			bool hasIntValue = false;
			S32 value = 0;
			if (dynPlug.GetDataType() == Aftk::E_GenericAttributeType_U32)
			{
				dynPlug.GetValue((U32&)value);
				hasIntValue = true;
			}
			else 
			if (dynPlug.GetDataType() == Aftk::E_GenericAttributeType_S32)
			{
				dynPlug.GetValue(value);
				hasIntValue = true;
			}

			if( hasIntValue )
			{
				switch(value)
				{
					case 0:
						return E_CollisionType_Sphere;
					break;

					case 1:
						return E_CollisionType_ConvexHull;
					break;

					case 2:
						return E_CollisionType_Box;
					break;

					case 3:
						return E_CollisionType_Cylinder;
					break;

					case 4:
						return E_CollisionType_Capsule;
					break;

					case 5:
						return E_CollisionType_ConvexHull;
					break;

					default:
					break;
				}
			}
			else
			{
				if (dynPlug.GetDataType() == Aftk::E_GenericAttributeType_String)
				{
					std::string value;
					dynPlug.GetValue(value);

					if(_stricmp( value.c_str(), "sphere" ) == 0 )
					{
						return E_CollisionType_Sphere;
					}

					if(_stricmp( value.c_str(), "convex hull" ) == 0 )
					{
						return E_CollisionType_ConvexHull;
					}

					if(_stricmp( value.c_str(), "box" ) == 0 )
					{
						return E_CollisionType_Box;
					}

					if(_stricmp( value.c_str(), "cylinder" ) == 0 )
					{
						return E_CollisionType_Cylinder;
					}

					if(_stricmp( value.c_str(), "capsule" ) == 0 )
					{
						return E_CollisionType_Capsule;
					}

					if(_stricmp( value.c_str(), "cone" ) == 0 )
					{
						//return E_CollisionType_ConvexHull;						
						return E_CollisionType_Cone;
					}
				}
			}
		}
	}

	return E_CollisionType_None;
}

bool IsSceneNodeLight( const Aftk::C_SceneNode* pAftkSceneNode )
{
	if (pAftkSceneNode && (
		(pAftkSceneNode->GetType() == Aftk::E_ObjType_ScenePointLight) ||
		(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneAreaLight) || 
		(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneDirectionalLight) ||
		(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneHemisphereLight) || 
		(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneLight) ||
		(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneNonAmbientLight) || 
		(pAftkSceneNode->GetType() == Aftk::E_ObjType_ScenePointLight) || 
		(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneSpotLight)
		))
		return true;
	else
		return false;
}

Maths::HVec3 CalculateSunDirection( F32 sunHeading, F32 sunElevation )
{
	// Calculate sun direction

	F32 sinHeading = sinf(sunHeading * Maths::Pi / 180.0f);
	F32 cosHeading = cosf(sunHeading * Maths::Pi / 180.0f);

	F32 sinElevation = sinf(sunElevation * Maths::Pi / 180.0f);
	F32 cosElevation = cosf(sunElevation * Maths::Pi / 180.0f);

	return Maths::HVec3( -sinHeading * cosElevation, -1.0f * sinElevation, -cosHeading * cosElevation );
}

const Aftk::C_SceneNode* GetSource(const Aftk::C_SceneNode* pAftkInstance)
{
	const Aftk::C_SceneNode* pSource;
	if (pAftkInstance->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		pSource = ((Aftk::C_SceneMeshRemappedInstanceNode*)pAftkInstance)->GetSourceNodePtr();
	else if (pAftkInstance->HasType(Aftk::E_ObjType_SceneInstance))
		pSource = ((Aftk::C_SceneInstanceNode*)pAftkInstance)->GetSourceNodePtr();
	else
		return 0;

	return pSource;
}


/// Return the texture name from a polygon.
Util::C_String GetTextureName(const Aftk::C_Polygon* pPoly)
{
	assert(pPoly);
	if(!pPoly)
		return Util::C_String("");

	S16 polygonMappingIndex = pPoly->GetMaterialMappingIndex();
	if( polygonMappingIndex != -1 )
    {
		const Aftk::C_MaterialResource* pMaterialResource = pPoly->GetMeshNodePtr()->GetPolygonMaterialMappingPtr(polygonMappingIndex);
        if(pMaterialResource)
        {
            const Aftk::C_MaterialNode& aftkMaterial = pMaterialResource->GetContent();
            
            U32 noShaderImageNodes = aftkMaterial.GetNumShaderImageNodes();
		    for(U32 shaderImageNode = 0; shaderImageNode < noShaderImageNodes; shaderImageNode++)
		    {
				const Aftk::C_ShaderImageNode* pImageNode = aftkMaterial.GetShaderImageNodePtr(shaderImageNode);
				std::string fileString;
				pImageNode->GetFilePlug().GetValue(fileString);
				return Util::C_String(fileString.c_str());
			}
		}
	}

	return Util::C_String("");
}


/// Convert an Aftk Position3 into a HVec3.
Maths::HVec3 GetHVec3(const Aftk::Position3& rkPos)
{
	return Maths::HVec3(rkPos.X(), rkPos.Y(), rkPos.Z());
}

S32 GetLod(const Aftk::C_SceneNode& node)
{
	const Aftk::C_SceneNode* pParent = node.GetParentPtr();

	S32 lod = -1;

	if (pParent && pParent->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		const Aftk::C_SceneLodGroupNode* pLodGroup = (Aftk::C_SceneLodGroupNode*) pParent;

		for (U32 i=0; i < pLodGroup->GetNumLodLevels(); i++)
		{
			if (pLodGroup->GetLodLevelInfo(i).GetSceneNodePtr()->GetName() == node.GetName())
			{
				lod = i;
				break;
			}
		}
	}

	return lod;
}

const Aftk::C_GroupLodLevelInfo* GetLodLevelInfo(const Aftk::C_SceneNode& node, int level)
{
	const Aftk::C_SceneNode* pParent = node.GetParentPtr();

	if (pParent && pParent->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		const Aftk::C_SceneLodGroupNode* pLodGroup = (Aftk::C_SceneLodGroupNode*) pParent;

		if (level > -1)
		{
			return &pLodGroup->GetLodLevelInfo(level);
		}
		else
		{
			for (U32 i=0; i < pLodGroup->GetNumLodLevels(); i++)
			{
				if (pLodGroup->GetLodLevelInfo(i).GetSceneNodePtr()->GetName() == node.GetName())
				{
					return &pLodGroup->GetLodLevelInfo(i);
				}
			}
		}
	}

	return 0;
}

void GetInstanceDrawDistance(const Aftk::C_SceneNode& node, float& distanceMin, float& distanceMax)
{
	Aftk::C_Bounds aftkBounds = node.GetWorldBounds();

	distanceMin = -FLT_MAX;
	distanceMax = FLT_MAX;

	Aftk::Vector3F32 size = aftkBounds.GetMax() - aftkBounds.GetMin();

	// Get the middle value.
	float v0 = Maths::Min(Maths::Min(size.X(), size.Y()), size.Z());
	float v1 = Maths::Max(Maths::Min(size.X(), size.Y()), size.Z());
	float v2 = Maths::Max(v1, Maths::Max(size.X(), size.Y()));
	float s = Maths::Min(v1, Maths::Max(size.X(), size.Y()));

	distanceMax = s * 200.f;

	const Aftk::C_SceneNode* pSource = AftkHelpers::GetSource(&node);
	
	if (!pSource) // It isn't an instance so we must be dealing with the base mesh.
		pSource = &node;

	// look for actor
	if (pSource)
	{
		// Look for a draw distance on the base instance.
		{
			AftkUt::C_NodeFlagParser actorString(*pSource);
			float drawDistance = -1.f;
			actorString.GetParameterValue("MaxDrawDistance", drawDistance);
			if (drawDistance > 0.f) distanceMax = drawDistance;
		}

		// Look for a draw distance on the node.
		{
			AftkUt::C_NodeFlagParser actorString(node);
			float drawDistance = -1.f;
			actorString.GetParameterValue("MaxDrawDistance", drawDistance);
			if (drawDistance > 0.f) distanceMax = drawDistance;
		}

		// Is it part of a lod group?
		if (const Aftk::C_GroupLodLevelInfo* pLodInfo = GetLodLevelInfo(*pSource))
		{
			float thresholdScale = 1.f;

			// Look for a threshold scale on the base instance.
			{
				AftkUt::C_NodeFlagParser actorString(*node.GetParentPtr());
				float f = -1.f;
				actorString.GetParameterValue("LODThresholdScale", f);
				if (f > 0.f) thresholdScale = f;
			}

			// Look for a threshold scale on the instance.
			{
				AftkUt::C_NodeFlagParser actorString(node);
				float f = -1.f;
				actorString.GetParameterValue("LODThresholdScale", f);
				if (f > 0.f) thresholdScale = f;
			}

			// Acquire max draw distance from LOD group.
			{
				AftkUt::C_NodeFlagParser actorString(*pSource->GetParentPtr());
				float drawDistance = -1.f;
				actorString.GetParameterValue("MaxDrawDistance", drawDistance);
				if (drawDistance > 0.f) distanceMax = drawDistance;
			}

			float drawDistance = pLodInfo->GetThreshold();
			if (drawDistance > 0.f) distanceMax = drawDistance * thresholdScale;

			S32 lod = GetLod(*pSource);

			if (lod > 0)
			{
				const Aftk::C_GroupLodLevelInfo* pLodInfo = GetLodLevelInfo(*pSource, lod - 1);
				distanceMin = pLodInfo->GetThreshold() * thresholdScale;
			}
		}
	}
}

// Returns true if a draw distance is required.
bool GetPolygonDrawDistance(const Aftk::C_SceneNode& node, float& distanceMin, float& distanceMax)
{
	distanceMin = -FLT_MAX;
	distanceMax = FLT_MAX;

	// Instances always have a draw distance so we assume there will be one,
	// but ignore any data on the base mesh.
	if (node.GetNumInstancesOfThisNode() > 0)
		return true;

	// Look for a draw distance on the node.
	{
		AftkUt::C_NodeFlagParser actorString(node);
		float drawDistance = -1.f;
		actorString.GetParameterValue("MaxDrawDistance", drawDistance);
		if (drawDistance > 0.f) distanceMax = drawDistance;
	}

	// Is it part of a lod group?
	if (const Aftk::C_GroupLodLevelInfo* pLodInfo = GetLodLevelInfo(node))
	{
		float thresholdScale = 1.f;

		// Look for a threshold scale on the node.
		{
			AftkUt::C_NodeFlagParser actorString(node);
			float f = -1.f;
			actorString.GetParameterValue("LODThresholdScale", f);
			if (f > 0.f) thresholdScale = f;
		}

		float drawDistance = pLodInfo->GetThreshold();
		if (drawDistance > 0.f) distanceMax = drawDistance * thresholdScale;
		S32 lod = GetLod(node);

		if (lod > 0)
		{
			const Aftk::C_GroupLodLevelInfo* pLodInfo = GetLodLevelInfo(node, lod - 1);
			distanceMin = pLodInfo->GetThreshold() * thresholdScale;
		}
	}

	return (distanceMin != -FLT_MAX || distanceMax != FLT_MAX);
}

} // namespace AftkHelpers
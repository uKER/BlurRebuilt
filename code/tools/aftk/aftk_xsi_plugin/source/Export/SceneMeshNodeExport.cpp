#include "XSIA3DExportPCH.h"

#include "../XSIDefinitions.h"
#include "../XSIErrorMacros.h"

#include "ExportOptions.h"
#include "ISceneNodeExport.h"
#include "SceneNodeExport.h"
#include "SceneMeshNodeExport.h"
#include "MaterialExport.h"

#include <core/containers/Vector.h>

namespace XSITrans
{

class JenkinsHashKeyOpSetU32
{
public:
	//Hashing function.
	static unsigned int GetHash(const Ut::UtSet<U32>& key)
	{
		unsigned int hash = 0;
		for (Ut::UtSet<U32>::ConstIterator it = key.Begin(); it != key.End(); ++it)
			hash += JenkinsHash(it->GetKey());

		return hash;
	}

	static bool Equal(const Ut::UtSet<U32>& key1, const Ut::UtSet<U32>& key2)
	{
#ifdef BUILD_CONFIG_DEBUG
		//Validate set operator== incase it goes missing again!
		if (key1.Size() != key2.Size())
		{
			assert(key1 != key2);
			return false;
		}

		for (Ut::UtSet<U32>::ConstIterator it = key1.Begin(); it != key1.End(); ++it)
			if (key2.Find(it->GetKey()) == key2.End())
			{
				assert(key1 != key2);
				return false;
			}

		assert(key1 == key2);
		return true;
#else
		//This rather depends upon utset having operator== overloaded.
		return key1 == key2;
#endif
	}

private:
	//Jenkins' hash
	static U32 JenkinsHash(U32 a)
	{
	   a = (a+0x7ed55d16) + (a<<12);
	   a = (a^0xc761c23c) ^ (a>>19);
	   a = (a+0x165667b1) + (a<<5);
	   a = (a+0xd3a2646c) ^ (a<<9);
	   a = (a+0xfd7046c5) + (a<<3);
	   a = (a^0xb55a4f09) ^ (a>>16);
	   return a;
	}
};

typedef Ut::OpenHashMap<Ut::UtSet<U32>, const Aftk::C_BlindDataResource*, JenkinsHashKeyOpSetU32> PolySetHashTable;

struct C_PolySetMembership
{
	S32 setIndices[XSI_MAX_SETS_PER_POLY];
	U32 nSets;
};

C_SceneMeshNodeExport::~C_SceneMeshNodeExport()
{
}

const Aftk::C_BlindDataResource* GetPolySetCombinationBlindData(
	XSI::PolygonMesh& xsiMesh,
	XSI::CRefArray& clusters,
	Aftk::C_SceneMeshNode& aftkMesh,
	const C_ExportOptions& options,
	PolySetHashTable& polySetCombinationTable,
	Ut::UtSet<U32>& polySetCombinationIndices)
{
	Aftk::C_Scene& aftkScene = *aftkMesh.GetScenePtr();
	if (polySetCombinationIndices.Size() == 0)
		return 0;

	PolySetHashTable::Iterator it;

	it = polySetCombinationTable.Find(polySetCombinationIndices);
	if (it == polySetCombinationTable.End())
	{
		PolySetHashTable::InsertionReturnType result =
		polySetCombinationTable.Insert(polySetCombinationIndices);
		//Update hash-table iterator.
		it = result.first;

		//Build blind data.
		Aftk::C_BlindDataNode bd;

		Aftk::C_GenericAttribute attr;
		for (
			Ut::UtSet<U32>::ConstIterator setIt = polySetCombinationIndices.Begin(); 
			setIt != polySetCombinationIndices.End(); ++setIt)
		{
			XSI::Cluster cluster = clusters[setIt->GetValue()];

			Util::C_String setName = cluster.GetName().GetAsciiString();
			//Remove namespacing for references.
			S32 pos = setName.Search("polySet");
			if (pos != -1)
				setName = setName(pos, setName.Length());
			else
			{
				//Ignore this set as it's nothing to do with polyflags.
				continue;
			}

			attr.SetName(setName.Chars());

			pos = setName.Search("polySetDropDown_");
			if (pos != -1)
			{
				Ut::Vector<Util::C_String> strings;
				setName.Tokenise(strings, '_');
				if (strings.Size() == 3)
				{
					Util::C_String typeName = strings[0];
					typeName += "_";
					typeName += strings[1].Chars();
					attr.SetName(typeName.Chars());
					attr.SetValue(strings[2].Chars());
				}
			}

			//Add attribute to blind data.
			bd.AddAttribute(attr);
		}

		if (bd.GetNumAttributes() == 0)
		{
			it->GetValue() = 0;
			return 0;
		}

		//Add blind data to scene.
		it->GetValue() = aftkScene.AddBlindDataNode(bd);
	}
	return it->GetValue();
}

bool ExportBlindData(
	Ut::BitVector& processedClusters,
	Aftk::C_SceneMeshNode& aftkMesh,
	const C_ExportOptions& options,
	XSI::PolygonMesh& xsiMesh)
{
	PolySetHashTable polySetCombinationTable;

	U32 nPolys = aftkMesh.GetNumPolygons(0);
	if (nPolys == 0)
		return Aftk::C_Status::E_Status_Success;

	Aftk::C_Status status;

	XSI::CGeometryAccessor ga = xsiMesh.GetGeometryAccessor();
	XSI::CRefArray clusters = ga.GetClusters(XSI::siClusterPolygonType);

	S32 nSets = clusters.GetCount();
	if (nSets > XSI_MAX_SETS_PER_POLY)
	{
		AFTK_DO_BEGIN << "There are > " << XSI_MAX_SETS_PER_POLY << " poly sets on mesh [" << aftkMesh.GetName() << "] this is not supported. " AFTK_DO_ERROR_RETURN_END_1;
	}

	S32 numMeshPolys = ga.GetPolygonCount();
	Ut::UtSet<U32> polySetCombination;
	XSI::CClusterElementArray elements;
	XSI::CLongArray elementArray;
	U32 n = 0;

	//Allocate an array which will store an ordered set of the member indices for each cluster.
	Ut::UtVector<Ut::DynamicVector< Ut::UtSet<S32> > > polySetComponentsSets;
	polySetComponentsSets.Resize(nSets);
	//Store ordered cluster memeber indices.
	for (S32 iSet = 0; iSet < nSets; iSet++) 
	{
		XSI::Cluster cluster = clusters[iSet];

		elements = cluster.GetElements();
		S32 nElements = elements.GetCount();
		elementArray = elements.GetArray();
		assert(nElements == elementArray.GetCount());

		Ut::UtSet<S32>& set = polySetComponentsSets[iSet];
		for (S32 kElement = 0; kElement < nElements; kElement++)
			set.Insert(elementArray[kElement]);
	}

	//Traverse polygons and build set membership information.
	for (S32 polyIndex = 0; polyIndex < numMeshPolys; polyIndex++)
	{
		//Storage for set membership
		C_PolySetMembership polySetMembership;
		memset(&polySetMembership, 0, sizeof(C_PolySetMembership));

		//Traverse sets per-poly.
		for (U32 iSet = 0; iSet < (U32)nSets; iSet++) 
		{
			XSI::Cluster cluster = clusters[iSet];
			Util::C_String setName = cluster.GetName().GetAsciiString();

			const Ut::UtSet<S32>& polySetComponentSet = polySetComponentsSets[iSet];

			if (polySetComponentSet.Size() == 0 ||
				polySetComponentSet.Find(polyIndex) == polySetComponentSet.End() ||
				setName.Search("polySet") == -1)
				continue;

			//Flag this cluster as having been processed.
			processedClusters.SetBitValue(iSet, true);
			polySetMembership.setIndices[polySetMembership.nSets] = iSet;
			polySetMembership.nSets++;
		}

		//Build poly-set combination.
		polySetCombination.Clear();
		for (U32 jSet = 0; jSet < polySetMembership.nSets; jSet++)
		{
			S32 meshLocalPolySetIndex = polySetMembership.setIndices[jSet];
			if (meshLocalPolySetIndex != -1)
				polySetCombination.Insert(meshLocalPolySetIndex);
		}
		Aftk::C_Polygon& aftkPoly = aftkMesh.GetPolygon(polyIndex);

		const Aftk::C_BlindDataResource* pRes = 
			GetPolySetCombinationBlindData(
			xsiMesh,
			clusters,
			aftkMesh,
			options,
			polySetCombinationTable,
			polySetCombination);
		if (pRes)
		{
			S16 bdLocalIndex = aftkMesh.AddPolygonBlindDataMapping(pRes);
			//Note that this will replace any legacy (non set based) poly-flags.
			aftkPoly.SetBlindDataMappingIndex(bdLocalIndex);
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

bool ExportPolygons(
	Aftk::C_SceneMeshNode& aftkMesh,
	const C_ExportOptions& options,
	XSI::PolygonMesh& mesh)
{
	// gets a geometry accessor object from the new mesh
	XSI::CGeometryAccessor ga = mesh.GetGeometryAccessor();
	LONG nPolyCount		= ga.GetPolygonCount();
	LONG nVtxCount		= ga.GetVertexCount();
	LONG nEdgeCount		= ga.GetEdgeCount();
	LONG nNodeCount		= ga.GetNodeCount();
	LONG nTriangleCount = ga.GetTriangleCount();

	// polygon vertex indices
	XSI::CLongArray polyVtxIdxArray;
	XSI::CStatus st = ga.GetVertexIndices(polyVtxIdxArray);

	XSI::CLongArray polySizeArray;
	st = ga.GetPolygonVerticesCount(polySizeArray);

	XSI::CDoubleArray vertexPositions;
	ga.GetVertexPositions(vertexPositions);

	XSI::CFloatArray nodeNormals;
	ga.GetNodeNormals(nodeNormals);

	XSI::CLongArray nodeIndices;
	ga.GetNodeIndices(nodeIndices);

	//UVs...
	XSI::CRefArray uvs = ga.GetUVs();
	S32 nUVs = uvs.GetCount();
	//Set UV set name and store UV data.
	Ut::Vector<XSI::CFloatArray> uvData;
	uvData.Resize(nUVs);
	for (S32 i = 0; i < nUVs; i++)
	{
		XSI::ClusterProperty uvSet = uvs[i];
		//Add UV set.
		U32 uvSetIndex = aftkMesh.AddTextureCoordinateSet();
		Aftk::C_TextureCoordinateSet& info = aftkMesh.GetTextureCoordinateSet(i);
		//Set name.
		info.SetName(uvSet.GetName().GetAsciiString());

		//Store data.
		uvSet.GetValues(uvData[i]);
	}
	//...UVs

	//Vertex Colours...
	XSI::CRefArray colours = ga.GetVertexColors();
	S32 nColours = colours.GetCount();
	//Set Colour set name and store colour data.
	Ut::Vector<XSI::CFloatArray> colourData;
	colourData.Resize(nColours);
	for (S32 i = 0; i < nColours; i++)
	{
		XSI::ClusterProperty colourSet = colours[i];
		
		Aftk::C_VertexDataChannelInfo info;
		//Set name and type.
		info.SetName(colourSet.GetName().GetAsciiString());
		info.SetDataType(Aftk::E_GenericAttributeType_U32);
		//Add Colour set.
		aftkMesh.AddVertexDataChannelInfo(info);

		//Store data.
		colourSet.GetValues(colourData[i]);
	}
	//...Vertex Colours

	S32 nPolys = polySizeArray.GetCount();

	aftkMesh.SetPolygonInitialCapacity(nPolys);
	U32 polyOffset = 0;

	for (S32 polyIndex = 0; polyIndex < nPolys; polyIndex++)
	{
		aftkMesh.AddPolygon();
		Aftk::C_Polygon& aftkPolygon = aftkMesh.GetPolygon(polyIndex);
		
		U32 nIndices = polySizeArray[polyIndex];
		aftkPolygon.SetPolygonVertexInitialCapacity(nIndices);

		for (U32 i = 0; i < nIndices; i++)
		{
			U32 nodeIndexIndex = polyOffset + i;
			S32 pvIndex = polyVtxIdxArray[nodeIndexIndex];
			U32 vertexOffset = pvIndex*3;
			U32 nodeIndex = nodeIndices[nodeIndexIndex] * 3;
			U32 colourIndex = nodeIndices[nodeIndexIndex] * 4;

			//Create Aftk PV
			U32 aftkPVIndex = aftkMesh.AddPolygonVertex();
			Aftk::C_PolygonVertex& pv = aftkMesh.GetPolygonVertex(aftkPVIndex);

			Aftk::Position3 pos;
			pos[0] = (F32)vertexPositions[vertexOffset];
			pos[1] = (F32)vertexPositions[vertexOffset+1];
			pos[2] = (F32)vertexPositions[vertexOffset+2];

			Aftk::Normal3 norm;
			norm[0] = nodeNormals[nodeIndex];
			norm[1] = nodeNormals[nodeIndex+1];
			norm[2] = nodeNormals[nodeIndex+2];

			S32 vPosIndex = aftkMesh.AddVertexPosition(pos);
			pv.SetPositionIndex(vPosIndex);
			S32 vNormIndex = aftkMesh.AddVertexNormal(norm);
			pv.SetNormalIndex(vNormIndex);

			//Add UVs
			for (S32 uvSet = 0; uvSet < nUVs; uvSet++)
			{
				U32 uvIndex = aftkMesh.AddTextureCoordinate(
					uvSet, 
					Aftk::TextureCoordinate4(
						uvData[uvSet][nodeIndex],
						1.0f - uvData[uvSet][nodeIndex+1], 
						0, 
						0));

				pv.SetTextureCoordinateIndex(uvSet, uvIndex);
			}

			//Add vertex colours
			for (S32 colourSet = 0; colourSet < nColours; colourSet++)
			{
				U32 colour = Aftk::ColourRGBAToU32(
					colourData[colourSet][colourIndex],
					colourData[colourSet][colourIndex+1],
					colourData[colourSet][colourIndex+2],
					colourData[colourSet][colourIndex+3]);

				float r = colourData[colourSet][colourIndex];
				float g = colourData[colourSet][colourIndex+1];
				float b = colourData[colourSet][colourIndex+2];
				float a = colourData[colourSet][colourIndex+3];
				pv.SetData(colourSet, colour);
			}

			aftkPolygon.AddPolygonVertexIndex(aftkPVIndex);
		}

		aftkPolygon.ReverseWindingOrder();

		polyOffset += polySizeArray[polyIndex];
	}

	return true;
}

bool ExportMaterials(
	Ut::BitVector& processedClusters,
	Aftk::C_SceneMeshNode& aftkMesh,
	const C_ExportOptions& options,
	XSI::PolygonMesh& mesh)
{
	XSI::CGeometryAccessor ga = mesh.GetGeometryAccessor();
	XSI::CRefArray clusters = ga.GetClusters(XSI::siClusterPolygonType);
	S32 nClusters = clusters.GetCount();

	for (S32 i = 0; i < nClusters; i++)
	{
		XSI::Cluster cluster = clusters[i];

		XSI::Material xsiMaterial = cluster.GetMaterial();
		if (!xsiMaterial.IsValid())
			continue;

		Aftk::C_MaterialNode aftkMtl;
		aftkMtl.SetMaterialTypeId(0);
		aftkMtl.SetOrigin(Aftk::E_MaterialOrigin_XSI);

		if (!C_MaterialExport::Export(
			aftkMtl,
			xsiMaterial))
			return false;

		Aftk::C_Scene* pScene = aftkMesh.GetScenePtr();
		const Aftk::C_MaterialResource* pRes = 
			pScene->AddMaterialNode(aftkMtl);

		S32 materialIndex = aftkMesh.AddPolygonMaterialMapping(pRes);

		XSI::CClusterElementArray elements = cluster.GetElements();

		S32 nElements = elements.GetCount();
		XSI::CLongArray elementArray = elements.GetArray();
		assert(nElements == elementArray.GetCount());
		
		//Iterate over all of the members of this cluster.
		for (S32 j = 0; j < nElements; j++)
		{
			S32 polyIndex = elementArray[j];
			Aftk::C_Polygon& poly = aftkMesh.GetPolygon(polyIndex);

			poly.SetMaterialMappingIndex(materialIndex);
		}
	}

	return true;
}

bool ExportGenericClusters(
	Ut::BitVector& processedClusters,
	Aftk::C_SceneMeshNode& aftkMesh,
	const C_ExportOptions& options,
	XSI::PolygonMesh& mesh)
{
	XSI::CGeometryAccessor ga = mesh.GetGeometryAccessor();
	XSI::CRefArray clusters = ga.GetClusters(XSI::siClusterPolygonType);
	S32 nClusters = clusters.GetCount();

	for (S32 i = 0; i < nClusters; i++)
	{
		//Skip if already processed;
		if (processedClusters.GetBitValue(i))
			continue;

		XSI::Cluster cluster = clusters[i];

		XSI::CClusterElementArray elements = cluster.GetElements();

		S32 nElements = elements.GetCount();
		XSI::CLongArray elementArray = elements.GetArray();
		assert(nElements == elementArray.GetCount());

		if (nElements == 0)
			continue;

		std::string clusterName = cluster.GetName().GetAsciiString();
		S32 clusterIndex = aftkMesh.AddPolygonCluster();
		Aftk::C_PolygonCluster* pCluster = aftkMesh.GetPolygonClusterPtr(clusterIndex);
		pCluster->SetName(clusterName);

		//Iterate over all of the members of this cluster.
		for (S32 j = 0; j < nElements; j++)
		{
			S32 polyIndex = elementArray[j];
			pCluster->AddPolygonIndex(0, polyIndex);
		}

		//Flag as processed.
		processedClusters.SetBitValue(i, true);
	}

	return true;
}

bool ExportOrphanedPolygons(
	Aftk::C_SceneMeshNode& aftkMesh,
	const C_ExportOptions& options,
	XSI::PolygonMesh& mesh)
{
	XSI::CGeometryAccessor ga = mesh.GetGeometryAccessor();
	XSI::CRefArray mats = ga.GetMaterials();

	LONG nCount = mats.GetCount();

	if (nCount == 0)
		return false;

	XSI::Material xsiMaterial(mats[0]);

	if (!xsiMaterial.IsValid())
		return false;

	Aftk::C_MaterialNode aftkMtl;
	aftkMtl.SetMaterialTypeId(0);
	aftkMtl.SetOrigin(Aftk::E_MaterialOrigin_XSI);

	if (!C_MaterialExport::Export(
		aftkMtl,
		xsiMaterial))
		return false;

	Aftk::C_Scene* pScene = aftkMesh.GetScenePtr();
	const Aftk::C_MaterialResource* pRes = 
		pScene->AddMaterialNode(aftkMtl);

	S32 materialIndex = aftkMesh.AddPolygonMaterialMapping(pRes);

	U32 nPolygons = aftkMesh.GetNumPolygons(0);

	for (U32 i = 0; i < nPolygons; i++)
	{
		Aftk::C_Polygon& poly = aftkMesh.GetPolygon(i);
		if (poly.GetMaterialMappingIndex() == -1)
			poly.SetMaterialMappingIndex(materialIndex);
	}

	return true;
}

bool ExportClusters(
	Aftk::C_SceneMeshNode& aftkMesh,
	const C_ExportOptions& options,
	XSI::PolygonMesh& mesh)
{
	XSI::CGeometryAccessor ga = mesh.GetGeometryAccessor();
	XSI::CRefArray clusters = ga.GetClusters(XSI::siClusterPolygonType);
	S32 nClusters = clusters.GetCount();
	
	Ut::BitVector processedClusters;
	processedClusters.Resize(nClusters);
	processedClusters.ClearBits();

	if (!ExportBlindData(
		processedClusters,
		aftkMesh,
		options,
		mesh))
		return false;

	if (!ExportMaterials(
		processedClusters,
		aftkMesh,
		options,
		mesh))
		return false;

	if (!ExportGenericClusters(
		processedClusters,
		aftkMesh,
		options,
		mesh))
		return false;

	if (!ExportOrphanedPolygons(
		aftkMesh,
		options,
		mesh))
		return false;

	return true;
}

bool ExportActorProperties(
	Aftk::C_SceneMeshNode& aftkMesh,
	const C_ExportOptions& options,
	XSI::X3DObject& xsiNode)
{
	XSI::CRefArray properties;

	properties = xsiNode.GetProperties();

	for (int i = 0; i < properties.GetCount(); i++)
	{
		if (properties[i].IsA(XSI::siCustomPropertyID))
		{
			XSI::CustomProperty customProperty(properties[i]);

			XSI::CRefArray parameters;
			parameters = customProperty.GetParameters();
			for (int j = 0; j < parameters.GetCount(); j++)
			{
				XSI::Parameter param(parameters[j]);
				if (!strcmp(param.GetName().GetAsciiString(), "actorString"))
				{
					XSI::CValue::DataType dataType = param.GetValueType();
					XSI::CValue value = param.GetValue();
					if (!value.IsEmpty() && dataType == XSI::siString)
					{
						XSI::CString str = (XSI::CString)value;
						std::string actorString = std::string(str.GetAsciiString());

						Aftk::C_DynamicPlug* pActorPlug = new Aftk::C_DynamicPlug;
						aftkMesh.AddPlugPtr(pActorPlug);

						pActorPlug->SetName("actorString3");
						pActorPlug->SetValue(actorString);
						break;
					}
				}
			}
		}
	}

	return true;
}

void GetCollisionPrimitiveAttribute(std::string primitiveType, XSI::Parameter& parameter, float& x, float& y, float& z, float& w)
{
	XSI::CString paramName = parameter.GetName();
	std::string paramStr = std::string(paramName.GetAsciiString());

	if (paramStr == "Radius" || paramStr == "Base Radius" || paramStr == "Width")
		x = (float)parameter.GetValue();
	else if (paramStr == "Height")
		y = (float)parameter.GetValue();
	else if (paramStr == "Length")
		z = (float)parameter.GetValue();
}

void AddCollisionPrimitivePlug(std::string primitiveType, Aftk::C_SceneMeshNode& aftkMesh, XSI::X3DObject& xsiNode)
{
	Aftk::C_DynamicPlug* pCollisionPlug = new Aftk::C_DynamicPlug;
	aftkMesh.AddPlugPtr(pCollisionPlug);

	pCollisionPlug->SetName("scale");

	float x, y, z, w;
	x = y = z = w = 0.0f;

	Aftk::C_DynamicPlug* pChildX = new Aftk::C_DynamicPlug();
	pChildX->SetName("x");
	pCollisionPlug->AddChildPtr(pChildX);

	Aftk::C_DynamicPlug* pChildY = new Aftk::C_DynamicPlug();
	pChildY->SetName("y");
	pCollisionPlug->AddChildPtr(pChildY);

	Aftk::C_DynamicPlug* pChildZ = new Aftk::C_DynamicPlug();
	pChildZ->SetName("z");
	pCollisionPlug->AddChildPtr(pChildZ);

	Aftk::C_DynamicPlug* pChildW = new Aftk::C_DynamicPlug();
	pChildW->SetName("w");
	pCollisionPlug->AddChildPtr(pChildW);

	XSI::CParameterRefArray parameters = xsiNode.GetParameters();
	S32 nParameters = parameters.GetCount();

	//OutputDebugString("\n");

	for (S32 i = 0; i < nParameters; i++)
	{
		XSI::Parameter parameter(parameters[i]);
		XSI::CString paramName = parameter.GetName();
		std::string paramStr = std::string(paramName.GetAsciiString());

		//OutputDebugString("primitiveType = ");
		//OutputDebugString(primitiveType.c_str());
		//OutputDebugString(" paramName = ");		
		//OutputDebugString(paramStr.c_str());
		//OutputDebugString("\n");

		GetCollisionPrimitiveAttribute(primitiveType, parameter, x, y, z, w);
	}

	//OutputDebugString("\n");

	pChildX->SetValue(x);
	pChildY->SetValue(y);
	pChildZ->SetValue(z);
	pChildW->SetValue(w);
	
	bool breakHere = true;
}

bool ExportCollisionPrimitives(
	Aftk::C_SceneMeshNode& aftkMesh,
	const C_ExportOptions& options,
	XSI::X3DObject& xsiNode)
{
	XSI::CRefArray properties;

	properties = xsiNode.GetProperties();

	for (int i = 0; i < properties.GetCount(); i++)
	{
		if (properties[i].IsA(XSI::siCustomPropertyID))
		{
			XSI::CustomProperty customProperty(properties[i]);

			XSI::CRefArray parameters;
			parameters = customProperty.GetParameters();
			for (int j = 0; j < parameters.GetCount(); j++)
			{
				XSI::Parameter param(parameters[j]);
				if (!strcmp(param.GetName().GetAsciiString(), "collisionPrimitiveType"))
				{
					XSI::CValue::DataType dataType = param.GetValueType();
					XSI::CValue value = param.GetValue();
					if (!value.IsEmpty() && dataType == XSI::siString)
					{
						XSI::CString str = (XSI::CString)value;
						std::string typeString = std::string(str.GetAsciiString());

						Aftk::C_DynamicPlug* pCollisionPlug = new Aftk::C_DynamicPlug;
						aftkMesh.AddPlugPtr(pCollisionPlug);

						pCollisionPlug->SetName("collisionPrimitive");
						pCollisionPlug->SetValue(typeString);

						AddCollisionPrimitivePlug(typeString, aftkMesh, xsiNode);
						
						break;
					}
				}
			}
		}
	}

	return true;
}

Aftk::C_SceneNode* C_SceneMeshNodeExport::Export(
	Aftk::C_Scene& aftkScene,
	const C_ExportOptions& options, 
	XSI::X3DObject& xsiNode)
{
	Aftk::C_SceneMeshNode* pAftkMeshNode;
	{
		pAftkMeshNode = new Aftk::C_SceneMeshNode(&aftkScene);
		if (!pAftkMeshNode)
			goto fail;

		if (!_Export(
			*pAftkMeshNode,
			options,
			xsiNode))
			goto fail;
	
		XSI::Primitive primitive = xsiNode.GetActivePrimitive();
		XSI::PolygonMesh mesh = primitive.GetGeometry();
		if (!mesh.IsValid())
			goto fail;

		if (!ExportPolygons(*pAftkMeshNode, options, mesh)) 
			goto fail;

		if (!ExportClusters(
			*pAftkMeshNode,
			options,
			mesh))
			goto fail;

		if (!ExportActorProperties(
			*pAftkMeshNode,
			options,
			xsiNode))
			goto fail;

		//Maybe export custom property stuff here?
		if (!ExportCollisionPrimitives(
			*pAftkMeshNode,
			options,
			xsiNode))
			goto fail;

		return pAftkMeshNode;
	}

fail:
	delete pAftkMeshNode;
	return 0;
}

bool C_SceneMeshNodeExport::_Export(
	Aftk::C_SceneMeshNode& aftkSceneMeshNode,
	const C_ExportOptions& options, 
	XSI::X3DObject& xsiNode)
{
	C_SceneNodeExport::_Export(
		aftkSceneMeshNode,
		options,
		xsiNode);

	return true;
}

}

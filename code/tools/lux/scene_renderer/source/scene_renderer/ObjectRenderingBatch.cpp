#include "stdafx.h"

#include <scene_renderer/ObjectRenderingBatch.h>

#include <scene_renderer/Common.h>
#include <scene_renderer/SinCos.h>
#include <scene_renderer/D3dWindow.h>

#include <scene_renderer/VertexBuffer.h>

#include <scene_renderer/Camera.h>

#include <scene_renderer/SceneRenderer.h>
#include <scene_renderer/VertexFormats.h>

using namespace SRD;

ObjectRenderingBatch::ObjectRenderingBatch()
{
	Reset();
}

ObjectRenderingBatch::~ObjectRenderingBatch()
{
	Clear();
}

void ObjectRenderingBatch::Reset()
{
	m_pD3dWindow = 0;
	m_pScene = 0;
	m_vertexBuffersBuiltFlag = false;
	m_pMeshInterfaceNode = 0;

	m_vertexBufferIndexArrays.clear();
	m_vertexBuffers.Clear();
	m_vertexBufferSizes.Clear();
}

void ObjectRenderingBatch::Clear()
{
	for (U32 i = 0; i < (U32)m_vertexBuffers.Size(); i++)
	{
		if (m_vertexBuffers[i] != 0)
		{
		    m_vertexBuffers[i]->Release();
		}
    }

	Reset();
}

void ObjectRenderingBatch::SetD3dWindowPtr(D3dWindow* pD3dWindow)
{
	m_pD3dWindow = pD3dWindow;
}

void ObjectRenderingBatch::SetAftkScenePtr(const Aftk::C_Scene* pScene)
{
	m_pScene = pScene;
}

void ObjectRenderingBatch::SetAftkSceneNodePtr(Aftk::C_SceneMeshNode* pMeshInterfaceNode)
{
	m_pMeshInterfaceNode = pMeshInterfaceNode;
}

const Aftk::C_MaterialResource*
							ObjectRenderingBatch::GetMaterialResourcePtrFromInstance(U32 instance)
{
	std::map< 
			const Aftk::C_MaterialResource*, 
			Ut::UtVector< Ut::DynamicVector< U32 > > >::iterator it;

	U32 i = 0;
	for (it = m_vertexBufferIndexArrays.begin(); it != m_vertexBufferIndexArrays.end(); it++)
	{
		if (i == instance)
		{
            return it->first;
		}
		i++;
	}
	return 0;
}

U32 ObjectRenderingBatch::GetNumVertexBufferIndexArrays()
{
	return (U32)m_vertexBufferIndexArrays.size();
}

U32 ObjectRenderingBatch::GetNumVertexBufferIndices(const Aftk::C_MaterialResource* pMaterialResource)
{
	return (U32)m_vertexBufferIndexArrays[pMaterialResource].Size();
}

U32 ObjectRenderingBatch::GetVertexBufferIndex(
	const Aftk::C_MaterialResource* pMaterialResource, U32 instance)
{
	return m_vertexBufferIndexArrays[pMaterialResource][instance];
}

U32 ObjectRenderingBatch::GetVertexBufferIndex(U32 materialIndex, U32 instance)
{
	std::map< 
		const Aftk::C_MaterialResource*, 
		Ut::UtVector< Ut::DynamicVector< U32 > > >::iterator it;


	U32 i = 0;
	for (it = m_vertexBufferIndexArrays.begin(); it != m_vertexBufferIndexArrays.end(); it++)
	{
		if (i == materialIndex)
		{
            return (it->second)[instance];
		}
	}
	return 0;
}

IDirect3DVertexBuffer9* ObjectRenderingBatch::GetVertexBufferPtr(U32 instance) const
{
	return m_vertexBuffers[instance];
}

U32	ObjectRenderingBatch::GetNumTrianglesInVertexBuffer(U32 instance) const
{
	return m_vertexBufferSizes[instance];
}

void ObjectRenderingBatch::OnUpdateDevice()
{
	if (m_vertexBuffersBuiltFlag == false)
	{
		RebuildVertexBuffers();
		m_vertexBuffersBuiltFlag = true;
	}

	// do rendering

	for (U32 i = 0; i < (U32)m_vertexBuffers.Size(); i++)
	{
		HRESULT hr;
		IDirect3DVertexBuffer9* pVertexBuffer = m_vertexBuffers[i];
		if (pVertexBuffer != 0)
		{
			IDirect3DDevice9* pD3DDevice = m_pD3dWindow->GetD3dDevicePtr();
			hr = pD3DDevice->SetStreamSource(0, pVertexBuffer, 0, 
				sizeof(D3DVertexBasicTex));
			if (FAILED(hr)) 
			{
				return;
			}	
			hr = pD3DDevice->SetFVF(D3DFVF_BASIC_TEX);
			if (FAILED(hr)) 
			{
				return;
			}	
			U32 triCount = m_vertexBufferSizes[i];
			hr = pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, triCount);
			if (FAILED(hr)) 
			{
				return;
			}
		}
	}
	
}

bool ObjectRenderingBatch::RebuildVertexBuffers()
{
	if (m_vertexBuffersBuiltFlag == false)
	{
		return BuildVertexBuffers();
	}
	else
	{
		for (U32 i = 0; i < (U32)m_vertexBuffers.Size(); i++)
		{
			if (m_vertexBuffers[i] != 0)
			{
				m_vertexBuffers[i]->Release();
			}
		}
		m_vertexBufferIndexArrays.clear();
		m_vertexBuffers.Clear();
		m_vertexBuffersBuiltFlag = false;

		return BuildVertexBuffers();
	}
}

bool ObjectRenderingBatch::BuildVertexBuffers()
{
	if (m_pMeshInterfaceNode == 0)
	{
		return false;
	}
	//To efficiently render our mesh we need to build vertex buffers sorted by material type.
	//Stage 1. We iterate over all of the polygons in the scene and store their indices in a map ordered by 
	//material resource pointer. Note that a null material resource pointer is quite valid.

	//Store arrays of polygon indices per material
	std::map< 
		const Aftk::C_MaterialResource*, 
		Ut::UtVector< Ut::DynamicVector< U32 > > > polygonIndexArrays;

	for (U32 i = 0; i < m_pMeshInterfaceNode->GetNumPolygons(0); i++)
	{
		Aftk::C_Polygon* pPoly = &m_pMeshInterfaceNode->GetPolygon(i);


		const Aftk::C_MaterialResource* pPolyMaterialResource = 0;
		pPolyMaterialResource = GetMaterialResourcePtr(pPoly);

		std::map< 
			const Aftk::C_MaterialResource*, 
			Ut::UtVector< Ut::DynamicVector< U32 > > >::iterator it;
		it = polygonIndexArrays.find(pPolyMaterialResource);

		if (it == polygonIndexArrays.end())
		{
			//We couldn't find an existing instance of this material. Create a new entry in our
			//material list and associate this polygon index with it.
			polygonIndexArrays[pPolyMaterialResource] = Ut::UtVector< Ut::DynamicVector< U32 > >();
			polygonIndexArrays[pPolyMaterialResource].PushBack(i);
		}
		else
		{
			//We found an existing instance of this material. Add this polygon index to the material's 
			//associated array of polygon indices.
			polygonIndexArrays[pPolyMaterialResource].PushBack(i);
		}
	}
#ifdef KEN
	{
		char buf[1024];
		_snprintf(buf, sizeof(buf), "Found %i materials \r\n", (S32)polygonIndexArrays.size() );
		OutputDebugString(buf);
	}
#endif

	//////////////// We now have a list of polygon indices per material ////////////////

	//Stage 2. For each material we encountered, build a vertex buffer for the relevent polygons

	std::map< 
		const Aftk::C_MaterialResource*, 
		Ut::UtVector< Ut::DynamicVector< U32 > > >::iterator it;
	
	U32 materialCount = 0;
	for (it = polygonIndexArrays.begin(); it != polygonIndexArrays.end(); it++)
	{
		Ut::UtVector< Ut::DynamicVector< U32 > >* pPolygonIndices = &it->second;
		{
#ifdef KEN
			char buf[1024];
			_snprintf(buf, sizeof(buf), "Material %i num polys : %i\r\n", materialCount, pPolygonIndices->Size() );
			OutputDebugString(buf);
#endif
			materialCount++;

			//Build a vertex buffer from this poly index array.
			if (!BuildVertexBufferFromIndexArray(it->first, it->second))
			{
				return false;
			}
		}
	}

	m_vertexBuffersBuiltFlag = true;
	return true;
}

bool ObjectRenderingBatch::BuildVertexBufferFromIndexArray(
	const Aftk::C_MaterialResource* pMaterialResource,
	const Ut::UtVector< Ut::DynamicVector< U32 > >& polygonIndices)
{
	//Okay, before we begin, we need to find the number of triangles in this array, bearing in mind that the
	//array might contain some n-gons.

	U32 numTriangles = 0;
	for (U32 i = 0; i < (U32)polygonIndices.Size(); i++)
	{
		const Aftk::C_Polygon* pPoly = &m_pMeshInterfaceNode->GetPolygon(polygonIndices[i]);
		numTriangles += pPoly->GetNumTriangles();
	}

	HRESULT hr;


	U32 numRemainingTriangles = numTriangles;

	U32 maxBufferSize = 2000;
	U32 currentBufferStartIndex = 0;
	U32 currentBufferEndIndex;
	U32 currentBufferSize;

	U32 currentAftkPolygonIndex = polygonIndices[0];
	U32 currentIndexArrayIndex = 0;

	while (numRemainingTriangles > 0)
	{
		if (numRemainingTriangles > maxBufferSize)
		{
			currentBufferEndIndex = currentBufferStartIndex + maxBufferSize - 1;
			currentBufferSize = maxBufferSize;
		}
		else
		{
			currentBufferEndIndex = currentBufferStartIndex + numRemainingTriangles -1;
			currentBufferSize = numRemainingTriangles;
		}
		
		IDirect3DVertexBuffer9* pCurrentVertexBuffer	= 0;
		D3DVertexBasicTex* pVertexArray					= 0;
		S32 d3dVertexIndex								= 0;
		//Create vertex buffer
		{
#ifdef KEN
			char buf[1024];
			_snprintf(buf, sizeof(buf), "tri: %i\r\n", currentBufferSize );
			OutputDebugString(buf);
#endif
			
			hr = m_pD3dWindow->GetD3dDevicePtr()->
				CreateVertexBuffer( currentBufferSize * 3 * sizeof(D3DVertexBasicTex),
					D3DUSAGE_WRITEONLY, 
					D3DFVF_BASIC_TEX,
					D3DPOOL_MANAGED, 
					&pCurrentVertexBuffer,
					NULL);
			if (FAILED(hr)) 
			{
				return false;
			}
			hr = pCurrentVertexBuffer->Lock( 
				0, 
				currentBufferSize * 3 * sizeof(D3DVertexBasicTex), 
				(void**)&pVertexArray, 
				0 );
			if (FAILED(hr)) 
			{
				return false;
			}
		}

		//Look for the first vertex data channel.
		S32 vertexColourChannel = -1;
		Aftk::E_GenericAttributeType vertexColourType = 
			Aftk::E_GenericAttributeType_Unknown;
		for (U32 iChannel = 0; iChannel < m_pMeshInterfaceNode->GetNumVertexDataChannels(); iChannel++)
		{
			const Aftk::C_VertexDataChannelInfo* pChannelInfo = &m_pMeshInterfaceNode->GetVertexDataChannelInfo(iChannel);
			if (pChannelInfo->GetName() == "vertexColour")
			{
				vertexColourChannel = iChannel;
				vertexColourType = pChannelInfo->GetDataType();
			}
		}

		//////////////// Copy into vertex buffer ////////////////

		const Aftk::Matrix44* pMatrix = &m_pMeshInterfaceNode->GetWorldMatrix();

		U32 numUVSets = m_pMeshInterfaceNode->GetNumTextureCoordinateSets();

		// find lightmap UV set
		int iLMapUVSet = -1;
		for (U32 i=0; i<m_pMeshInterfaceNode->GetNumTextureCoordinateSets(); i++)
		{
			if ( m_pMeshInterfaceNode->GetTextureCoordinateSetName(i) == "luxlightmap" )
			{
				iLMapUVSet = i;
				break;
			}
		}


		for (U32 numNewTriangles = 0; numNewTriangles < currentBufferSize; )
		{
			U32 currentAftkPolygonIndex = polygonIndices[currentIndexArrayIndex];
			Aftk::C_Polygon* pAftkPolygon = &m_pMeshInterfaceNode->GetPolygon(currentAftkPolygonIndex);

			U32 numPolygonVertexIndices = pAftkPolygon->GetNumPolygonVertices();

			// Perform on-the-fly simple triangulation
			for (U32 i = 1; i < numPolygonVertexIndices-1; i++)
			{
				const Aftk::C_PolygonVertex* pPolygonVertexIndices[3];
				pPolygonVertexIndices[0] = &pAftkPolygon->GetPolygonVertex(0);
				pPolygonVertexIndices[1] = &pAftkPolygon->GetPolygonVertex(i);
				pPolygonVertexIndices[2] = &pAftkPolygon->GetPolygonVertex(i+1);

				//Loop thru the polygon vertices and create a triangle
				for (U32 j = 0; j < 3; j++)
				{
					const Aftk::C_PolygonVertex* pAftkPolygonVertex = pPolygonVertexIndices[j];

					U32 aftkVertexPositionIndex = pAftkPolygonVertex->GetPositionIndex();
					Position3 aftkVertexPosition = 
						m_pMeshInterfaceNode->GetVertexPosition(aftkVertexPositionIndex);
					
					U32 aftkVertexNormalIndex = pAftkPolygonVertex->GetNormalIndex();
					Normal3 aftkVertexNormal = 
						m_pMeshInterfaceNode->GetVertexNormal(aftkVertexNormalIndex);

					// Convert vertex position and normal into world coordinates
					Position3 aftkVertexPositionWorld;
					Normal3 aftkVertexNormalWorld;
					aftkVertexPositionWorld = LPVS::ApplyMatrix(*pMatrix, aftkVertexPosition);
					aftkVertexNormalWorld	= LPVS::ApplyMatrix(*pMatrix, aftkVertexNormal);
					aftkVertexNormalWorld = Normal3(aftkVertexNormalWorld);

					// Copy data into vertex array
					pVertexArray->SetPosition(aftkVertexPositionWorld);
					pVertexArray->SetNormal(aftkVertexNormalWorld);

					if (vertexColourChannel != -1)
					{
						switch (vertexColourType)
						{
						case Aftk::E_GenericAttributeType_Vector4F32:
						{
							Aftk::Vector4F32 aftkColour;
							pAftkPolygonVertex->GetData(aftkColour, vertexColourChannel);

							pVertexArray->SetColour(
								(U8)(aftkColour.X()*255.0f), 
								(U8)(aftkColour.Y()*255.0f), 
								(U8)(aftkColour.Z()*255.0f), 
								(U8)(aftkColour.W()*255.0f)
								);
						}
						break;
						case Aftk::E_GenericAttributeType_U32:
						{
							U32 aftkColour;
							U8 *p = (U8*)&aftkColour;
							pAftkPolygonVertex->GetData(aftkColour, vertexColourChannel);

							pVertexArray->SetColour( p[2], p[1], p[0], p[3] );
						}
						break;
						default:
							pVertexArray->SetColour(255,255,255,255);
						}
					}
					else
					{
						//We don't have any vertex colours
						pVertexArray->SetColour((U8)(255), (U8)(255), (U8)(255), (U8)(255) );
					}

					// handle the case of one set of UVs - output main UV set
					if (numUVSets >= 1)
					{
						S32 uvIndex = pAftkPolygonVertex->GetTextureCoordinateIndex(0);

						if (uvIndex < 0)
						{
							pVertexArray->SetTextureCoordinate(0, Aftk::TextureCoordinate4(0,0,0,0));
						}
						else
						{
							const Aftk::TextureCoordinate4* pAftkUv = 
								&m_pMeshInterfaceNode->GetTextureCoordinate(0, uvIndex);
							pVertexArray->SetTextureCoordinate(0, pAftkUv->X(), pAftkUv->Y());
						}
					}

					// if there is a lightmap UV set, output that as well
					if (iLMapUVSet != -1)
					{
						// if we have multiple UV channels
						S32 uvIndex = pAftkPolygonVertex->GetTextureCoordinateIndex(iLMapUVSet);

						if (uvIndex < 0)
						{
							pVertexArray->SetTextureCoordinate(1, Aftk::TextureCoordinate4(0,0,0,0));
						}
						else
						{
							const Aftk::TextureCoordinate4* pAftkUv = 
								&m_pMeshInterfaceNode->GetTextureCoordinate(iLMapUVSet, uvIndex);
							pVertexArray->SetTextureCoordinate(1, pAftkUv->X(), pAftkUv->Y());
						}
					}

					//Increment vertex array pointer
					pVertexArray++;
					//Increment vertex index
					d3dVertexIndex++;
				}
				numNewTriangles++;
			}
			currentIndexArrayIndex++;
		}

		//////////////// END copy into vertex buffer ////////////////

		//Complete vertex buffer
		{
			hr = pCurrentVertexBuffer->Unlock();
			if (FAILED(hr)) 
			{
				return false;
			}
			currentBufferStartIndex = currentBufferEndIndex + 1;
			numRemainingTriangles -= currentBufferSize;

			//Add this vertex buffer to our array of vertex buffers
			m_vertexBuffers.PushBack(pCurrentVertexBuffer);
			U32 newVertexBufferIndex = (U32)m_vertexBuffers.Size()-1;
			m_vertexBufferSizes.PushBack(currentBufferSize);

			//Add an index to this vertex buffer organised by material.
			{
				std::map< 
					const Aftk::C_MaterialResource*, 
					Ut::UtVector< Ut::DynamicVector< U32 > > >::iterator it;

				it = m_vertexBufferIndexArrays.find(pMaterialResource);

				if (it == m_vertexBufferIndexArrays.end())
				{
					//We couldn't find an existing instance of this material. Create a new entry in our
					//vertex buffer index list and associate this vertex buffer index with it.
					m_vertexBufferIndexArrays[pMaterialResource] = Ut::UtVector< Ut::DynamicVector< U32 > >();
					m_vertexBufferIndexArrays[pMaterialResource].PushBack(newVertexBufferIndex);
				}
				else
				{
					//We found an existing instance of this material. Add this vertex buffer to the material's 
					//associated array of vertex buffer indices.
					m_vertexBufferIndexArrays[pMaterialResource].PushBack(newVertexBufferIndex);
				}
			}
		}
	} // end while

	return true;
}



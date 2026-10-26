#include <Render Lib/stdafx.h>

#include <Render Lib/SceneRendererSettings.h>
#include <Render Lib/GenericSceneRendererSettings.h>

#include <Render Lib/Primitive3DVertex.h>
#include <Render Lib/VBWrapper.h>
#include <Render Lib/DebugRender.h>

#include <Render Lib/ObjectRenderingBatch.h>

#include <Render Lib/D3DWindow.h>

#include <Render Lib/VertexBuffer.h>

#include <Render Lib/Camera.h>

#include <Render Lib/SceneRenderer.h>
#include <Render Lib/VertexFormats.h>

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
	m_pD3DWindow = 0;
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

void ObjectRenderingBatch::SetD3DWindowPtr(D3DWindow* pD3DWindow)
{
	m_pD3DWindow = pD3DWindow;
}

void ObjectRenderingBatch::SetAftkScenePtr(const AftkScene* pScene)
{
	m_pScene = pScene;
}

void ObjectRenderingBatch::SetAftkSceneMeshInterfaceNodePtr(AftkISceneMeshNode* pMeshInterfaceNode)
{
	m_pMeshInterfaceNode = pMeshInterfaceNode;
}

const AftkSharedResource< AftkMaterialNode >*
	ObjectRenderingBatch::GetMaterialResourcePtr(U32 instance)
{
	std::map< 
			const AftkSharedResource< AftkMaterialNode >*, 
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

U32 ObjectRenderingBatch::GetNumVertexBufferIndices(const AftkSharedResource< AftkMaterialNode >* pMaterialResource)
{
	return (U32)m_vertexBufferIndexArrays[pMaterialResource].Size();
}

U32 ObjectRenderingBatch::GetVertexBufferIndex(
	const AftkSharedResource< AftkMaterialNode >* pMaterialResource, U32 instance)
{
	return m_vertexBufferIndexArrays[pMaterialResource][instance];
}

U32 ObjectRenderingBatch::GetVertexBufferIndex(U32 materialIndex, U32 instance)
{
	std::map< 
		const AftkSharedResource< AftkMaterialNode >*, 
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

const AftkISceneMeshNode* ObjectRenderingBatch::GetAftkSceneMeshInterfaceNodePtr() const
{
	return m_pMeshInterfaceNode;
}

AftkISceneMeshNode* ObjectRenderingBatch::GetAftkSceneMeshInterfaceNodePtr()
{
	return m_pMeshInterfaceNode;
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
			IDirect3DDevice9* pD3DDevice = m_pD3DWindow->GetD3DDevicePtr();
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
		const AftkSharedResource< AftkMaterialNode >*, 
		Ut::UtVector< Ut::DynamicVector< U32 > > > polygonIndexArrays;

	for (U32 i = 0; i < m_pMeshInterfaceNode->GetNumPolygons(); i++)
	{
		AftkPolygon* pPoly = &m_pMeshInterfaceNode->GetPolygon(i);


		const AftkSharedResource< AftkMaterialNode >* pPolyMaterialResource = 0;
		pPolyMaterialResource = pPoly->GetMaterialResourcePtr();

		std::map< 
			const AftkSharedResource< AftkMaterialNode >*, 
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

	//////////////// We now have a list of polygon indices per material ////////////////

	//Stage 2. For each material we encountered, build a vertex buffer for the relevent polygons

	std::map< 
		const AftkSharedResource< AftkMaterialNode >*, 
		Ut::UtVector< Ut::DynamicVector< U32 > > >::iterator it;
	
	U32 materialCount = 0;
	for (it = polygonIndexArrays.begin(); it != polygonIndexArrays.end(); it++)
	{
		Ut::UtVector< Ut::DynamicVector< U32 > >* pPolygonIndices = &it->second;
		{
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
	const AftkSharedResource< AftkMaterialNode >* pMaterialResource,
	const Ut::UtVector< Ut::DynamicVector< U32 > >& polygonIndices)
{
	//Okay, before we begin, we need to find the number of triangles in this array, bearing in mind that the
	//array might contain some n-gons.

	U32 numTriangles = 0;
	for (U32 i = 0; i < (U32)polygonIndices.Size(); i++)
	{
		const AftkPolygon* pPoly = &m_pMeshInterfaceNode->GetPolygon(polygonIndices[i]);
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
			hr = m_pD3DWindow->GetD3DDevicePtr()->
				CreateVertexBuffer( currentBufferSize * 3 * sizeof(D3DVertexBasicTex),
					D3DUSAGE_WRITEONLY, 
					D3DFVF_BASIC_TEX,
					D3DPOOL_MANAGED, 
					&pCurrentVertexBuffer,
					NULL);
			if (FAILED(hr)) 
			{
				//DEBUGMANAGER << "Failed to create vertex buffer " << "\n";
				return false;
			}
			hr = pCurrentVertexBuffer->Lock( 
				0, 
				currentBufferSize * 3 * sizeof(D3DVertexBasicTex), 
				(void**)&pVertexArray, 
				0 );
			if (FAILED(hr)) 
			{
				//DEBUGMANAGER << "Failed to lock vertex buffer " << "\n";
				return false;
			}
		}

		//////////////// Copy into vertex buffer ////////////////

		HMat43 matrix(Tu::TemplateToNightmare(m_pMeshInterfaceNode->GetWorldMatrix()));

		U32 numUVSets = m_pMeshInterfaceNode->GetNumTextureCoordinateSets();

		for (U32 numNewTriangles = 0; numNewTriangles < currentBufferSize; )
		{
			U32 currentAftkPolygonIndex = polygonIndices[currentIndexArrayIndex];
			AftkPolygon* pAftkPolygon = &m_pMeshInterfaceNode->GetPolygon(currentAftkPolygonIndex);

			for (U32 triangleIndex = 0; triangleIndex < pAftkPolygon->GetNumTriangles(); triangleIndex++)
			{
				const AftkPolygonVertex* pPolygonVertexIndices[3];
				AftkStatus status = pAftkPolygon->GetTriangle(triangleIndex, pPolygonVertexIndices);
				assert(status);

				//Loop thru the polygon vertices and create a triangle
				for (U32 j = 0; j < 3; j++)
				{
					const AftkPolygonVertex* pAftkPolygonVertex = pPolygonVertexIndices[j];

					U32 aftkVertexPositionIndex = pAftkPolygonVertex->GetPositionIndex();
					HVec3 aftkVertexPosition = 
						Tu::TemplateToNightmare(
							m_pMeshInterfaceNode->GetVertexPosition(aftkVertexPositionIndex));
					
					U32 aftkVertexNormalIndex = pAftkPolygonVertex->GetNormalIndex();
					HVec3 aftkVertexNormal = 
						Tu::TemplateToNightmare(
							m_pMeshInterfaceNode->GetVertexNormal(aftkVertexNormalIndex));

					// Convert vertex position and normal into world coordinates
					HVec3 aftkVertexPositionWorld;
					HVec3 aftkVertexNormalWorld;
					aftkVertexPositionWorld = aftkVertexPosition * matrix;
					aftkVertexNormalWorld = aftkVertexNormal.TransformNormal(matrix);

					// Copy data into vertex array
					pVertexArray->SetPosition(aftkVertexPositionWorld);
					pVertexArray->SetNormal(aftkVertexNormalWorld);

					if (pAftkPolygonVertex->GetNumData() != 0)
					{
						const AftkVector4F32* pAftkColour = &pAftkPolygonVertex->GetData(0);

						pVertexArray->SetColour(
							(U8)(pAftkColour->X()*255.0f), 
							(U8)(pAftkColour->Y()*255.0f), 
							(U8)(pAftkColour->Z()*255.0f), 
							(U8)(pAftkColour->W()*255.0f)
							);
					}
					else
					{
						pVertexArray->SetColour((U8)(255), (U8)(255), (U8)(255), (U8)(255) );
					}

					for (U32 set = 0; set < numUVSets; set++)
					{
						S32 uvIndex = pAftkPolygonVertex->GetTextureCoordinateIndex(set);

						if (uvIndex < 0)
						{
							pVertexArray->SetTextureCoordinate(set, AftkTextureCoordinate4(0,0,0,0));
						}
						else
						{
							const AftkTextureCoordinate4* pAftkUv = 
								&m_pMeshInterfaceNode->GetTextureCoordinate(set, uvIndex);
							pVertexArray->SetTextureCoordinate(set, pAftkUv->X(), pAftkUv->Y());
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
				//DEBUGMANAGER << "Failed to unlock vertex buffer " << "\n";
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
					const AftkSharedResource< AftkMaterialNode >*, 
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



#include "stdafx.h"

namespace AftkUt
{
	namespace RadialEdgeMesh
	{

		void C_PolygonVertex::CopyDataFromAftkToLinked(const Aftk::C_PolygonVertex* apv, C_Mesh* mesh)
		{
			Ut::UtVector<Ut::DynamicVector<Aftk::C_VertexDataChannelInfo> >& infoArray = 
				mesh->GetAftkVertexDataChannelInfo();

			const U32 nChannels = infoArray.Size();

			m_colour = -1;
			for (U32 channel = 0; channel < nChannels; channel++)
			{
				
				U32 dataSizeInBytes = GetDataTypeSize(infoArray[channel].GetDataType());
				U8* ptr = (U8*) apv->GetDataPtr(channel);
				for (U32 byte = 0; byte < dataSizeInBytes; byte++)
				{		
					m_dataByteArray.PushBack(*(ptr+byte));
				}
				std::string name = infoArray[channel].GetName();
				if (name == "vertexColour")
				{
					m_colour = *((U32*)ptr);
				}
			}
		}
			
		void C_PolygonVertex::CopyDataFromLinkedToAftk(Aftk::C_PolygonVertex* apv, C_Mesh* mesh)
		{
			Ut::UtVector<Ut::DynamicVector<Aftk::C_VertexDataChannelInfo> >& infoArray = 
				mesh->GetAftkVertexDataChannelInfo();
			const U32 nChannels = infoArray.Size();
			
			U32 offset = 0;

			for (U32 channel = 0; channel < nChannels; channel++)
			{
				
				U32 dataSizeInBytes = GetDataTypeSize(infoArray[channel].GetDataType());
				U8* buffer = new U8[dataSizeInBytes];
				for (U32 byte = 0; byte < dataSizeInBytes; byte++)
				{		
					buffer[byte] = m_dataByteArray[offset+byte];
				}

				//big switch statement as generic set is private :-(
				//apv->SetData(channel, infoArray[channel].GetDataType(), buffer);

				switch (infoArray[channel].GetDataType())
				{
					case Aftk::E_GenericAttributeType_U8:
						apv->SetData(channel, *(U8*)buffer);
						break;
					case Aftk::E_GenericAttributeType_U16:
						apv->SetData(channel, *(U16*)buffer);
						break;
					case Aftk::E_GenericAttributeType_U32:
						//vertex colours use this
						//apv->SetData(channel, *(U32*)buffer);
						apv->SetData(channel, m_colour);
						break;
					case Aftk::E_GenericAttributeType_U64:
						apv->SetData(channel, *(U64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_S8:
						apv->SetData(channel, *(S8*)buffer);
						break;
					case Aftk::E_GenericAttributeType_S16:
						apv->SetData(channel, *(S16*)buffer);
						break;
					case Aftk::E_GenericAttributeType_S32:
						apv->SetData(channel, *(S32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_S64:
						apv->SetData(channel, *(S64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Boolean:
						apv->SetData(channel, *(bool*)buffer);
						break;
					case Aftk::E_GenericAttributeType_F32:
						apv->SetData(channel, *(F32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_F64:
						apv->SetData(channel, *(F64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2S32:
						apv->SetData(channel, *(Aftk::Vector2S32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2S16:
						apv->SetData(channel, *(Aftk::Vector2S16*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2S8:
						apv->SetData(channel, *(Aftk::Vector2S8*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2U32:
						apv->SetData(channel, *(Aftk::Vector2U32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2U16:
						apv->SetData(channel, *(Aftk::Vector2U16*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2U8:
						apv->SetData(channel, *(Aftk::Vector2U8*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3S32:
						apv->SetData(channel, *(Aftk::Vector3S32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3S16:
						apv->SetData(channel, *(Aftk::Vector3S16*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3S8:
						apv->SetData(channel, *(Aftk::Vector3S8*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3U32:
						apv->SetData(channel, *(Aftk::Vector3U32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3U16:
						apv->SetData(channel, *(Aftk::Vector3U16*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3U8:
						apv->SetData(channel, *(Aftk::Vector3U8*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4S32:
						apv->SetData(channel, *(Aftk::Vector4S32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4S16:
						apv->SetData(channel, *(Aftk::Vector4S16*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4S8:
						apv->SetData(channel, *(Aftk::Vector2S32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4U32:
						apv->SetData(channel, *(Aftk::Vector4U32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4U16:
						apv->SetData(channel, *(Aftk::Vector4U16*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4U8:
						apv->SetData(channel, *(Aftk::Vector4U8*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4U64:
						apv->SetData(channel, *(Aftk::Vector4U64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4S64:
						apv->SetData(channel, *(Aftk::Vector4S64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3U64:
						apv->SetData(channel, *(Aftk::Vector3U64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3S64:
						apv->SetData(channel, *(Aftk::Vector3S64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2U64:
						apv->SetData(channel, *(Aftk::Vector2U64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2S64:
						apv->SetData(channel, *(Aftk::Vector2S64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2F32:
						apv->SetData(channel, *(Aftk::Vector2F32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector2F64:
						apv->SetData(channel, *(Aftk::Vector2F64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3F32:
						apv->SetData(channel, *(Aftk::Vector3F32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector3F64:
						apv->SetData(channel, *(Aftk::Vector3F64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4F64:
						apv->SetData(channel, *(Aftk::Vector4F64*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Vector4F32:
						apv->SetData(channel, *(Aftk::Vector4F32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Matrix44F32:
						apv->SetData(channel, *(Aftk::Matrix44F32*)buffer);
						break;
					case Aftk::E_GenericAttributeType_Null:
					case Aftk::E_GenericAttributeType_String:
					case Aftk::E_GenericAttributeType_Unknown:
					default:
						break;
				}

				offset+=dataSizeInBytes;
				delete[] buffer;
			}
		}

		void C_Vertex::GetOrderedEdgesAndPolys(ListEdge* startEdgeItem,
										Ut::UtVector<Ut::DynamicVector<ListEdge*> >& edges,
										Ut::UtVector<Ut::DynamicVector<ListPoly*> >& polys,
										Aftk::C_Status& stat)
		{
			//Only works for 2-manifold meshes, will return failure otherwise
			
			//taking the start edge, iterate around the vertex,
			//choosing the next edge by sharing polys.
			
			//we want to choose the forward poly if the vertex is vertex 0 
			
			if (startEdgeItem == 0)
			{	
				stat = Aftk::C_Status::E_Status_Failure;
				return;
			}
			C_Edge& startEdge = startEdgeItem->m_data;
			
			if (startEdge.m_pPolygons.Size() != 2)
			{
				stat = Aftk::C_Status::E_Status_Failure;
				return;				
			}
			
			int edgeVertex = (&startEdge.m_pVertices[0]->m_data == this)?(0):(1);
			
			ListPoly* startPolyItem = 0;
			
			for (ListPoly* polyItem = startEdge.GetFirstPoly(); polyItem; polyItem = polyItem->GetNext())
			{
				//find the startedge on this poly, and if its the right way around, break
				
				for (ListEdge* edgeItem = polyItem->m_data.GetFirstEdge(); edgeItem; edgeItem = edgeItem->GetNext())
				{
					C_Edge& edge = edgeItem->m_data;
					
					if (&edge == &startEdge)
					{
						if (((edgeItem->m_flags & LIST_ITEM_EDGE_REVERSED) && (edgeVertex == 1)) ||
							((!(edgeItem->m_flags & LIST_ITEM_EDGE_REVERSED)) && (edgeVertex == 0)))
						{
							startPolyItem = polyItem;
							break;
						}
					}
				}
			}
			if (!startPolyItem)
			{
				stat = Aftk::C_Status::E_Status_Failure;
				return;
			}
			
			//we now have the starting edge and starting poly
			
			
			ListEdge* sortedEdge = startEdgeItem;
			C_Edge* currentEdge = 0;
			C_Polygon* currentPl = &startPolyItem->m_data;
			ListPoly* sortedPl = startPolyItem;
			
			while (currentEdge != &startEdge)
			{
				//add edge to list
				currentEdge = &sortedEdge->m_data;	
							
				edges.PushBack(sortedEdge);
				polys.PushBack(sortedPl);
				
				//get next poly
			
				if (currentEdge->m_pPolygons.Size() != 2)
				{
					stat = Aftk::C_Status::E_Status_Failure;
					return;
				}
				
				ListPoly* plItem = 0;
				for (plItem = currentEdge->GetFirstPoly(); plItem; plItem = plItem->GetNext())
				{
					if (&plItem->m_data != currentPl)
					{
						sortedPl = plItem;
						break;
					}
				}
				
				if (!plItem)
				{
					stat = Aftk::C_Status::E_Status_Failure;
					return;
				}
				
				//find edge on poly which is also on vertex but not current edge
				
				ListEdge* edItem = 0;
				for (edItem = plItem->m_data.GetFirstEdge(); edItem; edItem = edItem->GetNext())
				{
					ListEdge* tmpEdge = HasEdge(edItem);
					if (tmpEdge && (&edItem->m_data != currentEdge))
					{
						//we have our edge, break;
						sortedEdge = tmpEdge;
						break;
					}
				}
				
				if (!edItem)
				{
					stat = Aftk::C_Status::E_Status_Failure;
					return;
				}
			
				currentEdge = &sortedEdge->m_data;	
				currentPl = &plItem->m_data;
			
			}
			
			
			
		}

		bool C_Vertex::IntersectsPlane(
			const Aftk::Position3& planePos,
			const Aftk::Normal3& planeNorm) const 
		{
			Maths::HVec3 planePosVec = Aftk::TemplateToNightmare(planePos);
			Maths::HVec3 planeNormVec = Aftk::TemplateToNightmare(planeNorm);

			Maths::HVec3 vertex = Aftk::TemplateToNightmare(m_pos);
			Maths::HVec3 vertexVec = vertex - planePosVec;

			float dotProduct = planeNormVec | vertexVec;

			if ((dotProduct < (-1.0f*g_Tolerances.PosEpsilon())) || (dotProduct > g_Tolerances.PosEpsilon()))
			{
				return false;
			}


			return true;
		}


		void C_PolygonVertex::Lerp(
			const C_PolygonVertex& pv1, 
			const C_PolygonVertex& pv2,
			float f)
		{
			m_normal = Aftk::VectorLerp<Aftk::Normal3, 3> (pv1.m_normal, pv2.m_normal, f);
			//m_colour = Aftk::VectorLerp<Aftk::Vector4F32, 4> (pv1.m_colour, pv2.m_colour, f);
			
			m_dataByteArray = pv1.m_dataByteArray;

			int nLayers = Maths::Min(pv1.m_textureCoordinates.Size(), pv1.m_textureCoordinates.Size());
			m_textureCoordinates.Resize(nLayers);
			
			for (int i=0; i<nLayers; i++)
			{
				m_textureCoordinates[i].m_binormals = Aftk::VectorLerp<Aftk::Vector3F32, 3>
														(pv1.m_textureCoordinates[i].m_binormals,
														pv2.m_textureCoordinates[i].m_binormals, f);
				m_textureCoordinates[i].m_tangents = Aftk::VectorLerp<Aftk::Vector3F32, 3>
														(pv1.m_textureCoordinates[i].m_tangents,
														pv2.m_textureCoordinates[i].m_tangents, f);
				m_textureCoordinates[i].m_uvwx = Aftk::VectorLerp<Aftk::TextureCoordinate4, 4>
														(pv1.m_textureCoordinates[i].m_uvwx,
														pv2.m_textureCoordinates[i].m_uvwx, f);
			}
		}

		bool C_Vertex::IsBoundaryVertex() const 
		{
			for (ListEdge* edItem = m_pEdges.Begin(); edItem; edItem = edItem->GetNext())
			{
				C_Edge* edge = &edItem->m_data;
				if (edge->m_pPolygons.Size() < 2)
				{
					return true;
				}
		
			}
			return false;
		}
		
		bool C_Vertex::PolysShareAttributes() const 
		{
			for (ListEdge* edItem = m_pEdges.Begin(); edItem; edItem = edItem->GetNext())
			{
				C_Edge* edge = &edItem->m_data;
				if (!edge->PolysShareAttributes())
				{
					return false;
				}
			}
			return true;
		}
		
		bool C_Vertex::HasIntegerUVVariation() const 
		{
			//check each polyvert, and look for integer variation on the UVs
			
			U32 nSets = m_pPolygonVerts.Begin()->m_data.NumTextureCoordinatesSets();
			
			for (ListPolyVert* pvItem = m_pPolygonVerts.Begin(); pvItem; pvItem = pvItem->GetNext())
			{
				if (pvItem->m_data.NumTextureCoordinatesSets() != nSets)
				{
					return false;
				}
			}
						
			for (ListPolyVert* pvItem = m_pPolygonVerts.Begin(); pvItem; pvItem = pvItem->GetNext())
			{
				C_PolygonVertex* pv = &pvItem->m_data;
				
				U32 nSets = pv->NumTextureCoordinatesSets();
				
				for (U32 set = 0; set < nSets; set++)
				{
					Aftk::TextureCoordinate4 tc;
					tc = pv->GetTextureCoordinates(set).m_uvwx;
					
					//test every uv against every other! n^2 horridness, look at later!
					
					for (ListPolyVert* pvTestItem = m_pPolygonVerts.Begin(); pvTestItem; pvTestItem = pvTestItem->GetNext())
					{
						if (pvTestItem == pvItem)
						{
							continue;
						}
						
						C_PolygonVertex* pvTest = &pvTestItem->m_data;
						Aftk::TextureCoordinate4 tcTest;
					
						tcTest = pvTest->GetTextureCoordinates(set).m_uvwx;
					
						Aftk::TextureCoordinate4 diff = tc - tcTest;
						
						int uInt = (int)(0.5f + diff[0]);
						int vInt = (int)(0.5f + diff[1]);
						
						if (uInt && (fabs(diff[0]-uInt) < g_Tolerances.UVEpsilon()))
						{
							return true;
						}
						
						if (vInt && (fabs(diff[1]-vInt) < g_Tolerances.UVEpsilon()))
						{
							return true;
						}
						
					}
				}
			}
			
			return false;
		}
		
		void C_Vertex::SetUVsToHighestSharedValue()
		{
			if (!m_pPolygonVerts.Begin())
			{
				return;
			}
			U32 nSets = m_pPolygonVerts.Begin()->m_data.NumTextureCoordinatesSets();
			
			for (ListPolyVert* pvItem = m_pPolygonVerts.Begin(); pvItem; pvItem = pvItem->GetNext())
			{
				if (pvItem->m_data.NumTextureCoordinatesSets() != nSets)
				{
					return;
				}
			}
			
			for (U32 set = 0; set < nSets; set++)
			{				
				float uvMax[2];
				uvMax[0] = -FLT_MAX;
				uvMax[1] = -FLT_MAX;
				C_Polygon* plMax = 0;
				
				for (ListPolyVert* pvItem = m_pPolygonVerts.Begin(); pvItem; pvItem = pvItem->GetNext())
				{
					C_PolygonVertex* pv = &pvItem->m_data;
					
					Aftk::TextureCoordinate4 tc;
					tc = pv->GetTextureCoordinates(set).m_uvwx;
					
					//test every uv against every other
					
					for (ListPolyVert* pvTestItem = pvItem->GetNext(); pvTestItem; pvTestItem = pvTestItem->GetNext())
					{
						C_PolygonVertex* pvTest = &pvTestItem->m_data;
						
						if ((pv->m_pl->m_aftkBlindDataIndex != pvTest->m_pl->m_aftkBlindDataIndex) || 
							(pv->m_pl->m_aftkMaterialIndex != pvTest->m_pl->m_aftkMaterialIndex))
						{
							continue;
						}
						
						Aftk::TextureCoordinate4 tcTest;
						tcTest = pvTest->GetTextureCoordinates(set).m_uvwx;
					
						Aftk::TextureCoordinate4 diff = tc - tcTest;
						for (int uv=0; uv<2; uv++)
						{		
					
							int uvInt = (int)floor(0.5f + diff[uv]);
							
						
							//if there is an integer difference, check if its the biggest value.
							
							if (uvInt && (fabs(diff[uv]-uvInt) < g_Tolerances.UVEpsilon()) && (tc[uv] > uvMax[uv]))
							{
								uvMax[uv] = tc[uv];
								plMax = pv->m_pl;
							}
						}		
					}
						
				}
				
				//we have the biggest u and v for the polyverts on this set
				//so assign them to all the integer offsetted UVs, and increase
				//the UVs of the other polyverts on the poly by the difference.
				
				if (!plMax)
				{
					continue;
				}
				
				for (ListPolyVert* pvItem = m_pPolygonVerts.Begin(); pvItem; pvItem = pvItem->GetNext())
				{
					C_PolygonVertex* pv = &pvItem->m_data;
					C_Polygon* pl = pv->m_pl;
					
					if ((pl->m_aftkBlindDataIndex != plMax->m_aftkBlindDataIndex) || 
						(pl->m_aftkMaterialIndex != plMax->m_aftkMaterialIndex))
					{
						continue;
					}
					
					Aftk::TextureCoordinate4 tc;
					tc = pv->GetTextureCoordinates(set).m_uvwx;
					
					
					Ut::UtVector<Ut::DynamicVector<ListVert*> > verts;
					pl->GetOrderedEdgeVerts(verts);
					
					for (int uv=0; uv<2; uv++)
					{		
						int uvInt = (int)floor(0.5f + (uvMax[uv] - tc[uv]));						
						if (uvInt && (fabs((uvMax[uv] - tc[uv])-uvInt) < g_Tolerances.UVEpsilon()))
						{
							for (int i=0; i<verts.Size(); i++)
							{
								ListPolyVert* pvOtherItem = verts[i]->m_data.GetPolygonVertex(pl);
								Aftk::TextureCoordinate4* tcOther;
								tcOther = &pvOtherItem->m_data.m_textureCoordinates[set].m_uvwx;
								(*tcOther)[uv] += uvInt;
							}
						}
					}	
				}
			}			
		}

		C_Edge* C_Vertex::GetNextEdgeAround(const C_Edge* edge, const C_Polygon* poly) const 
		{
			for (ListEdge* edItem = poly->GetFirstEdge(); edItem; edItem = edItem->GetNext())
			{
				C_Edge* ed = &edItem->m_data;
				
				if ((ed->HasVertex(this)) && (ed != edge))
				{
					return ed;
				}
			}
			return 0;
		}
		
		
		bool C_PolygonVertex::operator==(const C_PolygonVertex& other)
		{
			if (other.NumTextureCoordinatesSets() != NumTextureCoordinatesSets())
			{
				return false;
			}
			
			if (other.m_colour != m_colour)
			{
				return false;
			}

			for (U32 set=0; set<NumTextureCoordinatesSets(); set++)
			{
			
				const TextureCoordinates& tc = GetTextureCoordinates(set);
				const TextureCoordinates& tcOther = other.GetTextureCoordinates(set);
			
				if (!(tc == tcOther))
				{
					return false;
				}
			}
			
			return true;
		}
		
		bool C_PolygonVertex::HasSameColourAs(const C_PolygonVertex& other) const
		{
			if (other.m_colour == m_colour)
			{
				return true;
			}
			return false;
		}
		
		bool C_PolygonVertex::HasSameTangentSpaceAs(const C_PolygonVertex& other, float tolerance) const 
		{
			if (other.NumTextureCoordinatesSets() != NumTextureCoordinatesSets())
			{
				return false;
			}
			
			if (!Aftk::EqualityTestFVector(m_normal, other.m_normal, g_Tolerances.NormalEpsilon()))
			{
				return false;
			}
		
			for (U32 set=0; set<NumTextureCoordinatesSets(); set++)
			{
			
				const TextureCoordinates& tc = GetTextureCoordinates(set);
				const TextureCoordinates& tcOther = other.GetTextureCoordinates(set);
			
				if (!(tc.HasSameTangentSpaceAs(tcOther, tolerance)))
				{
					return false;
				}
			}
			
			return true;
		}
		
		bool C_Vertex::GetBoundaryEdge(
			C_Edge** pEdgeOut, 
			C_Polygon** pPoly,
			const C_Edge* edge, 
			const C_Polygon* poly, 
			const C_Edge* startEdge, 
			const bool bFirst)
		{			
			if (!(edge && poly))
			{
				return false;
			}
			
			C_Edge* nextEdge = GetNextEdgeAround(edge, poly);
			
			if ((!nextEdge) || (nextEdge->m_pPolygons.Size() != 2))
			{
				return false;
			}

			if ((edge == startEdge) && (!bFirst))
			{
				//can const cast here, as we're not changing the value of startEdge, just 
				//assigning it to the non const returned pEdgeOut
				(*pEdgeOut) = (const_cast<C_Edge*> (startEdge));
				return false;
			}
			
			C_Polygon* nextEdgePolys[2];
			
			nextEdgePolys[0] = &nextEdge->GetFirstPoly()->m_data;
			nextEdgePolys[1] = &nextEdge->GetFirstPoly()->GetNext()->m_data;
			
			C_Polygon* nextPoly;
			(nextEdgePolys[0] == poly)?(nextPoly = nextEdgePolys[1]):(nextPoly = nextEdgePolys[0]);

			C_PolygonVertex* pv = &GetPolygonVertex(poly)->m_data;
			C_PolygonVertex* pvNext = &GetPolygonVertex(nextPoly)->m_data;

			//at this point we have the next edge, poly and polyvert
			//so test against previous ones. If all the same return true and write pEdgeOut,
			//otherwise return false
			
			//The only thing returning a true value will be a boundary	
			
			//if (boundary){ (*pEdgeOut) = nextEdge; return true;}
			
			//we need to test the tangent space and the material
			
			bool bSameMaterial, bSameTangentSpace, bSameUVs, bSameBlindData, bVertexInPlane;
			(poly->m_aftkMaterialIndex==nextPoly->m_aftkMaterialIndex)?(bSameMaterial=true):(bSameMaterial=false);
			(poly->m_aftkBlindDataIndex==nextPoly->m_aftkBlindDataIndex)?(bSameBlindData=true):(bSameBlindData=false);
			(pv->HasSameTangentSpaceAs(*pvNext, g_Tolerances.UVEpsilon()))?(bSameTangentSpace=true):(bSameTangentSpace=false);
			(poly->IsVertexInPlane(this, startEdge->GetOtherVertex(this)))?(bVertexInPlane=true):(bVertexInPlane=false);
			(*pv == *pvNext)?(bSameUVs=true):(bSameUVs=false);
			if (bSameMaterial && bSameTangentSpace && bSameUVs && bSameBlindData && bVertexInPlane)
			{
				return GetBoundaryEdge(pEdgeOut, pPoly, nextEdge, nextPoly, startEdge, false);
			}
			else
			{
				
				(*pEdgeOut) = nextEdge;
				(*pPoly) = nextPoly;
				return true;
			}
		}
		
		
		C_Edge* C_Vertex::GetNormalBoundaryEdge(
			const C_Edge* edge, 
			const C_Polygon* poly, 
			const C_Edge* startEdge) const 
		{
			//bail out if we don't have valid pointers
			
			if (!(edge && poly)) return 0;
			if (edge->m_pPolygons.Size() != 2) return 0;
			
			C_PolygonVertex* pv = &GetPolygonVertex(poly)->m_data;
			
			C_Edge* nextEdge = GetNextEdgeAround(edge, poly);
			
			if ((!nextEdge) || (nextEdge->m_pPolygons.Size() != 2)) return 0;
			if (nextEdge == startEdge) return const_cast<C_Edge*>(startEdge);
			
			C_Polygon* nextEdgePolys[2];
			
			nextEdgePolys[0] = &nextEdge->GetFirstPoly()->m_data;
			nextEdgePolys[1] = &nextEdge->GetFirstPoly()->GetNext()->m_data;
			
			C_Polygon* nextPoly;
			(nextEdgePolys[0] == poly)?(nextPoly = nextEdgePolys[1]):(nextPoly = nextEdgePolys[0]);
			C_PolygonVertex* pvNext = &GetPolygonVertex(nextPoly)->m_data;
			
			if (!pvNext) return 0;
			
			if (!Aftk::EqualityTestFVector(pv->m_normal, pvNext->m_normal))
			{
				return nextEdge;
			}
		
			return GetNormalBoundaryEdge(nextEdge, nextPoly, startEdge);		
		
		}
	
		C_Edge* C_Vertex::GetUVBoundaryEdge(
			const C_Edge* edge, 
			const C_Polygon* poly,
			const C_Edge* startEdge) const 
		{
			//bail out if we don't have valid pointers
			
			if (!(edge && poly)) return 0;
			if (edge->m_pPolygons.Size() != 2) return 0;
			
			C_PolygonVertex* pv = &GetPolygonVertex(poly)->m_data;
			
			C_Edge* nextEdge = GetNextEdgeAround(edge, poly);
			
			if ((!nextEdge) || (nextEdge->m_pPolygons.Size() != 2)) return 0;
			if (nextEdge == startEdge) return const_cast<C_Edge*>(startEdge);
			
			C_Polygon* nextEdgePolys[2];
			
			nextEdgePolys[0] = &nextEdge->GetFirstPoly()->m_data;
			nextEdgePolys[1] = &nextEdge->GetFirstPoly()->GetNext()->m_data;
			
			C_Polygon* nextPoly;
			(nextEdgePolys[0] == poly)?(nextPoly = nextEdgePolys[1]):(nextPoly = nextEdgePolys[0]);
			C_PolygonVertex* pvNext = &GetPolygonVertex(nextPoly)->m_data;
			
			if (!pvNext) return 0;
			
			if (!(*pv == *pvNext))
			{
				return nextEdge;
			}
		
			return GetUVBoundaryEdge(nextEdge, nextPoly, startEdge);
		}
		
		C_Edge* C_Vertex::GetMaterialBoundaryEdge(
			const C_Edge* edge, 
			const C_Polygon* poly, 
			const C_Edge* startEdge) const 
		{
			//bail out if we don't have valid pointers
			
			if (!(edge && poly)) return 0;
			if (edge->m_pPolygons.Size() != 2) return 0;
			
			S16 material = poly->m_aftkMaterialIndex;
						
			C_Edge* nextEdge = GetNextEdgeAround(edge, poly);
			
			if ((!nextEdge) || (nextEdge->m_pPolygons.Size() != 2)) return 0;
			if (nextEdge == startEdge) return const_cast<C_Edge*>(startEdge);
			
			C_Polygon* nextEdgePolys[2];
			
			nextEdgePolys[0] = &nextEdge->GetFirstPoly()->m_data;
			nextEdgePolys[1] = &nextEdge->GetFirstPoly()->GetNext()->m_data;
			
			C_Polygon* nextPoly;
			(nextEdgePolys[0] == poly)?(nextPoly = nextEdgePolys[1]):(nextPoly = nextEdgePolys[0]);

			S16 nextMaterial = nextPoly->m_aftkMaterialIndex;
			
			if (nextMaterial != material)
			{
				return nextEdge;
			}
		
			return GetMaterialBoundaryEdge(nextEdge, nextPoly, startEdge);
		
		}

		bool C_Vertex::PolygonVerticesShareAttributes() const 
		{
			ListPolyVert* firstPv = GetFirstPolygonVertex();
			ListPolyVert* pv = firstPv->GetNext();
			
			while (pv)
			{
				if (!(pv->m_data == firstPv->m_data))
				{
					return false;
				}
				pv = pv->GetNext();
			}
			return true;
		}
		
		bool C_Vertex::PolygonVerticesShareColour() const 
		{
			ListPolyVert* firstPv = GetFirstPolygonVertex();
			ListPolyVert* pv = firstPv->GetNext();
			
			while (pv)
			{
				if ((pv->m_data.m_colour != firstPv->m_data.m_colour))
				{
					return false;
				}
				pv = pv->GetNext();
			}
			return true;
		}
		
		bool C_Vertex::PolygonVerticesShareTangentSpace() const 
		{
			ListPolyVert* firstPv = GetFirstPolygonVertex();
			ListPolyVert* pv = firstPv->GetNext();
			
			while (pv)
			{
				if (!(pv->m_data.HasSameTangentSpaceAs(firstPv->m_data, g_Tolerances.UVEpsilon())))
				{
					return false;
				}
				pv = pv->GetNext();
			}
			return true;
		}
		
		bool C_Vertex::EdgesPolygonVerticesShareTangentSpace() const 
		{	
			for (ListEdge* edItem = GetFirstEdge(); edItem; edItem=edItem->GetNext())
			{
				for (ListPoly* plItem = edItem->m_data.GetFirstPoly(); plItem; plItem=plItem->GetNext())
				{
					if (!edItem->m_data.PolygonVerticesShareTangentSpace(&plItem->m_data))
					{
						return false;
					}
				}
			}
			
			return true;
		}

		bool C_Vertex::EdgesPolygonVerticesShareColour() const 
		{	
			for (ListEdge* edItem = GetFirstEdge(); edItem; edItem=edItem->GetNext())
			{
				for (ListPoly* plItem = edItem->m_data.GetFirstPoly(); plItem; plItem=plItem->GetNext())
				{
					if (!edItem->m_data.PolygonVerticesShareColour(&plItem->m_data))
					{
						return false;
					}
				}
			}
			
			return true;
		}

		bool C_Vertex::ArePolygonVerticesOnSameSide(
			const C_Edge* edge1, 
			const C_Edge* edge2, 
			const C_PolygonVertex* pv1, 
			const C_PolygonVertex* pv2) const 
		{
			//start at edge1, go round vertex looking for pv1 and pv2, if both found return true, else false

			for (ListPoly* plItem = edge1->GetFirstPoly(); plItem; plItem = plItem->GetNext())
			{
				C_Polygon* poly = &plItem->m_data;
				const C_Edge* edge = edge1;
				bool bPVs[2];
				bPVs[0] = false;
				bPVs[1] = false;
				while (edge && (edge!=edge2))
				{
					
					C_Edge* nextEdge = GetNextEdgeAround(edge, poly);
					
					if ((!nextEdge) || (nextEdge->m_pPolygons.Size() != 2))
					{
						return false;
					}

					C_Polygon* nextEdgePolys[2];
					
					nextEdgePolys[0] = &nextEdge->GetFirstPoly()->m_data;
					nextEdgePolys[1] = &nextEdge->GetFirstPoly()->GetNext()->m_data;
					
					C_Polygon* nextPoly;
					(nextEdgePolys[0] == poly)?(nextPoly = nextEdgePolys[1]):(nextPoly = nextEdgePolys[0]);

					C_PolygonVertex* pv = &GetPolygonVertex(poly)->m_data;
					C_Vertex* otherVt = edge->GetOtherVertex(this);
					C_PolygonVertex* otherPv = &otherVt->GetPolygonVertex(poly)->m_data;

					if ((pv == pv1) || (otherPv == pv1))
					{
						bPVs[0] = true;
					}
					if ((pv == pv2) || (otherPv == pv2))
					{
						bPVs[1] = true;
					}
	
					if (bPVs[0] && bPVs[1])
					{
						return true;
					}

					poly = nextPoly;
					edge = nextEdge;
				}

			}

			return false;

		}
		
		void C_Vertex::TestPlaneList(const Ut::Vector<C_Plane>* pPlanes)
		{
			m_planeTestBits = 0;
			m_planeIntersectBits = 0;
			//test vertex against each plane in pPlanes, and store result in bitfield.
			Maths::C_Vertex vert = Aftk::TemplateToNightmare(m_pos);
			const Ut::Vector<C_Plane>& planes = *pPlanes;

			for (int i=0; i< pPlanes->Size(); i++)
			{
				Maths::C_Plane plane(Maths::C_Normal(planes[i].m_norm[0], planes[i].m_norm[1], planes[i].m_norm[2]),
									Maths::C_Vertex(planes[i].m_pos[0], planes[i].m_pos[1], planes[i].m_pos[2]));
				float fDistance = plane.Distance(vert);
				if (fDistance > 0)
				{
					//set bit at plane index to 1
					m_planeTestBits |= (1 << i);
				}

				if ((fDistance < g_Tolerances.PosEpsilon()) && (fDistance > -g_Tolerances.PosEpsilon()))
				{
					//set intersect bit
					m_planeIntersectBits |= (1 << i);
				}
			}
		}

		bool C_Vertex::TestCachedPlane(U32 index)
		{
			assert(index < 8*sizeof(m_planeTestBits));

			if ((m_planeTestBits & (1 << index)) == 0)
			{
				return false;
			}
			return true;
		}

		bool C_Vertex::IntersectCachedPlane(U32 index)
		{
			assert(index < 8*sizeof(m_planeTestBits));

			if ((m_planeIntersectBits & (1 << index)) == 0)
			{
				return false;
			}
			return true;
		}

	}

}

//implementation file for linked surface node classes

#include <stdafx.h>

namespace AftkUt
{
	namespace RadialEdgeMesh
	{
		Debug::Logger& g_Log(Debug::GetLogger("tools.aftkut.radialmesh"));
		C_Tolerances g_Tolerances;

		C_Mesh::C_Mesh(Aftk::C_SceneMeshNode* pAftkMesh, bool bUseInputAsOutput)
		{
			m_pAftkMeshIn = pAftkMesh;

			if (bUseInputAsOutput)
			{
				m_pAftkMeshOut = m_pAftkMeshIn;
			}
			else
			{
				//we need a duplicate of the aftkmesh
				m_pAftkMeshOut = new Aftk::C_SceneMeshNode(*m_pAftkMeshIn);
			}
		}

		C_Mesh::~C_Mesh(void)
		{
			//Delete the LinkedMesh list pointers

			ListItem<C_Vertex>* vertex;
			ListItem<C_Edge>* edge;
			ListItem<C_Polygon>* polygon;
			
			//delete vertex data

			for (vertex = m_vertices.Begin(); vertex != 0; vertex = vertex->GetNext())
			{
				delete &vertex->m_data;
			}

			//delete edge data

			for (edge = m_edges.Begin(); edge != 0; edge = edge->GetNext())
			{
				delete &edge->m_data;
			}

			//delete poly data

			for (polygon = m_polygons.Begin(); polygon != 0; polygon = polygon->GetNext())
			{
				delete &polygon->m_data;		
			}
		}
//#pragma optimize("", off)

		void C_Mesh::Build(Aftk::C_Status& stat, bool bWorldSpace)
		{
			//Attempt to ensure our data is valid.
			m_pAftkMeshIn->Repair(); 
			//Needed to verify mesh integrity before making linked mesh
			m_pAftkMeshIn->Validate(); 

			//Create a linked mesh node from an aftkmesh

			//So add all the vertices to the mesh

			//if not trianulated then polys can flip, as we don't check 
			//all edges of a poly for normal inversion later.
			Aftk::C_MeshTriangulate::Triangulate(m_pAftkMeshIn);

			U32 nDataChannels = m_pAftkMeshIn->GetNumVertexDataChannels();
			for (U32 channel = 0; channel < nDataChannels; channel++)
			{
				m_aftkVertexDataChannelInfo.PushBack(m_pAftkMeshIn->GetVertexDataChannelInfo(channel));
			}

			Ut::UtVector<Ut::DynamicVector<ListItem<C_Vertex>*> > verts;

			U32 numVertices = m_pAftkMeshIn->GetNumVertexPositions();
			U32 numNormals = m_pAftkMeshIn->GetNumVertexNormals();

			verts.Resize(numVertices);

			Aftk::Matrix44 mtx = m_pAftkMeshIn->GetWorldMatrix();
			for (U32 i = 0; i < numVertices; i++)
			{
				C_Vertex* vt = new C_Vertex;
				VALIDATE_ALLOC_STAT_VOID(vt);
				vt->m_pos = m_pAftkMeshIn->GetVertexPosition(i);
				if (bWorldSpace)
					vt->m_pos = Aftk::ApplyMatrix(mtx, vt->m_pos);	// JAN: transform vertex into world-space
				ListVert* vtItem = CreateVertex(vt, stat);
				CHECK_STATUS_VOID(stat);
				verts[i] = vtItem;
			}


			//then connect by adding polys
			U32 nUVSets = m_pAftkMeshIn->GetNumTextureCoordinateSets();
			U32 numPolygons = m_pAftkMeshIn->GetNumPolygons(0);
			S32 basePolygonIndex = -1;

			for (U32 i = 0; i < numPolygons; i++)
			{
				Ut::UtVector<Ut::DynamicVector<ListItem<C_Vertex>*> > positions;
				Ut::UtVector<Ut::DynamicVector<C_PolygonVertex> > attrArray;
				const Aftk::C_Polygon& pl = m_pAftkMeshIn->GetPolygon(0, i);

				U32 numPolyVerts = pl.GetNumPolygonVertices();
				for (U32 j = 0; j < numPolyVerts; j++)
				{
					U32 polyVertIndex = pl.GetPolygonVertexIndex(j);
					const Aftk::C_PolygonVertex& pv = m_pAftkMeshIn->GetPolygonVertex(polyVertIndex);
					S32 vertIndex = pv.GetPositionIndex();

					positions.PushBack(verts[vertIndex]);
					
					C_PolygonVertex attr;
					attr.m_aftkIndex = polyVertIndex;
					//get attr data


					for (U32 k=0; k<nUVSets; k++)
					{
						TextureCoordinates texcoords;
						int index = pv.GetTextureCoordinateIndex(k);
						if (index != -1)
						{
							texcoords.m_uvwx =  m_pAftkMeshIn->GetTextureCoordinate(k, index);
						}
						index = pv.GetBinormalIndex(k);
						if (index != -1)
						{
							texcoords.m_binormals = m_pAftkMeshIn->GetVertexBinormal(k, index);
						}
						index = pv.GetTangentIndex(k);
						if (index != -1)
						{
							texcoords.m_tangents = m_pAftkMeshIn->GetVertexTangent(k, index);
						}
						attr.SetTextureCoordinates(k, texcoords);
					}
					if (pv.GetNormalIndex() == -1)
					{
						attr.SetNormal((Aftk::Normal3&)Aftk::Vector3F32(0,0,0));
					}
					else
					{
						attr.SetNormal(m_pAftkMeshIn->GetVertexNormal(pv.GetNormalIndex()));
					}
					attr.CopyDataFromAftkToLinked(&pv, this);

					assert(pv.GetNumTextureCoordinateIndices() == 
						m_pAftkMeshIn->GetNumTextureCoordinateSets());
					assert(attr.NumTextureCoordinatesSets() == 
						m_pAftkMeshIn->GetNumTextureCoordinateSets());

					attrArray.PushBack(attr);
				}

				//At this point we have the indices into the vertex list, so can 
				//add polys

				ListPoly* plItem = CreatePolygon(positions, stat);
				CHECK_STATUS_VOID(stat);

				if (pl.GetBasePolygonIndex() == -1)
				{
					basePolygonIndex++;
				}

				plItem->m_data.m_aftkIndex = i;
				plItem->m_data.m_basePolygonIndex = basePolygonIndex;
				plItem->m_data.m_aftkMaterialIndex = pl.GetMaterialMappingIndex();
				plItem->m_data.m_aftkBlindDataIndex = pl.GetBlindDataMappingIndex();

				for (int iPos = 0; iPos < positions.Size(); iPos++)
				{
					ListVert* lv = positions[iPos];
					ListPolyVert* lpv = lv->m_data.GetPolygonVertex(&plItem->m_data);
					assert(lpv);
					lpv->m_data.Set(attrArray[iPos]);
				}
			}

#ifdef BUILD_CONFIG_DEBUG
			//Sanity check.
			{
				RadialEdgeMesh::ListVert* pVertex = GetFirstVertex();
				while (pVertex)
				{
					RadialEdgeMesh::ListPolyVert* pPolyVert = pVertex->m_data.GetFirstPolygonVertex();
					while (pPolyVert)
					{
						assert(pPolyVert->m_data.NumTextureCoordinatesSets() == 
							m_pAftkMeshIn->GetNumTextureCoordinateSets());
						pPolyVert = pPolyVert->GetNext();
					}
					pVertex = pVertex->GetNext();
				}
			}
#endif
		}
//#pragma optimize("", on)

		S32 C_Mesh::AddAftkPolygonVertexFromLinked(
			const ListPoly& plItem,
			const C_Vertex& vt)
		{
			Aftk::C_PolygonVertex apv;

			//add the linked pv data to the aftk pv
			apv.SetPositionIndex(vt.m_index);

			//to add the normal, tangent space and texcoords to the 
			//aftk pv, we need to add them to the mesh and 
			//get indices

			ListPolyVert* pvItem = vt.GetPolygonVertex(&plItem.m_data);
			assert(pvItem);
			S32 newPvIndex = -1;
			if (pvItem)
			{
				C_PolygonVertex& pv = pvItem->m_data;

				pv.CopyDataFromLinkedToAftk(&apv, this);

				U32 normIndex = m_pAftkMeshOut->AddVertexNormal(pv.GetNormal());
				apv.SetNormalIndex(normIndex);
				
				U32 nUVSets = pv.NumTextureCoordinatesSets();

				for (U32 uvSet = 0; uvSet < nUVSets; uvSet++)
				{
					TextureCoordinates texcoords = pv.GetTextureCoordinates(uvSet);

					U32 uvIndex = m_pAftkMeshOut->AddTextureCoordinate(uvSet, texcoords.m_uvwx);
					apv.SetTextureCoordinateIndex(uvSet, uvIndex);
					U32 bnIndex = m_pAftkMeshOut->AddVertexBinormal(uvSet, texcoords.m_binormals);
					apv.SetBinormalIndex(uvSet, bnIndex);
					U32 tnIndex = m_pAftkMeshOut->AddVertexTangent(uvSet, texcoords.m_tangents);
					apv.SetTangentIndex(uvSet, tnIndex);
				}

				newPvIndex = m_pAftkMeshOut->AddPolygonVertex(apv);

				pv.m_aftkIndex = newPvIndex;
			}	
			//add the aftk pv to the mesh


			return newPvIndex;
		}

		Aftk::C_SceneMeshNode* C_Mesh::BuildAftk(Aftk::C_Status& stat, 
			Ut::Vector<AftkUt::RadialEdgeMesh::ListPoly*>* polyList) 
		{
			//Create an aftkmesh mesh node from a linked mesh

			//Use the existing mesh, but delete all the geometry

			if (m_polygons.Size() < 1)
			{
			//	return;
			}

			//Backup our data channel info
			{
				Ut::UtVector<Ut::DynamicVector<Aftk::C_VertexDataChannelInfo> > dataChannelInfoList;
				for (U32 i = 0; i < m_pAftkMeshOut->GetNumVertexDataChannels(); i++)
					dataChannelInfoList.PushBack(m_pAftkMeshOut->GetVertexDataChannelInfo(i));

				//This now clears our data channel descriptions.
				m_pAftkMeshOut->ClearGeometry();
				for (U32 i = 0; i < (U32)dataChannelInfoList.Size(); i++)
				{
					m_pAftkMeshOut->SetVertexDataChannelInfo(i, dataChannelInfoList[i]);
				}
			}

			//preallocate arrays

			const U32 numVerts = m_vertices.Size();
			const U32 numPolys = m_polygons.Size();
			m_pAftkMeshOut->SetVertexPositionInitialCapacity(numVerts);
			m_pAftkMeshOut->SetPolygonInitialCapacity(0, numPolys);
			
			U32 posIndex=0;

			for (ListVert* vtItem = m_vertices.Begin(); vtItem; vtItem = vtItem->GetNext())
			{
				C_Vertex& vt = vtItem->m_data;

				m_pAftkMeshOut->AddVertexPosition(vt.m_pos);

				vt.m_index = posIndex++; //set index for building polys later...
				
			}

			Ut::Vector<ListPoly*>* polys=0;
			if (!polyList)
			{
				polys = new Ut::Vector<ListPoly*>;
				polys->Reserve(m_polygons.Size());
				for (ListPoly* plItem = m_polygons.Begin(); plItem; plItem = plItem->GetNext())
				{
					polys->PushBack(plItem);
				}
			}
			else
			{
				polys = polyList;
			}

			//sort poly list in such a way that the triangles are ordered according to their
			//original base polygons pre-triangulation 

			Ut::Vector<C_PolySortItem> polySortArray;
			for (int i=0; i<polys->Size(); i++)
			{
				ListPoly* plItem = (*polys)[i];
				C_PolySortItem plSortItem(plItem);
				polySortArray.PushBack(plSortItem);
			}

			if (polys->Size() > 0)
			{
				std::sort(&polySortArray[0], &polySortArray[polys->Size()-1]);
			}

			S32 prevBasePolyIndex = -1;
			S32 currentTri = 0;

			for (int i=0; i<polys->Size(); i++)
			{
				ListPoly* plItem = polySortArray[i].m_pPolyItem;

				C_Polygon& pl = plItem->m_data;
				if (pl.m_pEdges.Size() == 0)
				{
					continue;
				}

				Aftk::C_Polygon apl;

				if (pl.m_basePolygonIndex != prevBasePolyIndex)
				{
					apl.SetBasePolygonIndex(-1);
					currentTri = i;
				}
				else
				{
					apl.SetBasePolygonIndex(currentTri);
				}

				prevBasePolyIndex = pl.m_basePolygonIndex;
				
				//We need to wire up the vertices from the edges in the right order...
				//remember edges can be stored either way round in a radial edge mesh. 

				ListEdge* edItem;
				C_Vertex* previousVerts[2];

				edItem = pl.m_pEdges.End();
				C_Edge& end = edItem->m_data;

				//preload the "previous" verts with those from the end of the list.

				previousVerts[0] = &end.m_pVertices[0]->m_data;
				previousVerts[1] = &end.m_pVertices[1]->m_data;

				for (edItem = pl.m_pEdges.Begin(); edItem; edItem = edItem->GetNext())
				{
					C_Edge& ed = edItem->m_data;
					
					C_Vertex* vts[2];

					vts[0] = &ed.m_pVertices[0]->m_data;
					vts[1] = &ed.m_pVertices[1]->m_data;
					
					int currentVertex = -1;

					if ((vts[0] == previousVerts[0]) || (vts[0] == previousVerts[1]))
					{
						currentVertex = 0;				
					}
					else if ((vts[1] == previousVerts[0]) || (vts[1] == previousVerts[1]))
					{
						currentVertex = 1;
					}

					#ifdef  AFTKUT_RADIAL_EDGE_MESH_CHECK_ERRORS
					if (currentVertex == -1)
					{
						stat = Aftk::C_Status::E_Status_Failure;
						return 0;
					}
					#endif
					
					S32 apvIndex = AddAftkPolygonVertexFromLinked(*plItem, *vts[currentVertex]);

					#ifdef  AFTKUT_RADIAL_EDGE_MESH_CHECK_ERRORS	
					if (apvIndex == -1)
					{
						stat = Aftk::C_Status::E_Status_Failure;
						return 0;
					}
					#endif

					apl.AddPolygonVertexIndex(apvIndex);
					
					previousVerts[0] = vts[0];
					previousVerts[1] = vts[1];
						
					
				}
				
				S32 newPolyIndex = m_pAftkMeshOut->AddPolygon(0,0, apl);
				Aftk::C_Polygon& newPoly = m_pAftkMeshOut->GetPolygon(newPolyIndex);

				newPoly.SetMaterialMappingIndex(pl.m_aftkMaterialIndex);
				newPoly.SetBlindDataMappingIndex(pl.m_aftkBlindDataIndex);
			}
			
			m_pAftkMeshOut->Repair();
			m_pAftkMeshOut->DeleteUnusedPolygonBlindDataMappings();
			m_pAftkMeshOut->DeleteUnusedPolygonMaterialMappings();

			return m_pAftkMeshOut;
		}



		ListItem<C_Vertex>*	C_Mesh::CreateVertex(C_Vertex* vt, Aftk::C_Status& stat)
		{
			VALIDATE_ALLOC_STAT(vt)	
			
			vt->m_index = m_vertices.Size();
			ListItem<C_Vertex>* listItem = m_vertices.Add(vt);

			return listItem;
		}


		ListItem<C_Edge>* C_Mesh::CreateEdge(
			ListItem<C_Vertex>* vItem1,  
			ListItem<C_Vertex>* vItem2, 
			ListItem<C_Polygon>* pItem,
			Aftk::C_Status& stat)
		{
			//if we can find an existing edge between these 2 vertices, just add it to the poly

			//For each edge on vertex v1

			VALIDATE_PTR(vItem1)
			VALIDATE_PTR(vItem2)

			C_Vertex& v1 = vItem1->m_data;
			C_Vertex& v2 = vItem2->m_data;

			VALIDATE_PTR(pItem)

			C_Polygon& pl = pItem->m_data;

			for (ListItem<C_Edge>* edgeitem = v1.m_pEdges.Begin(); edgeitem != 0; edgeitem = edgeitem->GetNext())
			{
				//get the edge from the list item

				C_Edge& edge = edgeitem->m_data;
				
				//If the edge already exists simply add the relevant references

				if ((edge.m_pVertices[0] == vItem1) && (edge.m_pVertices[1] == vItem2))
					 
				{
					edge.m_pPolygons.Add(&pl);

					//add the edge to the poly's and verts edge list

					ListEdge* newEdgeItem = pl.m_pEdges.Add(&edge);

					//This edge is facing forwards round the poly

					newEdgeItem->m_flags &=~ LIST_ITEM_EDGE_REVERSED;

					return newEdgeItem;
				} 
				else if ((edge.m_pVertices[1] == vItem1) && (edge.m_pVertices[0] == vItem2))
				{
					edge.m_pPolygons.Add(&pl);

					//add the edge to the poly's and verts edge list

					ListEdge* newEdgeItem = pl.m_pEdges.Add(&edge);

					//This edge is facing backwards round the poly

					newEdgeItem->m_flags |= LIST_ITEM_EDGE_REVERSED;
					return newEdgeItem;

				}

			}

			//otherwise add a new edge to the poly and the mesh
			C_Edge* ed = new C_Edge();
			VALIDATE_ALLOC_STAT(ed)
			
			//if valid, add to mesh edge list

			ed->m_index = m_edges.Size();

			ListItem<C_Edge>* newItem = m_edges.Add(ed);
			if (!newItem)
			{
				LOGERROR(g_Log, "C_Mesh::CreateEdge: Failed to add new edge to mesh edge list\n");
			}
			ed->m_pMeshListItem = newItem;

			//add the 2 vertices to the edge's vertex pointers

			ed->m_pVertices[0] = vItem1;
			ed->m_pVertices[1] = vItem2;

			//add the poly to the edge's poly pointer list

			ed->m_pPolygons.Add(&pl);

			//add the edge to the poly's and verts edge list

			pl.m_pEdges.Add(ed);
			v1.m_pEdges.Add(ed);
			v2.m_pEdges.Add(ed);


			return newItem;
		}

		ListItem<C_Polygon>* C_Mesh::CreatePolygon(	
			Ut::UtVector<Ut::DynamicVector<ListItem<C_Vertex>*> >& verts, 
			Aftk::C_Status& stat)
		{
			int nVerts = verts.Size();

			if (nVerts < 3)
			{
				LOGERROR(g_Log, "C_Mesh::CreatePolygon: Trying to generate a poly with less than three sides\n");
				return 0;
			}

			C_Polygon* pl = new C_Polygon();
			VALIDATE_ALLOC_STAT(pl)

			//if valid, add to mesh poly list

			pl->m_index = m_polygons.Size();
			ListItem<C_Polygon>* listItem = m_polygons.Add(pl);

			pl->m_pMeshListItem = listItem;

			//OK, we have some verts, link together to make polys...

			for (int i=0; i<nVerts; i++)
			{
				U32 start, end;
				start = i;
				end = (i+1)%nVerts;
				C_PolygonVertex* pv = new C_PolygonVertex();
				VALIDATE_ALLOC_STAT(pv)
				ListPolyVert* pvItem = verts[i]->m_data.m_pPolygonVerts.Add(pv);
				pv->m_pl = pl;
				CreateEdge(verts[start], verts[end], listItem, stat);				
			}
			
			pl->CalculateCentroid();

			return listItem;
		}

		ListItem<C_Polygon>* C_Mesh::CreateTriangle(
			ListItem<C_Vertex>* v1, 
			ListItem<C_Vertex>* v2,  
			ListItem<C_Vertex>* v3, 
			Aftk::C_Status& stat)
		{
			//helper function for adding tris, just avoids having to build list to pass to CreatePolygon(...)
			Ut::UtVector<Ut::DynamicVector<ListItem<C_Vertex>*> > verts;
			verts.PushBack(v1);
			verts.PushBack(v2);
			verts.PushBack(v3);
			return CreatePolygon(verts, stat);
		}

		ListItem<C_Polygon>* C_Mesh::CreateQuad(
			ListItem<C_Vertex>* v1,  
			ListItem<C_Vertex>* v2, 
			ListItem<C_Vertex>* v3, 
			ListItem<C_Vertex>* v4, 
			Aftk::C_Status& stat)
		{
			//helper function for adding quads, just avoids having to build list to pass to CreatePolygon(...)

			Ut::UtVector<Ut::DynamicVector<ListItem<C_Vertex>*> > verts;
			verts.PushBack(v1);
			verts.PushBack(v2);
			verts.PushBack(v3);
			verts.PushBack(v4);
			return CreatePolygon(verts, stat);
		}



		bool C_Mesh::ValidateMesh()
		{
			ListItem<C_Vertex>* vertex;
			ListItem<C_Edge>* edge;
			ListItem<C_Polygon>* polygon;

			//Print mesh connections from indices at creation.

			//TODO: renumber indices to allow this function to be useful at any point.

			for (vertex = m_vertices.Begin(); vertex != 0; vertex = vertex->GetNext())
			{
				C_Vertex& vt = vertex->m_data;

				for (edge = vt.m_pEdges.Begin(); edge != 0; edge = edge->GetNext())
				{
					LOGDEBUG(AftkUt::g_Log, "Vertex %d is connected to edge %d\n", vt.m_index, edge->m_data.m_index);
				}
				
			}


			for (polygon = m_polygons.Begin(); polygon != 0; polygon = polygon->GetNext())
			{
				C_Polygon& pl = polygon->m_data; 
				for (edge = pl.m_pEdges.Begin(); edge != 0; edge = edge->GetNext())
				{
					C_Edge& ed = edge->m_data;
					Aftk::Position3 vt0, vt1;
					vt0 = ed.m_pVertices[0]->m_data.m_pos;
					vt1 = ed.m_pVertices[1]->m_data.m_pos;
					LOGDEBUG(AftkUt::g_Log, "Polygon %d is connected to edge %d, vt0=%f,%f,%f vt1=%f,%f,%f",
						pl.m_index, ed.m_index, vt0[0], vt0[1], vt0[2], vt1[0], vt1[1], vt1[2]);

					if ((edge->m_flags & LIST_ITEM_EDGE_REVERSED) != 0)
					{
						LOGDEBUG(AftkUt::g_Log, " reversed\n");		
					}
					else
					{
						LOGDEBUG(AftkUt::g_Log, "\n");
					}
				}
			}

			return true;
		}

		void C_Mesh::DeleteEdge(
			ListItem<C_Edge>* edgeItem,
			Aftk::C_Status& stat, 
			bool bDeleteVertices)
		{
			//remove from vertices and polys. This could completely spaz the mesh
			//it should only be called to actually delete an edge, not dissolving.

			C_Edge& edge = edgeItem->m_data;

			for (int i=0; i<2; i++)
			{
				C_Vertex& vertex = edge.m_pVertices[i]->m_data;
				vertex.m_pEdges.RemoveAll(&edge);
				
				if ((vertex.m_pEdges.Size() == 0) && (bDeleteVertices))
				{
					m_vertices.Remove(&vertex);
					delete &vertex;
				}
			}

			for (ListPoly* plItem = edge.m_pPolygons.Begin(); plItem; plItem = plItem->GetNext())
			{
				C_Polygon& pl = plItem->m_data;

				pl.m_pEdges.RemoveAll(&edge);
			}

			m_edges.Remove(&edge);
			delete &edge;
			
		}

		void C_Mesh::DeletePoly(
			C_Polygon* pl, 
			Aftk::C_Status& stat,
			bool bDeleteEdges)
		{
			//remove from edges
			
			VALIDATE_PTR_VOID(pl);
			
			for (ListEdge* polyEdgeItem = pl->m_pEdges.Begin(); polyEdgeItem; )
			{
				ListEdge* nextPolyEdge = polyEdgeItem;
				while (&nextPolyEdge->m_data == &polyEdgeItem->m_data)
				{
					nextPolyEdge = nextPolyEdge->GetNext();		
					if (nextPolyEdge == 0)
					{
						break;
					}			
				}
				C_Edge& polyEdge = polyEdgeItem->m_data;
				polyEdge.m_pPolygons.RemoveAll(pl);
				for (int i=0; i<2; i++)
				{
					//remove from polyvert list on vertices
					C_Vertex& vert = polyEdge.m_pVertices[i]->m_data;
					ListPolyVert* pv = vert.GetPolygonVertex(pl);
					if (pv)
					{
						delete &pv->m_data;
						vert.m_pPolygonVerts.Remove(pv);
					}
					
				}
				//make this delete unneeded edges as well
				if ((polyEdge.m_pPolygons.Size() == 0) && bDeleteEdges)
				{
					DeleteEdge(polyEdgeItem, stat);
				}
				polyEdgeItem = nextPolyEdge;
			}

			//remove from mesh

			m_polygons.Remove(pl->m_pMeshListItem);
			delete pl;
		}



		void C_Mesh::CollapseVertex(ListItem<C_Vertex>* vItem1, Aftk::C_Status& stat)
		{
			//remove a vertex, and its surrounding edges and polys to create new poly
		
			//we need an ordered list of all the vertices to make the new poly from
			//we can get the verts from each poly, and add them to a vector


			
		}

		void C_Mesh::RemoveDegeneratePolys(C_Polygon* deletedEdgePoly, Aftk::C_Status& stat)
		{			
			int nEdges = deletedEdgePoly->m_pEdges.Size();
			
			if (nEdges != 2)
			{
				return;
			}
			
			assert(nEdges == 2);
			
			//remove one edge, move polys to other.
			
			ListEdge* keepEdgeItem = deletedEdgePoly->m_pEdges.Begin();
			ListEdge* deleteEdgeItem = keepEdgeItem->GetNext();
			C_Edge* keepEdge = &keepEdgeItem->m_data;
			C_Edge* deleteEdge = &deleteEdgeItem->m_data;
			
			
			C_Vertex* vts[2];
			vts[0] = &keepEdge->m_pVertices[0]->m_data;
			vts[1] = &keepEdge->m_pVertices[1]->m_data;

			if (deleteEdge == keepEdge)
			{
				//printf("Dodgy edges\n");
				//need to deal with this specifically. Weird case.
				DeletePoly(deletedEdgePoly, stat);
				//DeleteEdge(deleteEdgeItem, stat);
				
				//stat = Aftk::C_Status::E_Status_Failure;
				return;
			}

			for (ListPoly* plItem = deleteEdgeItem->m_data.GetFirstPoly(); plItem; plItem = plItem->GetNext())
			{
				if (&plItem->m_data == deletedEdgePoly)
				{
					continue;
				}
				C_Polygon* pl = &plItem->m_data;
				ListEdge* edgeItem = pl->HasEdge(deleteEdgeItem);
				
				assert(edgeItem);
				
				ListEdge* newEdgeItem = pl->m_pEdges.AddAfter(keepEdge, &edgeItem->m_data);
				
				
 				newEdgeItem->m_flags = keepEdgeItem->m_flags;
				newEdgeItem->m_data.m_pPolygons.Add(pl);	
			
				pl->m_pEdges.Remove(edgeItem);
							
				
			}	
				
			for (int i=0; i<2; i++)
			{
				vts[i]->m_pEdges.Remove(deleteEdge);
			}
			

			keepEdge->m_pPolygons.Remove(deletedEdgePoly);
			m_polygons.Remove(deletedEdgePoly->m_pMeshListItem);
			m_edges.Remove(deleteEdge->m_pMeshListItem);
			
			delete deleteEdge;
			
			delete deletedEdgePoly;		
		


		}

		void C_Mesh::RemoveDegeneratePolys(Aftk::C_Status& stat)
		{
			//remove degenerate polys
			
			ListPoly* meshPolyItem = m_polygons.Begin();
			
			while (meshPolyItem)
			{
				ListPoly* tmp = meshPolyItem->GetNext();
				RemoveDegeneratePolys(&meshPolyItem->m_data, stat);
				meshPolyItem = tmp;
			}	
		}

		void C_Mesh::CollapseEdge(
			ListItem<C_Edge>* edgeItem,
			Aftk::C_Status& stat,
			int deleteIndex, 
			C_Edge* boundaryEdge)
		{
			C_Edge* edge = &edgeItem->m_data;
			int keepIndex;
			(deleteIndex==1)?(keepIndex=0):(keepIndex=1);
			
			if (m_polygons.Size() == 1)
			{
				stat = Aftk::C_Status::E_Status_Failure;
				return;
			}
			
			for (ListPoly* plItem = edge->GetFirstPoly(); plItem; plItem = plItem->GetNext())
			{
				C_Polygon* pl = &plItem->m_data;
				if (pl->m_pEdges.Size() < 3)
				{
					stat = Aftk::C_Status::E_Status_Failure;
					return;
				}
			}		
			
			ListVert* deleteVtItem = edge->m_pVertices[deleteIndex];
			ListVert* keepVtItem = edge->m_pVertices[keepIndex];
			C_Vertex* deleteVt = &edge->m_pVertices[deleteIndex]->m_data;
			C_Vertex* keepVt = &edge->m_pVertices[keepIndex]->m_data;
			
			for (ListEdge* vtEdgeItem = deleteVt->GetFirstEdge(); vtEdgeItem; vtEdgeItem = vtEdgeItem->GetNext())
			{
				C_Edge* vtEdge = &vtEdgeItem->m_data;
				if (&vtEdge->m_pVertices[0]->m_data == &vtEdge->m_pVertices[1]->m_data )
				{
					stat = Aftk::C_Status::E_Status_Failure;
					return;
				}
			}
			
			//remove an edge by collapsing it and deleting a vertex.
			
			//copy across polyverts
			//for each polyvertex on the deleteVt, check its not one of the edge polys, 
			//and then add the polyVertex to the keepVt
			
			int nEdgePolys = edge->m_pPolygons.Size();
			
			C_Polygon* edgePolys[2];
			edgePolys[0] = edgePolys[1] = &edge->GetFirstPoly()->m_data;
			C_PolygonVertex* originalPvItems[2];
			originalPvItems[0] = originalPvItems[1] = &keepVt->GetPolygonVertex(edgePolys[0])->m_data;

			if (nEdgePolys > 1)
			{
				edgePolys[1] = &edge->GetFirstPoly()->GetNext()->m_data;
				originalPvItems[1] = &keepVt->GetPolygonVertex(edgePolys[1])->m_data;
			}
			
			
			for (ListPolyVert* plVtItem = deleteVt->m_pPolygonVerts.Begin(); plVtItem; plVtItem = plVtItem->GetNext())
			{
				if (!edge->HasPoly(plVtItem->m_data.m_pl))
				{
					//set the texture coords to be from the correct pv on the keep vertex.
					//The keep vertex has 2 possible polyverts, we need the matching one. 
					
					C_PolygonVertex* pv = new C_PolygonVertex();
					pv->m_pl = plVtItem->m_data.m_pl;
					
					if (boundaryEdge) 
					{
						//there is some boundary in the UVs potentially, we need to copy the UVs from 
						//the correct polygon vertex 

						if (deleteVt->ArePolygonVerticesOnSameSide(edge, boundaryEdge, &plVtItem->m_data, originalPvItems[0]))
						{
							pv->Set(*originalPvItems[0]);
						}
						else if (deleteVt->ArePolygonVerticesOnSameSide(edge, boundaryEdge, &plVtItem->m_data, originalPvItems[1]))
						{
							pv->Set(*originalPvItems[1]);
						}
						else
						{
							//printf("Problem assigning correct UVs\n");
							pv->Set(*originalPvItems[0]);
						}
					
					}
					else
					{
						//all the UVs are the same, copy data from the first pv
						pv->Set(*originalPvItems[0]);
					}

					
					keepVt->m_pPolygonVerts.Add(pv);
				}
			}
			
			//for each poly, delete the edge 
			//for each edge on the deleted vertex, replace with other vertex
				
						
			for (ListPoly* plItem = edge->GetFirstPoly(); plItem; plItem = plItem->GetNext())
			{
				C_Polygon* pl = &plItem->m_data;
				pl->m_pEdges.Remove(edge);
				
			}
			
			
			//now for each edge around the deletevertex, replace the vertex with the keepvertex
			
			for (ListEdge* vtEdgeItem = deleteVt->GetFirstEdge(); vtEdgeItem; vtEdgeItem = vtEdgeItem->GetNext())
			{
				C_Edge* vtEdge = &vtEdgeItem->m_data;
				assert(&vtEdge->m_pVertices[0]->m_data != &vtEdge->m_pVertices[1]->m_data );
				for (int i=0; i<2; i++)
				{
					C_Vertex* vt = &vtEdge->m_pVertices[i]->m_data;
					
					if (vt == deleteVt)
					{
						vtEdge->m_pVertices[i] = keepVtItem;
						keepVt->m_pEdges.RemoveAll(vtEdge);
						assert(!keepVt->HasEdge(vtEdgeItem));
						keepVt->m_pEdges.Add(vtEdge);
					}
				}
			}		
			
			//remove the edge from the keepVertex
			
			keepVt->m_pEdges.RemoveAll(edge);
			
			//remove degenerate edges from the keepvertex
			
			//NEEDS TO WORK BETTER, I am creating the degenerate edge, but not removing it from all the vertices...
			
			for (ListEdge* vtEdgeItem = keepVt->GetFirstEdge(); vtEdgeItem; )
			{
				ListEdge* nextItem = vtEdgeItem->GetNext();
				if (nextItem)
				{
					while (&nextItem->m_data == &vtEdgeItem->m_data)
					{ 
						nextItem = nextItem->GetNext();
					}
				}
				C_Edge* vtEdge = &vtEdgeItem->m_data;
				C_Vertex* vt0 = &vtEdge->m_pVertices[0]->m_data;
				C_Vertex* vt1 = &vtEdge->m_pVertices[1]->m_data;
					
				if (vt0 == vt1)
				{
					//this edge has the same vertex at either end, remove
					vt0->m_pEdges.RemoveAll(vtEdge);
					for (ListPoly* plItem = vtEdge->GetFirstPoly(); plItem; plItem = plItem->GetNext())
					{
						plItem->m_data.m_pEdges.RemoveAll(vtEdge);
					}
					if (vtEdge != edge)
					{
						m_edges.RemoveAll(vtEdge);
						delete vtEdge;
					}

				}
				vtEdgeItem = nextItem;
			}			
			
			m_vertices.Remove(deleteVtItem);
			delete deleteVt;
			//m_edges.Remove(edge);
			delete edge;

			RemoveDegeneratePolys(stat);
			
		}
					
		void C_Mesh::CollapsePoly(ListItem<C_Polygon>* plItem, Aftk::C_Status& stat)
		{
			
		}

		ListItem<C_Vertex>* C_Mesh::SplitEdge(
            ListItem<C_Edge>* edgeItem, 
			float splitFactor,
			Aftk::C_Status& stat)
		{
			//This deals only with the structure splitting, not any spatial splitting, i.e. positions not updated
			//The vertex returned is the new vertex caused by the split.
			
			VALIDATE_PTR(edgeItem);

			C_Edge& edge = edgeItem->m_data;

			//find vertices on edge

			ListVert* oldEdgeVertItems[2];
			C_Vertex* oldEdgeVerts[2];
			
			for (int i=0; i<2; i++)
			{
				VALIDATE_PTR(edge.m_pVertices[i]);
				oldEdgeVertItems[i] = edge.m_pVertices[i];
				oldEdgeVerts[i] = &edge.m_pVertices[i]->m_data;
			}	

			//Disconnect old edge from vt[1] 
			
			oldEdgeVerts[1]->m_pEdges.Remove(&edge);

			//create new vertex

			C_Vertex* newVertex = new C_Vertex();
			VALIDATE_ALLOC_STAT(newVertex);

			//add new vertex to mesh
			ListVert* newVertexItem = CreateVertex(newVertex, stat);

			Aftk::Position3 pos1, pos2;

			pos1 = oldEdgeVerts[1]->m_pos;
			pos2 = oldEdgeVerts[0]->m_pos;
			
			newVertex->m_pos = Aftk::VectorLerp<Aftk::Position3, 3>(pos1,pos2, splitFactor);
			
			ListPoly* polygon;
			ListEdge* newEdgeItem;

			//add new edge to mesh

			C_Edge* newEdge = new C_Edge();

			newEdge->m_index = m_edges.Size();
			newEdgeItem = m_edges.Add(newEdge);
			newEdge->m_pMeshListItem = newEdgeItem;
			newEdgeItem->m_flags = edgeItem->m_flags;

			//add vertices to edges

			newEdge->m_pVertices[0] = newVertexItem;
			newEdge->m_pVertices[1] = oldEdgeVertItems[1];
			
			//add edges to vertices

			newVertex->m_pEdges.Add(&edge);
			newVertex->m_pEdges.Add(newEdge);

			oldEdgeVerts[1]->m_pEdges.Add(newEdge);

			edge.m_pVertices[1] = newVertexItem;

			for (polygon = edge.m_pPolygons.Begin(); polygon != 0; polygon = polygon->GetNext())
			{
				C_Polygon& pl = polygon->m_data;
				C_PolygonVertex* pv = new C_PolygonVertex();
				VALIDATE_ALLOC_STAT(pv);
				ListPolyVert* pvItem = newVertex->m_pPolygonVerts.Add(pv);
				pvItem->m_data.m_pl = &pl;

				for (ListEdge* plEdge = pl.m_pEdges.Begin(); plEdge != 0; plEdge = plEdge->GetNext())
				{
					C_Edge& ed = plEdge->m_data;
					if (&ed == &edge)
					{
						ListEdge* newPolyEdgeItem = new ListEdge(newEdge);
						newPolyEdgeItem->m_data.m_pPolygons.Add(&pl);

						ListPolyVert* pv1 = oldEdgeVerts[0]->GetPolygonVertex(&pl);
						ListPolyVert* pv2 = oldEdgeVerts[1]->GetPolygonVertex(&pl);

						if ((plEdge->m_flags & LIST_ITEM_EDGE_REVERSED) != 0)
						{
							newPolyEdgeItem->m_flags |= LIST_ITEM_EDGE_REVERSED;
							pl.m_pEdges.AddBefore(newPolyEdgeItem, plEdge);
							if (pv1 && pv2)
							{
								pv->Lerp(pv1->m_data, pv2->m_data, 1.0f-splitFactor);
							}
						}
						else
						{
							pl.m_pEdges.AddAfter(newPolyEdgeItem, plEdge);
							if (pv1 && pv2)
							{
								pv->Lerp(pv2->m_data, pv1->m_data, splitFactor);
							}
						}
					}
					
				}
				
			}

			return newVertexItem;
		}

		void C_Mesh::SplitPoly(
			ListItem<C_Polygon>* plItem, 
			ListItem<C_Edge>* edItem1, 
			ListItem<C_Edge>* edItem2, 
			float splitFactor1,
			float splitFactor2,
			Aftk::C_Status& stat, 
			ListPoly** pl0,
			ListPoly** pl1)
		{
			//split 2 edges, and store vertices

			ListVert* newVtItem1  = SplitEdge(edItem1, splitFactor1, stat);
			ListVert* newVtItem2  = SplitEdge(edItem2, splitFactor2, stat);

			if ((!newVtItem1) || (!newVtItem2))
			{
				stat = Aftk::C_Status::E_Status_Failure;
				return;
			}

			SplitPoly(plItem, newVtItem1, newVtItem2, stat, pl0, pl1);
		}

		ListEdge* C_Edge::AddToPoly(ListPoly* plItemNew, ListPoly* plItemOld)
		{
			C_Polygon& plNew = plItemNew->m_data;

			//Add polyverts

			for (int i=0; i<2; i++)
			{
				C_Vertex& vt = m_pVertices[i]->m_data;
				C_PolygonVertex* pvNew = new C_PolygonVertex();
				ListPolyVert* pvOldItem = vt.GetPolygonVertex(&plItemOld->m_data);
				pvNew->Set(pvOldItem->m_data);
				pvNew->m_pl = &plNew;
				vt.m_pPolygonVerts.Add(pvNew);
			}

			m_pPolygons.Add(&plNew);
			ListEdge* edgeItem = plNew.m_pEdges.Add(this);
			return edgeItem;
		}

		void C_Mesh::SplitPoly(
															ListItem<C_Polygon>* plItem, 
															ListItem<C_Vertex>* newVtItem1, 
															ListItem<C_Vertex>* newVtItem2, 
															Aftk::C_Status& stat, 
															ListPoly** pl0,
															ListPoly** pl1)
		{
			C_Polygon& pl = plItem->m_data;

			U32 blindData = pl.m_aftkBlindDataIndex;
			U32 materialData = pl.m_aftkMaterialIndex;

			U32 nVerts = pl.m_pEdges.Size();

			Ut::UtVector<Ut::DynamicVector<ListVert*> > origPolyVerts;
			pl.GetOrderedEdgeVerts(origPolyVerts);


			U32 newVtIndex[2];
			U32 nNewVerts = 0;


			for (U32 i=0; i<nVerts; i++)
			{
				if ((newVtItem1 == origPolyVerts[i]) ||
					(newVtItem2 == origPolyVerts[i]))
				{
					newVtIndex[nNewVerts++] = i;
				}
			}

			assert(nNewVerts==2);
			U32 newVertIndexDiff = newVtIndex[1] - newVtIndex[0];

			//The following check prevents the attempted creation of polys
			//by splitting down an existing edge, i.e. adjacent vertices 
			//around the poly
			if ((newVertIndexDiff < 2) || (newVertIndexDiff > (nVerts-2)))
			{
				return;
			}

			//add simple poly, between new polys
			C_Polygon* pNewPoly;
			Ut::UtVector<Ut::DynamicVector<ListItem<C_Vertex>*> > newVerts;
			for (U32 i=newVtIndex[0]; i<=newVtIndex[1]; i++)
			{
				newVerts.PushBack(origPolyVerts[i]);
			}
			
			ListItem<C_Polygon>* newPoly1 = CreatePolygon(newVerts,stat);
			if (newPoly1)
			{
				pNewPoly = &newPoly1->m_data;

				newPoly1->m_data.m_aftkBlindDataIndex = blindData;
				newPoly1->m_data.m_aftkMaterialIndex = materialData;
				newPoly1->m_data.m_index = m_polygons.Size();
				//polyvert info (uvs, normals etc)
				for (U32 i=newVtIndex[0]; i<=newVtIndex[1]; i++)
				{
					C_Vertex& vert = newVerts[i-newVtIndex[0]]->m_data;
					C_PolygonVertex& pv = vert.GetPolygonVertex(pNewPoly)->m_data;

					C_Vertex& otherVert = origPolyVerts[i]->m_data;
					C_PolygonVertex& otherPv = otherVert.GetPolygonVertex(&pl)->m_data;
					pv.Set(otherPv);
				}
			}
			newVerts.Clear();

			U32 nNewPolyVerts = 1+nVerts - newVertIndexDiff;

			for (U32 i=0; i<nNewPolyVerts; i++)
			{
				int index = (i+newVtIndex[1])%nVerts;
				newVerts.PushBack(origPolyVerts[index]);
			}
			ListItem<C_Polygon>* newPoly2 = CreatePolygon(newVerts,stat);
			if (newPoly2)
			{
				pNewPoly = &newPoly2->m_data;
				newPoly2->m_data.m_aftkBlindDataIndex = blindData;
				newPoly2->m_data.m_aftkMaterialIndex = materialData;
				newPoly2->m_data.m_index = m_polygons.Size();
				//polyvert info (uvs, normals etc)
				for (U32 i=0; i<nNewPolyVerts; i++)
				{
					int index = (i+newVtIndex[1])%nVerts;
					C_Vertex& vert = newVerts[i]->m_data;
					C_PolygonVertex& pv = vert.GetPolygonVertex(pNewPoly)->m_data;

					C_Vertex& otherVert = origPolyVerts[index]->m_data;
					C_PolygonVertex& otherPv = otherVert.GetPolygonVertex(&pl)->m_data;
					pv.Set(otherPv);
				}
			}
			DeletePoly(&pl, stat, false);

			if ((pl0 != 0) && (pl1 != 0))
			{
				*pl0 = newPoly1;
				*pl1 = newPoly2;
			}
		}

		//Mesh intersection members	

		void C_Mesh::CutByPlane(	const Aftk::Position3& planePos, const Aftk::Normal3& planeNormal)
		{
			//cut all polys in mesh

			Ut::UtVector<Ut::DynamicVector<ListPoly*> > polys;

			for (ListPoly* polyItem = m_polygons.Begin(); polyItem; polyItem = polyItem->GetNext())
			{
				polys.PushBack(polyItem);
			}
			
			CutByPlane(planePos, planeNormal, polys);	

		}

		void C_Mesh::CutByPlane(	const Aftk::Position3& planePos, const Aftk::Normal3& planeNormal, 
												Ut::UtVector<Ut::DynamicVector<ListPoly*> >& polys)
		{
			//cut all polys in list

			int nPolys = polys.Size();

			for (int i=0; i<nPolys; i++)
			{
				CutByPlane(planePos, planeNormal, polys[i]);
			}

		}

		void C_Mesh::CutByPlane(	const Aftk::Position3& planePos, const Aftk::Normal3& planeNormal, 
												ListPoly* polyItem, ListPoly** front, ListPoly** back)
		{
			//cut one poly

			C_Polygon& poly = polyItem->m_data;

			//firstly, do we intersect at all?
			//this is slightly conservative, so some poly intersections might
			//not resuly in a cut poly, as vertices and edge checks are more accurate. 

			if (!poly.IntersectsPlane(planePos, planeNormal))
			{
				return;
			}

			//if we intersect, it must be by two edges, we are assuming convex poly,

			ListEdge* edges[2];
			edges[0] = 0;
			edges[1] = 0;
			Aftk::Position3* vertPos[2][2];
			ListPoly* newPolys[2];
			newPolys[0] = newPolys[1] = 0;
			float splitFactor[2];
			int edgesSet = 0;

			for (ListEdge* edgeItem = poly.m_pEdges.Begin(); edgeItem; edgeItem = edgeItem->GetNext())
			{
				C_Edge& edge = edgeItem->m_data;
				float factor = 0.5;
				if (edge.IntersectsPlane(planePos, planeNormal, &factor))
				{
					if (edgesSet == 2)
					{
						return;
					}
					splitFactor[edgesSet] = factor;
					edges[edgesSet] = edgeItem;
					vertPos[edgesSet][0] = &edge.m_pVertices[0]->m_data.m_pos;
					vertPos[edgesSet][1] = &edge.m_pVertices[1]->m_data.m_pos;
					edgesSet++;
				}
			}

			//At this point we have our poly and some edges to split:
			Aftk::C_Status stat;

			if (edgesSet != 2)
			{
				//if this poly is intersected, but not at the edges, it must be the verts
				Ut::UtVector<Ut::DynamicVector<ListVert*> > ppVerts;
				poly.GetOrderedEdgeVerts(ppVerts);
				int nVerts = poly.m_pEdges.Size();
				assert(nVerts == ppVerts.Size());
				ListVert* vertsToSplit[2];
				//if one edge is intersected, split it to make a new vert
				bool bCreateNewEdge = false;
				if (edgesSet == 1)
				{
					vertsToSplit[0] = SplitEdge(edges[0], splitFactor[0], stat);
					bCreateNewEdge = true;
				}
				//then add the other intersecting verts
				for (int i=0; i<nVerts; i++)
				{
					if (edgesSet == 2)
					{
						break;
					}
					
					if (ppVerts[i]->m_data.IntersectsPlane(planePos, planeNormal))
					{
						vertsToSplit[edgesSet] = ppVerts[i];
						edgesSet++;
					}
				}
				//now we have the verts, split the poly using verts
				if (edgesSet == 2)
				{
					SplitPoly(polyItem, vertsToSplit[0], vertsToSplit[1], stat, &newPolys[0], &newPolys[1]);
				}
			}
			else
			{
				SplitPoly(polyItem, edges[0], edges[1], splitFactor[0], splitFactor[1],
					stat, &newPolys[0], &newPolys[1]);			
			}

			if (newPolys[0] && newPolys[1] && front && back)
			{
				C_Plane plane(planePos, planeNormal);

				if (newPolys[0]->m_data.IsCentroidInFrontOfPlane(plane))
				{
					*front = newPolys[0];
					*back = newPolys[1];
				}
				else
				{
					*front = newPolys[1];
					*back = newPolys[0];
				}
			}
		}

		int C_Mesh::RemoveCoPlanarPolys(bool bProcessBoundaryEdges, Aftk::C_Status& stat)
		{
			//First check any actor properties to see if we need to ignore this, or define 
			//non standard behaviour

			bool bProcess = true;

			bool status;
			C_NodeFlagParser parser(*m_pAftkMeshIn, &status);
			if (status)
			{
				if (parser.GetFlagGroupCategory() == "System")
				{
					if (parser.GetFlagGroupName() == "MeshReduction")
					{
						std::string setting;
						status = parser.GetParameterValue("ReductionSettings", setting);
						if (status)
						{
							int settingEnum = atoi(setting.c_str());

							if (settingEnum == 1)
							{
								bProcess = false;
							}
							else if (settingEnum == 2)
							{
								bProcessBoundaryEdges = false;
							}
						}
					}
				}
			}
			if (!bProcess)
			{
				return 0;
			}

			int ctr = 0;
			ListEdge* edgeItem;

			for (int iteration=0; iteration < 5; iteration++)
			{

			edgeItem = GetFirstEdge();
			
			
			while (edgeItem)
			{
				C_Edge* edge = &edgeItem->m_data;
				

				bool bCollapse = true;
				int deleteIndex = 1;
				
				//only attempt to collapse 2-manifold edges
				bCollapse = bCollapse && ((edge->m_pPolygons.Size() < 3) && (edge->m_pPolygons.Size() > 0));
				//Avoid any edges with the same vertex at both ends (WTF!)
				bCollapse = bCollapse && (&edge->m_pVertices[0]->m_data != &edge->m_pVertices[1]->m_data);				
			
				if (edge->WillCollapseAlterBoundary(deleteIndex))
				{
					//If collapsing this edge from one end is moving a boundary vertex (not allowed)
					//try the other one instead
					deleteIndex = 0;
					bCollapse = bCollapse && !edge->WillCollapseAlterBoundary(deleteIndex);
					//If bCollapse is still false, then both vertices cause a disallowed
					//move of a boundary vertex. 
				}	
				if (!bProcessBoundaryEdges)
				{
					//If the "Process boundary edges" flag is active, then ANY boundary vertex must 
					//be forbidden from a related edge collapse
					C_Vertex* vt = &edge->m_pVertices[deleteIndex]->m_data;
					if (vt->IsBoundaryVertex())
					{
						bCollapse = false;
					}
				}
				
				//Do not allow collapse which will result in huge edges (they cause problems)
				bCollapse = bCollapse && !edge->WillCollapseGenerateEdgeLongerThan(g_Tolerances.MaxEdgeLength(), deleteIndex);
				
				//Do not allow collapse which alters the poly normals
				bCollapse = bCollapse && !edge->WillCollapseAlterPolyNormals(deleteIndex);
				C_Edge* boundaryEdge = 0;
				bCollapse = bCollapse && edge->CanCollapseEdge(deleteIndex, &boundaryEdge);

				if (bCollapse)
				{
					Aftk::C_Status edgeStat;
					CollapseEdge(edgeItem, edgeStat, deleteIndex, boundaryEdge);
				}

				ListEdge* nextEdgeItem = edgeItem->GetNext();
				
				if (bCollapse)
				{
					m_edges.Remove(edgeItem);
				}
				edgeItem = nextEdgeItem;
				
			}
			}
			return ctr;
		
		}
		
		void C_Mesh::CreateUVIntegerOffsets(Aftk::C_Status& stat)
		{			
/*
			Algorithm:

			For each vertex, look at the UVs around it. Of those that differ from each other
			by an integer, set them all to the highest integer, noting the difference from the original
			value and adding this to the other UVs in the same poly as the polyvert

*/			

			//LOGDEBUG(g_Log, "Begin CreateUVIntegerOffsets\n");

			int nChanged = 0;
			for (int repeats=0; repeats<10; repeats++)
			{
				for (ListVert* vtItem = GetFirstVertex(); vtItem; vtItem = vtItem->GetNext())
				{
					C_Vertex* vt = &vtItem->m_data;
					vt->SetUVsToHighestSharedValue();
					nChanged++;
				}

				//LOGDEBUG(g_Log, "Iteration %d - %d uvs changed\n", repeats, nChanged);
			}
			//printf("Number of UVs edited: %d\n", nChanged);
			stat = Aftk::C_Status::E_Status_Success;
			//LOGDEBUG(g_Log, "End CreateUVIntegerOffsets\n");
		}

		ListItem<C_Edge>* C_Mesh::FlipEdge(ListItem<C_Edge>* edgeItem)
		{
			C_Edge* edge = &edgeItem->m_data;
			C_Polygon* polys[2];

			polys[0] = &edge->m_pPolygons.Begin()->m_data;
			polys[1] = &edge->m_pPolygons.Begin()->GetNext()->m_data;

			//We have 2 triangles on this edge, implying a quad. Flip this edge to use
			//the other 2 vertices, by rebuilding polys. Delete existing polys after 
			//getting vertex data

			Ut::UtVector<Ut::DynamicVector<ListVert*> > polyVerts[2];

			polys[0]->GetOrderedEdgeVerts(polyVerts[0]);
			polys[1]->GetOrderedEdgeVerts(polyVerts[1]);

			ListVert* triVerts[2][3];

			//Reorder polygon vertices, so vertex not on this edge is in the middle

			for (int tri=0; tri<2; tri++)
			{
				for (int i=0; i<3; i++)
				{
					if ((&polyVerts[tri][i]->m_data != &edge->m_pVertices[0]->m_data) &&
						(&polyVerts[tri][i]->m_data != &edge->m_pVertices[1]->m_data))
					{
						triVerts[tri][0] = polyVerts[tri][(i+2)%3];
						triVerts[tri][1] = polyVerts[tri][i];
						triVerts[tri][2] = polyVerts[tri][(i+1)%3];
					}
				}
			}

			//delete existing triangle,  delete existing edge, but store pv
			//info and material and blind data info to put on new polys and pvs.

			Aftk::C_Status stat;

			U32 matIndex[2], bdIndex[2];

			matIndex[0] = polys[0]->m_aftkMaterialIndex;
			bdIndex[0] = polys[0]->m_aftkBlindDataIndex;
			matIndex[1] = polys[1]->m_aftkMaterialIndex;
			bdIndex[1] = polys[1]->m_aftkBlindDataIndex;

			C_PolygonVertex pvs[4];

			pvs[0].Set(triVerts[0][1]->m_data.GetPolygonVertex(polys[0])->m_data);
			pvs[1].Set(triVerts[1][1]->m_data.GetPolygonVertex(polys[1])->m_data);
			pvs[2].Set(triVerts[0][0]->m_data.GetPolygonVertex(polys[0])->m_data);
			pvs[3].Set(triVerts[0][2]->m_data.GetPolygonVertex(polys[0])->m_data);

			DeletePoly(polys[0], stat, false);
			DeletePoly(polys[1], stat, false);
			DeleteEdge(edgeItem, stat, false);

			//rebuild triangles

			ListPoly* plNewItem1 = CreateTriangle(triVerts[0][1], triVerts[0][2], triVerts[1][1], stat);
			ListPoly* plNewItem2 = CreateTriangle(triVerts[1][1], triVerts[1][2], triVerts[0][1], stat);

			plNewItem1->m_data.m_aftkMaterialIndex = matIndex[0];
			plNewItem2->m_data.m_aftkMaterialIndex = matIndex[1];
			plNewItem1->m_data.m_aftkBlindDataIndex = bdIndex[0];
			plNewItem2->m_data.m_aftkBlindDataIndex = bdIndex[1];

			triVerts[0][1]->m_data.GetPolygonVertex(&plNewItem1->m_data)->m_data.Set(pvs[0]);
			triVerts[0][1]->m_data.GetPolygonVertex(&plNewItem2->m_data)->m_data.Set(pvs[0]);
			
			triVerts[1][1]->m_data.GetPolygonVertex(&plNewItem1->m_data)->m_data.Set(pvs[1]);
			triVerts[1][1]->m_data.GetPolygonVertex(&plNewItem2->m_data)->m_data.Set(pvs[1]);

			triVerts[0][0]->m_data.GetPolygonVertex(&plNewItem2->m_data)->m_data.Set(pvs[2]);
			triVerts[0][2]->m_data.GetPolygonVertex(&plNewItem1->m_data)->m_data.Set(pvs[3]);
			
			

			return 0;
		}

		void C_Mesh::ClipByPlaneList(const Ut::Vector<C_Plane>* planes, Ut::Vector<ListPoly*>* polys)
		{
			if (!planes)
			{
				LOGDEBUG(g_Log, "C_Mesh::ClipByPlaneList: No planes to clip by!");
				return;
			}

			Ut::Vector<ListPoly*> localPolys;

			const U32 nPlanes = planes->Size();
			GetPolysInsidePlaneList(planes, &localPolys);


			for (U32 i=0; i<nPlanes; i++)
			{
				int nPolys = localPolys.Size();
				for (S32 j=0; j<nPolys; j++)
				{
					ListPoly* front=0, *back=0;
					CutByPlane((*planes)[i].m_pos, (*planes)[i].m_norm, localPolys[j], &front, &back);
					if (front && back)
					{
						//remove the deleted (split) poly, and replace with front poly.
						//add back poly to end of list

						localPolys[j] = front;
						localPolys.PushBack(back);
					}
				}
				//localPolys.Clear();
			}

			if (polys)
			{
				//return the polys within the clip planes
				GetPolysInsidePlaneList(planes, polys, false);
			}
		}
		
#define RADIAL_MESH_USE_FAST_INTERSECTION_TEST

		void C_Mesh::GetPolysInsidePlaneList(	const Ut::Vector<C_Plane>* pPlanes, 
												Ut::Vector<ListPoly*>* pPolys, bool bAllowIntersect)
		{
			assert(pPlanes);
			assert(pPolys);

			const U32 nPlanes = pPlanes->Size();
			const Ut::Vector<C_Plane>& planes = *pPlanes;

#ifdef RADIAL_MESH_USE_FAST_INTERSECTION_TEST

			//prime vertices, by doing test against planes and caching in bitfield on vert
			for (ListVert* vtItem = m_vertices.Begin(); vtItem; vtItem=vtItem->GetNext())
			{
				vtItem->m_data.TestPlaneList(pPlanes);
			}

			Ut::Vector<U32> vtTestBits;
			Ut::Vector<U32> vtIntersectBits;
			Ut::Vector<Aftk::Position3> positions;

			//test polys using cached data
			for (ListPoly* plItem = m_polygons.Begin(); plItem; plItem=plItem->GetNext())
			{
				C_Polygon& pl = plItem->m_data;
				vtTestBits.Clear();
				vtIntersectBits.Clear();
				positions.Clear();

				bool bKeepPoly = true;

				for (ListEdge* edgeItem = pl.GetFirstEdge(); edgeItem; edgeItem = edgeItem->GetNext())
				{
					//return the first vertex on each edge, but allowing for reversed edges
					C_Vertex* vt;
					if (edgeItem->m_flags & LIST_ITEM_EDGE_REVERSED)
					{
						vt = &edgeItem->m_data.m_pVertices[1]->m_data;
					}
					else
					{
						vt = &edgeItem->m_data.m_pVertices[0]->m_data;
					}
					vtTestBits.PushBack(vt->m_planeTestBits);
					vtIntersectBits.PushBack(vt->m_planeIntersectBits);
					positions.PushBack(vt->m_pos);
				}
				//first check the planes against the vertices of the convex poly. 
				
				int nVerts = vtTestBits.Size();

				if (bAllowIntersect)
				{
					//test for verts being totally behind a plane
					for (U32 i=0; i<nPlanes; i++)
					{
						int nBehind = 0;
						for (int vtIndex=0; vtIndex<nVerts; vtIndex++)
						{
							if (((vtTestBits[vtIndex] & (1 << i)) == 0))
							{
								//this vertex is behind this plane
								nBehind++;
							}
						}
						if (nBehind == nVerts)
						{
							//poly totally behind plane, discard
							bKeepPoly = false;
							break;
						}
					}
					if (bKeepPoly)
					{
						//not many polys will get here, but those that do need a more accurate test
						//of whether they are within the convex hull
						
						if (!PolygonIntersectsConvexHull(positions, *pPlanes))
						{
							bKeepPoly = false;
						}
					}
				}
				else
				{
					for (U32 i=0; i<nPlanes; i++)
					{
						//test for verts being in front
						int nInFront = 0;
						for (int vtIndex=0; vtIndex<nVerts; vtIndex++)
						{
							if ((vtTestBits[vtIndex] & (1 << i)) ||
								(vtIntersectBits[vtIndex] & (1 << i)))
							{
								nInFront++;
							}
						}
						if (nInFront != nVerts)
						{
							//poly partly behind plane, discard
							bKeepPoly = false;
							break;
						}					
					}
				}
				
				if (bKeepPoly)
				{
					pPolys->PushBack(plItem);
				}
			}
#else		

			for (ListPoly* plItem = m_polygons.Begin(); plItem; plItem=plItem->GetNext())
			{
				bool bKeepPoly = true;

				//first check the planes against the vertices of the convex poly. 

				for (U32 i=0; i<nPlanes; i++)
				{
					C_Polygon& pl = plItem->m_data;
					if (bAllowIntersect)
					{
						//test for polys being totally behind, if so, discard
						C_Plane plane(planes[i].m_pos, -planes[i].m_norm);
						if (pl.IsInFrontOfPlane(plane))
						{
							bKeepPoly = false;
						}
					}
					else
					{
						C_Plane plane(planes[i].m_pos, planes[i].m_norm);
						if (!pl.IsInFrontOfPlane(plane))
						{
							bKeepPoly = false;
						}
					}
				}
				if (bKeepPoly)
				{
					pPolys->PushBack(plItem);
				}
			}
#endif
		}


	
//close namespaces
	}
}













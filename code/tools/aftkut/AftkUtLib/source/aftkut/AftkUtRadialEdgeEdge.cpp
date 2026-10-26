#include "stdafx.h"


namespace AftkUt
{
	namespace RadialEdgeMesh
	{
		bool C_Edge::HasPoly(const C_Polygon* pl) const
		{
			for (ListPoly* plItem = m_pPolygons.Begin(); plItem; plItem = plItem->GetNext())
			{
				if (pl == &plItem->m_data)
				{
					return true;
				}
			}
			return false;
		}

		void C_Edge::GetPolygons(Ut::UtVector<Ut::DynamicVector<ListPoly*> >& polys) const
		{
			polys.Clear();
			for (ListPoly* plItem = m_pPolygons.Begin(); plItem; plItem = plItem->GetNext())
			{
				polys.PushBack(plItem);
			}
		}

		//edge intersection members
		bool C_Edge::IntersectsPlane(
			const Aftk::Position3& planePos, 
			const Aftk::Normal3& planeNorm, 
			float* intersection) const 
		{
			//Do we intersect the plane? If so, and intersection float is valid, set the parameter of intersection

			Maths::HVec3 planePosVec = Aftk::TemplateToNightmare(planePos);
			Maths::HVec3 planeNormVec = Aftk::TemplateToNightmare(planeNorm);

			Maths::HVec3 vertex[2];
			vertex[0] = Aftk::TemplateToNightmare(m_pVertices[0]->m_data.m_pos);
			vertex[1] = Aftk::TemplateToNightmare(m_pVertices[1]->m_data.m_pos);

			Maths::HVec3 edgeVec = vertex[1] - vertex[0];
			Maths::HVec3 vertex0Vec = vertex[0] - planePosVec;

			float edgeDotN = edgeVec | planeNormVec;

			if (fabs(edgeDotN) < g_Tolerances.Epsilon())
			{
				//edge is parallel with plane
				return false;
			}

			float t = (-1.0f*(vertex0Vec | planeNormVec)) / edgeDotN;
			
			if ((t <= g_Tolerances.PosEpsilon()) || (t >= (1.0f-g_Tolerances.PosEpsilon())))
			{
				//parameter at interscetion lies outside of edge vertices, so no intersection
				return false;
			}

			if (intersection)
			{
				*intersection = t;
			}

			return true;
		}

		float C_Edge::GetLength() const
		{
			Maths::HVec3 vertex[2];
			vertex[0] = Aftk::TemplateToNightmare(m_pVertices[0]->m_data.m_pos);
			vertex[1] = Aftk::TemplateToNightmare(m_pVertices[1]->m_data.m_pos);
			
			Maths::HVec3 vec = vertex[1] - vertex[0];
			
			return vec.Length();	
		
		}
		
		bool C_Edge::PolysShareAttributes() const
		{
			if (!m_pPolygons.Begin())
			{
				return false;
			}
			C_Polygon* firstPl = &m_pPolygons.Begin()->m_data;
			S16 firstMaterialIndex = firstPl->m_aftkMaterialIndex;		
			S16 firstBlindDataIndex = firstPl->m_aftkBlindDataIndex;
			
			for (ListPoly* plItem = m_pPolygons.Begin(); plItem; plItem = plItem->GetNext())
			{
				C_Polygon* pl = &plItem->m_data;
				if ((pl->m_aftkMaterialIndex != firstMaterialIndex) ||
					(pl->m_aftkBlindDataIndex != firstBlindDataIndex))
				{
					return false;
				}
			}
			return true;
		}

		template <typename T>
		bool IsVecEqual(T& vec1, T& vec2)
		{
			T diff = vec2 - vec1;
			
			float length = diff.Length(); //find out if there is a squared version to avoid sqrt
			
			if (length < 0.0001f)
			{
				return true;
			}
			
			return false;
		}

		bool C_Edge::WillCollapseAffectMaterials(const int deleteIndex) const
		{
			//check that the delete vertex has no material variation
			
			int keepIndex;
			(deleteIndex==1)?(keepIndex=0):(keepIndex=1);
			int nPolys = m_pPolygons.Size();
			if (nPolys < 1)
			{
				return true;
			}
			ListVert* deleteVertItem = m_pVertices[deleteIndex];
			ListVert* keepVertItem = m_pVertices[keepIndex];
			C_Vertex* deleteVertex = &deleteVertItem->m_data;
			C_Vertex* keepVertex = &keepVertItem->m_data;
			
			C_Edge* materialBoundaryEdge =  deleteVertex->GetMaterialBoundaryEdge(this, &GetFirstPoly()->m_data, this);
			
			if (materialBoundaryEdge == this)
			{
				if (this->IsMaterialBoundaryEdge())
				{
					return true;
				}
				return false;
			}
			else
			{
				return true;
			}
			
		}

		bool C_Edge::CanCollapseEdge(const int deleteIndex, C_Edge** pBoundaryEdge) const
		{
			int keepIndex;
			(deleteIndex==1)?(keepIndex=0):(keepIndex=1);
			
			//if we have no polys, bail
				
			int nPolys = m_pPolygons.Size();
			if (nPolys < 1)
			{
				return false;
			}
			
			//Get pointers to the vertices and polys
			
			ListVert* deleteVertItem = m_pVertices[deleteIndex];
			ListVert* keepVertItem = m_pVertices[keepIndex];
			C_Vertex* deleteVertex = &deleteVertItem->m_data;
			C_Vertex* keepVertex = &keepVertItem->m_data;
			C_Polygon* pl[2];
			
			//do not attempt to collapse if vertex colours are different

			if (!(deleteVertex->EdgesPolygonVerticesShareColour() &&
				 (keepVertex->EdgesPolygonVerticesShareColour()) &&
				 (deleteVertex->PolygonVerticesShareColour()) &&
				 (keepVertex->PolygonVerticesShareColour()) ))
			{
				return false;
			}

			pl[0] = &m_pPolygons.Begin()->m_data;
			
			//if there's only one poly, we're on an edge

			C_Edge* boundaryEdge[2];
			boundaryEdge[0] = 0;
			boundaryEdge[1] = 0;
			C_Polygon* polygonBoundary = 0;
			if (nPolys < 2)
			{				
				if (!deleteVertex->GetBoundaryEdge(&boundaryEdge[0], &polygonBoundary, this, pl[0], this, true))
				{
					if (deleteVertex->EdgesPolygonVerticesShareTangentSpace())
					{
						return true;
					}
					
				}
				return false;
			}
			
			pl[1] = &m_pPolygons.Begin()->GetNext()->m_data;
			
			//now go round the vertex looking for a boundary
			//only collapse if  we're a boundary too
			//AND only if it forms a straight line
			
			polygonBoundary = 0;
					
			//do we find a boundary?
			if (deleteVertex->GetBoundaryEdge(&boundaryEdge[0], &polygonBoundary, this, pl[0], this, true))
			{
				if (boundaryEdge[0] != this)
				{
					if (deleteVertex->GetBoundaryEdge(&boundaryEdge[1], &polygonBoundary, boundaryEdge[0], 
														polygonBoundary, boundaryEdge[0], true))
					{
						if ((boundaryEdge[1] == this) && (IsEdgeCoLinear(boundaryEdge[0])) && 
							(this->PolygonVerticesShareTangentSpace(pl[0])) && 
							(this->PolygonVerticesShareTangentSpace(pl[1])))
						{
							(*pBoundaryEdge) = boundaryEdge[0];
							return true;
						}
						else
						{
							return false;
						}
					}
				}
			}
			//if not, do we get back to the beginning
			else if (boundaryEdge[0] == this)
			{
				if (deleteVertex->EdgesPolygonVerticesShareTangentSpace())
				{
					return true;
				}
			}
			//if not, an error occured, so bail
			return false;
		}

		bool C_Edge::IsEdgeCoLinear(const C_Edge* edge) const
		{
			C_Vertex* other;
			int sharedIndex, nonSharedIndex;
			
			if (edge->HasVertex(&m_pVertices[0]->m_data))
			{
				other = edge->GetOtherVertex(&m_pVertices[0]->m_data);
				sharedIndex = 0;
				nonSharedIndex = 1;
			}
			else if (edge->HasVertex(&m_pVertices[1]->m_data))
			{
				other = edge->GetOtherVertex(&m_pVertices[1]->m_data);
				sharedIndex = 1;
				nonSharedIndex = 0;
			}
			else
			{
				return false;
			}
			
			assert(other);
			
			Maths::HVec3 otherPos;
			otherPos = Aftk::TemplateToNightmare(other->m_pos);

			Maths::HVec3 sharedPos = Aftk::TemplateToNightmare(m_pVertices[sharedIndex]->m_data.m_pos);
			Maths::HVec3 nonSharedPos = Aftk::TemplateToNightmare(m_pVertices[nonSharedIndex]->m_data.m_pos);
		
			Maths::HVec3 vecThis, vecOther;
			
			vecThis = nonSharedPos - sharedPos;
			vecOther = sharedPos - otherPos;
			
			vecThis.Normalise();
			vecOther.Normalise();

			float dotVec = (vecThis | vecOther);
			
			if (dotVec > (1.0f-g_Tolerances.Epsilon()))
			{
				return true;
			}
			
			return false;				
		}

		bool C_Edge::WillCollapseAffectUVMapping(const int deleteIndex) const
		{
			int keepIndex;
			(deleteIndex==1)?(keepIndex=0):(keepIndex=1);
			int nPolys = m_pPolygons.Size();
			if (nPolys < 1)
			{
				return true;
			}
			ListVert* deleteVertItem = m_pVertices[deleteIndex];
			ListVert* keepVertItem = m_pVertices[keepIndex];
			C_Vertex* deleteVertex = &deleteVertItem->m_data;
			C_Vertex* keepVertex = &keepVertItem->m_data;
			C_Polygon* pl[2];
			pl[0] = &m_pPolygons.Begin()->m_data;
			C_PolygonVertex* keepPv[2];
			keepPv[0] = &keepVertex->GetPolygonVertex(pl[0])->m_data;
			
			bool bSuccess = true;

			bSuccess = bSuccess && deleteVertex->EdgesPolygonVerticesShareTangentSpace();
			bSuccess = bSuccess && keepVertex->EdgesPolygonVerticesShareTangentSpace();
			bSuccess = bSuccess && deleteVertex->PolygonVerticesShareTangentSpace();
			bSuccess = bSuccess && PolygonVerticesShareTangentSpace(pl[0]);
			bSuccess = bSuccess && deleteVertex->PolysShareAttributes();
			

			//special case 1 poly edge edges
			if (nPolys < 2)
			{				
				if (bSuccess)
				{
					return false;
				}
				return true;
			}
			
			pl[1] = &m_pPolygons.Begin()->GetNext()->m_data;
			keepPv[1] = &keepVertex->GetPolygonVertex(pl[1])->m_data;
		
			bSuccess = bSuccess && PolygonVerticesShareTangentSpace(pl[1]);
		
			//if success, then the vertex has no variation around it, and can be collapsed. 
			
			if (bSuccess)
			{
				return false;
			}			
			
			return true;
			
			C_Edge* materialBoundaryEdge =  deleteVertex->GetMaterialBoundaryEdge(this, &GetFirstPoly()->m_data, this);
			C_Edge* mappingBoundaryEdge =  deleteVertex->GetUVBoundaryEdge(this, &GetFirstPoly()->m_data, this);
			C_Edge* normalBoundaryEdge =  deleteVertex->GetUVBoundaryEdge(this, &GetFirstPoly()->m_data, this);
							
			C_Edge* materialBoundaryOtherEdge =  deleteVertex->GetMaterialBoundaryEdge(this, &GetFirstPoly()->GetNext()->m_data, this);
			C_Edge* mappingBoundaryOtherEdge =  deleteVertex->GetUVBoundaryEdge(this, &GetFirstPoly()->GetNext()->m_data, this);
			C_Edge* normalBoundaryOtherEdge =  deleteVertex->GetUVBoundaryEdge(this, &GetFirstPoly()->GetNext()->m_data, this);
				
			bool bNormalEdge = false;
			
			if ((normalBoundaryEdge == normalBoundaryOtherEdge) &&
				(normalBoundaryEdge != this))
			{
				bNormalEdge = true;
			}	
				
			if (!(materialBoundaryEdge && materialBoundaryEdge && materialBoundaryOtherEdge && mappingBoundaryOtherEdge))
			{
				return true;
			}
			
			bool bMatBoundary, bMapBoundary, bOtherMapBoundary, bOtherMatBoundary;
			
			bMatBoundary = this->IsMaterialBoundaryEdge();
			bMapBoundary = this->IsMappingBoundaryEdge();

			if ((materialBoundaryEdge == materialBoundaryOtherEdge) && 
				(materialBoundaryEdge != this))
			{
				bOtherMatBoundary=true;
			}
			else
			{
				bOtherMatBoundary=false;
			}
			
			if ((mappingBoundaryEdge == mappingBoundaryOtherEdge) && 
				(mappingBoundaryEdge != this))
			{
				bOtherMapBoundary=true;
			}
			else
			{
				bOtherMapBoundary=false;
			}
			
			C_Edge* boundaryEdge = 0;
			
			//if this edge is a boundary, check that the other edge boundary is along the same vector

			//allowable cases:
			
			//1. Both material and mapping boundary on both edges
			if (bMatBoundary && bMapBoundary && bOtherMatBoundary && bOtherMapBoundary &&
				(materialBoundaryEdge == mappingBoundaryEdge))
			{
				boundaryEdge = materialBoundaryOtherEdge;
			}
			//2. Only material boundaries on both edges
			else if((bMatBoundary && bOtherMatBoundary) && (!bMapBoundary) && (!bOtherMapBoundary))
			{
				//boundaryEdge = materialBoundaryOtherEdge;
			}
			//3. Only mapping boundaries on both edges
			else if((bMapBoundary && bOtherMapBoundary) && (!bMatBoundary) && (!bOtherMatBoundary))
			{
				//boundaryEdge = mappingBoundaryOtherEdge;
			}
	
			
			if (boundaryEdge)
			{
		
	//debug:move affected edges
			//m_pVertices[0]->m_data.m_pos[1] += 0.2f;
			//m_pVertices[1]->m_data.m_pos[1] += 0.2f;
				C_Vertex* other = boundaryEdge->GetOtherVertex(deleteVertex);
				assert(other);
			
				Maths::HVec3 otherPos;
				otherPos = Aftk::TemplateToNightmare(other->m_pos);

				Maths::HVec3 deletePos = Aftk::TemplateToNightmare(deleteVertex->m_pos);
				Maths::HVec3 keepPos = Aftk::TemplateToNightmare(keepVertex->m_pos);
		
				Maths::HVec3 vecThis, vecOther;
				
				vecThis = keepPos - deletePos;
				vecOther = deletePos - otherPos;

				vecThis.Normalise();
				vecOther.Normalise();
				
				float dotVec = (vecThis | vecOther);
				
				if (((dotVec > 0.99) && deleteVertex->EdgesPolygonVerticesShareTangentSpace())/* ||
					((dotVec > 0.99) && bNormalEdge)*/)
				{
					return false;
				}
				else
				{
					return true;
				}
			}
			return true;
		
		}

		bool C_Edge::VerticesShareAttributes(const int deleteIndex) const
		{		
			int keepIndex;
			(deleteIndex==1)?(keepIndex=0):(keepIndex=1);
			
			//Check vertex normals are the same. 
/*
			Maths::HVec3 norm[2];
			for (int i=0; i<2; i++)
			{
				norm[i] = Aftk::TemplateToNightmare(m_pVertices[i]->m_data.m_norm);
			}
			
			float normDot = norm[0] | norm[1];
			
			if (normDot < 0.99)
			{
				return false;
			}
*/			
			//check UV coords are same for the polyverts which will be merged
			
			for (int i=0; i<2; i++)
			{
				C_Vertex* vt = &m_pVertices[i]->m_data;
				C_PolygonVertex* pvFirst = &vt->GetFirstPolygonVertex()->m_data;
				int nUVsets = pvFirst->NumTextureCoordinatesSets();
				
				for (ListPolyVert* pvItem = vt->GetFirstPolygonVertex(); pvItem; pvItem = pvItem->GetNext())
				{
					C_PolygonVertex* pv = &pvItem->m_data;
					if (pv->NumTextureCoordinatesSets() != nUVsets)
					{
						return false;
					}
				}
				
				
				//this vertex
				for (int set =0; set < nUVsets; set++)
				{
					TextureCoordinates testTcUV = pvFirst->GetTextureCoordinates(set);
					Maths::HVec4  testuv = Aftk::TemplateToNightmare(testTcUV.m_uvwx);
					for (ListPolyVert* pvItem = vt->GetFirstPolygonVertex(); pvItem; pvItem = pvItem->GetNext())
					{
						C_PolygonVertex* pv = &pvItem->m_data;
						
						TextureCoordinates tcUV = pv->GetTextureCoordinates(set);
						Maths::HVec4 uv = Aftk::TemplateToNightmare(tcUV.m_uvwx);
						
						if (!IsVecEqual(uv, testuv))
						{
							return false;
						}
					}
				}

			}
					
			//Check the UV derivatives are the same on the 2 vertices and all the
			//vertices surrounding them
			
			//get the polyverts!
			
			Ut::UtVector<Ut::DynamicVector<C_PolygonVertex*> > pvs;

			C_Vertex* vt = &m_pVertices[deleteIndex]->m_data;
			
			
			//this vertex
			for (ListPolyVert* pvItem = vt->GetFirstPolygonVertex(); pvItem; pvItem = pvItem->GetNext())
			{
				pvs.PushBack(&pvItem->m_data);
			}
			//connected vertices, we only need the pvs which are on the polys of the delete vertex
			
			for (ListEdge* edItem = vt->GetFirstEdge(); edItem; edItem = edItem->GetNext())
			{
				C_Edge* ed = &edItem->m_data;
				if (ed == this)
				{
					continue;
				}	
				C_Vertex* otherVt = ed->GetOtherVertex(vt);
				
				for (ListPoly* plItem = ed->GetFirstPoly(); plItem; plItem = plItem->GetNext())
				{
					ListPolyVert* pvItem = otherVt->GetPolygonVertex(&plItem->m_data);
					if (pvItem)
					{
						pvs.PushBack(&pvItem->m_data);
					}
				}
			}
			
			
			int nPvs = pvs.Size();
			
			if (nPvs == 0)
			{
				return false;
			}
			
			//first check they all have the same number of UVsets, or this is all pointless (and could
			//go wrong, as we need to iterate by set first!)
			
			U32 nSets = pvs[0]->NumTextureCoordinatesSets();
			
			for (int i=0; i<nPvs; i++)
			{
				if (pvs[i]->NumTextureCoordinatesSets() != nSets)
				{
					return false;
				}
			}
			
			//so all the polyverts have the same number of sets, iterate by set, and check all the pvs 
			//have the same tangent vectors. Christ.
			for (U32 set = 0; set < nSets; set++)
			{
				//get the first pv's tangent vectors, and compare the rest against this
				
				C_PolygonVertex* pv = pvs[0];
				TextureCoordinates testTc = pv->GetTextureCoordinates(set);
				
				Maths::HVec3 testNormal = Aftk::TemplateToNightmare(pv->GetNormal());
				Maths::HVec3 testTangent = Aftk::TemplateToNightmare(testTc.m_tangents);
				Maths::HVec3 testBinormal = Aftk::TemplateToNightmare(testTc.m_binormals);
				
				for (int i=0; i<nPvs; i++)
				{
					pv = pvs[i];
					if (set == 0)
					{
						//only need to do normals for one set
						Maths::HVec3 normal = Aftk::TemplateToNightmare(pv->GetNormal());
						if (!IsVecEqual(normal, testNormal))
						{
							return false;
						}
					}
					
					TextureCoordinates tc = pv->GetTextureCoordinates(set);
					Maths::HVec3 tangent = Aftk::TemplateToNightmare(tc.m_tangents);
					Maths::HVec3 binormal = Aftk::TemplateToNightmare(tc.m_binormals);
					
					if (!IsVecEqual(tangent, testTangent))
					{
						return false;
					}
					
					if (!IsVecEqual(binormal, testBinormal))
					{
						return false;
					}
					
				}
			}
			
			
			return true;
		}

		bool C_Edge::HasBoundaryVertex() const
		{
			if (m_pVertices[0]->m_data.IsBoundaryVertex() || m_pVertices[1]->m_data.IsBoundaryVertex())
			{
				return true;
			}
			return false;
		}
		
		bool C_Edge::WillCollapseAlterPolyNormals(const int deleteIndex, const float angle) const
		{
			//look at each poly around the deletevertex, will removing this vertex
			//by collapsing the edge to the other vertex cause problems?
			
			float threshold = cos(DegToRad(angle));
			
			int keepIndex;
			(deleteIndex==1)?(keepIndex=0):(keepIndex=1);
						
			
			ListVert* deleteVertItem = m_pVertices[deleteIndex];
			ListVert* keepVertItem = m_pVertices[keepIndex];
			C_Vertex* deleteVertex = &deleteVertItem->m_data;
			C_Vertex* keepVertex = &keepVertItem->m_data;
			
			Maths::HVec3 keepPos, deletePos; 
			deletePos = Aftk::TemplateToNightmare(deleteVertex->m_pos);
		
			keepPos = Aftk::TemplateToNightmare(keepVertex->m_pos);
			
			for (ListEdge* edgeItem = deleteVertex->GetFirstEdge(); edgeItem; edgeItem = edgeItem->GetNext())
			{
				C_Edge* edge = &edgeItem->m_data;
			
				for (ListPoly* polyItem = edge->GetFirstPoly(); polyItem; polyItem = polyItem->GetNext())
				{
					C_Polygon* poly = &polyItem->m_data;
					Ut::UtVector<Ut::DynamicVector<ListVert*> > verts;
					poly->GetOrderedEdgeVerts(verts);
					int pv = -1;
					int nVerts = verts.Size();
					for (int i=0; i<nVerts; i++)
					{
						if (&verts[i]->m_data == deleteVertex)
						{
							pv = i;
							break;
						}
					}
					//TODO[IW]: Removing this assert, it means the mesh is in some way degenerate, but I don't
					//have time to look at it right now. Presumably this isn't causing any problems as asserts
					//have been disabled for weeks and everything was OK. I presume there is enough
					//error checking elsewhere in the structure to prevent any further problems.
					//assert(pv != -1);//FIXME etc. 

					Maths::HVec3 positions[2];
					
					int prevIndex = (pv+nVerts-1)%nVerts;
					int postIndex = (pv+1)%nVerts;
					
					positions[0] = Aftk::TemplateToNightmare(verts[prevIndex]->m_data.m_pos);
					positions[1] = Aftk::TemplateToNightmare(verts[postIndex]->m_data.m_pos);
					
					Maths::HVec3 vec1 = positions[0] - deletePos;
					Maths::HVec3 vec2 = positions[1] - deletePos;

					vec1.Normalise();
					vec2.Normalise();
					
					Maths::HVec3 normBefore = vec1 ^ vec2;
					
					float dotDirBefore = vec1 | vec2;
					
					vec1 = positions[0] - keepPos;
					vec2 = positions[1] - keepPos;
					
					if ((vec1.Length() == 0.f) || (vec2.Length() == 0.f))
					{
						continue;
					}

					vec1.Normalise();
					vec2.Normalise();
					
					Maths::HVec3 normAfter = vec1 ^ vec2;
					float dotDirAfter = vec1 | vec2;
					
					//do not allow creation of sliverous polys
					
					if ((fabs(dotDirAfter) > (1.0f-g_Tolerances.SliverousEpsilon())) ||
						(fabs(dotDirBefore) > (1.0f-g_Tolerances.SliverousEpsilon())))
					{
						return true;
					}
					
					normBefore.Normalise();
					normAfter.Normalise();

					float dotNorm = normBefore | normAfter;
					
					//do not allow normals of polys to change 
					//unless they're along a collapsing edge...
					
					if ((dotNorm <= threshold))
					{
						return true;
					}
				
				}
				
			}
			
			return false;
		}
		
		bool C_Edge::WillCollapseAlterBoundary(const int deleteIndex) const
		{
			//need the idea of finding the neighbouring boundary edges
			//from the deletevertex, then dot the current vectors of these edges
			//with the ones from the keepvertex once the deletevertex is removed.
			//If the vectors are suitably parallel, the collapse can go ahead without
			//affecting the boundary
			
			int keepIndex;
			(deleteIndex==1)?(keepIndex=0):(keepIndex=1);
			
			//get the other boundary edges on the delete vertex.
			//for each one, test the effect of deleting the vertex 
			
			C_Vertex* vts[2];
			vts[0] = &m_pVertices[0]->m_data;
			vts[1] = &m_pVertices[1]->m_data;

			for (ListEdge* edItem = vts[deleteIndex]->GetFirstEdge(); edItem; edItem = edItem->GetNext())
			{
				C_Edge* ed = &edItem->m_data;
				
				if ((ed == this) || (ed->m_pPolygons.Size() == 2))
				{
					continue;
				}

				if (ed->m_pPolygons.Size() != 1)
				{
					return true;
				}

				if (!IsEdgeCoLinear(ed))
				{
					return true;
				}

			
			}
			return false;

		}

		bool C_Edge::IsBoundaryEdge() const
		{
			//if both vertices are boundary vertices, then we are a boundary edge 
			
			if (m_pVertices[0]->m_data.IsBoundaryVertex() &&
				m_pVertices[1]->m_data.IsBoundaryVertex())
			{
				return true;
			}
			return false;
		}
		
		bool C_Edge::IsMaterialBoundaryEdge() const
		{
			assert(m_pPolygons.Size() == 2);
			C_Polygon* polys[2];
			
			polys[0] = &GetFirstPoly()->m_data;
			polys[1] = &GetFirstPoly()->GetNext()->m_data;
			
			if (polys[0]->m_aftkMaterialIndex == polys[1]->m_aftkMaterialIndex)
			{
				return false;
			}
			return true;
		}
		
		bool C_Edge::IsMappingBoundaryEdge() const
		{
			assert(m_pPolygons.Size() == 2);
			
			C_Polygon* polys[2];
			
			polys[0] = &GetFirstPoly()->m_data;
			polys[1] = &GetFirstPoly()->GetNext()->m_data;
		
			C_Vertex* vts[2];
			
			vts[0] = &m_pVertices[0]->m_data;
			vts[1] = &m_pVertices[1]->m_data;
		
			bool bSame = true;
		
			for (int i=0; i<2; i++)
			{
				//for each vert
				
				C_PolygonVertex* pvs[2];
				
				pvs[0] = &vts[i]->GetPolygonVertex(polys[0])->m_data;
				pvs[1] = &vts[i]->GetPolygonVertex(polys[1])->m_data;
				
				assert(pvs[0] && pvs[1]);
				
				if (!pvs[0]->HasSameTangentSpaceAs(*pvs[1], g_Tolerances.UVEpsilon()))
				{
					return true;
				}
				
			}
			return false;
		}
		
		bool C_Edge::PolygonVerticesShareTangentSpace(const C_Polygon* pl) const
		{
			C_PolygonVertex* pvs[2];
			pvs[0] = &m_pVertices[0]->m_data.GetPolygonVertex(pl)->m_data;
			pvs[1] = &m_pVertices[1]->m_data.GetPolygonVertex(pl)->m_data;
		
			if (pvs[0]->HasSameTangentSpaceAs(*pvs[1], g_Tolerances.UVEpsilon()))
			{
				return true;
			}
			return false;
		}
		
		bool C_Edge::PolygonVerticesShareColour(const C_Polygon* pl) const
		{
			C_PolygonVertex* pvs[2];
			pvs[0] = &m_pVertices[0]->m_data.GetPolygonVertex(pl)->m_data;
			pvs[1] = &m_pVertices[1]->m_data.GetPolygonVertex(pl)->m_data;
		
			if (pvs[0]->m_colour == pvs[1]->m_colour)
			{
				return true;
			}
			return false;
		}

		bool C_Edge::WillCollapseGenerateEdgeLongerThan(const F32 maxLength, const int deleteIndex) const
		{
			int keepIndex;
			(deleteIndex==1)?(keepIndex=0):(keepIndex=1);	
			
			ListVert* deleteVertItem = m_pVertices[deleteIndex];
			ListVert* keepVertItem = m_pVertices[keepIndex];
			C_Vertex* deleteVertex = &deleteVertItem->m_data;
			C_Vertex* keepVertex = &keepVertItem->m_data;

			Maths::HVec3 deletePos = Aftk::TemplateToNightmare(deleteVertex->m_pos);
			Maths::HVec3 keepPos = Aftk::TemplateToNightmare(keepVertex->m_pos);

			//go round all the edges on deleteVertex, and see if after moving to keep vertex
			//the edges exceed the maxLength

			for (ListEdge* edItem = deleteVertex->GetFirstEdge(); edItem; edItem = edItem->GetNext())
			{
				C_Edge* ed = &edItem->m_data;
				
				if (ed == this)
				{
					continue;
				}

				C_Vertex* vtOther = ed->GetOtherVertex(deleteVertex);
				Maths::HVec3 otherPos = Aftk::TemplateToNightmare(vtOther->m_pos);

				Maths::HVec3 edgeVec = otherPos - keepPos;

				float length = edgeVec.Length();

				if (length > maxLength)
				{
					return true;
				}

			}


			return false;
		}
		bool C_Edge::CanFlipEdge() const
		{
			//Get the 2 polys and check they're triangles

			if (m_pPolygons.Size() != 2)
			{
				return false;
			}

			C_Polygon* polys[2];

			polys[0] = &m_pPolygons.Begin()->m_data;
			polys[1] = &m_pPolygons.Begin()->GetNext()->m_data;

			if ((polys[0]->m_pEdges.Size() != 3) || (polys[1]->m_pEdges.Size() != 3))
			{
				return false;
			}	

			//check materials are the same, and that the polys are coplanar
			//and that the pv data is the same at the vertices on this edge,
			//as they will be merged

			if ((polys[0]->m_aftkMaterialIndex != polys[1]->m_aftkMaterialIndex) ||
				(polys[0]->m_aftkBlindDataIndex != polys[1]->m_aftkBlindDataIndex))
			{
				return false;
			}

			for (int i=0; i<2; i++)
			{
				//for each vertex on this edge, check both pvs are the same

				C_Vertex* vt = &m_pVertices[i]->m_data;

				if (!(vt->GetPolygonVertex(polys[0])->m_data == vt->GetPolygonVertex(polys[1])->m_data))
				{
					return false;
				}

			}

			Maths::HVec3 normal[2];

			polys[0]->GetPolyNormal(normal[0], &m_pVertices[0]->m_data);
			polys[1]->GetPolyNormal(normal[1], &m_pVertices[0]->m_data);

			normal[0].Normalise();
			normal[1].Normalise();

			float dot = normal[0] | normal[1];

			if (dot < (1.0f-g_Tolerances.Epsilon()))
			{
				return false;
			}

			return true;
		}

	}

}










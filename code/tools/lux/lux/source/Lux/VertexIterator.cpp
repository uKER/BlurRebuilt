#include "stdafx.h"
#include "Lux/VertexIterator.h"
#include "Lux/Sampler.h"
#include "Lux/ImageFilter.h"
#include "Lux/RenderChannel.h"

using namespace LUX;

VertexIterator::VertexIterator(Aftk::I_SceneMeshNode *p)
{
	m_pMesh = p;
	m_mWorldMatrix = m_pMesh->GetWorldMatrix();

	// reserve buffer to store samples
	int nTriangles = m_pMesh->GetNumPolygons(0);
	m_Buffer.Destroy();
	m_Buffer.Reserve(nTriangles);
	m_Buffer.Resize(nTriangles);

	memset(&m_buffer[0], 0, sizeof(VtxSample)*nTriangles);

	m_bSHMode = false;
}

void VertexIterator::SetSHMode(bool bOn)
{
	m_bSHMode = bOn;
}

// run the iterator
void VertexIterator::Go(t_mode mode, int nSamples, SurfaceIteratorCallbackPtr pCallback, void *pContext)
{
	float3 vPos, vNormal;

	if (mode == DiscontinuitySampling)
	{
		m_nSamples = nSamples * 3;
	}
	else
		m_nSamples = 1;

	// reset sample counters
	m_iOutVertex = 0;
	m_iOutPolygon = 0;
	m_iOutSample = 0;

	assert(m_iOutPolygon < m_pMesh->GetNumPolygons(0));

	int nPolygons = m_pMesh->GetNumPolygons(0);

	// for each poly
	for (int iPoly=0; iPoly<nPolygons; iPoly++)
	{
		// for each vertex
		for (int iVtx=0; iVtx<3; iVtx++)
		{
			int iPV = m_pMesh->GetPolygon(iPoly).GetPolygonVertexIndex(iVtx);

			// get position
			LUX::Position3 v = m_pMesh->GetEvaluatedPolygonVertexPosition(iPV);
			vPos = LPVS::ApplyMatrix(m_mWorldMatrix, v );

			// get normal
			LUX::Normal3 norm = m_pMesh->GetEvaluatedPolygonVertexNormal(iPV);
			vNormal = LPVS::ApplyMatrix(m_mWorldMatrix, norm);

			// for each sample
			for (int iSample=0; iSample<m_nSamples; iSample++)
			{
				(*pCallback)(pContext, vPos, vNormal, iSample);
			}
		}
	}
}

// save result to this vertex
void VertexIterator::AddSample(const LightSample &sample)
{
	float4 &vBuf = m_buffer[m_iOutPolygon].vSample[m_iOutVertex];
	float3 vColour = sample.vColour;

	if (!m_bSHMode)
	{
		// add the RGB of the sample to the vertex colour
		float4 v(vColour.x, vColour.y, vColour.z, 0);
		switch (sample.target)
		{
		default:
			vBuf = vBuf + v;
			break;
		case RenderChannel::Red:
			vBuf.x += vColour.y;
			break;
		case RenderChannel::Green:
			vBuf.y += vColour.y;
			break;
		case RenderChannel::Blue:
			vBuf.z += vColour.y;
			break;
		}
	}
	else
	{
		float tmp[3][ 4 ];

		// average the RGB colour into greyscale
		float fAvg = (vColour.x + vColour.y + vColour.z) / 3.0f;

		// encode the sample into a SH vector and add it to the accumulation buffer
		SHEvalDirectionalLight( 2, &sample.vDir.x,
			fAvg, fAvg, fAvg, tmp[0], tmp[1], tmp[2]);

		// add the result to the accumulation buffer
		vBuf.x += tmp[0][0], vBuf.y += tmp[0][1], vBuf.z += tmp[0][2], vBuf.w += tmp[0][3];
	}

	// go to the next sample
	if (++m_iOutSample>=m_nSamples)
	{
		m_iOutSample = 0;
		if (++m_iOutVertex>=3)
		{
			m_iOutVertex = 0;
			m_iOutPolygon++;
		}
	}
}

// figure out where to put the vertex colour data
void VertexIterator::CreateVertexDataChannel(Aftk::C_SceneMeshNode **ppMeshNode, int *piVertexColourChannel)
{

	Aftk::C_SceneMeshNode *pMesh;
	Aftk::C_SceneMeshRemappedInstanceNode *pInstance = NULL;
	std::string channelName = "luxVertexColour";

	if (m_pMesh->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		// this is a normal meshnode
		pMesh = (Aftk::C_SceneMeshNode*)m_pMesh;
	}
	else
	// if this node is an instance, store the vertex data on the source mesh
	if (m_pMesh->HasType( Aftk::E_ObjType_SceneMeshRemappedInstance ))
	{
		pInstance = (Aftk::C_SceneMeshRemappedInstanceNode*)m_pMesh;

		pMesh = pInstance->GetSourceNodePtr();

		// create a unique channel name using the instance's unique ID number
		std::ostringstream o;
		o << "luxVertexColour" << pInstance->GetUniqueId();
		channelName = o.str();

		// set blind data to associate this instance with the vertex colours set on it's source node
		Aftk::C_GenericAttribute vtxColAttr;
		vtxColAttr.SetName("luxVertexColour");
		vtxColAttr.SetValue(channelName);
		AddBlindData(pInstance, vtxColAttr);
	}
	else
	{
		// not sure what this is - can't write out vertex data
		assert(0);
		return;
	}

	// find channel for putting vertex colour info in
	bool found = false;
	S32 vertexColourChannel;
	for (U32 i = 0; i < pMesh->GetNumVertexDataChannels(); i++)
	{
		if (pMesh->GetVertexDataChannelInfo(i).GetName() == channelName)
		{
			if (pMesh->GetVertexDataChannelInfo(i).GetDataType() == Aftk::E_GenericAttributeType_U32)
			{
				// change channel info type - destroys previous contents
				Aftk::C_VertexDataChannelInfo info;

				info.SetName(channelName);
				info.SetDataType(Aftk::E_GenericAttributeType_Vector4F32);

				pMesh->SetVertexDataChannelInfo(i, info);
			}

			found = true;
			vertexColourChannel = i;
			break;
		}
	}

	// set vertex channel info
	if (!found)
	{
		Aftk::C_VertexDataChannelInfo info;

		info.SetName(channelName);
		info.SetDataType(Aftk::E_GenericAttributeType_Vector4F32);

		vertexColourChannel = pMesh->AddVertexDataChannelInfo(info);
	}

	// set output variables
	*ppMeshNode = pMesh;
	*piVertexColourChannel = vertexColourChannel;
}

// save result to the vertices
void VertexIterator::End(LUX::RenderChannel *pChannel)
{
	int nTriangles = m_pMesh->GetNumPolygons(0);

	// get mesh + channel to store data
	Aftk::C_SceneMeshNode *pDataMesh;
	int	vertexColourChannel;

	CreateVertexDataChannel(&pDataMesh, &vertexColourChannel);

	m_pDataMesh = pDataMesh;
	m_vertexColourChannel = vertexColourChannel;

	// read colour data from buffer and store in vertex colour channel
	for (int iTriangle=0; iTriangle<nTriangles; iTriangle++)
	{
		for (int iVtx=0; iVtx<3; iVtx++)
		{
			Aftk::C_PolygonVertex *pV = &pDataMesh->GetPolygon(iTriangle).GetPolygonVertex(iVtx);

			float fScale = 1.0f/(float)m_nSamples;

			// get colour from atlas
			float4 vAvgSample = m_buffer[iTriangle].vSample[iVtx] * fScale;
			Aftk::Vector4F32 vCol;

			if (!m_bSHMode)
			{
				// if the sample is 0 don't do anything
				if (vAvgSample.x + vAvgSample.y + vAvgSample.z <= 0)
					continue;

				// write colour to vertex
				pV->GetData(vCol, vertexColourChannel);

				if (pChannel->m_target == RenderChannel::RGB)
				{
					vCol.x += vAvgSample.x;
					vCol.y += vAvgSample.y;
					vCol.z += vAvgSample.z;
				}
				else
				{
					float fAverage = (vAvgSample.x + vAvgSample.y + vAvgSample.z)/3;
					switch (pChannel->m_target)
					{
					case RenderChannel::Red:
						vCol.x += fAverage;
						break;
					case RenderChannel::Green:
						vCol.y += fAverage;
						break;
					case RenderChannel::Blue:
						vCol.z += fAverage;
						break;
					}
				}
			}
			else
			{
				// write colour to vertex
				vCol.x = vAvgSample.x;
				vCol.y = vAvgSample.y*0.5f + 0.5f;
				vCol.z = vAvgSample.z*0.5f + 0.5f;
				vCol.w = vAvgSample.w*0.5f + 0.5f;
			}

			pV->SetData(vertexColourChannel,vCol);
		}
	}

	memset(&m_buffer[0], 0, sizeof(VtxSample)*nTriangles);
}

// apply brightness and gamma correction to an object's vertex colours
void VertexIterator::ApplyBrightnessAndGammaToVertexColours(float fBrightness, float fGamma)
{
	int nTriangles = m_pDataMesh->GetNumPolygons(0);

	float fInvGamma = 1.0f / fGamma;

	// read colour data from buffer and store in vertex colour channel
	for (int iTriangle=0; iTriangle<nTriangles; iTriangle++)
	{
		for (int iVtx=0; iVtx<3; iVtx++)
		{
			Aftk::C_PolygonVertex *pV = &m_pDataMesh->GetPolygon(iTriangle).GetPolygonVertex(iVtx);

			// write colour to vertex
			Aftk::Vector4F32 vCol;
			pV->GetData(vCol, m_vertexColourChannel);

			vCol = vCol * fBrightness;
			vCol.w = 1.0f;

			if (!m_bSHMode)
			{
				vCol.x = powf(vCol.x , fInvGamma);
				vCol.y = powf(vCol.y , fInvGamma);
				vCol.z = powf(vCol.z , fInvGamma);
			}

			pV->SetData(m_vertexColourChannel,vCol);
		}
	}
}

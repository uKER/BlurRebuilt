#include "stdafx.h"

#include "Lux/RenderChannel.h"
#include "Lux/SurfaceIterator.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/SamplePoints.h"
#include "Lux/IIntegrator.h"
#include "CServ/ClusterServHeaders.h"
#include "Lux/I_JobRenderer.h"
#include "Lux/SamplePointIterator.h"
#include "Lux/Common.h"
#include "lux_common/SamplePoint.h"
#include "lux_common/LpvsUtility.h"
#include "lux_common/StringManipulation.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

// read the sample point info from the XML file
void SamplePoints::ReadXMLNode(DOMNode *pNode)
{
    if (!pNode) return;
    if (pNode->getNodeType() != DOMNode::ELEMENT_NODE) return;

	char *pElementName = XMLString::transcode(pNode->getNodeName());
	if (stricmp(pElementName, "samplepoints") == 0 && pNode->hasAttributes())
	{
		// get all the attributes of the node
		DOMNamedNodeMap *pAttributes = pNode->getAttributes();
		int nSize = pAttributes->getLength();

		for(int i = 0; i < nSize; ++i) 
		{
			DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(i);
			char *pName = XMLString::transcode(pAttributeNode->getName());
			char *pValue = XMLString::transcode(pAttributeNode->getValue());

			// get filename
			if (stricmp(pName, "filename") == 0)
			{
				char* pValue = XMLString::transcode(pAttributeNode->getValue());
				m_filename = pValue;
			}
			// open a binary file containing sample points
			else if (stricmp(pName, "inputfilename") == 0)
			{
				FILE *f = fopen(pValue, "rb");
				if (f)
				{
					while (1)
					{
						// read some data
						float3 vPos;
						size_t iBytesRead = fread(&vPos, sizeof(float3), 1, f);
						if (iBytesRead < 1)
							break;

						// add this position to the point list
						m_SamplePositions.PushBack(vPos);
					}
				}
			}
			// get target type
			else if (stricmp(pName, "type") == 0)
			{
				if (stricmp(pValue, "SH") == 0)
					m_type = SphericalHarmonics;
			}

			// get output type (binary or text)
			else if (stricmp(pName,"output") == 0)
			{
				if (stricmp(pValue,"binary") == 0)
					m_bBinaryOutput = true;
			}

			XMLString::release(&pValue);
			XMLString::release(&pName);
		}
		
		// iterate through children - these should be the sampling points
        for (DOMNode *child = pNode->getFirstChild(); child != 0; child = child->getNextSibling())
		{
			char *pElementName = XMLString::transcode(child->getNodeName());

		    if (child->getNodeType() != DOMNode::ELEMENT_NODE)
			{
				XMLString::release(&pElementName);
				continue;
			}

			// we don't care about the node name
			// get all the attributes of the node
			DOMNamedNodeMap *pAttributes = child->getAttributes();
			int nSize = pAttributes->getLength();

			// if there is an attribute, assume that it's the position
			if (nSize)
			{
				DOMAttr *pAttributeNode = (DOMAttr*) pAttributes->item(0);
				char *pName = XMLString::transcode(pAttributeNode->getName());

				// get position
				Position3 position;
				char* pValue = XMLString::transcode(pAttributeNode->getValue());
				StringUtility::StringToVector(pValue, position);
				m_SamplePositions.PushBack(position);
				XMLString::release(&pValue);
				XMLString::release(&pName);
			}

			XMLString::release(&pElementName);
		}
	}
    XMLString::release(&pElementName);

	// TODO: put this back in
/*
	if (m_SamplePositions.Size())
		gpSamplePoints = &m_SamplePositions[0];
	else
		gpSamplePoints = NULL;
	gnSamplePoints = m_SamplePositions.Size();
*/
}

// constructor
SamplePoints::SamplePoints()
{
	m_bBinaryOutput = false;
	m_nScalarsPerSample = -1;
	m_fInvGamma = 1.f;
	m_fBrightness = 1.f;
}

void SamplePoints::SetGamma(float fGamma)
{
	m_fInvGamma = 1.f/fGamma;
}

void SamplePoints::SetBrightness(float fBrightness)
{
	m_fBrightness = fBrightness;
}

// write out a vector of floats to a file (in text format)
void SamplePoints::WriteVector(FILE *f, float *pVec, int c)
{
	for (int i=0; i<c; i++)
	{
		float x = pVec[i];

		fprintf(f, "%f", x);
		if (i!=c-1)
			fprintf(f, " ");
		else
			fprintf(f, "\n");
	}
}

const int SamplePoints::m_iBatchSize = 100;

// add jobs 
void SamplePoints::AddJobs(JobScheduler *pScheduler)
{
	// add jobs of 100 samplepoints
	int nTotalBatches = (m_SamplePositions.Size()-1)/m_iBatchSize;
	nTotalBatches++;
	for (int i=0; i<nTotalBatches; i++)
		pScheduler->AddJob(i | SAMPLEPOINT_JOB, m_iBatchSize); // cost equals the number of sample points in the batch
}

// generate a job, given a job ID
BaseObject *SamplePoints::GenerateJob(int iID)
{
	SamplePointJob *pJob = new SamplePointJob();
	iID = iID & JOB_INDEX_MASK;

	int iFirstSample = iID * m_iBatchSize;
	int nTotalSamples = m_SamplePositions.Size();

	// add the samplepoints to the job
	pJob->m_SampleList.Reserve(100);
	for (int i=iFirstSample; i<nTotalSamples && i<iFirstSample+100; i++)
	{
		pJob->m_SampleList.PushBack( m_SamplePositions[i] );
	}
	pJob->m_iSamplePointIdx = iFirstSample;
	pJob->m_fInvGamma = m_fInvGamma;
	pJob->m_fBrightness = m_fBrightness;
	pJob->m_iRenderMode = m_iRenderMode;
	pJob->m_bBouncePass = m_bBouncePass;
	pJob->m_iLightingPassBits = m_iLightingPassBits;

	return pJob;
}

// set current rendermode and bounce pass info
void SamplePoints::SetRenderMode(int iMode, bool bBounce, Ut::BitArray128 iLightingPassBits)
{
	m_iRenderMode = iMode;
	m_bBouncePass = bBounce;
	m_iLightingPassBits = iLightingPassBits;
}

// store job result
void SamplePoints::StoreJobResult(BaseObject *pInResult)
{
	if (strcmp(pInResult->getPersistenceID(), "SPointResult"))
		return;
	SamplePointResult *pResult = (SamplePointResult*) pInResult;

	if (pResult->m_nSamples == 0)
		return;

	// calculate scalars per sample and allocate the correct result buffer size
	if (m_nScalarsPerSample == -1)
	{
		m_nScalarsPerSample = pResult->m_SampleResults.Size() / pResult->m_nSamples;
		m_SampleResults.Resize(m_SamplePositions.Size() * m_nScalarsPerSample);
	}

	// get index of the first sample point that this job result is for
	int iLast = pResult->m_iSamplePointIdx + 100;
	if (m_SamplePositions.Size() < iLast)
		iLast = m_SamplePositions.Size();

	// copy the results into our sample array
	memcpy(	&m_SampleResults[pResult->m_iSamplePointIdx * m_nScalarsPerSample],
			&pResult->m_SampleResults[0],
			sizeof(float) * pResult->m_SampleResults.Size());
}

// this is the function called by the main illumation function to save out the sample points
bool SamplePoints::SaveSamplePoints(float fBrightness, float fGamma, bool bPrescaleSH, bool bSaveExtraData)
{
	bool nFoundQNAN = 0;

	// apply brightness and pre-scaling
	if (m_nScalarsPerSample == 14)
	{
		float fInvGamma = 1.0f/fGamma;
		for (int i=0; i<m_SampleResults.Size(); i+=m_nScalarsPerSample)
		{
			float4 &vr = (float4&)m_SampleResults[i];
			float4 &vg = (float4&)m_SampleResults[i+4];
			float4 &vb = (float4&)m_SampleResults[i+8];

			vr = vr * fBrightness;
			vg = vg * fBrightness;
			vb = vb * fBrightness;

			if (bPrescaleSH)
			{	
				vr = PreScaleSHVector(vr);
				vg = PreScaleSHVector(vg);
				vb = PreScaleSHVector(vb);
			}
		}
	}

	// save sample point data to file
	if (m_filename.length() == 0)
		return false;

	// copy samplepoints into array for lpvsapp renderer to display
	if (gbDisplaySamplePoints)
	{
		int n = m_SamplePositions.Size();
		gSamplePoints.Resize(n);
		int iResult = 0;
		for (int i=0; i<n; i++, iResult+=m_nScalarsPerSample)
		{
			gSamplePoints[i].m_vPos = m_SamplePositions[i];
			for (int j=0; j<4; j++)
			{
				gSamplePoints[i].m_fSH_R[j] = m_SampleResults[iResult+j+0];
				gSamplePoints[i].m_fSH_G[j] = m_SampleResults[iResult+j+4];
				gSamplePoints[i].m_fSH_B[j] = m_SampleResults[iResult+j+8];
			}
		}
	}

	// save samples
	if (m_bBinaryOutput)
	{
		// write out binary file containing data
		FILE *f = fopen(m_filename.c_str(), "wb");

		if (!f)
			return false;

		// write number of samples
		int c = m_SampleResults.Size() / m_nScalarsPerSample;
		fwrite(&c, sizeof(c), 1, f);

		// write the sample data itself
		if (m_SampleResults.Size() > 0)
		{
			fwrite(&m_SampleResults[0], m_SampleResults.Size(), sizeof(float), f);
		}

		for (int i=0; i<m_SampleResults.Size(); i++)
		{
			if (CheckBuggeredFloat(m_SampleResults[i]))
				nFoundQNAN++;
		}
		
		fclose(f);
	}
	else
	{
		// print text format
		FILE *f = fopen(m_filename.c_str(), "w");

		if (!f)
			return false;

		if (m_nScalarsPerSample == 14)
		{
			// print in old format
			int iNode = 0;
			for (int i=0; i<m_SampleResults.Size(); i+=m_nScalarsPerSample, iNode++)
			{
				fprintf(f,"%5d\n",iNode);
				fprintf(f,"R: ");	WriteVector(f, &m_SampleResults[i+0], 4);
				fprintf(f,"G: ");	WriteVector(f, &m_SampleResults[i+4], 4);
				fprintf(f,"B: ");	WriteVector(f, &m_SampleResults[i+8], 4);
				if (bSaveExtraData)
				{
					float4 v(m_SampleResults[i+12], m_SampleResults[i+13], 0, 0);
					fprintf(f,"E: ");	WriteVector(f, &v.x, 4);
				}

				for (int j=0; j<m_nScalarsPerSample; j++)
				{
					if (CheckBuggeredFloat(m_SampleResults[i+j]))
						nFoundQNAN++;
				}
			}
		}
		else
		{
			// print in new generic format
			int iNode = 0;
			for (int i=0; i<m_SampleResults.Size(); i+=m_nScalarsPerSample, iNode++)
			{
				fprintf(f,"%5d\n",iNode);
				WriteVector(f, &m_SampleResults[i], m_nScalarsPerSample);

				for (int j=0; j<m_nScalarsPerSample; j++)
				{
					if (CheckBuggeredFloat(m_SampleResults[i+j]))
						nFoundQNAN++;
				}
			}
		}

		fclose(f);
	}

	if (nFoundQNAN)
	{
		LOGDEBUG( g_Log, "Found %d QNANs in samplepoint output data.\n", nFoundQNAN);
	}

	return true;
}


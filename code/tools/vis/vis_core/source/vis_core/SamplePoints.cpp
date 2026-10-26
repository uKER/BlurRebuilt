#include "stdafx.h"

#include "vis_core/SamplePoints.h"
#include "vis_core/SampleArea.h"

SampleAreaList::~SampleAreaList()
{

}

// read binary input file
void SampleAreaList::ReadBinaryInputFile(const std::string &filename)
{
	FILE *f = fopen(filename.c_str(), "rb");
	if (!f)
	{
		LOGINFO( g_Log, "Couldn't open binary input file '%s'.\n",filename.c_str() );
		return;
	}

	// repeat until the end of the file
	while (!feof(f))
	{
		// read number of points for this cell
		int nPoints;
		if (fread(&nPoints, sizeof(int), 1, f) == 0)
		{
			fclose(f);
			break;
		}

		if (nPoints == 0)
		{
			fclose(f);
			ReadNewFileFormat(filename);
			break;
		}

		// read the sample points
		SampleList *p = new SampleList();

		for (int i=0; i<nPoints; i++)
		{
			float3 vPos;
			fread(&vPos, sizeof(float3), 1, f);

			PvsPoint pt;
			pt.vPos = vPos;
			pt.iType = PVS_TYPE_POSITION;
			p->m_vPoints.PushBack(pt);
			pt.vPos.y += 2.0f;
			p->m_vPoints.PushBack(pt);
		}

		p->m_nSamples = p->m_vPoints.Size() * 4;

		m_Areas.PushBack(p);
	}

	fclose(f);
}

void SampleAreaList::ReadNewFileFormat(const std::string &filename)
{
	std::fstream str(filename.c_str(), std::ios::binary|std::ios::in);
	ost::Engine ar(str, ost::Engine::modeRead);

	int iDummy, iVersion, n;

	ar >> iDummy >> iVersion >> n;
	m_Areas.Resize(n);
	for (int i=0; i<n; i++)
	{
		m_Areas[i] = new SampleList;
		m_Areas[i]->read(ar);
	}
}

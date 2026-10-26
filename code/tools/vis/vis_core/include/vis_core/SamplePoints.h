#pragma once

#include "vis_core/SampleArea.h"

typedef Ut::Vector<SampleArea*> TSampleAreaVector;

class SampleAreaList
{
public:
						~SampleAreaList();

	void				ReadBinaryInputFile(const std::string &filename);
	void				ReadNewFileFormat(const std::string &filename);

	float3				m_vDefaultCellDimensions;

	TSampleAreaVector	m_Areas;
	std::string			m_filename;
};

#ifndef __CHART_SEAMS_H__
#define __CHART_SEAMS_H__

/// store a possible chart seam between these to triangles on a particular meshnode
struct ChartSeam
{
	int iTriangle1;
	int iTriangle2;
	int iEdge1;			// shared edge on triangle 1
};

/// contains a list of all the seams on a set of charts on a particular meshnode
/// we have one of these structs for each group of charts that have common edges with potential seams
struct ChartSeamGroup
{
	Ut::Vector<ChartSeam> seams;
	int iChart;				// index of the first chart in this group
	int	iLightingPass;		// index of the lighting pass that this group is in
};

/// contains groups of charts that need to be processed together because they have common edges with potential seams
typedef Ut::Vector<ChartSeamGroup> TSeamGroupList;

#endif

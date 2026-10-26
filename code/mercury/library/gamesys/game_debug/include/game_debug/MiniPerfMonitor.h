#ifndef __MINIPERFMONITOR_H__
#define __MINIPERFMONITOR_H__

namespace GameDebug
{

struct TimingStats;

/// display a small performance monitor graph on the screen
class C_MiniPerfMonitor
{
public:
	/// constructor
	C_MiniPerfMonitor();

	/// initialise
	void Initialise();

	/// free
	void Free();
	
	/// render the graph
	void Render();

	/// set limits for each subsystem
	void SetLimits(float fGPU, float fCPU, float fPhysics, float fUpdate, float fRender);

private:
	TimingStats *m_pStats;

	int m_iMaxReset;
	float m_fMax[5];

	float m_fLimit[5];
	float m_fAvg[5];
	float m_fLastAvg[5];
	float m_fFPSAvg;
	float m_fFPSLastAvg;

	int m_iAvgCount;
};

}; // namespace GameDebug

#endif

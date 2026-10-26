#ifndef __LUX_CORE_H__
#define __LUX_CORE_H__

namespace LPVS
{
	class MultiProgressNotification;
}

namespace Progress
{
	class I_ProgressBar;
};

namespace LUX
{
	class Core
	{
	public:
						Core();

		// functions
		bool			ReadScene(LPVS::MultiProgressNotification* pProgress, Progress::I_ProgressBar *p_Progress);
		bool			LightScene(Progress::I_ProgressBar *pProgress);
		bool			WriteScene(Progress::I_ProgressBar *pProgress);

		virtual void	CloseScene();
		virtual void	Destroy();

		virtual void	Reset();

		const Lux*		GetLightingSystemPtr() const;
		Lux*			GetLightingSystemPtr();

		// member variables
		Aftk::C_Scene				m_aftk;
		LUX::SpatialHierarchy*	m_pSpatialHierarchy;

		Lux						m_lux;

		//Pointer to a function that provides a logging facility.
		void (*m_pLogCallbackFunc)(const std::string& text);

	private:
		U32 GetScenePolyCount() const;
	};

	extern Debug::Logger &g_Log;
};

#endif

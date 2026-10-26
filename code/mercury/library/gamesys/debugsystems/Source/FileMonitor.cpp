#include "DebugSystemsPch.h"
#include <system/time/simpletimer.h>
#include <system/io/async.h>
#include <Display/Render/BasicRender.h>
#include <Display/Render/FontRender.h>

#include <DebugSystems/FileMonitor.h>

namespace DebugSystems
{

MT::Thread* g_pBenchmarkThread = NULL;
bool g_stopBenchmark = false;

Debug::Int	g_debugFileMonitorBenchmarkHWThread("GameSys.FileMonitor.Benchmark.HWThread", 1, Debug::Persist());
Debug::String	g_debugFileMonitorBenchmarkFile("GameSys.FileMonitor.Benchmark.File", "cache/boot.pak", Debug::Persist());
Debug::Int	g_debugFileMonitorBenchmarkBufferSize("GameSys.FileMonitor.Benchmark.OpenParamsBufferSize", 128*1024, Debug::Persist());
Debug::Int	g_debugFileMonitorBenchmarkBufferCount("GameSys.FileMonitor.Benchmark.OpenParamsBufferCount", 0, Debug::Persist());
Debug::Int	g_debugFileMonitorBenchmarkReadSize("GameSys.FileMonitor.Benchmark.ReadSize", 524288, Debug::Persist());
Debug::Int	g_debugFileMonitorBenchmarkIterations("GameSys.FileMonitor.Benchmark.Iterations", 0, Debug::Persist());

class C_BenchmarkThread : public MT::Thread
{
public:
	C_BenchmarkThread() : 
		MT::Thread()
	{
	}

	virtual ~C_BenchmarkThread()
	{
	}

	virtual void ThreadMain()
	{
		g_stopBenchmark = false;
		const char* filename = (const char*)g_debugFileMonitorBenchmarkFile;

		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(
			filename, 
			Sys::E_FileAccess_Read, 
			Sys::C_OpenParams(
				g_debugFileMonitorBenchmarkBufferSize,
				false,
				true,
				g_debugFileMonitorBenchmarkBufferCount));
		if (pFile)
		{
			U8* pBuf = new U8[g_debugFileMonitorBenchmarkReadSize];

			U32 count = 0;
			
			Time::C_SimpleTimer totalTime;
			U64 bytesRead = 0;
			while(true)
			{
				if (g_stopBenchmark)
					break;

				if (pFile->GetPosition() >= pFile->Length())
				{
					OutputDebugString("Restarting benchmark file\n");
					SAFE_DELETE_PTR(pFile);
					pFile = Sys::GetDefaultFileSystem()->Open(
						filename, 
						Sys::E_FileAccess_Read, 
						Sys::C_OpenParams(
							g_debugFileMonitorBenchmarkBufferSize,
							false,
							true,
							g_debugFileMonitorBenchmarkBufferCount));
					if (!pFile)
					{
						OutputDebugString("Missing file, aborting benchmark");
						return;
					}
				}

				U32 readSize = g_debugFileMonitorBenchmarkReadSize;
				if ( pFile->GetPosition() + readSize > pFile->Length())
					readSize = (U32)(pFile->Length() -  pFile->GetPosition());

				{
					Time::C_SimpleTimer simpleTimer;
					if (pFile->Read(pBuf, readSize))
					{
						bytesRead += readSize;
						F32 sizeMb = (F32)readSize / (1024.f*1024.f);
						F32 deltaSecs = simpleTimer.Stop() / 1000.f;
						char txt[2048];
						sprintf(txt, "%u %s read in %.2f secs @ %.2fmb/s\n", count, filename, deltaSecs, sizeMb / deltaSecs);
						OutputDebugString(txt);
					}
				}

				++count;

				if (g_debugFileMonitorBenchmarkIterations == 0)
					continue;

				if (count == g_debugFileMonitorBenchmarkIterations)
					break;
			}


			char txt[2048];
			F32 sizeMb = (F32)((F64)bytesRead / (1024.0*1024.0));
			F32 deltaSecs = totalTime.Stop() / 1000.f;
			sprintf(txt, "Average %s read in %.2f secs @ %.2fmb/s\n", filename, deltaSecs, sizeMb / deltaSecs);
			OutputDebugString(txt);
			SAFE_DELETE_PTR(pFile);
		}
	}

private:
};

static void ToggleBenchmark(void* unused)
{
	if (g_pBenchmarkThread)
	{
		OutputDebugString("Benchmark stopping...\n");
		g_stopBenchmark = true;
		if (!g_pBenchmarkThread->IsRunning())
		{
			g_pBenchmarkThread->Join();
			SAFE_DELETE_PTR(g_pBenchmarkThread);
		}
	}
	else
	{
		OutputDebugString("Benchmark starting...\n");
		g_pBenchmarkThread = new C_BenchmarkThread();
		g_pBenchmarkThread->Start();
	}
}

static void StopBenchmark(void* unused)
{
	g_stopBenchmark = true;
}


Debug::Bool	g_debugFileMonitorEnable("GameSys.FileMonitor.Enable", false, Debug::Persist());
Debug::Bool	g_debugFileMonitorCacheStats("GameSys.FileMonitor.CacheStats", false, Debug::Persist());
Debug::Function g_debugFileMonitorToggleBenchmark("GameSys.FileMonitor.Benchmark.Toggle", ToggleBenchmark, 0);
Debug::Function g_debugFileMonitorStopBenchmark("GameSys.FileMonitor.Benchmark.Stop", StopBenchmark, 0);


Debug::Float	g_debugFileMonitorPosX("GameSys.FileMonitor.PosX",		30.f, Debug::Range(0, 1024));
Debug::Float	g_debugFileMonitorPosY("GameSys.FileMonitor.PosY",		30.f, Debug::Range(0, 768));
Debug::Float	g_debugFileMonitorSize("GameSys.FileMonitor.Size",		14.f, Debug::Range(0, 50));
Debug::Int	g_debugFileMonitorAlpha("GameSys.FileMonitor.Alpha",		200, Debug::Range(0, 255));


C_FileMonitor::C_FileMonitor()
{
	m_pCacheStats = NULL;
}

C_FileMonitor::~C_FileMonitor()
{
	SAFE_DELETE_PTR(m_pCacheStats);
}

void C_FileMonitor::Initialise()
{
}

void C_FileMonitor::Render()
{
	if (!g_debugFileMonitorEnable)
		return;

	Sys::C_FsPerfLogger::Instance().GetFsReports(m_reports);

	Sys::C_GlobalFsStats globalFsStats;
	Sys::C_FsPerfLogger::Instance().GetGlobalFsStats(globalFsStats);

	F32 posx = (F32)g_debugFileMonitorPosX;
	F32 posy = (F32)g_debugFileMonitorPosY;
	F32 s = (F32)g_debugFileMonitorSize;
	F32 totalWidth = 800;

	F32 x = posx;
	F32 y = posy;

	U32 nRows = 4;

	F32 totalHeight = 0;
	for (S32 i = 0; i < m_reports.Size(); ++i)
	{
		Sys::C_FsReport r = m_reports[i];
		nRows += r.stats.openFiles.Size() + 1;
	};

	totalHeight = nRows*(s+5.0f);

	Display::BasicRender()->DrawColoured2DRect(
		posx, posy-s*0.5f, totalWidth, totalHeight, Maths::RGBA(0, 0, 0, g_debugFileMonitorAlpha));

	x = x+s*3.0f;

	//
	Maths::RGBA cread(200, 220, 220);
	Maths::RGBA cwrite(0, 220, 0);
	Maths::RGBA cappend(200, 0, 0);

	//Read, write, append colours (green, red, yellow)
	Maths::RGBA colours[] = { Maths::RGBA(0, 220, 0), Maths::RGBA(220, 220, 0), Maths::RGBA(220, 220, 0) };

	const F64 k1Mb = 1024.f*1024.f;

	//Cache stats
	
	{
		Btl::String str;
		str.Printf("Peak open files %i", globalFsStats.peakOpenFiles);
		Display::FontRender()->Draw(str.Chars(), x, y, s, s, 
			Maths::RGBA(200, 220, 220), Display::E_FontRenderAlign_Lt);
		y += s+5.0f;
	}
	{
		Btl::String str;
		str.Printf("Open files %i", globalFsStats.numOpenFiles);
		Display::FontRender()->Draw(str.Chars(), x, y, s, s, 
			Maths::RGBA(200, 220, 220), Display::E_FontRenderAlign_Lt);
		y += s+5.0f;
	}
	
	{
		Btl::String str;
		str.Printf("Physical read %.2fMb write %.2fMb", 
			F32((F64)globalFsStats.readBytesPhysical / k1Mb),
			F32((F64)globalFsStats.writeBytesPhysical / k1Mb));

		Display::FontRender()->Draw(str.Chars(), x, y, s, s, 
			Maths::RGBA(200, 220, 220), Display::E_FontRenderAlign_Lt);
		y += s+5.0f;
	}
	{
		Btl::String str;
		str.Printf("Virtual read %.2fMb write %.2fMb",
			F32((F64)globalFsStats.readBytesVirtual / k1Mb),
			F32((F64)globalFsStats.writeBytesVirtual / k1Mb));

		Display::FontRender()->Draw(str.Chars(), x, y, s, s, 
			Maths::RGBA(200, 220, 220), Display::E_FontRenderAlign_Lt);
		y += s+5.0f;
	}


	for (S32 i = 0; i < m_reports.Size(); ++i)
	{
		Sys::C_FsReport r = m_reports[i];
		F32 fsReadMb = F32((F64)r.stats.readBytes / k1Mb);
		F32 fsWriteMb = F32((F64)r.stats.writeBytes / k1Mb);

		Btl::String str;
		str.Printf("%s open %u read %.2fMb write %.2fMb", 
			r.fsName.Chars(), r.stats.openFiles.Size(), fsReadMb, fsWriteMb);
		Display::FontRender()->Draw(str.Chars(), x, y, s, s, 
			Maths::RGBA(200, 220, 220), Display::E_FontRenderAlign_Lt);
		y += s+5.0f;

		Btl::Vector<Sys::C_FsFileInfo> fileReports;
		Sys::C_FsPerfLogger::Instance().GetFileReports(r.fsName, fileReports);
		for (S32 j = 0; j < fileReports.Size(); ++j)
		{
			const Sys::C_FsFileInfo& info = fileReports[j];
			F32 readMb = F32((F64)info.readBytes / k1Mb);
			F32 writeMb = F32((F64)info.writeBytes / k1Mb);

			str.Printf("  %s read %.2fMb write %.2fMb", info.fileName.Chars(), readMb, writeMb);
			Display::FontRender()->Draw(str.Chars(), x, y, s, s, 
				colours[info.access-1], Display::E_FontRenderAlign_Lt); //access starts from 1, hence the subtraction
			y += s+5.0f;
		}
	}
}

}

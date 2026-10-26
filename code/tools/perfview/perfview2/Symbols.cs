using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.IO;

namespace ProfilingApplication
{
	struct SymbolInfo
	{
		public string	m_sSymbolName;
		public string	m_sFilename;
		public int		m_iLineNumber;
//		public string	m_sModuleName;
	};

	// lookup symbols. ideally this should work on PS3, Xbox and PC. fun fun fun
	class Symbols
	{
		int m_iPlatform;
		bool m_bSuccesfullyinitialised;

		// for PS3 - run addr2line in a separate process and feed it symbols to look up
		System.Diagnostics.Process m_Addr2LineProc;

		~Symbols()
		{
			if (m_Addr2LineProc != null)
			{
				if (m_bSuccesfullyinitialised)
				{
					if (!m_Addr2LineProc.HasExited)
						m_Addr2LineProc.Kill();
					m_Addr2LineProc.Close();
				}
				m_Addr2LineProc = null;
			}
		}

		public void Initialise(int iPlatform, List<ModuleInfo> modules)
		{
			m_iPlatform = iPlatform;
			m_bSuccesfullyinitialised = false;

			if (iPlatform == 0)
			{
				if (!DbgHelp.bInitialised)
				{
					DbgHelp.Initialise();
					DbgHelp.bInitialised = true;
				}

				// initialise for Xbox
				for (int i=0; i<modules.Count; i++)
				{
					ModuleInfo info = modules[i];
					DbgHelp.LoadSymbolsForModule((uint)info.iBaseAddress, (uint)info.cSize, info.signature, info.signature.Length);
				}

				m_bSuccesfullyinitialised = true;
			}
			else if (iPlatform == 1)
			{
				// get CELL path
				string sCellRoot = System.Environment.GetEnvironmentVariable("SCE_PS3_ROOT");
				if (sCellRoot == null)
				{
					MessageBox.Show("Error loading symbols:\nSCE_PS3_ROOT environment variable not set.", "Symbol Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
					return;
				}

				// initialise to PS3 - all paths are hardcoded for now
				m_Addr2LineProc = new System.Diagnostics.Process();
				m_Addr2LineProc.EnableRaisingEvents = false;
				m_Addr2LineProc.StartInfo.UseShellExecute = false;
				m_Addr2LineProc.StartInfo.RedirectStandardInput = true;
				m_Addr2LineProc.StartInfo.RedirectStandardOutput = true;
				m_Addr2LineProc.StartInfo.CreateNoWindow = true;
				m_Addr2LineProc.StartInfo.RedirectStandardError = true;
				m_Addr2LineProc.StartInfo.FileName = sCellRoot + "\\host-win32\\ppu\\bin\\ppu-lv2-addr2line.exe";
				if (!File.Exists(m_Addr2LineProc.StartInfo.FileName))
				{
					MessageBox.Show("Eror loading symbols:\n" + m_Addr2LineProc.StartInfo.FileName + " not found.", "Symbol Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
					return;
				}

				m_Addr2LineProc.StartInfo.Arguments = "-f -C -e z:\\devroot_ps3\\code\\ravin\\bin\\ShooterGame_ps3test\\Release\\ShooterGame_ps3.ppu.elf";
				m_bSuccesfullyinitialised = m_Addr2LineProc.Start();
			}
		}

		public SymbolInfo LookupSymbol(int iAddr)
		{
			SymbolInfo info;
			info.m_sSymbolName = "??";
			info.m_sFilename = "??";
			info.m_iLineNumber = -1;

			if (!m_bSuccesfullyinitialised)
				return info;

			if (m_iPlatform == 0)
			{
				// XBOX
				uint rva = (uint)iAddr;

				StringBuilder sbSymbolName = new StringBuilder(1024);
				StringBuilder sbFileName = new StringBuilder(1024);
				int fileLineNumber = 0;

				bool result = DbgHelp.GetName(rva, sbSymbolName, sbSymbolName.Capacity - 1);
				int result2 = DbgHelp.GetFilenamePosition(rva, sbFileName, sbFileName.Capacity - 1, ref fileLineNumber);

				string name = sbSymbolName.ToString();
				if (name.CompareTo("<unknown>") == 0)
					info.m_sSymbolName = String.Format("0x{0:x}", rva);
				else
					info.m_sSymbolName = name;

				info.m_sFilename = sbFileName.ToString();
				info.m_iLineNumber = fileLineNumber;
			}
			else if (m_iPlatform == 1)
			{
				// PS3: send symbol to look up to addr2line
				m_Addr2LineProc.StandardInput.WriteLine(iAddr.ToString("X"));
				m_Addr2LineProc.StandardInput.Flush();
				// read function name
				string s = m_Addr2LineProc.StandardOutput.ReadLine();
				if (s != null)
				{
					info.m_sSymbolName = s;
					s = m_Addr2LineProc.StandardOutput.ReadLine();
					if (s != null)
					{
						int idx = s.LastIndexOf(':');
						if (idx > 0 && idx<s.Length-1)
						{
							info.m_sFilename = s.Substring(0,idx);
							info.m_iLineNumber = int.Parse(s.Substring(idx+1));
						}
					}
				}
			}
			return info;
		}
	}

public class DbgHelp
{
	static public bool bInitialised = false;

    [DllImport("AddrToNameDLL.dll")]
    public static extern int Initialise();
    
    [DllImport("AddrToNameDLL.dll")]
    public static extern void Shutdown();

    [DllImport("AddrToNameDLL.dll")]
    public static extern void Reset();

    [DllImport("AddrToNameDLL.dll")]
    public static extern bool LoadSymbolsForModule(UInt32 baseAddress, UInt32 size, byte[] buffer, Int32 bufferSize);

    [DllImport("AddrToNameDLL.dll")]
    public static extern void End();

    [DllImport("AddrToNameDLL.dll")]
    public static extern bool GetName(uint addr, StringBuilder buf, int nMaxCount);

    [DllImport("AddrToNameDLL.dll")]
    public static extern int GetFilenamePosition(uint addr, StringBuilder buf, int nMaxCount, ref int linePos);
}

}

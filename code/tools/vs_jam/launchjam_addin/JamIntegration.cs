using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.VisualStudio.Shell;
using System.IO;
using System.ComponentModel.Design;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using Microsoft.VisualStudio.Shell.Interop;
using EnvDTE;
using Microsoft.VisualStudio;
using Microsoft.Win32;
using Microsoft.VisualStudio.OLE.Interop;
using System.Windows.Forms;
using Microsoft.VisualStudio.VCProjectEngine;

#pragma warning disable 0168

namespace BizarreCreations.JamIntegration
{
	class JamIntegration
	{
		JamProperties m_Properties = new JamProperties();

		class IBCmdLine
		{
			public string sCmdLine;
			public string sApp;
		};

		Dictionary<string,IBCmdLine> m_IBCommandLineMap = new Dictionary<string,IBCmdLine>();

		public JamIntegration(MenuCommandsPackage parent)
		{
			m_Parent = parent;
			m_Properties.ReadFromRegistry();
			m_Properties.SetEnvironmentItem();

			ReadIBConfigFile();
		}

		string m_sJAMExePath = "jam.exe";

		void ReadIBConfigFile()
		{
			char[] ctrim = { '\n', '\r', '\t' } ;
			char[] ctrim2 = { '\"', ';', ' ' } ;

			string s = ReadTextFile(Environment.GetEnvironmentVariable("BIZARRE_ROOT") + "\\tools\\code_tools\\build_tools\\jambase\\ib_cmdlines.txt");
			// pull out the individual lines
			string[] lines = s.Split('\n');
			int n = lines.Length;
			// remove trailing CRLF
			for (int i=0; i<n; i++)
				lines[i] = lines[i].TrimEnd(ctrim);

			foreach(string line in lines)
			{
				string[] _s = line.Split('\t');

				if (_s.Length == 3)
				{
					IBCmdLine tmp = new IBCmdLine();
					tmp.sCmdLine = _s[2];
					tmp.sApp = _s[1];

					m_IBCommandLineMap[_s[0]] = tmp;
				}
			}
		}

		public string ReadTextFile(string sPath)
		{
			FileStream fpTemplate = File.Open(sPath,
				System.IO.FileMode.Open, System.IO.FileAccess.Read, System.IO.FileShare.Read);

			StreamReader textReader = new StreamReader(fpTemplate);
			string sTemplate = textReader.ReadToEnd();
			fpTemplate.Close();
			return sTemplate;
		}

		MenuCommandsPackage m_Parent;

		// handle 'build selection' command
		public void BuildSelection(bool bRebuild)
		{
			EnvDTE.DTE vs = (EnvDTE.DTE) m_Parent._GetService(typeof(EnvDTE.DTE));

			ProjectItem item = vs.SelectedItems.Item(1).ProjectItem;
			if (item == null)
				return;
			string sFilename = item.get_FileNames(0);

			if (!bRebuild)
				CompileFile(sFilename, "");
			else
				CompileFile(sFilename, "-a");
		}

		public static void SaveTextFile(string sPath, string sText)
		{
			FileStream fpVcproj = File.Open(sPath,
				System.IO.FileMode.Create, System.IO.FileAccess.Write, System.IO.FileShare.Write);
			StreamWriter textWrite = new StreamWriter(fpVcproj);
			textWrite.Write(sText);
			textWrite.Close();
			fpVcproj.Close();
		}

		// handle 'build selection' command
		public void CheckoutOrAdd()
		{
			EnvDTE.DTE vs = (EnvDTE.DTE) m_Parent._GetService(typeof(EnvDTE.DTE));

			string sFilename = vs.ActiveDocument.FullName;
			string sTmp = System.Environment.GetEnvironmentVariable("TEMP") + "\\vsp4.bat";

			SaveTextFile(sTmp, 
				"P4 add " + sFilename + "\n" +
				"P4 edit " + sFilename + "\n");

			LaunchJAM("/Q /C " + sTmp, Path.GetDirectoryName(sFilename), System.Environment.GetEnvironmentVariable("ComSpec"));
		}

		// rebuild the vcproj file using JAM
		public void BuildCustomTarget(string sTarget)
		{
			string sSolutionDir = GetSolutionDir();
			string sConfig = GetActiveConfig();
			char[] separator = { '_' };
			string[] sConfigs = sConfig.Split(separator,2);
			if (sConfigs.Length != 2)
				throw new System.InvalidOperationException("The solution is not compatible with JAM integration.");
			string sJAMCMD = "-sTARGET=" + sConfigs[1] + " -sBUILD=" + sConfigs[0] + " " + sTarget;

			LaunchJAM(sJAMCMD, sSolutionDir, m_sJAMExePath);
		}

		// rebuild the solution using IB + JAM
		public void BuildSolutionUsingIB(bool bRebuildAll)
		{
			string sSolutionDir = GetSolutionDir();
			string sConfig = GetActiveConfig();
			char[] separator = { '_' };
			string[] sConfigs = sConfig.Split(separator,2);
			if (sConfigs.Length != 2)
				throw new System.InvalidOperationException("The solution is not compatible with JAM integration.");
			string sJAMCMD = "-sTARGET=" + sConfigs[1] + " -sBUILD=" + sConfigs[0] + " " + m_Properties.GenerateCmdLineString(true) + " " + GetBuildTarget();
			string sApp = m_sJAMExePath;
			if (bRebuildAll)
				sJAMCMD += " -a";

			// get the IB augmented commandline from the map
			IBCmdLine ibcmd = m_IBCommandLineMap[sConfigs[1]];
			if (ibcmd != null)
			{
				sJAMCMD = ibcmd.sCmdLine.Replace("$$CMDLINE$$", sJAMCMD);
				sJAMCMD = sJAMCMD.Replace("$$APP$$", sApp);
				sApp = ibcmd.sApp.Replace("$$IBPATH$$", GetIncredibuildPath());
				sApp = sApp.Replace("$$SN_COMMON_PATH$$", GetSNCommonPath());
				sJAMCMD = sJAMCMD.Replace("$$SN_COMMON_PATH$$", GetSNCommonPath());
			}

			LaunchJAM(sJAMCMD, sSolutionDir, sApp);
		}

		// cancel the build that's currently in progress
		public void CancelBuild()
		{
			Bizarre.LaunchJAM launch = new Bizarre.LaunchJAM();
			launch.CancelBuild();
			EnvDTE.DTE dte = (EnvDTE.DTE) m_Parent._GetService(typeof(EnvDTE.DTE));
			try
			{
				dte.ExecuteCommand("Build.Cancel", "");
			}
			catch (Exception ex)
			{
			}
		}

		// compile the file with the given filename
		void CompileFile(string sFilename, string sExtraOptions)
		{
			string sExt = System.IO.Path.GetExtension(sFilename);
			string sSolutionDir = GetSolutionDir();
			string sConfig = GetActiveConfig();
			char[] separator = { '_' };
			string[] sConfigs = sConfig.Split(separator,2);
			if (sConfigs.Length != 2)
				throw new System.InvalidOperationException("The solution is not compatible with JAM integration.");
			string sJAMCMD = "-sTARGET=" + sConfigs[1] + " -sBUILD=" + sConfigs[0] + " " + m_Properties.GenerateCmdLineString(false) + " " + sExtraOptions + " ";

			if (sExt.ToLower() == ".jam")
			{
				// we are compiling a .jam file
				LaunchJAM(sJAMCMD, Path.GetDirectoryName(sFilename), m_sJAMExePath);
			}
			else
			{
				// we are compiling a source file
				LaunchJAM(sJAMCMD + "-b" + RelativePath(sFilename,sSolutionDir), sSolutionDir, m_sJAMExePath);
			}
		}

		// return a relative path
		private static string RelativePath(string relativeTo, string absolutePath)
        {
			relativeTo = relativeTo.ToLower();
			absolutePath = absolutePath.ToLower();

            string[] absoluteDirectories = absolutePath.Split('\\');
            string[] relativeDirectories = relativeTo.Split('\\');

            //Get the shortest of the two paths
            int length = absoluteDirectories.Length < relativeDirectories.Length ? absoluteDirectories.Length : relativeDirectories.Length;

            //Use to determine where in the loop we exited
            int lastCommonRoot = -1;
            int index;

            //Find common root
            for (index = 0; index < length; index++)
                if (absoluteDirectories[index] == relativeDirectories[index])
                    lastCommonRoot = index;
                else
                    break;

            //If we didn't find a common prefix then throw
            if (lastCommonRoot == -1)
                throw new ArgumentException("Paths do not have a common base");

            //Build up the relative path
            StringBuilder relativePath = new StringBuilder();

            //Add on the ..
            for (index = lastCommonRoot + 1; index < absoluteDirectories.Length; index++)
                if (absoluteDirectories[index].Length > 0)
                    relativePath.Append("..\\");

            //Add on the folders
            for (index = lastCommonRoot + 1; index < relativeDirectories.Length - 1; index++)
                relativePath.Append(relativeDirectories[index] + "\\");
            relativePath.Append(relativeDirectories[relativeDirectories.Length - 1]);

            return relativePath.ToString();
        }

		private string GetSolutionDir()
		{
            IVsSolution solution = (IVsSolution)m_Parent._GetService(typeof(SVsSolution));
			object prop;
			solution.GetProperty((int)__VSPROPID.VSPROPID_SolutionDirectory, out prop);
			if (prop == null)
				throw new System.InvalidOperationException("The operation failed because no Solution was loaded");
			return (string)prop;
		}

		private string GetActiveConfig()
		{
			EnvDTE.DTE dte = (EnvDTE.DTE) m_Parent._GetService(typeof(EnvDTE.DTE));
			Project prj = dte.Solution.Projects.Item(1);
			Configuration config = prj.ConfigurationManager.ActiveConfiguration;
			ConfigurationManager configmgr = config.Collection;
			return configmgr.ActiveConfiguration.ConfigurationName;
		}

		private string GetBuildTarget()
		{
			try
			{
				EnvDTE.DTE dte = (EnvDTE.DTE) m_Parent._GetService(typeof(EnvDTE.DTE));
				VCProject proj = (VCProject)dte.Solution.Projects.Item(1).Object;
				IVCCollection cfgs = (IVCCollection)proj.Configurations;
				VCConfiguration cfg = (VCConfiguration)cfgs.Item(1);
				IVCCollection tools = (IVCCollection)cfg.Tools;
				VCNMakeTool tool = (VCNMakeTool)tools.Item(1);
				string s = tool.BuildCommandLine;
				int idx = s.LastIndexOf('=');
				s = s.Substring(idx+1);
				idx = s.IndexOf(' ');
				s = s.Substring(idx+1);
				return s;
			}
			catch (Exception e)
			{
				return "";
			}
		}

		// save all open files
		void SaveAll()
		{
			EnvDTE.DTE dte = (EnvDTE.DTE) m_Parent._GetService(typeof(EnvDTE.DTE));
			dte.Documents.SaveAll();
		}

		private string GetIncredibuildPath()
		{
			RegistryKey masterKey = Registry.LocalMachine.CreateSubKey ("SOFTWARE\\Xoreax\\IncrediBuild\\Builder");
			if (masterKey == null)
			{
				Console.WriteLine("Incredibuild not installed!");
				return "";
			}
			else
			{
				return (string)masterKey.GetValue("Folder");
			}
		}

		private string GetSNCommonPath()
		{
			return System.Environment.GetEnvironmentVariable("SN_COMMON_PATH");
		}

		// use the unmanaged C++ DLL to execute the commandline on a different thread
		private void LaunchJAM(string sCommandLine, string sWorkingDirectory, string sApp)
		{
			IVsLaunchPadFactory launchPad = (IVsLaunchPadFactory)m_Parent._GetService(typeof(IVsLaunchPadFactory));

//			EnvDTE80.DTE2 dte = (EnvDTE80.DTE2) m_Parent._GetService(typeof(EnvDTE.DTE));
			EnvDTE.DTE dte = (EnvDTE.DTE) m_Parent._GetService(typeof(EnvDTE.DTE));
			((OutputWindow)dte.Windows.Item(EnvDTE.Constants.vsWindowKindOutput).Object).Parent.Activate();
//			dte.ToolWindows.OutputWindow.Parent.Activate();

			Bizarre.LaunchJAM launch = new Bizarre.LaunchJAM();
			if (launch.IsBuildInProgress())
				return;
			SaveAll();
			IVsOutputWindow ow = m_Parent._GetService(typeof(SVsOutputWindow)) as IVsOutputWindow;
			Guid owguid = new Guid();
			owguid = VSConstants.GUID_BuildOutputWindowPane;
			IVsOutputWindowPane owp;
			ow.GetPane(ref owguid, out owp);
			owp.Activate();
			owp.Clear();
			
			launch.Go(sApp, sCommandLine, sWorkingDirectory, launchPad, owp);
		}

		public JamProperties RetrieveSettings()
		{
			return m_Properties;
		}

		public void SetSettings(JamProperties prop)
		{
			m_Properties = prop;
			m_Properties.SetEnvironmentItem();
		}
	}
}

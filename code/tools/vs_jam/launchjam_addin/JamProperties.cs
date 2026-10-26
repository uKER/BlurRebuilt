using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Win32;
using System.Runtime.InteropServices;
using System.Security;
using System.Security.Permissions;

namespace SetEnv
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	public class SampleSetEnvironmentVariable
	{
// Import the kernel32 dll.
[DllImport("kernel32.dll",CharSet=CharSet.Auto, SetLastError=true)]

[return:MarshalAs(UnmanagedType.Bool)]

// The declaration is similar to the SDK function
public static extern bool SetEnvironmentVariable(string lpName, string lpValue);

		public SampleSetEnvironmentVariable()
		{
		}

		public static bool SetEnvironmentVariableEx(string environmentVariable, string variableValue)
		{
			try
			{
				// Get the write permission to set the environment variable.
				EnvironmentPermission environmentPermission = new EnvironmentPermission(EnvironmentPermissionAccess.Write,environmentVariable);

				environmentPermission.Demand(); 

				return SetEnvironmentVariable(environmentVariable, variableValue);
			}
			catch( SecurityException e)
			{
				Console.WriteLine("Exception:" + e.Message);
			}
			return false;
		}
	}

/*
			// Create a sample environment variable and set its value (for the current process).
			SampleSetEnvironmentVariable.SetEnvironmentVariableEx("TESTENV", "TestValue");

			// Verify that environment variable is set correctly.
			Console.WriteLine("The value of TESTENV is: " + Environment.GetEnvironmentVariable("TESTENV"));
 */ 
}

namespace BizarreCreations.JamIntegration
{
	public class JamProperties
	{
		public int mNumProcessors;
		public bool mDontRunActions;
		public bool mBuildNewestFirst;
		public bool mQuitEarly;
		public int mDebuggingLevel;
		public bool mShowCommands;
		public bool mShowCauses;
		public bool mShowMakeTree;
		public bool mShowActions;
		public bool mShowDependencies;
		public string mCustomOptions;
		public bool mShowIncludes;
		public bool mEnableNetworkCaching;

		public JamProperties()
		{
			mNumProcessors = 1;
			mQuitEarly = true;
			mDebuggingLevel = 0;
			mBuildNewestFirst = mDontRunActions = mShowCommands = mShowDependencies = mShowCauses = mShowMakeTree = mShowActions = false;
			mEnableNetworkCaching = true;
			mShowIncludes = false;
			mCustomOptions = "";
		}

		public void ReadFromRegistry()
		{
			try
			{
				RegistryKey masterKey = Registry.LocalMachine.CreateSubKey("SOFTWARE\\Bizarre Creations\\Visual Studio Jam Integration");
				mNumProcessors = (int)masterKey.GetValue("NumProcessors");
				mDontRunActions = 0 != (int)masterKey.GetValue("DontRunActions");
				mBuildNewestFirst = 0 != (int)masterKey.GetValue("BuildNewestFirst");
				mQuitEarly = 0 != (int)masterKey.GetValue("QuitEarly");
				mDebuggingLevel = (int)masterKey.GetValue("DebuggingLevel");
				mShowDependencies = 0 != (int)masterKey.GetValue("ShowDependencies");
				mShowCommands = 0 != (int)masterKey.GetValue("ShowCommands");
				mShowCauses = 0 != (int)masterKey.GetValue("ShowCauses");
				mShowMakeTree = 0 != (int)masterKey.GetValue("ShowMakeTree");
				mShowActions = 0 != (int)masterKey.GetValue("ShowActions");	
				mCustomOptions = (string)masterKey.GetValue("CustomOptions");
				mEnableNetworkCaching = 0 != (int)masterKey.GetValue("EnableNetworkCaching");
				mShowIncludes = 0 != (int)masterKey.GetValue("ShowIncludes");
			}
			catch (Exception)
			{
			}
		}

		public void WriteToRegistry()
		{
			try
			{
				RegistryKey masterKey = Registry.LocalMachine.CreateSubKey("SOFTWARE\\Bizarre Creations\\Visual Studio Jam Integration");
				masterKey.SetValue("NumProcessors", mNumProcessors);
				masterKey.SetValue("DontRunActions", mDontRunActions?1:0);
				masterKey.SetValue("BuildNewestFirst", mBuildNewestFirst?1:0);
				masterKey.SetValue("QuitEarly", mQuitEarly?1:0);
				masterKey.SetValue("DebuggingLevel", mDebuggingLevel);
				masterKey.SetValue("ShowDependencies", mShowDependencies?1:0);
				masterKey.SetValue("ShowCommands", mShowCommands?1:0);
				masterKey.SetValue("ShowCauses", mShowCauses?1:0);
				masterKey.SetValue("ShowMakeTree", mShowMakeTree?1:0);
				masterKey.SetValue("ShowActions", mShowActions?1:0);
				masterKey.SetValue("CustomOptions", mCustomOptions);
				masterKey.SetValue("EnableNetworkCaching", mEnableNetworkCaching?1:0);
				masterKey.SetValue("ShowIncludes", mShowIncludes?1:0);
			}
			catch (Exception)
			{
			}
		}

		public string GenerateCmdLineString(bool bIncredibuild)
		{
			string rtn = "";
			string debugflag = "";

			if (mShowActions)
				debugflag += "a";
			if (mShowCauses)
				debugflag += "c";
			if (mShowDependencies)
				debugflag += "d";
			if (mShowCommands)
				debugflag += "x";
			if (mShowMakeTree)
				debugflag += "m";

			// if we're using incredibuild, the number of processors is set elsewhere
			if (!bIncredibuild)
				rtn += "-j" + mNumProcessors.ToString();
			
			rtn += " -d" + debugflag + mDebuggingLevel.ToString();

			if (mDontRunActions)
				rtn += " -n";
			if (mBuildNewestFirst)
				rtn += " -g";
			if (mQuitEarly && !bIncredibuild)	// disable 'quit early' when using incredibuild
				rtn += " -q";
			if (mShowIncludes)
				rtn += " -sSHOW_INCLUDES=1";
			if (mEnableNetworkCaching)
				rtn += " -sNETWORK_CACHING=1";
			else
				rtn += " -sNETWORK_CACHING=0";
			rtn += " " + mCustomOptions;

			return rtn;
		}

		public void SetEnvironmentItem()
		{
			string cmdline = GenerateCmdLineString(false);

			// Create a sample environment variable and set its value (for the current process).
			SetEnv.SampleSetEnvironmentVariable.SetEnvironmentVariableEx("JAMOPTS", cmdline);

			// also write it to the registry
			// this is done in case we boot up visual studio without the add-in loading
			// the latest JAMOPTS will be retrieved from the registry instead and it will still work
			try
			{
				RegistryKey masterKey = Registry.CurrentUser.CreateSubKey("Environment");
				masterKey.SetValue("JAMOPTS", cmdline);
			}
			catch (Exception)
			{
			}
		}
	}
}


using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using Microsoft.Win32; 
using System.Diagnostics;
using System.Collections;

namespace DevenvSetupCustomAction
{
	[RunInstaller(true)]
	public partial class DevenvSetup : Installer
	{
		public DevenvSetup()
		{
			InitializeComponent();
		}

		public override void Install(IDictionary stateSaver)
		{
		   base.Install(stateSaver);

		   using(RegistryKey setupKey =       Registry.LocalMachine.OpenSubKey(
				 @"SOFTWARE\Microsoft\VisualStudio\8.0\Setup\VS"))
		   {
			  if (setupKey != null)
			  {
				 string devenv =             setupKey.GetValue("EnvironmentPath").ToString();
				 if (!string.IsNullOrEmpty(devenv))
				 {
					Process.Start(devenv, "/setup").WaitForExit();
				 }
			  }
		   }
		}
	}
}
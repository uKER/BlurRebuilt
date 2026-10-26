/***************************************************************************

Copyright (c) Microsoft Corporation. All rights reserved.
This code is licensed under the Visual Studio SDK license terms.
THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.

***************************************************************************/

using System;
using System.ComponentModel.Design;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Text;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using EnvDTE;
using Microsoft.VisualStudio;
using System.IO;
using Microsoft.VisualStudio.OLE.Interop;
using System.Windows.Forms;

#pragma warning disable 0649
#pragma warning disable 0169

namespace BizarreCreations.JamIntegration
{
	/// <summary>
	/// This is the class that implements the package. This is the class that Visual Studio will create
	/// when one of the commands will be selected by the user, and so it can be considered the main
	/// entry point for the integration with the IDE.
	/// Notice that this implementation derives from Microsoft.VisualStudio.Shell.Package that is the
	/// basic implementation of a package provided by the Managed Package Framework (MPF).
	/// </summary>
	[PackageRegistration(UseManagedResourcesOnly = true)]
	[DefaultRegistryRoot(@"Microsoft\VisualStudio\8.0")]
	[ProvideMenuResource(1000, 1)]
	[Guid("50609434-6E6F-404b-A67C-40DC0002D8EE")]
	[ComVisible(true)]
	[ProvideLoadKey("Standard", "1.0", "JAM Integration", "Bizarre Creations", 104)]
	public sealed class MenuCommandsPackage : Package
	{
		#region Member Variables
		private OleMenuCommand dynamicVisibilityCommand1;
		private OleMenuCommand dynamicVisibilityCommand2;
		#endregion

		private JamIntegration m_Jam;

		/// <summary>
		/// Default constructor of the package. This is the constructor that will be used by VS
		/// to create an instance of your package. Inside the constructor you should do only the
		/// more basic initializazion like setting the initial value for some member variable. But
		/// you should never try to use any VS service because this object is not part of VS
		/// environment yet; you should wait and perform this kind of initialization inside the
		/// Initialize method.
		/// </summary>
		public MenuCommandsPackage()
		{
			Trace.WriteLine(String.Format(CultureInfo.CurrentUICulture, "Entering constructor for: {0}", this.ToString()));
		}

		public object _GetService(Type serviceType)
		{
			return GetService(serviceType);
		}

		/// <summary>
		/// Initialization of the package; this is the place where you can put all the initialization
		/// code that relies on services provided by Visual Studio.
		/// </summary>
                [SecurityPermission(SecurityAction.Demand, Flags=SecurityPermissionFlag.UnmanagedCode)]
		protected override void Initialize()
		{
			// Trace the beginning of this method and call the base implementation.
			Trace.WriteLine(String.Format(CultureInfo.CurrentUICulture, "Entering Initialize() of: {0}", this.ToString()));
			base.Initialize();

			// Now get the OleCommandService object provided by the MPF; this object is the one
			// responsible for handling the collection of commands implemented by the package.
			OleMenuCommandService mcs = GetService(typeof(IMenuCommandService)) as OleMenuCommandService;
			if (null != mcs)
			{
				// Now create one object derived from MenuCommnad for each command defined in
				// the CTC file and add it to the command service.

				// For each command we have to define its id that is a unique Guid/integer pair.
				CommandID id;
				OleMenuCommand command;

				// Create the MenuCommand objects for all our functions
				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidBuildXWithJam);
				command = new OleMenuCommand(new EventHandler(BuildXWithJAM), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidRebuildXWithJam);
				command = new OleMenuCommand(new EventHandler(RebuildXWithJAM), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidBuildSolutionWithIBJam);
				command = new OleMenuCommand(new EventHandler(BuildSolutionWithIB_JAM), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidRebuildSolutionWithIBJam);
				command = new OleMenuCommand(new EventHandler(RebuildSolutionWithIB_JAM), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidCancelJAMBuild);
				command = new OleMenuCommand(new EventHandler(CancelJAMBuild), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidGenerateNewVcproj);
				command = new OleMenuCommand(new EventHandler(GenerateNewVcproj), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidBuildWithJamCtx);
				command = new OleMenuCommand(new EventHandler(BuildWithJAMCtx), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidJamProperties);
				command = new OleMenuCommand(new EventHandler(ShowJamPropertiesDlg), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidBuildAndRunUnitTest);
				command = new OleMenuCommand(new EventHandler(BuildAndRunUnitTest), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidBuildDocumentation);
				command = new OleMenuCommand(new EventHandler(BuildDocumentation), id); mcs.AddCommand(command);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidPerforceCheckoutOrAdd);
				command = new OleMenuCommand(new EventHandler(CheckoutOrAdd), id); mcs.AddCommand(command);

/*
				// Create the DynamicMenuCommand object for the command defined with the TEXTCHANGES
				// flag.
				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidDynamicTxt);
				command = new DynamicTextCommand(id, VSPackage.ResourceManager.GetString("DynamicTextBaseText"));
				mcs.AddCommand(command);

				// Now create two OleMenuCommand objects for the two commands with dynamic visibility
				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidDynVisibility1);
				dynamicVisibilityCommand1 = new OleMenuCommand(new EventHandler(DynamicVisibilityCallback), id);
				mcs.AddCommand(dynamicVisibilityCommand1);

				id = new CommandID(GuidsList.guidMenuAndCommandsCmdSet, PkgCmdIDList.cmdidDynVisibility2);
				dynamicVisibilityCommand2 = new OleMenuCommand(new EventHandler(DynamicVisibilityCallback), id);
				// This command is the one that is invisible by default, so we have to set its visble
				// property to false because the default value of this property for every object derived
				// from MenuCommand is true.
				dynamicVisibilityCommand2.Visible = false;
				mcs.AddCommand(dynamicVisibilityCommand2);
 */ 
			}

			try
			{
				m_Jam = new JamIntegration(this);
			}
			catch (Exception exception)
			{
				MessageBox.Show(exception.Message, "Error loading JAM plugin",
						MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
			}
		}

		#region Commands Actions
		/// <summary>
		/// This function prints text on the debug ouput and on the generic pane of the 
		/// Output window.
		/// </summary>
		/// <param name="text"></param>
		private void OutputCommandString(string text)
		{
			// Build the string to write on the debugger and Output window.
			StringBuilder outputText = new StringBuilder();
			//outputText.Append(" ================================================\n");
			//outputText.AppendFormat("  MenuAndCommands: {0}\n", text);
			//outputText.Append(" ================================================\n\n");
			outputText.AppendFormat("{0}\n",text);
			Trace.WriteLine(outputText.ToString());

			// Now print the string on the Output window.

			// The first step is to get a reference to IVsOutputWindow.
			IVsOutputWindow outputWindow = GetService(typeof(SVsOutputWindow)) as IVsOutputWindow;
			// If we fail to get it we can exit now.
			if (null == outputWindow)
			{
				Trace.WriteLine("Failed to get a reference to IVsOutputWindow");
				return;
			}
			// Now get the window pane for the general output.
			Guid guidGeneral = Microsoft.VisualStudio.VSConstants.GUID_BuildOutputWindowPane;
			IVsOutputWindowPane windowPane;
			if (Microsoft.VisualStudio.ErrorHandler.Failed(outputWindow.GetPane(ref guidGeneral, out windowPane)))
			{
				Trace.WriteLine("Failed to get a reference to the Output window General pane");
				return;
			}
			if (Microsoft.VisualStudio.ErrorHandler.Failed(windowPane.OutputString(outputText.ToString())))
			{
				Trace.WriteLine("Failed to write on the Output window");
			}
		}

		public void LogToOutputWindow(string message, bool bCheckForTask)
		{

			IVsOutputWindow output = Microsoft.VisualStudio.Shell.Package.GetGlobalService(typeof(SVsOutputWindow))
			as IVsOutputWindow;

			if(output != null)
			{
				// can't write to the output window - just a pane
				// see References.Services HelperMethodsClass SDK Sample
				Guid generalPaneGuid = VSConstants.GUID_BuildOutputWindowPane;
				IVsOutputWindowPane pane;

				// fetch the pane wrapped in error handling:
				if(ErrorHandler.Failed(output.GetPane(ref generalPaneGuid,
				out pane)) || pane == null)
				{
					// ideally, throw an exception or trace/output...
					return;
				}

				if(bCheckForTask)
				{
					IVsLaunchPad launch = Microsoft.VisualStudio.Shell.Package.GetGlobalService(typeof(SVsLaunchPad))
					as IVsLaunchPad;

					uint[] pnPriority = new uint[1];
					string[] pbstrFilename = new string[1];
					uint[] pnLineNum = new uint[1];
					string[] pbstrTaskItemText = new string[1];
					int[] pfTaskItemFound = new int[1];

					if(ErrorHandler.Failed(launch.ParseOutputStringForTaskItem(message, pnPriority, pbstrFilename, pnLineNum, pbstrTaskItemText, pfTaskItemFound)))
					{
						// ideally, throw an exception or trace/output...
						return;
					}
					if(pfTaskItemFound[0] == 1)
					{
						bool b =
						ErrorHandler.Failed(pane.OutputTaskItemString(message, VSTASKPRIORITY.TP_HIGH, VSTASKCATEGORY.CAT_BUILDCOMPILE, "Galaxy MetaData", (int)_vstaskbitmap.BMP_COMPILE, pbstrFilename[0], pnLineNum[0], pbstrTaskItemText[0]));
						return;
					}

				}
				// wrap attempts to write in an error handler:
				if(ErrorHandler.Failed(pane.OutputString(message + System.Environment.NewLine)))
				{
					// throw an exception/etc.
					return;
				}
			}
			else //no output window found, could be the console (MSBuild), write to Console instead.
			{
				Console.WriteLine(message);
			}
		}

		/// <summary>
		/// Event handler called when the user selects the Build with JAM context menu
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void BuildWithJAMCtx(object caller, EventArgs args)
		{
			m_Jam.BuildSelection(false);
		}
		
		/// <summary>
		/// Event handler called when the user selects the Build X with JAM command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void BuildXWithJAM(object caller, EventArgs args)
		{
			m_Jam.BuildSelection(false);
		}

		/// <summary>
		/// Event handler called when the user selects the Rebuild X with JAM command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void RebuildXWithJAM(object caller, EventArgs args)
		{
			m_Jam.BuildSelection(true);
		}

		/// <summary>
		/// Event handler called when the user selects the Build Solution with IB + JAM command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void BuildSolutionWithIB_JAM(object caller, EventArgs args)
		{
			m_Jam.BuildSolutionUsingIB(false);
		}

		/// <summary>
		/// Event handler called when the user selects the Rebuild Solution with IB + JAM command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void RebuildSolutionWithIB_JAM(object caller, EventArgs args)
		{
			m_Jam.BuildSolutionUsingIB(true);
		}

		/// <summary>
		/// Event handler called when the user selects the Generate new Vcproj command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void GenerateNewVcproj(object caller, EventArgs args)
		{
			m_Jam.BuildCustomTarget("-a vcproj");
		}

		/// <summary>
		/// Event handler called when the user selects the cancel cam build command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void CancelJAMBuild(object caller, EventArgs args)
		{
			m_Jam.CancelBuild();
		}

		/// <summary>
		/// Event handler called when the user selects the build documentation command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void BuildDocumentation(object caller, EventArgs args)
		{
			m_Jam.BuildCustomTarget("-a docs");
		}

		/// <summary>
		/// Event handler called when the user selects the build documentation command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void CheckoutOrAdd(object caller, EventArgs args)
		{
			m_Jam.CheckoutOrAdd();
		}

		/// <summary>
		/// Event handler called when the user selects the build and run unit test command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void BuildAndRunUnitTest(object caller, EventArgs args)
		{
			m_Jam.BuildCustomTarget("runtest");
		}

		/// <summary>
		/// Event handler called when the user selects the jam build properties command
		/// </summary>
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "BizarreCreations.JamIntegration.OutputCommandString(System.String)")]
		private void ShowJamPropertiesDlg(object caller, EventArgs args)
		{
			JamPropertiesDlg dlg = new JamPropertiesDlg();

			dlg.SetSettings(m_Jam.RetrieveSettings());
			if (dlg.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				JamProperties settings = dlg.RetrieveSettings();
				settings.WriteToRegistry();
				m_Jam.SetSettings(settings);
			}
		}

/*		
		/// <summary>
		/// Event handler called when the user selects one of the two menus with
		/// dynamic visibility.
		/// </summary>
		private void DynamicVisibilityCallback(object caller, EventArgs args)
		{
			// This callback is supposed to be called only from the two menus with dynamic visibility
			// defined inside this package, so first we have to verify that the caller is correct.

			// Check that the type of the caller is the expected one.
			OleMenuCommand command = caller as OleMenuCommand;
			if (null == command)
				return;

			// Now check the command set.
			if (command.CommandID.Guid != GuidsList.guidMenuAndCommandsCmdSet)
				return;

			// This is one of our commands. Now what we want to do is to switch the visibility status
			// of the two menus with dynamic visibility, so that if the user clicks on one, then this 
			// will make it invisible and the other one visible.
			if (command.CommandID.ID == PkgCmdIDList.cmdidDynVisibility1)
			{
				// The user clicked on the first one; make it invisible and show the second one.
				dynamicVisibilityCommand1.Visible = false;
				dynamicVisibilityCommand2.Visible = true;
			}
			else if (command.CommandID.ID == PkgCmdIDList.cmdidDynVisibility2)
			{
				// The user clicked on the second one; make it invisible and show the first one.
				dynamicVisibilityCommand2.Visible = false;
				dynamicVisibilityCommand1.Visible = true;
			}
		}
 */
		#endregion

	}

}

/*
[DllImport("kernel32.dll")]
static extern bool CreateProcess(string lpApplicationName, string lpCommandLine, IntPtr lpProcessAttributes, IntPtr lpThreadAttributes,
                        bool bInheritHandles, uint dwCreationFlags, IntPtr lpEnvironment, 
                        string lpCurrentDirectory, ref STARTUPINFO lpStartupInfo,out PROCESS_INFORMATION lpProcessInformation);


[DllImport("kernel32.dll")]
static extern bool DebugActiveProcessStop(uint dwProcessId);

[DllImport("kernel32.dll")]
static extern bool DebugBreakProcess(IntPtr dwProcessHandle);

		// start debugging the specified application
		void DebugApplication(string sAppName)
		{
			// stop any debugging that's happening at the moment
			EnvDTE80.DTE2 dte = (EnvDTE80.DTE2)_GetService(typeof(EnvDTE.DTE));
			EnvDTE80.Debugger2 debugger = (EnvDTE80.Debugger2)dte.Debugger;
			debugger.TerminateAll();

			// create process with debugging
			STARTUPINFO si = new STARTUPINFO();
			PROCESS_INFORMATION pi = new PROCESS_INFORMATION();
			CreateProcess(sAppName, null, IntPtr.Zero, IntPtr.Zero, false, 1, IntPtr.Zero, null, ref si, out pi);

			// stop process and detach
			DebugBreakProcess(pi.hProcess);
			DebugActiveProcessStop(pi.dwProcessId);
			
			// find the process and attach to it with the visual studio debugger
			EnvDTE.Processes processes = debugger.LocalProcesses;
			foreach (EnvDTE80.Process2 proc in processes)
			{
				if (proc.ProcessID == pi.dwProcessId)
				{
					proc.Attach();
				}
			}		 
		}

*/
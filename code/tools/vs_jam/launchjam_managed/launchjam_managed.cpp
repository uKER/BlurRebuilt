using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;
using namespace Microsoft::VisualStudio::OLE::Interop;
using namespace Microsoft::VisualStudio::Shell::Interop;

//
// General Information about an assembly is controlled through the following
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
//
[assembly:AssemblyTitleAttribute("launchjam")];
[assembly:AssemblyDescriptionAttribute("")];
[assembly:AssemblyConfigurationAttribute("")];
[assembly:AssemblyCompanyAttribute("bizarre")];
[assembly:AssemblyProductAttribute("launchjam")];
[assembly:AssemblyCopyrightAttribute("Copyright (c) bizarre 2007")];
[assembly:AssemblyTrademarkAttribute("")];
[assembly:AssemblyCultureAttribute("")];

//
// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version
//      Build Number
//      Revision
//
// You can specify all the value or you can default the Revision and Build Numbers
// by using the '*' as shown below:

[assembly:AssemblyVersionAttribute("1.0.*")];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];

[assembly:SecurityPermission(SecurityAction::RequestMinimum, UnmanagedCode = true)];

// This is the main DLL file.

void LaunchJAM(char *pszApp, char *pszCommandline, char *pszDirectory, void *pLaunchPadFactory, void *pOutputWindowPane );
void CancelBuild();
bool IsBuildInProgress();

using namespace System::Runtime::InteropServices;

namespace Bizarre
{
	public ref class LaunchJAM
	{
	public:
		void Go(String^ app, String^ commandline, String^ directory, IVsLaunchPadFactory^ launchPadFactory, IVsOutputWindowPane^ outputWindowPane)
		{
			System::IntPtr appAnsiPtr = Marshal::StringToHGlobalAnsi(app);
			char *_app = (char *) appAnsiPtr.ToPointer();
			System::IntPtr cmdlineAnsiPtr = Marshal::StringToHGlobalAnsi(commandline);
			char *cmdline = (char *) cmdlineAnsiPtr.ToPointer();
			System::IntPtr dirAnsiPtr = Marshal::StringToHGlobalAnsi(directory);
			char *dir = (char *) dirAnsiPtr.ToPointer();

			void *p = (void*)Marshal::GetComInterfaceForObject(launchPadFactory, IVsLaunchPadFactory::typeid );
			void *p2 = (void*)Marshal::GetComInterfaceForObject(outputWindowPane, IVsOutputWindowPane::typeid );

			::LaunchJAM(_app, cmdline, dir, p, p2);

			Marshal::FreeHGlobal(cmdlineAnsiPtr);
			Marshal::FreeHGlobal(dirAnsiPtr);
			Marshal::FreeHGlobal(appAnsiPtr);
		}

		void CancelBuild()
		{
			::CancelBuild();
		}

		bool IsBuildInProgress()
		{
			return ::IsBuildInProgress();
		}
	};
}


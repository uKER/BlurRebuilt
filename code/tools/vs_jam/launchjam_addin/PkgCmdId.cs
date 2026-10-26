/***************************************************************************

Copyright (c) Microsoft Corporation. All rights reserved.
This code is licensed under the Visual Studio SDK license terms.
THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.

***************************************************************************/

using System;

namespace BizarreCreations.JamIntegration
{
	/// <summary>
	/// This class is used to expose the list of the IDs of the commands implemented
	/// by this package. This list of IDs must match the set of IDs defined inside the
	/// BUTTONS section of the CTC file.
	/// </summary>
	internal static class PkgCmdIDList
	{
		// Now define the list a set of public static members.
		public const int cmdidBuildXWithJam = 0x2001;
		public const int cmdidRebuildXWithJam = 0x2002;
		public const int cmdidBuildSolutionWithIBJam = 0x2003;
		public const int cmdidRebuildSolutionWithIBJam = 0x2004;
		public const int cmdidGenerateNewVcproj = 0x2005;
		public const int cmdidCancelJAMBuild = 0x2006;
		public const int cmdidBuildWithJamCtx = 0x2007;
		public const int cmdidJamProperties = 0x2008;
		public const int cmdidBuildAndRunUnitTest = 0x2009;
		public const int cmdidBuildDocumentation = 0x200a;
		public const int cmdidPerforceCheckoutOrAdd = 0x200b;
	}
}

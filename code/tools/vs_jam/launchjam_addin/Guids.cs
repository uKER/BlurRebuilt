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
	/// This class is used only to expose the list of Guids used by this package.
	/// This list of guids must match the set of Guids used inside the CTC file.
	/// </summary>
	internal static class GuidsList
	{
		// Now define the list of guids as public static members.
		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
		public static readonly Guid guidMenuAndCommandsPkg = new Guid("{50609434-6E6F-404b-A67C-40DC0002D8EE}");

		public static readonly Guid guidMenuAndCommandsCmdSet = new Guid("{10A79DAE-B33C-4FDB-8922-B056628858A1}");

		[System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
		public static readonly Guid guidGenericCmdBmp = new Guid("{0A4C51BD-3239-4370-8869-16E0AE8C0A46}");
	}
}

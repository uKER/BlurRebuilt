using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace application
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);


			BizarreToolbarManager btm = new BizarreToolbarManager();

			Application.AddMessageFilter(btm);

			Application.Idle += new EventHandler(btm.Application_Idle);
			Application.Run(btm);
		}
	}
}
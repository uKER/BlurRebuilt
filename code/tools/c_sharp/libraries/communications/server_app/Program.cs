using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace server_app
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

			ServerApp app = new ServerApp();
			app.Show();	// Make sure the window handle is created before we call initialise.
			app.Initialise();

			Application.Run(app);
		}
	}
}
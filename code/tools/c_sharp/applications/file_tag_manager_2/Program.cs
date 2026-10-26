using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace file_tag_manager_2
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			//Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			fileTagManager_MainForm ftmf = new fileTagManager_MainForm();
			ftmf.Initialise();
			Application.Run(ftmf);
		}
	}
}
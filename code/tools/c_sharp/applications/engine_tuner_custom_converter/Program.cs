using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace EngineTunerCustomConverter
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			if(args.Length == 0)
				args = new string[] { @"C:\Documents and Settings\keithp\Desktop\Gallardotuner before.xml" };
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Form form1 = new Form1(args);
			if(Form1.m_bQuit != true)
				Application.Run(form1);
		}
	}
}
using System;
using System.Xml;
using System.Xml.Serialization;
using System.Collections;
using System.IO;

namespace WaveBundler
{
	
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class MainClass
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			Console.WriteLine("\nWaveBundler v0.1\n");
			
			if (args.Length == 0)
			{
				Console.WriteLine("Usage: WaveBundler xmlfile [xmlfile2] [xmlfile3] [more files...]");
				return;
			}

			if (args.Length > 1)
				Console.WriteLine("Processing {0} files...", args.Length); 

			for (int i=0; i < args.Length; i++)
			{
				Console.Write("Processing file {0}... ", args[i]); 
				if (ProcessIniFile(args[i]) == true)
					Console.WriteLine("ok"); 
			}
		}

		// process an xml ini file and build a wavebank
		public static bool ProcessIniFile(string fileName)
		{
			// create an inifile and read it in
			WaveBankIniFile iniFile = WaveBankIniFileReader.Read(fileName);

			if (iniFile == null)
				return false;

			WaveBankFile bankFile = new WaveBankFile();
			bankFile.Write(iniFile);

			return true;
		}
	}
}

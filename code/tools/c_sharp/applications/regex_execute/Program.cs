using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.IO;

namespace regex_execute
{
	class Program
	{
		/// <summary>
		/// 
		/// </summary>
		/// <param name="args">
		/// </param>
		static int Main(string[] args)
		{
			RegexOptions regexOptions = new RegexOptions();

			string strTest = args.ToString();

			int argCount = 0;

			string strInput = null;
			string strRegex = null;
			string strReplacement = null;
			string strWriteTo = null;

			bool bView = false;
			bool bExecute = false;
			bool bNoWait = false;

			while(argCount < args.Length)
			{
				string strArg = args[argCount];
				argCount++;
				switch(strArg)
				{
					case "-input":
						{
							strInput = GetArg(args, argCount++);
							if(strInput == null)
							{
								Console.WriteLine("Cannot get the 'input' parameter.");
								Usage();
								return 1;
							}
						}
						break;
					case "-regex":
						{
							strRegex = GetArg(args, argCount++);
							if(strRegex == null)
							{
								Console.WriteLine("Cannot get the 'regex' parameter.");
								Usage();
								return 1;
							}
						}
						break;
					case "-replacement":
						{
							strReplacement = GetArg(args, argCount++);
							if(strInput == null)
							{
								Console.WriteLine("Cannot get the 'replacement' parameter.");
								Usage();
								return 1;
							}
						}
						break;
					case "-writeto":
						{
							strWriteTo = GetArg(args, argCount++);
							if(strWriteTo == null)
							{
								Console.WriteLine("Cannot get the 'writeto' parameter.");
								Usage();
								return 1;
							}
						}
						break;
					case "-execute":
						{
							bExecute = true;
						}
						break;
					case "-nowait":
						{
							bNoWait = true;
						}
						break;
					case "-view":
						{
							bView = true;
						}
						break;
					case "-ignorecase":
						regexOptions |= RegexOptions.IgnoreCase;
						break;
					default:
						{
							Usage();
							return 1;
						}
				}
			}

			if(strInput == null || strRegex == null || strReplacement == null)
			{
				Usage();
				return 1;
			}

			Regex regex = new Regex(strRegex, regexOptions);

			if(regex.IsMatch(strInput))
			{
				string strResult = regex.Replace(strInput, strReplacement);
				Match m = Regex.Match(strInput, strRegex);
				if(bView == true)
				{
					Console.WriteLine(strResult);
				}
				if(bExecute == true)
				{
					LaunchJob(strResult, bNoWait);
				}

				if(strWriteTo != null)
				{
					try
					{
						using(StreamWriter sw = new StreamWriter(strWriteTo))
						{
							// Add some text to the file.
							sw.Write(strResult);
						}
					}
					catch(Exception ex)
					{
						Console.WriteLine("Cannot write to the file '" + strWriteTo + "'. The system reports '" + ex.Message + "'");
					}
				}
			}
			else
			{
				Console.WriteLine("No match found.");
			}

			return 0;
		}

		//-view -ignorecase -input "Z:\ShitStorm\Export\Objects\Characters\Appearance\Player\Kuro\Kuro_Skin.a3d" -regex "Z:\\ShitsToRm\\Export\\(.*)\.a3d" -replacement "Z:\shitstorm\working\360\data\image\$1.xes"

		public static bool LaunchJob(string strCommand, bool bNoWait)
		{
			try
			{
				Process p = new Process();

				p.StartInfo = new ProcessStartInfo("cmd", "/c " + strCommand);
				//p.StartInfo = new ProcessStartInfo(strCommand);
				p.StartInfo.UseShellExecute = true;
				p.StartInfo.WorkingDirectory = Directory.GetCurrentDirectory();
				
				bool bResult = p.Start();
				if(bResult == true)
				{
					if(bNoWait == false)
						p.WaitForExit();
				}
				return bResult;
			}
			catch(Exception ex)
			{
				Console.WriteLine("Cannot open the specified file. System reports:\n" + ex.Message);
				return false;
			}
		}

		private static void Usage()
		{
			Console.WriteLine(
@"RegexExecute. Usage:

Required arguments:

-input [string]         The input string.
-regex [string]         A regex used to find a match.
-replacement [string]   A replacement pattern.

Optional arguments:

-writeto [filename]     A file to write the substituted input to.
-execute                If present, RegexExecute will execute the result.
-nowait                 If present, RegexExecute will not wait until a spawned process returns.
-view                   If present, RegexExecute will echo the result.
-ignorecase             If present, the regex will not be case sensitive.

Return value:

0 for success, otherwise 1.

Example:

Input:
RegexExecute -ignorecase -input bingBangBONG -regex bing(.*)bong -replacement $1Fish -view

Output:
BangFish

");
		}


		private static string GetArg(string[] args, int count)
		{
			if(count < args.Length)
				return args[count];
			else
				return null;
		}
	}
}

/*
Z:\ShitStorm\Export\Objects\Characters\Appearance\Player\Kuro\Kuro_Skin.a3d

Z:\ShitStorm\Export\Objects\Characters\Appearance\Player\Kuro\Kuro_Skin.a3d

Z:\ShitStorm\Export\Objects\Characters\Appearance\Player\Kuro\Kuro_Skin.a3d

Z:\ShitStorm\Export\Objects\Characters\Appearance\Player\Kuro\Kuro_Skin.a3d

Z:\ShitStorm\Export\Objects\Characters\Appearance\Player\Kuro\Kuro_Skin.a3d
*/
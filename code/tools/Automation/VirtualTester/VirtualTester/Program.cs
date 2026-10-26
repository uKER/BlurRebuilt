using System;
using System.Collections.Generic;
using System.Windows.Forms;

using Bizarre.Misc;

namespace VirtualTester
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(String[] args)
        {
            String console = String.Empty;
            String game = String.Empty;
            String platform = String.Empty;
            String build = String.Empty;
			String target = String.Empty;
            bool autoStart = false;
			bool resetOnFinish = false;
			bool shutdownOnFinish = false;
			int timeInMinutes = 2;
			int timeBetweenInput = 500;

			Arguments argParser = new Arguments(args);

			if (argParser["?"] != null)
			{
				DisplayHelp();
				return;
			}

			if (argParser["autostart"] != null || (argParser["a"] != null))
			{
				autoStart = true;
			}

			if (argParser["executor"] != null)
			{
				console = argParser["build"];
			}

			if (argParser["build"] != null)
			{
				build = argParser["build"];
				if (build == "custom")
				{
					build = "Custom...";
				}
			}

			if (argParser["console"] != null)
			{
				console = argParser["console"];
			}

			if (argParser["game"] != null)
			{
				game = argParser["game"];
			}

			if (argParser["millis"] != null)
			{
				timeBetweenInput = int.Parse(argParser["millis"]);
			}

			if (argParser["m"] != null)
			{
				timeBetweenInput = int.Parse(argParser["m"]);
			}

			if (argParser["platform"] != null)
			{
				platform = argParser["platform"];
			}

			if (argParser["reset"] != null || (argParser["r"] != null))
			{
				resetOnFinish = true;
			}

			if (argParser["shutdown"] != null || (argParser["s"] != null))
			{
				shutdownOnFinish = true;
			}

			if (argParser["time"] != null)
			{
				timeInMinutes = int.Parse(argParser["time"]);
			}

			if (argParser["ti"] != null)
			{
				timeInMinutes = int.Parse(argParser["ti"]);
			}

			if (argParser["target"] != null || (argParser["ta"] != null))
			{
				if ((argParser["ta"] != null))
				{
					target = argParser["ta"];
				}
				else
				{
					target = argParser["target"];
				}
				int dot = target.IndexOf('.');
				String plat = target.Substring(dot);
				switch (plat)
				{
					case ".xex":
						platform = "XBox360";
						break;
					case ".self":
						platform = "PS3";
						break;
				}

				if (platform == "XBox360")
				{
					int endGame = target.IndexOf('\\', 4);
					game = target.Substring(4, endGame-4); //4 takes into account "xe:\"
				}
				
				build = "Custom...";
			}

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new Form1(console, game, platform, build, target, autoStart, timeInMinutes, timeBetweenInput, resetOnFinish, shutdownOnFinish));
        }

        static void DisplayHelp()
        {
            System.Windows.Forms.MessageBox.Show("Command line options:\n\n" +
                "    -?\t\t\tHelp\n" +
                "    -autostart\t\tStart testing immediately (-a)\n" +
                "    -console <console>\tSpecify which console to run on\n" +
                "    -game <game>\t\tSpecify which game to test\n" +
                "    -platform <platform>\tSpecify which platform to test on\n" +
                "    -build <build>\t\tSpecify which build to run\n" +
				"    -target <target>\t\tSpecify the game to run (Rather than specifying build/game/platform) (-ta)\n" +
				"    -time <time>\t\tSpecify the time in seconds to run the test for (-ti)\n" +
				"    -millis <milliseconds>\tSpecify the time in milliseconds to wait between button presses (-m)\n" +
				"    -restart\t\tRestart the 360 console on completion (-r)\n" +
				"    -shutdown\t\tShutdown the application on completion (-s)\n\n" +
                
                "e.g. VirtualTester -restart -shutdown -time 120 -target \"xe:\\Racer2010\\racer2010.xex\" -autostart -millis 500");
        }
    }
}
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;

using PluginInterface;

using Bizarre.Test;

using Microsoft.Win32;

/*
 * Add playing back of controls - record time interval and which control was pressed (how to cater for debug version of game and it running slower?)
*/

namespace VirtualTester
{
    public partial class Form1 : Form, IPluginHost
    {
        // This little guy is used to stop multiple things using the callback at the same time
        private readonly Object lockingObject = new Object();

        AutomationConnection automationConnection;

		AvailableButtons allAvailableButtons = new AvailableButtons();
		Dictionary<String, AvailableButtons> screenActions = new Dictionary<string, AvailableButtons>();
		List<Mode> modes = new List<Mode>();
		private Mode currentMode;

        Thread testingThread;
        ManualResetEvent eventStopTestThread = new ManualResetEvent(false);
        ManualResetEvent eventTestThreadStopped = new ManualResetEvent(false);

        GameBase testGameInfo = null;
        IPlugin testPlugin = null;

        string LoggingPath;
        //Bizarre.Automation.CustomLog DebugOutputLog;
        //Bizarre.Automation.CustomLog ControlLog;

        Bizarre.Automation.CustomLogging DebugOutputLog;
        Bizarre.Automation.CustomLogging ControlLog;
        Bizarre.Automation.CustomLogging ControlLogReplay;

        //public delegate void ControlLogDelegate(System.Drawing.Color colour, string message);
        public delegate void UpdateRunningTimeDelegate(string message);
        public delegate void UpdateTestStatusDelegate(string message);
		public delegate void UpdateCurrentScreenDelegate(string message);
		public delegate void ResetTestStateDelegate(bool enable);

        string console;

        public delegate void toolStripStartStopTesting_ClickDelegate(bool fatal);

        int msBetweenInputs;
        int totalTestTime;

        string gameName = "";
        string platformName = "";
        string buildName = "";
        string executableName = "";

        string customExeName = "";
		string customExeNamePS3 = "";

        string rootLogFolder;

		string currentScreenText = "None";

        bool buildSelectionFromCommandLine; // Used to stop us storing this selection in the Registry if it came from cmd line

        bool testPaused = false;
        bool forceRestart = false;
		bool resetOnFinish = false;
		bool shutdownOnFinish = false;

		System.Diagnostics.Stopwatch testRunTimer;

		Thread timerThread;
		bool closingDown = false;

        private PluginInterface.IPlugin[] ipi;

        public bool Register(IPlugin ipi)
        {
            return true;
        }     

        protected void ScanForPlugins()
        {
            string path = Application.StartupPath;
            String[] recognisedPlugins = { "Racer2010"};
            string[] pluginFiles = System.IO.Directory.GetFiles(path, "Plugins\\*.dll");
            ipi = new PluginInterface.IPlugin[pluginFiles.Length];

            for (int i = 0; i < pluginFiles.Length; i++)
            {
                string args = System.IO.Path.GetFileNameWithoutExtension(pluginFiles[i]);

                //Ignore dll files which are not recognised
                bool recognised = false;
                for (int r = 0; r < recognisedPlugins.Length; ++r)
                {
                    if (recognisedPlugins[r] == args)
                    {
                        recognised = true;
                        break;
                    }
                }
                if (!recognised)
                    continue;

                Type ObjType = null;
                try
                {
                    // load it
                    System.Reflection.Assembly ass = null;
                    ass = System.Reflection.Assembly.LoadFrom(pluginFiles[i]);

                    if (ass != null)
                    {
                        ObjType = ass.GetType(args + ".Plugin", true);
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
                try
                {
                    // OK Lets create the object as we have the Report Type
                    if (ObjType != null)
                    {
                        ipi[i] = (PluginInterface.IPlugin)Activator.CreateInstance(ObjType);
                        ipi[i].Host = this;
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            }
        }

        public Form1(string console, string game, string platform, string build, string target, bool autoStart, int timeInMinutes, int timeBetweenInput, bool resetOnFinish, bool shutdownOnFinish)
        {
            this.console = console;
			this.resetOnFinish = resetOnFinish;
			this.shutdownOnFinish = shutdownOnFinish;

            automationConnection = null;

			allAvailableButtons.buttons = new OptionHandler<ControllerButtonFlags>();

            InitializeComponent();

			totalTestTimeMinutes.Value = timeInMinutes;
			timeBetweenInputsMS.Value = timeBetweenInput;

            ScanForPlugins();

            {
                string myDocuments = Environment.GetFolderPath(Environment.SpecialFolder.Personal).ToString();

                this.rootLogFolder = String.Format("{0}\\Virtual Tester", myDocuments);
            }

            PopulateAvailableTestingOptions(true);

            if ((game.Length > 0) && (platform.Length > 0) && (build.Length > 0))
			{
				if (target.Length > 0)
				{
					comboBoxProject.SelectedItem = gameName;
					buildSelectionFromCommandLine = true;
					customExeName = target;
					executableName = target;
					textExecutable.Text = target;
				}
                buildSelectionFromCommandLine = true;

                foreach (string gameName in comboBoxProject.Items)
                {
                    if (gameName.ToLower() == game.ToLower())
                    {
                        comboBoxProject.SelectedItem = gameName;

                        foreach (string platformName in comboBoxPlatform.Items)
                        {
                            if (platformName.ToLower() == platform.ToLower())
                            {
                                comboBoxPlatform.SelectedItem = platformName;

                                foreach (string buildName in comboBoxBuild.Items)
                                {
                                    if (buildName.ToLower() == build.ToLower())
                                    {
                                        comboBoxBuild.SelectedItem = buildName;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
				EnableDisableBuildSelection(true);
                buildSelectionFromCommandLine = false;
				this.textExecutable.Enabled = false;
            }

			if (autoStart)
			{
				toolStripStartStopTesting.Text = "<<< PAUSE VIRTUAL TESTER >>>";
				EnableDisableManualButtons(false);
				EnableDisableBuildSelection(false);
				forceRestart = true;
				StartTestThread();
			}
			else
			{
				EnableDisableManualButtons(false);
				timeBetweenInputsMS.Enabled = true;
				totalTestTimeMinutes.Enabled = true;
				if (build == "Custom...")
				{
					textExecutable.Enabled = true;
				}
			}

			this.

			timerThread = new Thread(new ThreadStart(this.TimerThread));
			timerThread.Start();
        }

        void MyStopHandler(string message)
        {
            //ControlLogCallback(System.Drawing.Color.DarkMagenta, message);
            StopTestThread();

            this.BackColor = System.Drawing.Color.Red;
        }

        void UpdateRunningTime(string message)
        {
            if (!labelRunningTime.InvokeRequired)
            {
                labelRunningTime.Text = message;
            }
            else
            {
                labelRunningTime.BeginInvoke(new UpdateRunningTimeDelegate(UpdateRunningTime), new object[] { message });
            }
        }

        void UpdateTestStatus(string message)
        {
            if (labelStatus.Text != message)
            {
                if (!labelStatus.InvokeRequired)
                {
                    labelStatus.Text = message;
                }
                else
                {
                    labelStatus.BeginInvoke(new UpdateTestStatusDelegate(UpdateTestStatus), new object[] { message });
                }
            }
        }

		void UpdateCurrentScreen(string message)
		{
			if (labelCurrentScreen.Text != message)
			{
				if (!labelCurrentScreen.InvokeRequired)
				{
					labelCurrentScreen.Text = message;
				}
				else
				{
					labelCurrentScreen.BeginInvoke(new UpdateCurrentScreenDelegate(UpdateCurrentScreen), new object[] { message });
				}
			}
		}

        //void ControlLogCallback(System.Drawing.Color colour, string message)
        //{
        //    lock (lockingObject)
        //    {
        //        if (!richTextBox1.InvokeRequired)
        //        {
        //            richTextBox1.SelectionColor = colour;
        //            richTextBox1.AppendText(message);
        //            richTextBox1.ScrollToCaret();
        //        }
        //        else
        //        {
        //            richTextBox1.BeginInvoke(new ControlLogDelegate(ControlLogCallback), new object[] { colour, message });
        //        }
        //    }
        //}

        protected void SetupLogging()
        {
            DateTime now = DateTime.Now;
            this.LoggingPath = String.Format("{0}\\{1}\\{2}.{3:00}.{4:00}.{5:00}{6:00}.{7:00} - {8}", rootLogFolder, gameName, now.Year, now.Month, now.Day, now.Hour, now.Minute, now.Second, console);

            ControlLog = new Bizarre.Automation.CustomLogging(this.LoggingPath + "\\Controls.txt", false, true);
            //ControlLog.AddMessageCallback(new Bizarre.Automation.CustomLogging.AddMessageHandler(ControlLogCallback));

            ControlLogReplay = new Bizarre.Automation.CustomLogging(this.LoggingPath + "\\Replay.txt", false, false);
            
            DebugOutputLog = new Bizarre.Automation.CustomLogging(this.LoggingPath + "\\DebugOutput.txt", false, false);
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
			try
			{
				labelStatus.Text = "Closing...";
			}
			catch { }

			if (IsTestThreadRunning())
				StopTestThread();
			
			closingDown = true;

			try
			{
				automationConnection.Shutdown();
			}
			catch
			{
			}

            if (!buildSelectionFromCommandLine)
            {
                RegistryKey key = Registry.CurrentUser.OpenSubKey("Software\\Bizarre Creations\\Virtual Tester", true);

                // If the return value is null, the key doesn't exist
                if (key == null)
                {
                    // The key doesn't exist; create it / open it
                    key = Registry.CurrentUser.CreateSubKey("Software\\Bizarre Creations\\Virtual Tester");
                }

                key.SetValue("Project", gameName);
                key.SetValue("Platform", platformName);
                key.SetValue("Build", buildName);
                key.SetValue("CustomExe", customExeName);
				key.SetValue("CustomExePS3", customExeNamePS3);
            }
        }
        
        void EnableDisableManualButtons(bool enable)
        {
            buttonGuide.Enabled = enable;
            buttonA.Enabled = enable;
            buttonB.Enabled = enable;
            buttonX.Enabled = enable;
            buttonY.Enabled = enable;
            buttonDD.Enabled = enable;
            buttonDL.Enabled = enable;
            buttonDU.Enabled = enable;
            buttonDR.Enabled = enable;
            buttonLS.Enabled = enable;
            buttonRS.Enabled = enable;
			buttonLT.Enabled = enable;
			buttonRT.Enabled = enable;
            buttonBack.Enabled = enable;
            buttonStart.Enabled = enable;
            timeBetweenInputsMS.Enabled = enable;
            totalTestTimeMinutes.Enabled = enable;
			rectangle_RA.Enabled = enable;
			rectangle_LA.Enabled = enable;
			ovalShapeLA.Enabled = enable;
			ovalShapeRA.Enabled = enable;

            if (enable == false)
            {
                buttonGoToCurrentLog.Enabled = true;
            }
        }

        public bool IsTestThreadRunning()
        {
            return (testingThread != null && testingThread.IsAlive);
        }

        public void StartTestThread()
        {
			if ((string)comboBoxPlatform.SelectedItem == "PS3")
			{
				automationConnection = new PS3AutomationConnection((string)comboBoxProject.SelectedItem, "Virtual Tester");
				automationConnection.RegisterButtonPressHandlerPS3(this.handleAutomatedPS3ButtonPress);
			}
			else
			{
				automationConnection = new XBoxAutomationConnection((string)comboBoxProject.SelectedItem, "Virtual Tester");
				automationConnection.RegisterButtonPressHandler360(this.handleAutomated360ButtonPress);
			}

			if (console.Length > 0)
				this.Text = "Virtual Tester - " + console;
			else
				this.Text = "Virtual Tester - " + automationConnection.DefaultConsole;

			automationConnection.AddStopHandler(MyStopHandler);

			toolStripStartStopTesting.Text = "<<< PAUSE VIRTUAL TESTER >>>";

			//Set up the plugins screen handling options the first time the thread is started
            if (allAvailableButtons.buttons.GetOptions().Count == 0)
            {
                PopulateAllAvailableOptions(allAvailableButtons);
				PopulateScreenConfigurations(modes, allAvailableButtons);
				currentMode = modes[0];
            }

            eventStopTestThread.Reset();
            eventTestThreadStopped.Reset();

            msBetweenInputs = (int)timeBetweenInputsMS.Value;
            totalTestTime = (int)totalTestTimeMinutes.Value;

            //Output("Starting the Monkey...");
            testingThread = new Thread(new ThreadStart(this.TestingThread));
            testingThread.Name = "Testing Thread";
            testingThread.Start();
            //Output("Monkey Started!");
        }

		//Resume a paused thread
        public void ResumeTestThread()
        {
            if (IsTestThreadRunning())
            {
                eventStopTestThread.Reset();
                eventTestThreadStopped.Reset();
				testRunTimer.Start();
                testPaused = false;
            }
        }

		//Pause a test and stop the timer
        public bool PauseTestThread()
        {
			if (IsTestThreadRunning() && labelStatus.Text == "Running...")
            {
                // set event "Stop"
                eventStopTestThread.Set();

				testRunTimer.Stop();

				//Make sure the thread is paused (not stopped) so that the thread doen't get killed
                testPaused = true;

                toolStripStartStopTesting.Text = "<<< RESUME VIRTUAL TESTER >>>";

				return true;
            }
			return false;
        }

		//Stop a test (Can not be resumed)
        public void StopTestThread()
        {
            if (IsTestThreadRunning())
            {
                // set event "Stop"
                eventStopTestThread.Set();

				testPaused = false;

                ////        // wait when thread  will stop or finish
                while (testingThread.IsAlive)
                {
                    // We cannot use here infinite wait because our thread
                    // makes syncronous calls to main form, this will cause deadlock.
                    // Instead of this we wait for event some appropriate time
                    // (and by the way give time to worker thread) and
                    // process events. These events may contain Invoke calls.
                    if (WaitHandle.WaitAll((new ManualResetEvent[] { eventTestThreadStopped }), 100, true))
                        break;

                    Application.DoEvents();
                }

                toolStripStartStopTesting.Text = "<<< START VIRTUAL TESTER >>>";
            }
        }

        private void toolStripStartStopTesting_ClickOnOtherThread(bool fatal)
        {
            toolStripStartStopTesting_Click(null, null);

            if (fatal)
            {
                this.BackColor = System.Drawing.Color.Red;
            }
        }

		private void TimerThread()
		{
			testRunTimer = new System.Diagnostics.Stopwatch();

			while (!closingDown)
			{
				UpdateRunTime();
				System.Threading.Thread.Sleep(1000);
			}
		}

		private void ResetState()
		{
			labelRunningTime.BeginInvoke(new ResetTestStateDelegate(EnableDisableManualButtons), new object[] { true });
			labelRunningTime.BeginInvoke(new ResetTestStateDelegate(EnableDisableBuildSelection), new object[] { true });

			toolStripStartStopTesting.Text = "<<< START VIRTUAL TESTER >>>";
			StopTestThread();
		}

        private void TestingThread()
        {
            UpdateTestStatus("Connecting...");

            if (console.Length == 0)
                console = automationConnection.DefaultConsole;


            if (!automationConnection.LaunchOrConnect(forceRestart, console, executableName))
            {
                this.BeginInvoke(new toolStripStartStopTesting_ClickDelegate(toolStripStartStopTesting_ClickOnOtherThread), new object[] { true });
                UpdateTestStatus("Failed Connect");
				testPaused = false;
				TestFinished(false);
                return;
            }

			//Running a second test without restarting the app
			if (closingDown)
			{
				closingDown = false;
				timerThread = new Thread(new ThreadStart(this.TimerThread));
				timerThread.Start();
			}

            SetupLogging();

            automationConnection.SetDebugOutputLog(DebugOutputLog);
            automationConnection.SetControlLog(ControlLog);
            automationConnection.SetControlReplay(ControlLogReplay);

            automationConnection.SetDebugVariable("UI.ShowUiScreens", true);

            Random random = new Random();

            string previousScreen = "";
			string previousSubscreen = "";

            int mode = -1;

            System.Diagnostics.Stopwatch screenTimer = new System.Diagnostics.Stopwatch();
            screenTimer.Start();
			testRunTimer.Start();

            int retryCount = 0;

            while (true)
            {
                if (testPaused)
                {
                    continue;
                }
				// Check to see if we want to stop the testing thread
                if (eventStopTestThread.WaitOne(0, true))			
                {
                    eventTestThreadStopped.Set();
                    automationConnection.SetDebugOutputLog(null);
                    automationConnection.SetControlLog(null);
                    automationConnection.SetControlReplay(null);
                    return;
                }

				//check if we have hit the user-specified time limit
				if (automationConnection.Asserted || testRunTimer.Elapsed.Hours*60 + testRunTimer.Elapsed.Minutes >= totalTestTime)
				{
					//Stop the testing thread
					TestFinished(true);
				}

				int timePassed = testRunTimer.Elapsed.Minutes * 60 + testRunTimer.Elapsed.Seconds;
				if (currentMode.timeToRun != -1 && timePassed > currentMode.timeToRun)
				{
					modes.RemoveAt(0);
					if (modes.Count > 0)
					{
						currentMode = modes[0];
						if (currentMode.timeToRun != -1)
						{
							currentMode.timeToRun += timePassed; //Makes sure the time to run counts from the current elapsed times
						}
					}
					//No modes left? The test is complete.
					else
					{
						TestFinished(true);
					}
				}

                try
                {

                    string currentScreen = automationConnection.CurrentUIScreen();
					string currentSubScreen = automationConnection.CurrentUISubscreen();

					if(currentSubScreen != "")
					{
						currentScreenText = currentSubScreen;
					}
					else
					{
						currentScreenText = currentScreen;
					}

					bool randomPress = false;

					if (previousScreen != currentScreen || previousSubscreen != currentSubScreen)
					{
						// Do special logic when we first go into a screen
						ControlLog.WriteLine("Screen: " + currentScreen + " --> " + currentSubScreen);
						ControlLogReplay.WriteLine("Screen: " + currentScreen + " --> " + currentSubScreen);

						screenTimer.Reset();
						screenTimer.Start();

						previousScreen = currentScreen;
						previousSubscreen = currentSubScreen;

						automationConnection.ResetPreviousButtonState();

						if(testPlugin.HandleArteryMessages(automationConnection, currentScreen, currentSubScreen, currentMode.arteryMessages))
						{

						}
					}
					else if (testPlugin.IsSpecialScreen(currentSubScreen != "" ? currentSubScreen.ToLower() : currentScreen.ToLower()))
					{
					}
					else
					{
						if (!testPlugin.HandleScreen(automationConnection, currentScreen, currentSubScreen, currentMode.screenActions))
						{
							randomPress = true;
						}
					}

                    if (randomPress)
                    {
                        if (mode != 0)
                        {
                            ControlLog.WriteLine("Insane...");
                            mode = 0;
                        }

                        //Button mash with the default options
                        currentMode.defaultOptions.buttons.GetRandomOption();
						Press(currentMode.defaultOptions.buttons.GetSelectedOption);
                    }

                    // Reset the retry count as things have been going rather well
                    retryCount = 0;

					if (!testPaused && labelStatus.Text != "Test finished")
					{
						UpdateTestStatus("Running...");
					}
                }
                catch (Exception ex)
                {
					System.Threading.WaitHandle.WaitAll(new System.Threading.ManualResetEvent[] { new System.Threading.ManualResetEvent(false) }, 100);
					//Asserted state. Exception willbe caused because of this. So ust ignore it.
					if (automationConnection.Asserted)
					{
						return;
					}

                    const int maxRetryCount = 3;

                    //System.Diagnostics.Debug.WriteLine("Exception caught: '" + ex.Message + "'... #" + gameFreezeCounter, InfoLog.PriorityLevel.High);
                    System.Diagnostics.Debug.WriteLine("Exception caught: '" + ex.Message + "'");
					DebugOutputLog.WriteLine("Virtual Tester: " + ex.Message);

                    //ControlLog.WriteLine(System.Drawing.Color.Red, "Screen: " + currentScreen);

                    // Give the poor thing at least 5 goes at doing something before we call it a day
                    if (++retryCount > maxRetryCount)
                    {
                        ControlLog.WriteLine("Something went wrong... Retry limit reached");

                        automationConnection.SetDebugOutputLog(null);
                        automationConnection.SetControlLog(null);
                        automationConnection.SetControlReplay(null);

                        this.BeginInvoke(new toolStripStartStopTesting_ClickDelegate(toolStripStartStopTesting_ClickOnOtherThread), new object[] { true });

                        try
                        {
                            System.Media.SoundPlayer simpleSound = new System.Media.SoundPlayer(Application.StartupPath + "\\media\\found.wav");
                            simpleSound.Play();

							this.BeginInvoke(new toolStripStartStopTesting_ClickDelegate(toolStripStartStopTesting_ClickOnOtherThread), new object[] { true });

                        }
                        catch (Exception)
                        {
                        }

						DebugOutputLog.WriteLine("Virtual Tester: Connection lost and could not be reconnected.");

						UpdateTestStatus("Connection Lost");

						testRunTimer.Stop();

						TestFinished(false);

						return;
                    }
                    else
                    {
						ControlLog.WriteLine(String.Format("Something went wrong... Retry {0} of {1}", retryCount, maxRetryCount));
						UpdateTestStatus(String.Format("Retry...{0}/{1}", retryCount, maxRetryCount));
						System.Threading.WaitHandle.WaitAll(new System.Threading.ManualResetEvent[] { new System.Threading.ManualResetEvent(false) }, 5000);
                    }
                }

                System.Threading.Thread.Sleep(msBetweenInputs);

                {
					//UpdateRunTime();

					UpdateCurrentScreen(currentScreenText);
                }
            }
        }

		private void UpdateRunTime()
		{
			String runTime = "";

			if (testRunTimer.Elapsed.Hours > 0)
			{
				runTime += testRunTimer.Elapsed.Hours.ToString() + "hr ";
			}

			if (testRunTimer.Elapsed.Minutes > 0)
			{
				runTime += testRunTimer.Elapsed.Minutes.ToString() + "min ";
			}

			runTime += testRunTimer.Elapsed.Seconds.ToString() + "sec";

			UpdateRunningTime(runTime);
		}

		private void TestFinished(bool succesful)
		{
			//Try and reset the buttons
			try
			{
				automationConnection.ResetPreviousButtonState();
			}
			catch
			{
				//If connection has been lost chances are the above won't work
			}
				
			//Stop the testing thread
			testRunTimer.Stop();
			eventTestThreadStopped.Set();
			StopTestThread();
			testPaused = true;
			if (automationConnection.Asserted)
			{
				UpdateTestStatus("Assert Failure");
			}
			else if(succesful)
			{
				UpdateTestStatus("Test finished");
			}
			closingDown = true;
			if (resetOnFinish || automationConnection.Asserted)
			{
				if (automationConnection.Asserted)
				{
					DebugOutputLog.WriteLine("Could not bypass assert. Ending test.");
				}
				automationConnection.Reset();
				if (shutdownOnFinish)
				{
					Form1_FormClosing(this, null);
					Environment.Exit(0);
				}
			}
			else if (shutdownOnFinish)
			{
				automationConnection.Shutdown();
				Form1_FormClosing(this, null);
				Environment.Exit(0);
			}
			else
			{
				automationConnection.Shutdown();
				ResetState();
			}
		}

        private void toolStripStartStopTesting_Click(object sender, EventArgs e)
        {
            if (!this.InvokeRequired)
            {
                if (toolStripStartStopTesting.Text.Contains("START") || toolStripStartStopTesting.Text.Contains("RESUME"))
                {
					if (toolStripStartStopTesting.Text.Contains("START"))
					{
						testRunTimer.Reset();
					}
                    toolStripStartStopTesting.Text = "<<< PAUSE VIRTUAL TESTER >>>";
					this.BackColor = System.Drawing.SystemColors.Control;
                    EnableDisableManualButtons(false);
                    EnableDisableBuildSelection(false);

                    //Resume test
                    if (testPaused && MessageBox.Show("Continue without restarting?", "Continue?", MessageBoxButtons.YesNo) == DialogResult.Yes)
                    {
                        forceRestart = false;
                        ResumeTestThread();
                    }
                    else
                    {
						//Forced restart
                        if (testPaused)
                        {
							testRunTimer.Reset();
							forceRestart = true;
                            StopTestThread();
							StartTestThread();
                        }
						//First time setup
						else 
						{
							forceRestart = false;
							StartTestThread();
						}
                    }
                    testPaused = false;
                }
				//Pause the thread
                else
                {
					if (PauseTestThread())
					{

						testPaused = true;

						EnableDisableManualButtons(true);
						EnableDisableBuildSelection(true);

						if (sender == toolStripStartStopTesting)
						{
							UpdateTestStatus("Manual Pause");
						}
					}
                }
            }
            else
            {
                this.BeginInvoke(new toolStripStartStopTesting_ClickDelegate(toolStripStartStopTesting_ClickOnOtherThread), new object[] { false });
            }
        }

        public void PressButtong(uint pad, ControllerButtonFlags buttons)
        {
            automationConnection.PressButton(pad, buttons);
        }

		private void GetCurrentGameScreen()
		{
			currentScreenText = automationConnection.CurrentUISubscreen();
			if (currentScreenText == "")
			{
				currentScreenText = automationConnection.CurrentUIScreen();
			}
		}

        #region Manual Button Press Handlers

        private void Press(ControllerButtonFlags flags)
        {
            automationConnection.MashButton(0, flags);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonGuide_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.Xbox360_Button);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.StartButton);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonBack_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.BackButton);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonDU_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.DPadUp);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonDL_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.DPadLeft);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonDR_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.DPadRight);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonDD_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.DPadDown);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonA_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.A_Button);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonB_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.B_Button);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonX_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.X_Button);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonY_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.Y_Button);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }
        
        private void buttonLS_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.LS);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

        private void buttonRS_Click(object sender, EventArgs e)
        {
            automationConnection.PressButton(0, ControllerButtonFlags.RS);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
        }

		private void buttonLT_Click(object sender, EventArgs e)
		{
			automationConnection.PressButton(0, ControllerButtonFlags.LeftTrigger);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
		}

		private void buttonRT_Click(object sender, EventArgs e)
		{
			automationConnection.PressButton(0, ControllerButtonFlags.RightTrigger);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
		}

		private void MouseDown_LA(object sender, MouseEventArgs e)
		{
			if (e.Button != MouseButtons.Left)
			{
				return;
			}
			Microsoft.VisualBasic.PowerPacks.RectangleShape rect = (Microsoft.VisualBasic.PowerPacks.RectangleShape)sender;

			int x = 0, y = 0;

			if (automationConnection.GetType() == typeof(XBoxAutomationConnection))
			{
				x = -32767 + (int)(65536 * (1.0 / rect.Width * e.X));
				y = -32767 + (int)(65536 * (1.0 / rect.Height * e.Y));
			}
			if (automationConnection.GetType() == typeof(PS3AutomationConnection))
			{
				x = (int)(255.0 * (1.0 / rect.Width * e.X));
				y = (int)(255.0 * (1.0 / rect.Height * e.Y));
			}

			ovalShapeLA.Location = new Point(rect.Location.X + e.X - ovalShapeLA.Width/2, rect.Location.Y + (e.Y) - ovalShapeLA.Height/2);
			ovalShapeLA.Refresh();

			automationConnection.SetLeftAxis(x, y);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);
			
		}

		private void MouseDown_RA(object sender, MouseEventArgs e)
		{
			if (e.Button != MouseButtons.Left)
			{
				return;
			}
			Microsoft.VisualBasic.PowerPacks.RectangleShape rect = (Microsoft.VisualBasic.PowerPacks.RectangleShape)sender;

			int x = 0, y = 0;

			if (automationConnection.GetType() == typeof(XBoxAutomationConnection))
			{
				x = -32767 + (int)(65536 * (1.0 / rect.Width * e.X));
				y = -32767 + (int)(65536 * (1.0 / rect.Height * e.Y));
			}
			if (automationConnection.GetType() == typeof(PS3AutomationConnection))
			{
				x = 255 * (int)(1.0 / rect.Width * e.X);
				y = 255 * (int)(1.0 / rect.Height * e.Y);
			}

			ovalShapeRA.Location = new Point(rect.Location.X + e.X - ovalShapeLA.Width / 2, rect.Location.Y + (e.Y) - ovalShapeLA.Height / 2);
			ovalShapeRA.Refresh();

			automationConnection.SetRightAxis(x, y);
			GetCurrentGameScreen();
			UpdateCurrentScreen(currentScreenText);

		}

        #endregion


        public void PopulateAllAvailableOptions(AvailableButtons allButtons)
        {
            testPlugin.PopulateAllAvailableOptions(allButtons);
        }

		public void PopulateScreenConfigurations(List<Mode> modes, AvailableButtons allButtons)
		{
			testPlugin.PopulateScreenConfigurations(modes, allButtons);
		}

        protected void PopulateBuildList(string build)
        {
            comboBoxBuild.Items.Clear();

            foreach (GameBase gameInfo in GameBase.gameConfigs)
            {
                // Look for the current selection
                if (gameInfo.Name == (string)comboBoxProject.SelectedItem)
                {
                    foreach (GameBase.GameInfo gameExe in gameInfo.exeInfo)
                    {
                        string platformName = GameBase.PlatformName(gameExe.Platform);

                        if (platformName == (string)comboBoxPlatform.SelectedItem)
                        {
                            comboBoxBuild.Items.Add(gameExe.Name);
                        }
                    }
                    comboBoxBuild.Items.Add("Custom...");

                    if (comboBoxBuild.Items.Count > 0)
                    {
                        if (build.Length > 0 && comboBoxBuild.Items.Contains(build))
                            comboBoxBuild.SelectedItem = build;
                        else
                            comboBoxBuild.SelectedItem = comboBoxBuild.Items[0];
                    }
                }
            }

            comboBoxBuild.Enabled = (comboBoxBuild.Items.Count >= 2);
        }

        protected void PopulatePlatformList(string platform, string build)
        {
            comboBoxPlatform.Items.Clear();

            foreach (GameBase gameInfo in GameBase.gameConfigs)
            {
                // Look for the current selection
                if (gameInfo.Name == (string)comboBoxProject.SelectedItem)
                {
                    foreach (GameBase.GameInfo gameExe in gameInfo.exeInfo)
                    {
                        string platformName = GameBase.PlatformName(gameExe.Platform);

                        if (!comboBoxPlatform.Items.Contains(platformName))
                        {
                            comboBoxPlatform.Items.Add(platformName);
                        }
                    }
                    if (comboBoxPlatform.Items.Count > 0)
                    {
                        if (platform.Length > 0 && comboBoxPlatform.Items.Contains(platform))
                            comboBoxPlatform.SelectedItem = platform;
                        else
                            comboBoxPlatform.SelectedItem = comboBoxPlatform.Items[0];
                    }
                }
            }

            comboBoxPlatform.Enabled = (comboBoxPlatform.Items.Count >= 2);

            PopulateBuildList(build);
        }

        protected void PopulateAvailableTestingOptions(bool initial)
        {
            string project = "", platform = "", build = "";

            if (initial)
            {
                RegistryKey key = Registry.CurrentUser.OpenSubKey("Software\\Bizarre Creations\\Virtual Tester");

                if (key != null)
                {
                    if (key.GetValue("Project") != null)
                    {
                        project = (string)key.GetValue("Project");
                    }

                    if (key.GetValue("Platform") != null)
                    {
                        platform = (string)key.GetValue("Platform");
                    }

                    if (key.GetValue("Build") != null)
                    {
                        build = (string)key.GetValue("Build");
                    }

                    if (key.GetValue("CustomExe") != null)
                    {
                        customExeName = (string)key.GetValue("CustomExe");
                    }

					if (key.GetValue("CustomExePS3") != null)
					{
						customExeNamePS3 = (string)key.GetValue("CustomExePS3");
					}
                }
            }

            comboBoxProject.Items.Clear();

            foreach (GameBase gameInfo in GameBase.gameConfigs)
            {
                comboBoxProject.Items.Add(gameInfo.Name);
            }
            if (comboBoxProject.Items.Count > 0)
            {
                if (project.Length > 0 && comboBoxProject.Items.Contains(project))
                {
                    comboBoxProject.SelectedItem = project;
                }
                else
                {
                    comboBoxProject.SelectedItem = comboBoxProject.Items[0];
                }
            }

            comboBoxProject.Enabled = (comboBoxProject.Items.Count >= 2);

            PopulatePlatformList(platform, build);
        }

        private void comboBoxProject_SelectionChangeCommitted(object sender, EventArgs e)
        {
            PopulatePlatformList("", "");
        }

        private void comboBoxPlatform_SelectionChangeCommitted(object sender, EventArgs e)
        {
            PopulateBuildList("");
        }

        void EnableDisableBuildSelection(bool enable)
        {
            if (enable)
            {
                comboBoxProject.Enabled = (comboBoxProject.Items.Count >= 2);
                comboBoxPlatform.Enabled = (comboBoxPlatform.Items.Count >= 2);
                comboBoxBuild.Enabled = (comboBoxBuild.Items.Count >= 2);
				textExecutable.Enabled = true;
            }
            else
            {
                comboBoxProject.Enabled = false;
                comboBoxPlatform.Enabled = false;
                comboBoxBuild.Enabled = false;
				textExecutable.Enabled = false;
            }
        }

        private void comboBoxBuild_SelectedIndexChanged(object sender, EventArgs e)
        {
            foreach (GameBase gameInfo in GameBase.gameConfigs)
            {
                // Look for the current selection
                if (gameInfo.Name == (string)comboBoxProject.SelectedItem)
                {
                    foreach (GameBase.GameInfo gameExe in gameInfo.exeInfo)
                    {
                        string platformName = GameBase.PlatformName(gameExe.Platform);

                        if (platformName == (string)comboBoxPlatform.SelectedItem)
                        {
                            if ((string)comboBoxBuild.SelectedItem == gameExe.Name)
                            {
                                // Found the one we want
                                this.gameName = gameInfo.Name;
                                this.platformName = platformName;
                                this.buildName = gameExe.Name;
                                this.executableName = gameExe.Path;

                                this.textExecutable.Text = gameExe.Path;
                                this.textExecutable.Enabled = false;
                                
                                this.testPlugin = gameInfo.Plugin;
                                
                                return;
                            }
                        }
                    }

                    if ((string)comboBoxBuild.SelectedItem == "Custom...")
                    {
                        // Found the one we want
                        this.gameName = gameInfo.Name;
                        this.platformName = GameBase.PlatformName(GameBase.Platforms.XBox360);
                        this.buildName = "Custom...";

						if ((string)comboBoxPlatform.SelectedItem == "PS3")
						{
							this.textExecutable.Text = this.customExeNamePS3;
							this.executableName = this.customExeNamePS3;
						}
						else
						{
							this.textExecutable.Text = this.customExeName;
							this.executableName = this.customExeName;
						}
                        this.textExecutable.Enabled = true;

                        this.testPlugin = gameInfo.Plugin;
                    }
                }
            }
        }

        private void textExecutable_TextChanged(object sender, EventArgs e)
        {
            if (this.textExecutable.Enabled == true)
            {
                if ((string)comboBoxBuild.SelectedItem == "Custom...")
                {
					if ((string)comboBoxPlatform.SelectedItem == "PS3")
					{
						this.customExeNamePS3 = this.textExecutable.Text;
						this.executableName = this.customExeNamePS3;
					}
					else
					{
						this.customExeName = this.textExecutable.Text;
						this.executableName = this.customExeName;
					}
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Process.Start("explorer", this.LoggingPath);
		}

		#region Automated Button Press Handlers

		public void handleAutomated360ButtonPress(List<ControllerButtonFlags> buttonsPressed, PreviousButtonState prevButtons)
		{
			if (InvokeRequired)
			{
				this.Invoke(new PAD_BUTTON_PRESS_HANDLER(handleAutomated360ButtonPress), buttonsPressed, prevButtons);
				return;
			}
			if (testPaused)
			{
				return;
			}

			ResetButtonColours(false);

			if(buttonsPressed.Contains(ControllerButtonFlags.A_Button))
			{
				buttonA.BackColor = System.Drawing.Color.Green;
			}

			if(buttonsPressed.Contains(ControllerButtonFlags.B_Button))
			{
				buttonB.BackColor = System.Drawing.Color.Red;
			}

			if(buttonsPressed.Contains(ControllerButtonFlags.BackButton))
			{
				buttonBack.BackColor = System.Drawing.Color.White;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.DPadDown))
			{
				buttonDD.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.DPadLeft))
			{
				buttonDL.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.DPadRight))
			{
				buttonDR.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.DPadUp))
			{
				buttonDU.BackColor = System.Drawing.Color.Gray;
			}

			//if (buttonsPressed.Contains(ControllerButtonFlags.LeftThumbstickButton))
			//{
			//}

			if (buttonsPressed.Contains(ControllerButtonFlags.LeftTrigger))
			{
				buttonLT.BackColor = System.Drawing.Color.White;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.LS))
			{
				buttonLS.BackColor = System.Drawing.Color.White;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_LeftTrigger))
			{
				buttonLT.BackColor = System.Drawing.SystemColors.Control;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_RightTrigger))
			{
				buttonRT.BackColor = System.Drawing.SystemColors.Control;
			}

			//if (buttonsPressed.Contains(ControllerButtonFlags.RightThumbstickButton))
			//{
			//}

			if (buttonsPressed.Contains(ControllerButtonFlags.RightTrigger))
			{
				buttonRT.BackColor = System.Drawing.Color.White;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.RS))
			{
				buttonRS.BackColor = System.Drawing.Color.White;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.StartButton))
			{
				buttonStart.BackColor = System.Drawing.Color.White;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.X_Button))
			{
				buttonX.BackColor = System.Drawing.Color.Blue;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.Xbox360_Button))
			{
				buttonGuide.BackColor = System.Drawing.Color.Green;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.Y_Button))
			{
				buttonY.BackColor = System.Drawing.Color.Yellow;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.LeftThumbX))
			{
				int x = rectangle_LA.Location.X 
					+ rectangle_LA.Width/2 + (rectangle_LA.Width/2 * prevButtons.LeftThumbX) / short.MaxValue
					- ovalShapeLA.Width / 2;
				ovalShapeLA.Location = new Point(x, ovalShapeLA.Location.Y);
				ovalShapeLA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.LeftThumbY))
			{
				int y = rectangle_LA.Location.Y
					+ rectangle_LA.Height / 2 + (rectangle_LA.Height/2 * prevButtons.LeftThumbY) / short.MaxValue
					- ovalShapeLA.Height / 2;
				ovalShapeLA.Location = new Point(ovalShapeLA.Location.X, y);
				ovalShapeLA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.RightThumbX))
			{
				int x = rectangle_RA.Location.X
					+ rectangle_RA.Width / 2 + (rectangle_RA.Width/2 * prevButtons.RightThumbX) / short.MaxValue
					- ovalShapeRA.Width / 2;
				ovalShapeRA.Location = new Point(x, ovalShapeRA.Location.Y);
				ovalShapeRA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.RightThumbY))
			{
				int y = rectangle_RA.Location.Y
					+ rectangle_RA.Height / 2 + (rectangle_RA.Height/2 * prevButtons.RightThumbY) / short.MaxValue
					- ovalShapeRA.Height / 2;
				ovalShapeRA.Location = new Point(ovalShapeRA.Location.X, y);
				ovalShapeRA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_LeftThumbX))
			{
				int x = rectangle_LA.Location.X
					+ rectangle_LA.Width / 2
					- ovalShapeLA.Width / 2;
				ovalShapeLA.Location = new Point(x, ovalShapeLA.Location.Y);
				ovalShapeLA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_LeftThumbY))
			{
				int y = rectangle_LA.Location.Y
					+ rectangle_LA.Height / 2
					- ovalShapeLA.Height / 2;
				ovalShapeLA.Location = new Point(ovalShapeLA.Location.X, y);
				ovalShapeLA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_RightThumbX))
			{
				int x = rectangle_RA.Location.X
					+ rectangle_RA.Width / 2
					- ovalShapeRA.Width / 2;
				ovalShapeRA.Location = new Point(x, ovalShapeRA.Location.Y);
				ovalShapeRA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_RightThumbY))
			{
				int y = rectangle_RA.Location.Y
					+ rectangle_RA.Height / 2
					- ovalShapeRA.Height / 2;
				ovalShapeRA.Location = new Point(ovalShapeRA.Location.X, y);
				ovalShapeRA.Refresh();
			}
		}

		public void handleAutomatedPS3ButtonPress(List<ControllerButtonFlags> buttonsPressed, PreviousButtonState prevButtons)
		{
			if (InvokeRequired)
			{
				this.Invoke(new PAD_BUTTON_PRESS_HANDLER(handleAutomatedPS3ButtonPress), buttonsPressed, prevButtons);
				return;
			}

			if (testPaused)
			{
				return;
			}

			ResetButtonColours(false);

			if (buttonsPressed.Contains(ControllerButtonFlags.A_Button))
			{
				buttonA.BackColor = System.Drawing.Color.Blue;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.B_Button))
			{
				buttonB.BackColor = System.Drawing.Color.Red;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.BackButton))
			{
				buttonBack.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.DPadDown))
			{
				buttonDD.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.DPadLeft))
			{
				buttonDL.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.DPadRight))
			{
				buttonDR.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.DPadUp))
			{
				buttonDU.BackColor = System.Drawing.Color.Gray;
			}

			//if (buttonsPressed.Contains(ControllerButtonFlags.LeftThumbstickButton))
			//{
			//}

			if (buttonsPressed.Contains(ControllerButtonFlags.LeftTrigger))
			{
				buttonLT.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.LS))
			{
				buttonLS.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_LeftTrigger))
			{
				buttonLT.BackColor = System.Drawing.SystemColors.Control;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_RightTrigger))
			{
				buttonRT.BackColor = System.Drawing.SystemColors.Control;
			}

			//if (buttonsPressed.Contains(ControllerButtonFlags.RightThumbstickButton))
			//{
			//}

			if (buttonsPressed.Contains(ControllerButtonFlags.RightTrigger))
			{
				buttonRT.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.RS))
			{
				buttonRS.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.StartButton))
			{
				buttonStart.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.X_Button))
			{
				buttonX.BackColor = System.Drawing.Color.Pink;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.Xbox360_Button))
			{
				buttonGuide.BackColor = System.Drawing.Color.Gray;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.Y_Button))
			{
				buttonY.BackColor = System.Drawing.Color.Green;
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.LeftThumbX))
			{
				int x = rectangle_LA.Location.X
					+ rectangle_LA.Width / 2 + (rectangle_LA.Width / 2 * prevButtons.LeftThumbX) / 255
					- ovalShapeLA.Width / 2;
				ovalShapeLA.Location = new Point(x, ovalShapeLA.Location.Y);
				ovalShapeLA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.LeftThumbY))
			{
				int y = rectangle_LA.Location.Y
					+ rectangle_LA.Height / 2 + (rectangle_LA.Height / 2 * prevButtons.LeftThumbY) / 255
					- ovalShapeLA.Height / 2;
				ovalShapeLA.Location = new Point(ovalShapeLA.Location.X, y);
				ovalShapeLA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.RightThumbX))
			{
				int x = rectangle_RA.Location.X
					+ rectangle_RA.Width / 2 + (rectangle_RA.Width / 2 * prevButtons.RightThumbX) / 255
					- ovalShapeRA.Width / 2;
				ovalShapeRA.Location = new Point(x, ovalShapeRA.Location.Y);
				ovalShapeRA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.RightThumbY))
			{
				int y = rectangle_RA.Location.Y
					+ rectangle_RA.Height / 2 + (rectangle_RA.Height / 2 * prevButtons.RightThumbY) / 255
					- ovalShapeRA.Height / 2;
				ovalShapeRA.Location = new Point(ovalShapeRA.Location.X, y);
				ovalShapeRA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_LeftThumbX))
			{
				int x = rectangle_LA.Location.X
					+ rectangle_LA.Width / 2
					- ovalShapeLA.Width / 2;
				ovalShapeLA.Location = new Point(x, ovalShapeLA.Location.Y);
				ovalShapeLA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_LeftThumbY))
			{
				int y = rectangle_LA.Location.Y
					+ rectangle_LA.Height / 2
					- ovalShapeLA.Height / 2;
				ovalShapeLA.Location = new Point(ovalShapeLA.Location.X, y);
				ovalShapeLA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_RightThumbX))
			{
				int x = rectangle_RA.Location.X
					+ rectangle_RA.Width / 2
					- ovalShapeRA.Width / 2;
				ovalShapeRA.Location = new Point(x, ovalShapeRA.Location.Y);
				ovalShapeRA.Refresh();
			}

			if (buttonsPressed.Contains(ControllerButtonFlags.R_RightThumbY))
			{
				int y = rectangle_RA.Location.Y
					+ rectangle_RA.Height / 2
					- ovalShapeRA.Height / 2;
				ovalShapeRA.Location = new Point(ovalShapeRA.Location.X, y);
				ovalShapeRA.Refresh();
			}
		}

		private void ResetButtonColours(bool alsoResetPermStateButtons)
		{
			buttonA.BackColor = buttonB.BackColor = buttonBack.BackColor = buttonDD.BackColor
				= buttonDL.BackColor = buttonDR.BackColor = buttonDU.BackColor = buttonGuide.BackColor
				= buttonLS.BackColor = buttonRS.BackColor = buttonStart.BackColor = buttonX.BackColor 
				= buttonY.BackColor = System.Drawing.SystemColors.Control;

			if (alsoResetPermStateButtons)
			{
				buttonLT.BackColor = buttonRT.BackColor = System.Drawing.SystemColors.Control;
				ovalShapeLA.Location = new Point(rectangle_LA.Location.X+ rectangle_LA.Width / 2- ovalShapeLA.Width / 2,
											rectangle_LA.Location.Y + rectangle_LA.Height / 2 - ovalShapeLA.Height / 2);
				ovalShapeLA.Refresh();
				ovalShapeRA.Location = new Point(rectangle_RA.Location.X + rectangle_RA.Width / 2 - ovalShapeRA.Width / 2,
											rectangle_RA.Location.Y + rectangle_RA.Height / 2 - ovalShapeRA.Height / 2);
				ovalShapeRA.Refresh();
			}
		}

		#endregion

		private void openLogFolderToolStripMenuItem_Click(object sender, EventArgs e)
		{
			System.Diagnostics.Process.Start("explorer", rootLogFolder);
		}

		private void resetJoysticksToolStripMenuItem_Click(object sender, EventArgs e)
		{
			ovalShapeLA.Location = new Point(rectangle_LA.Location.X + rectangle_LA.Width / 2
				- ovalShapeLA.Width / 2, rectangle_LA.Location.Y + rectangle_LA.Height / 2 - ovalShapeLA.Height / 2);
			ovalShapeRA.Location = new Point(rectangle_RA.Location.X + rectangle_RA.Width / 2
				- ovalShapeRA.Width / 2, rectangle_RA.Location.Y + rectangle_RA.Height / 2 - ovalShapeRA.Height / 2);
			automationConnection.SetLeftAxis(0, 0);
			automationConnection.SetLeftAxis(0, 0);
			automationConnection.InjectButton(0, 0);
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Application.Exit();
		}
	}
}

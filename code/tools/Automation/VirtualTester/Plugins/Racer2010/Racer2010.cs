using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;

using PluginInterface;

using Bizarre.Test;

namespace Racer2010
{
    public class Plugin : PluginBase, IPlugin
    {
        private IPluginHost m_Host;

        private GameBase gameBase;

		/**
		 *	Populate the drop down box build options
		 **/
        private GameBase.GameInfo[] gameInfo = 
        {
            new GameBase.GameInfo(GameBase.Platforms.XBox360, "Local Build", "xe:\\racer2010\\racer2010.xex"),
            new GameBase.GameInfo(GameBase.Platforms.XBox360, "Release", "xe:\\racer2010\\racer2010_release.xex"),
            new GameBase.GameInfo(GameBase.Platforms.XBox360, "Speed", "xe:\\racer2010\\racer2010_speed.xex"),
			new GameBase.GameInfo(GameBase.Platforms.PS3, "Local Build", "z:\\racer2010\\image\\ps3\\racer2010.self"),
			new GameBase.GameInfo(GameBase.Platforms.PS3, "Release", "z:\\racer2010\\image\\ps3\\racer2010_release.self"),
            new GameBase.GameInfo(GameBase.Platforms.PS3, "Speed", "z:\\racer2010\\image\\ps3\\racer2010_speed.self"),
        };

        public string Name
        {
            get
            {
                return "Racer2010";
            }
        }

        public IPluginHost Host
        {
            set
            {
                m_Host = value;
                m_Host.Register(this);

                gameBase = new GameBase(this, gameInfo);
            }
        }

        public GameBase.GameInfo[] Configs
        {
            get
            {
                return gameInfo;
            }
        }

		/**
		 *	Screens placed here can use special logic which will overwrite the general settings in
		 *	PopulateScreenConfigurations.
		 **/
        public bool IsSpecialScreen(string screenName)
        {
            switch (screenName.ToLower())
            {
                case "":
                    return true;
				case "intro\\startscreen.lua":
					return false;
                default:
                    return false;
            }
        }

        /**
         *  Set up the control configurations used for different modes/screens here. Modes are esssentially
		 *  sub-tests that will run in order for a given time (Use -1 to run indefinitely).
		 *  Screens can also be set up to send artery messages when arriving on a given screen.
		 *  @param modes The list in which to store the different "modes".
		 *	@param allButtons The base buttons structure containing all the setup available buttons/controls
         * 
         **/
        public bool PopulateScreenConfigurations(List<Mode> modes, AvailableButtons allButtons)
        {
			//Probably want to avoid ever using the 360/select buttons so remove them from the allbuttons list
			RemoveButtonFromList(allButtons.buttons, ControllerButtonFlags.Xbox360_Button);
			RemoveButtonFromList(allButtons.buttons, ControllerButtonFlags.BackButton);

			//First lets create a mode to randomly navigate the interface but ensure it doesnt start a race.
			//Create a new mode and initialise it with its name and time to run. A time of -1 will run the test until the overall test time has elapsed.
			Mode uiNavigateMode = new Mode();
			uiNavigateMode.Init("UINavigate", 60);

			//Set the default buttons. These are what will be chosen from when there is no screen specific action to be taken.
			uiNavigateMode.defaultOptions = allButtons;

			//Now we create an available button options list to tell it what buttons it can press
			AvailableButtons screenButtons = new AvailableButtons();
			
			//Set it to initially have all the buttons enabled
			screenButtons.buttons = new OptionHandler<ControllerButtonFlags>(allButtons.buttons);

			//Remove the additional controls you do not want to be used by the screen
			RemoveButtonFromList(screenButtons.buttons, ControllerButtonFlags.StartButton);
			RemoveButtonFromList(screenButtons.buttons, ControllerButtonFlags.A_Button);

			//Add it do the dictionary of the mode for as many screens as you like
			uiNavigateMode.screenActions["raceworld\\groups.lua"] = screenButtons; //Careful with the form (Lower case, double backslashed and ending with .lua)

			//We need a buttons config that just presses B to back out of stuff so create a new empty buttons config and add B to it
			AvailableButtons bOnly = new AvailableButtons();
			bOnly.buttons = new OptionHandler<ControllerButtonFlags>();
			AddButtonToList(bOnly.buttons, ControllerButtonFlags.B_Button);

			uiNavigateMode.screenActions["debug\\debugscreen.lua"] = bOnly;
			uiNavigateMode.screenActions["debug\\debugcarscreen.lua"] = bOnly;
			uiNavigateMode.screenActions["debug\\debugeventscreen.lua"] = bOnly;
			uiNavigateMode.screenActions["debug\\debugcutscene.lua"] = bOnly;

			//Add it to the list of modes (The modes will be run in their list order)
			modes.Add(uiNavigateMode);

			//Now we will add a mode to start a Kanzen LA River race by going to the debug screen
			Mode kanzenRance = new Mode();
			kanzenRance.Init("KanzenLARiver", -1);

			//If we make this our default action option set we only need to worry about screens that need other input
			kanzenRance.defaultOptions = bOnly;

			//Occasionally A will need to be used to get out of popup screens so we add a new config for this
			AvailableButtons aOnly = new AvailableButtons();
			aOnly.buttons = new OptionHandler<ControllerButtonFlags>();
			AddButtonToList(aOnly.buttons, ControllerButtonFlags.A_Button);

			//Add this to the required screens
			kanzenRance.screenActions["profile\\asyncload.lua"] = aOnly;
			kanzenRance.screenActions["thecustompopup.lua"] = aOnly;

			//Getting out of the replay screen requires a right trigger press
			AvailableButtons rTOnly = new AvailableButtons();
			rTOnly.buttons = new OptionHandler<ControllerButtonFlags>();
			AddButtonToList(rTOnly.buttons, ControllerButtonFlags.RightTrigger);
			kanzenRance.screenActions["debug\\debugloadreplays.lua"] = rTOnly;

			//Once the game starts anything goes so lets add all the button options to the game screens
			kanzenRance.screenActions["ingame\\hud.lua"] = allButtons;
			kanzenRance.screenActions["ingame\\debughud.lua"] = allButtons;
			kanzenRance.screenActions["ingame\\debugcutscenehud.lua"] = allButtons;

			//The current settings will get us back to the start screen. From here we want to hit Y to get into the debug screen.
			AvailableButtons yOnly = new AvailableButtons();
			yOnly.buttons = new OptionHandler<ControllerButtonFlags>();
			AddButtonToList(yOnly.buttons, ControllerButtonFlags.Y_Button);

			kanzenRance.screenActions["intro\\startscreen.lua"] = yOnly;

			//Once we get to the debug screen we want to initiate a race. To do this we add artery messages for the screen.
			//Messages get priority over button combinations. As such they will always be sent first when a screen is entered.
			
			//Artery messages are put in a list and sent one after another.
			List<String> raceStartMessages = new List<string>();
			//Reset the race settings state
			raceStartMessages.Add("Reset");
			//3 laps
			raceStartMessages.Add("Laps 3");
			//20 vehicles
			raceStartMessages.Add("NumVehicles 20");
			//Kanzen LA River
			raceStartMessages.Add("RouteId 1292081956");
			//Setup 20 vehicles (1 player, 19 AI)
			for (uint veh = 0; veh < 20; veh++)
			{
				raceStartMessages.Add("Vehicle " + veh + " 1473652972 " + (veh == 0 ? "0 " : "1 ") + (20 - 1 - veh));
			}
			//And then start the event
			raceStartMessages.Add("StartEvent");

			//Set these messages to be sent on the debug sreen
			kanzenRance.arteryMessages.Add("debug\\debugscreen.lua", raceStartMessages);

			//And add it to our list of tasks
			modes.Add(kanzenRance);

            return true;
        }

		/**
         *	Add a list of buttons which you want to be considered available for potential use in the screen configurations.
		 *	@param allButtons The base buttons structure in which to store all of the buttons/controls which should be available
		 *	The weighting values in the AddOption method set how often a particular button or button combination will be used.
         * 
         **/
        public bool PopulateAllAvailableOptions(AvailableButtons allButtons)
        {
			OptionHandler<ControllerButtonFlags> allButtonOptions = allButtons.buttons;
            //Single Buttons
			int singleButtonWeighing = 10;
            allButtonOptions.AddOption(ControllerButtonFlags.A_Button, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.B_Button, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.X_Button, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.Y_Button, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.DPadDown, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.DPadLeft, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.DPadRight, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.DPadUp, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.LS, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.RS, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.LeftThumbstickButton, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.RightThumbstickButton, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.StartButton, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.BackButton, singleButtonWeighing);
            allButtonOptions.AddOption(ControllerButtonFlags.Xbox360_Button, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.LeftTrigger, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.RightTrigger, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.LeftThumbX, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.LeftThumbY, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.RightThumbX, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.RightThumbY, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.R_LeftTrigger, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.R_RightTrigger, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.R_LeftThumbX, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.R_LeftThumbY, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.R_RightThumbX, singleButtonWeighing);
			allButtonOptions.AddOption(ControllerButtonFlags.R_RightThumbY, singleButtonWeighing);

            //Combination of button options
            List<ControllerButtonFlags> twoButtonOptions = new List<ControllerButtonFlags>();
            List<ControllerButtonFlags> singleButtonOptions = allButtonOptions.GetOptions();
            List<ControllerButtonFlags> threeButtonOptions = new List<ControllerButtonFlags>();
			List<ControllerButtonFlags> fourButtonOptions = new List<ControllerButtonFlags>();

			//Find combinations of every 2, 3 and 4 button combination
            int count = 0;
            int count2 = 1;
			int count3 = 2;
            foreach (ControllerButtonFlags button in singleButtonOptions)
            {
                count++;
                for (int b = count; b < singleButtonOptions.Count; b++)
                {
					twoButtonOptions.Add(button | singleButtonOptions[b]);
                    count2++;
                    for (int b2 = count2; b2 < singleButtonOptions.Count; b2++)
                    {
						threeButtonOptions.Add(button | singleButtonOptions[b] | singleButtonOptions[b2]);
						count3++;
						for (int b3 = count3; b3 < singleButtonOptions.Count; b3++)
						{
							fourButtonOptions.Add(button | singleButtonOptions[b] | singleButtonOptions[b2] | singleButtonOptions[b3]);
						}
                    }
                }
            }

            //Add 2/3/4 button combinations to main button list
            allButtonOptions.AddOptions(twoButtonOptions, 5);   //Use 2 button combinations occasionally
            allButtonOptions.AddOptions(threeButtonOptions, 2); //Use 3 button combinations rarely
			allButtonOptions.AddOptions(fourButtonOptions, 1); //Use 4 button combinations very rarely

            return true;
        }
    }
}

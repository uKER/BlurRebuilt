using System;
using System.Collections.Generic;
using System.Text;

using Bizarre.Artery;

namespace Bizarre.Test
{
	/**
	 *	Contains some reusable functions that are not game specific (but can be overridden in the game plugin if required)
	 **/
	public abstract class PluginBase
	{
		/**
		 *	Given a list of messages (Single strings) this function splits them into individual words such that
		 *	the first word becomes the command and any other words become an array of values.
		 *	This information is then passed on to be sent over artery
		 **/
		public bool ProcessArteryMessages(Bizarre.Test.AutomationConnection gameAutomation, List<String> messages)
		{
			foreach (String msg in messages)
			{
				string[] words = msg.Split(' ');
				if (words.Length == 0)
				{
					continue;
				}
				if (words.Length == 1)
				{
					gameAutomation.SendCommand(words[0]);
				}
				else if (words.Length == 2)
				{
					uint value = uint.Parse(words[1]);
					gameAutomation.SendCommand(words[0], value);
				}
				else
				{
					object[] values = new object[words.Length - 1];
					for (int val = 1; val < words.Length; val++)
					{
						values[val - 1] = uint.Parse(words[val]);
					}
					gameAutomation.SendCommand(words[0], values);
				}
				System.Threading.Thread.Sleep(50);
			}
			return true;
		}

		/**
		 *  Select/hit random buttons/controls from the input optionhandler selections (Where an optionhandler defines the allowed button combinations)
		 **/
		public bool RandomButtonChoice(Bizarre.Test.AutomationConnection gameAutomation, AvailableButtons availableButtons)
		{
			gameAutomation.RandomButton(0, availableButtons.buttons);
			return true;
		}

		public bool HandleScreen(Bizarre.Test.AutomationConnection gameAutomation, string screenName, string subscreenName, Dictionary<String, AvailableButtons> screenActions)
		{
			String screen = subscreenName != "" ? subscreenName.ToLower() : screenName.ToLower();
			if (screenActions.ContainsKey(screen))
			{
				RandomButtonChoice(gameAutomation, screenActions[screen]);
				return true;
			}

			//No screen specific behaviour found;
			return false;
		}

		public bool HandleArteryMessages(Bizarre.Test.AutomationConnection gameAutomation, string screenName, string subscreenName, Dictionary<String, List<String>> arteryMessages)
		{
			String screen = subscreenName != "" ? subscreenName.ToLower() : screenName.ToLower();
			if (arteryMessages.ContainsKey(screen))
			{
				ProcessArteryMessages(gameAutomation, arteryMessages[screen]);
				return true;
			}



			return false;
		}

		public bool AddButtonToList(OptionHandler<ControllerButtonFlags> randomButtonOptions, ControllerButtonFlags button)
		{
			AddButtonToList(randomButtonOptions, button, 10);

			return true;
		}

		public bool AddButtonToList(OptionHandler<ControllerButtonFlags> randomButtonOptions, ControllerButtonFlags button, int weighting)
		{
			randomButtonOptions.AddOption(button, weighting);

			return true;
		}

		/**
         *   Set all the buttons that you do not want used in ANY combination.
         *    E.g. Removing "Y" would also remove "Y+B", "Y+Left+RightTrigger, etc).
         **/
		public bool RemoveButtonsFromList(OptionHandler<ControllerButtonFlags> randomButtonOptions, ControllerButtonFlags button)
		{
			RemoveButtonFromList(randomButtonOptions, button);

			return true;
		}

		public bool RemoveButtonFromList(OptionHandler<ControllerButtonFlags> randomButtonOptions, ControllerButtonFlags button)
		{
			//Calculate combinations which contain the button
			int buttonVal = (int)button;
			List<ControllerButtonFlags> buttonCombos = randomButtonOptions.GetOptions();
			List<ControllerButtonFlags> combosToRemove = new List<ControllerButtonFlags>();
			int optionVal;
			foreach (ControllerButtonFlags buttonCombo in buttonCombos)
			{
				optionVal = (int)buttonCombo;
				optionVal -= buttonVal;

				if (IsValidButtonCombinationValue(buttonVal, optionVal))
				{
					combosToRemove.Add(buttonCombo);
				}
			}

			//Update the list of availabble buttons
			foreach (ControllerButtonFlags remove in combosToRemove)
			{
				randomButtonOptions.RemoveOptions(remove);
			}

			return true;
		}

		public bool IsValidButtonCombinationValue(int buttonValue, int remainingValue)
		{
			if (remainingValue < 0)
			{
				return false;
			}
			else if (remainingValue == 0)
			{
				return true;
			}
			else
			{
				int count = 0;
				while (remainingValue > 0)
				{
					int val = 1;
					while (val * 2 <= remainingValue)
					{
						val *= 2;
					}
					if (val == buttonValue)
					{
						val /= 2;
					}
					remainingValue -= val;
					count++;
				}

				if (count > 3)
				{
					return false;
				}
				else
				{
					return true;
				}

			}
		}
	}
}
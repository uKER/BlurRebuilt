using System;
using System.Collections.Generic;
//using System.Text;

using Bizarre.Test;

namespace PluginInterface
{
    public interface IPluginHost
    {
        bool Register(IPlugin ipi);
    }

    public interface IPlugin
    {
        string Name { get;}
        IPluginHost Host { set;}
        //void Show();

        bool IsSpecialScreen(string screenName);

		bool HandleScreen(Bizarre.Test.AutomationConnection gameAutomation, string ScreenName, string subscreenName, Dictionary<String, AvailableButtons> screenActions);
		bool HandleArteryMessages(Bizarre.Test.AutomationConnection gameAutomation, string ScreenName, string subscreenName, Dictionary<String, List<String>> arteryMessages);

        //Set up different control configurations
		bool PopulateScreenConfigurations(List<Mode> modes, AvailableButtons allButtons);

        //Populate all the potential available options (1-3 button combinations)
        bool PopulateAllAvailableOptions (AvailableButtons allButtons);

		bool ProcessArteryMessages(Bizarre.Test.AutomationConnection gameAutomation, List<String> messages);

        bool RemoveButtonFromList(OptionHandler<ControllerButtonFlags> randomButtonOptions, ControllerButtonFlags button);
        bool IsValidButtonCombinationValue(int buttonValue, int remainingValue);

        GameBase.GameInfo[] Configs { get;}
    }

    /*
     * Name of the executable to run
     * Which keys we can and can't press
     * Unique screen logic
     */
    public class GameBase
    {
        private IPlugin plugin;

        [Flags]
        public enum Platforms
        {
            XBox360,
            PS3,
            Wii,
            PC
        }

        static public string PlatformName(Platforms platform)
        {
            switch (platform)
            {
                case GameBase.Platforms.XBox360:
                    return "XBox360";

                case GameBase.Platforms.PC:
                    return "PC";

                case GameBase.Platforms.PS3:
                    return "PS3";

                case GameBase.Platforms.Wii:
                    return "Wii";

                default:
                    System.Diagnostics.Debug.Assert(false);
                    return "Unknown";
            }
        }

        public string Name
        {
            get
            {
                return plugin.Name;
            }
        }

        public static List<GameBase> gameConfigs = new List<GameBase>();

        public class GameInfo
        {
            public GameInfo(Platforms platform, string name, string path)
            {
                this.platform = platform;
                this.name = name;
                this.path = path;
            }

            protected Platforms platform;
            protected String name;
            protected string path;          // Path to the exe on the devkit

            public string Name
            {
                get
                {
                    return name;
                }
            }

            public string Path
            {
                get
                {
                    return path;
                }
            }

            public Platforms Platform
            {
                get
                {
                    return platform;
                }
            }
        }

        public List<GameInfo> exeInfo = new List<GameInfo>();
        protected string gameName;

        public GameBase(IPlugin plugin, GameBase.GameInfo[] gameInfo)
        {
            this.plugin = plugin;

            foreach (GameInfo gi in gameInfo)
            {
                exeInfo.Add(gi);
            }

            gameConfigs.Add(this);
        }

        protected bool AddGameConfig(Platforms platform, string name, string path)
        {
            exeInfo.Add(new GameInfo(platform, name, path));

            return true;
        }

        public IPlugin Plugin
        {
            get
            {
                return plugin;
            }
        }
    }
}

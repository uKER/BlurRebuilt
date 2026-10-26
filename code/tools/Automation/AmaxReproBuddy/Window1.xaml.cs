using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using Automation;
using AmaxAutomation;
using Bizarre.Misc.LogSystem;

namespace AmaxReproBuddy
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        public Window1()
        {
            InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                string[] args = { };

                Bizarre.Artery.ArteryManager.Startup(false, "AutomationSystem");

                // Run the tests
                KitServer kitServer = new KitServer(args);

                KitServer.KitRequest kitRequest = new KitServer.KitRequest();
                kitRequest.XBOX_360 = 1;

                List<AutomationKit> kits = kitServer.LockKits(kitRequest);

                try
                {
                    WorkManager.BuildConfig buildConfig = new WorkManager.BuildConfig();
                    buildConfig.Platform360 = @"e:\amax\amax.xex";

                    for (int i = 0; i < 5; i++)
                    {
                        AmaxAutomationConnection aac = new AmaxAutomationConnection(kits[0], buildConfig);

                        bool connected = aac.AutoConnectForAutomation(false);

                        //               aac.SendCommand("StartEvent");

                        aac.ClearPadState(0);
                        aac.SelectButtonPress(0, AutomationKitButton.Button_A);
                        aac.SetPadState(0, true);

                        aac.WaitForRaceToLoad();

                        aac.WaitForTime(TimeSpan.FromSeconds(20));

                        aac.SendCommand("EndEvent");

                        aac.WaitForUiToLoad();
                    }
                }
                catch(Exception ex)
                {
                    System.Diagnostics.Debug.Assert(false, "Exception: " + ex.Message);
                }
                finally
                {
                    kitServer.UnlockKits(kits);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.Assert(false, "Exception: " + ex.Message);
            }
        }
    }
}

namespace AutomationHelper
{
    using System;
    using System.Text;

    public class DebugOutput
    {
        public static void NoWrite(string text)
        {
            Automation.WorkManager.MasterLog.WriteLine(text);
        }
    }
}

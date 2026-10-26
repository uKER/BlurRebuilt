namespace Automation
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Xml.Linq;

    using XDevkit;

    using Bizarre.Misc;

	public enum KitRequirementsStatus
	{
		Kit_NotRequired = 0,
		Kit_Required,
		Kit_Acquired,
		Kit_NotEnoughAvailable
	}

    public class KitServer
    {
        protected List<IAutomationKit> kits = new List<IAutomationKit>();

        private object lockObj = new object();

        public KitServer(string[] args)
        {
            Arguments argParser = new Arguments(args);

            // Has a kit file been specificed on the command line, if so, we use the kits from this, else we use the default kits from this PC
            if (argParser["kits"] != null)
            {
                string filename = argParser["kits"];

                XDocument kitsXml = XDocument.Load(filename);

                var kitlist = from xk in kitsXml.Descendants("kit")
                              select new
                              {
                                  Name = xk.Attribute("Name").Value,
                                  Type = xk.Attribute("Type").Value
                              };

                foreach (var k in kitlist)
                {
                    switch (k.Type.ToLower())
                    {
                        case "360":
                        case "xbox360":
                        case "xenon":
                            AddKit(AutomationKit.Create(AutomationKitType.Xbox360, k.Name));
                            break;
                        
                        case "ps3":
                            AddKit(AutomationKit.Create(AutomationKitType.PS3, k.Name));
                            break;
                        
                        case "win32":
                            AddKit(AutomationKit.Create(AutomationKitType.WIN32, k.Name));
                            break;
                        
                        case "win64":
                            AddKit(AutomationKit.Create(AutomationKitType.WIN64, k.Name));
                            break;

                        default:
                            throw new NotSupportedException();
                    }
                }
            }
            else
            {
                XboxManager xm = new XboxManager();

                AddKit(AutomationKit.Create(AutomationKitType.Xbox360, xm.DefaultConsole));

				//TODO: [PS3 SUPPORT] Handle "default" PS3 console here
            }
        }

        ~KitServer()
        {
            foreach (var k in kits)
            {
                try
                {
                    k.Disconnect();
                }
                catch
                {
                }
            }
        }

        public int TotalKits
        {
            get
            {
                return kits.Count();
            }
        }

        private bool AddKit(IAutomationKit kit)
        {
            if (kit.Connect())
            {
                kits.Add(kit);
                return true;
            }

            WorkManager.MasterLog.WriteLine(string.Format("KitServer: AddKit: '{0}' could not establish connection", kit.Name));
            
            return false;
        }

        private IEnumerable<IAutomationKit> FindAvailableKits(List<IAutomationKit> kitList, AutomationKitType type, int count)
        {
            var available = from k in kitList where (k.Type == type && k.Status == AutomationKitStatus.Available) select k;
            var capped = available.Take(count);

            return capped.AsEnumerable();
        }

        public List<KitServer.KitRequest> RequestEachAvailable(int count)
        {
            List<KitServer.KitRequest> requestList = new List<KitRequest>();

            int kit360 = kits.Count(k => k.Type == AutomationKitType.Xbox360);
            int kitPS3 = kits.Count(k => k.Type == AutomationKitType.PS3);
            int kitWIN32 = kits.Count(k => k.Type == AutomationKitType.WIN32);
            int kitWIN64 = kits.Count(k => k.Type == AutomationKitType.WIN64);

            if (kit360 > 0)
            {
                requestList.Add(new KitRequest() { XBOX_360 = count });
            }
            if (kitPS3 > 0)
            {
                requestList.Add(new KitRequest() { PS3 = count });
            }
            if (kitWIN32 > 0)
            {
                requestList.Add(new KitRequest() { WIN32 = count });
            }
            if (kitWIN64 > 0)
            {
                requestList.Add(new KitRequest() { WIN64 = count });
            }

            return requestList;
        }

        public List<AutomationKit> LockKits(KitRequest kitRequest)
        {
            while (true)
            {
                List<AutomationKit> lockedKits = new List<AutomationKit>();

                int kit360 = kits.Count(k => k.Type == AutomationKitType.Xbox360);
                int kitPS3 = kits.Count(k => k.Type == AutomationKitType.PS3);
                int kitWIN32 = kits.Count(k => k.Type == AutomationKitType.WIN32);
                int kitWIN64 = kits.Count(k => k.Type == AutomationKitType.WIN64);

				//If theres not enough resources of a kit type still try the other kit types
				KitRequirementsStatus use360 = KitRequirementsStatus.Kit_Required;
				KitRequirementsStatus usePS3 = KitRequirementsStatus.Kit_Required;
				KitRequirementsStatus useWin32 = KitRequirementsStatus.Kit_Required;
				KitRequirementsStatus useWin64 = KitRequirementsStatus.Kit_Required;

				// Check we have enough of each kit type required to complete the request for this kit
				if (kit360 == 0 || kit360 < kitRequest.XBOX_360 || kitRequest.XBOX_360 == 0)
				{
					use360 = KitRequirementsStatus.Kit_NotRequired;
				}
				if (kitPS3 == 0 || kitPS3 < kitRequest.PS3 || kitRequest.PS3 == 0)
                {
					usePS3 = KitRequirementsStatus.Kit_NotRequired;
				}
				if (kitWIN32 == 0 || kitWIN32 < kitRequest.WIN32 || kitRequest.WIN32 == 0)
				{
					useWin32 = KitRequirementsStatus.Kit_NotRequired;
				}
				if (kitWIN64 == 0 || kitWIN64 < kitRequest.WIN64 || kitRequest.WIN64 == 0)
				{
					useWin64 = KitRequirementsStatus.Kit_NotRequired;
				}

				//Cant do any tests? If not, return.
				if (use360 == KitRequirementsStatus.Kit_NotRequired && usePS3 == KitRequirementsStatus.Kit_NotRequired
					&& useWin32 == KitRequirementsStatus.Kit_NotRequired && useWin64 == KitRequirementsStatus.Kit_NotRequired)
				{
                    return null;
                }

                lock (lockObj)
                {
					IEnumerable<IAutomationKit> available360 = null, availablePS3 = null, availableWIN32 = null, availableWIN64 = null;
                    // Try and get the available kits we require
					if (use360 == KitRequirementsStatus.Kit_Required)
					{
						available360 = FindAvailableKits(kits, AutomationKitType.Xbox360, kitRequest.XBOX_360);
						if (available360.Count() >= kitRequest.XBOX_360)
						{
							use360 = KitRequirementsStatus.Kit_Acquired;
						}
					}
					if (usePS3 == KitRequirementsStatus.Kit_Required)
					{
						availablePS3 = FindAvailableKits(kits, AutomationKitType.PS3, kitRequest.PS3);
						if (availablePS3.Count() >= kitRequest.PS3)
						{
							usePS3 = KitRequirementsStatus.Kit_Acquired;
						}
					}
					if (useWin32 == KitRequirementsStatus.Kit_Required)
					{
						availableWIN32 = FindAvailableKits(kits, AutomationKitType.WIN32, kitRequest.WIN32);
						if (availableWIN32.Count() >= kitRequest.WIN32)
						{
							useWin32 = KitRequirementsStatus.Kit_Acquired;
						}
					}
					if (useWin64 == KitRequirementsStatus.Kit_Required)
					{
						availableWIN64 = FindAvailableKits(kits, AutomationKitType.WIN64, kitRequest.WIN64);
						if (availableWIN64.Count() >= kitRequest.WIN64)
						{
							useWin64 = KitRequirementsStatus.Kit_Acquired;
						}
					}

					//If all the available kits have been acquired
					if (use360 != KitRequirementsStatus.Kit_Required && usePS3 != KitRequirementsStatus.Kit_Required
						&& useWin32 != KitRequirementsStatus.Kit_Required && useWin64 != KitRequirementsStatus.Kit_Required)
					{
						if (available360 != null)
                    {
                        foreach (AutomationKit kit in available360)
                        {
                            kit.Status = AutomationKitStatus.Locked;
                            lockedKits.Add(kit);
                        }
						}
						if (availablePS3 != null)
						{
                        foreach (AutomationKit kit in availablePS3)
                        {
                            kit.Status = AutomationKitStatus.Locked;
                            lockedKits.Add(kit);
                        }
						}
						if (availableWIN32 != null)
						{
                        foreach (AutomationKit kit in availableWIN32)
                        {
                            kit.Status = AutomationKitStatus.Locked;
                            lockedKits.Add(kit);
                        }
						}
						if (availableWIN64 != null)
						{
                        foreach (AutomationKit kit in availableWIN64)
                        {
                            kit.Status = AutomationKitStatus.Locked;
                            lockedKits.Add(kit);
                        }
						}

                        System.Diagnostics.Debug.Assert(lockedKits.Count > 0, "Check we're returning at least one kit");

                        return lockedKits;
                    }
                }

                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(1));
            }
        }

        public int KitAvailable(KitRequest kitRequest)
        {
            int kit360 = kits.Count(k => k.Type == AutomationKitType.Xbox360);
            int kitPS3 = kits.Count(k => k.Type == AutomationKitType.PS3);
            int kitWIN32 = kits.Count(k => k.Type == AutomationKitType.WIN32);
            int kitWIN64 = kits.Count(k => k.Type == AutomationKitType.WIN64);

            // Check if we have enough registered kits to ever be able to do the job
            if (kitRequest.XBOX_360 > kit360 || kitRequest.PS3 > kitPS3 || kitRequest.WIN32 > kitWIN32 || kitRequest.WIN64 > kitWIN64)
                return -1;

            int readyKit360 = kits.Count(k => k.Type == AutomationKitType.Xbox360 && k.Status == AutomationKitStatus.Available);
            int readyKitPS3 = kits.Count(k => k.Type == AutomationKitType.PS3 && k.Status == AutomationKitStatus.Available);
            int readyKitWIN32 = kits.Count(k => k.Type == AutomationKitType.WIN32 && k.Status == AutomationKitStatus.Available);
            int readyKitWIN64 = kits.Count(k => k.Type == AutomationKitType.WIN64 && k.Status == AutomationKitStatus.Available);

            // Check if there's enough kits available of the type we want
            if (kitRequest.XBOX_360 > readyKit360 || kitRequest.PS3 > readyKitPS3 || kitRequest.WIN32 > readyKitWIN32 || kitRequest.WIN64 > readyKitWIN64)
                return 0;

            // This job should have the resources to run
            return 1;
        }

        public void UnlockKits(List<AutomationKit> lockedKits)
        {
            lock (lockObj)
            {
                foreach (AutomationKit kit in lockedKits)
                {
                    System.Diagnostics.Debug.Assert(kit.Status == AutomationKitStatus.Locked);

                    kit.Reset();

                    kit.Status = AutomationKitStatus.Available;
                }
            }
        }

        /// <summary>
        /// This is used to request a number of kits of potentially different types
        /// </summary>
        public class KitRequest
        {
            public KitRequest()
            {
                XBOX_360 = 0;
                PS3 = 0;
                WIN32 = 0;
                WIN64 = 0;
            }

            public int XBOX_360
            {
                get;
                set;
            }

            public int PS3
            {
                get;
                set;
            }

            public int WIN32
            {
                get;
                set;
            }

            public int WIN64
            {
                get;
                set;
            }
        }
    }
}

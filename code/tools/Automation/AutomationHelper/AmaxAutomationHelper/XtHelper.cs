namespace AmaxAutomationHelper
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using Automation;
    using AutomationHelper;

    using XtInterop;

    public class AmaxXtRecord
    {
        public AmaxXtRecord(XtRecord rec)
        {
            this.Path = rec.Name;
            this.Name = ((XtField)rec.Root.Data["Name"]).Value;
            this.Id = rec.Id;

            if (rec.Root.Data.Keys.Contains("SkipForAutomation"))
            {
                this.SkipForAutomation = ((XtField)rec.Root.Data["SkipForAutomation"]).Value == "1";
            }
            else
            {
                this.SkipForAutomation = false;
            }

            this.Record = rec;
        }

        public string Name { get; set; }
        public string Path { get; set; }

        public uint Id { get; set; }

        public bool SkipForAutomation { get; set; }
        
        public XtRecord Record { get; set; }
    }

    public class AmaxRacebookEvent : AmaxXtRecord
    {
        public AmaxRacebookEvent(XtRecord rec)
            : base(rec)
        {
        }
    }

    public class AmaxRacebookGroup : AmaxXtRecord
    {
        public int Index
        {
            get;
            set;
        }

        public AmaxRacebookGroup(XtRecord rec)
            : base(rec)
        {
        }

        public List<AmaxRacebookEvent> events = new List<AmaxRacebookEvent>();
    }

    public class AmaxCity : AmaxXtRecord
    {
        public AmaxCity(XtRecord rec)
            : base(rec)
        {
        }
    }

    public class AmaxRoute : AmaxXtRecord
    {
        public AmaxRoute(XtRecord rec)
            : base(rec)
        {
        }

        public AmaxCity City { get; set; }

        public int Index { get; set; }
    }

    public class AmaxVehicle : AmaxXtRecord
    {
        protected bool use;

        public AmaxVehicle(XtRecord rec)
            : base(rec)
        {
            this.use = rec.Root.Data.Keys.Contains("use") && (((XtField)rec.Root.Data["use"]).Value == "1");
        }

        public bool Use
        {
            get
            {
                return this.use;
            }
        }
    }

    public class AmaxXtHelper : XtHelper
    {
        private List<AmaxCity> cities = new List<AmaxCity>();
        private List<AmaxRoute> routes = new List<AmaxRoute>();

        private List<AmaxVehicle> vehicles = new List<AmaxVehicle>();
        
        private List<AmaxRacebookGroup> raceBookGroups = new List<AmaxRacebookGroup>();

        private List<AmaxXtRecord> raceBookVehicles = new List<AmaxXtRecord>();

        public List<AmaxCity> Cities
        {
            get
            {
                return this.cities;
            }
        }

        public List<AmaxRoute> Routes
        {
            get
            {
                return this.routes;
            }
        }

        public List<AmaxRacebookGroup> RacebookGroups
        {
            get
            {
                return this.raceBookGroups;
            }
        }

        public List<AmaxVehicle> Vehicles
        {
            get
            {
                return this.vehicles;
            }
        }

        public List<AmaxXtRecord> RacebookVehicles
        {
            get
            {
                return this.raceBookVehicles;
            }
        }

        public bool IsLoaded
        {
            get
            {
                return this.Db != null;
            }
        }

        public AmaxXtHelper()
        {
            if (WorkManager.XtHelper != null)
            {
                base.OpenFrom(WorkManager.XtHelper.Db);

                InitData();
            }
        }

        protected bool InitData()
        {
            List<XtRecord> cityList = FindRecordsByType("City");
            List<XtRecord> routeList = FindRecordsByType("Route");
            List<XtRecord> vehicleList = FindRecordsByType("AmaxVehicle");

            List<XtRecord> rbGroups = FindRecordsByTypeAndPack("RaceBookGroup", "FinalRacebook");
            List<XtRecord> rbEvents = FindRecordsByType("AmaxEvent");

            List<XtRecord> rbVehicles = FindRecordsByTypeAndPack("RaceBookVehicle", "FinalRacebook");

            foreach (var city in cityList)
            {
                this.cities.Add(new AmaxCity(city));
            }

            foreach (var route in routeList)
            {
                uint cityId = Convert.ToUInt32(((XtField)route.Root.Data["City"]).Value, 16);
                int routeIndex = Convert.ToInt32(((XtField)route.Root.Data["RouteNumber"]).Value, 10);

                this.routes.Add(new AmaxRoute(route)
                {
                    Index = routeIndex,
                    City = this.City(cityId)
                });
            }

            foreach (var v in vehicleList)
            {
                this.vehicles.Add(new AmaxVehicle(v));
            }

            foreach (var rbg in rbGroups)
            {
                System.Diagnostics.Debug.Assert(rbg.PackName == "FinalRacebook");

                AmaxRacebookGroup arbg = new AmaxRacebookGroup(rbg) { Index = Convert.ToInt32((rbg.Root.Data["Index"] as XtField).Value) };

                XtArray events = rbg.Root.Data["Events"] as XtArray;

                for (int i = 0; i < events.Count; i++)
                {
                    XtField xtf = events[i] as XtField;

                    uint eventId = Convert.ToUInt32(xtf.Value, 16);

                    XtRecord rec = rbEvents.Find(r => r.Id == eventId);

                    arbg.events.Add(new AmaxRacebookEvent(rec));
                }

                raceBookGroups.Add(arbg);
            }

            foreach (var rbv in rbVehicles)
            {
                System.Diagnostics.Debug.Assert(rbv.PackName == "FinalRacebook");

                raceBookVehicles.Add(new AmaxXtRecord(rbv));
            }

            return true;
        }

        public bool Init(string path)
        {
            bool ok = Open(path);

            if (ok)
            {
                InitData();
            }

            return ok;
        }

        public uint FindIdFromNameAndType(string name, string type)
        {
            if (Db != null)
            {
                var list = from records in Db.Records where (records.Type.CompareTo(type) == 0) select records;

                foreach (var record in list)
                {
                    string recordName = ((XtField)record.Root.Data["Name"]).Value;

                    if (recordName == name)
                    {
                        return record.Id;
                    }
                }
            }

            return uint.MaxValue;
        }

        public uint FindIdFromNameAndType(string name, string type, string pack)
        {
            if (Db != null)
            {
                var list = from records in Db.Records where (records.Type.CompareTo(type) == 0 && records.PackName == pack) select records;

                foreach (var record in list)
                {
                    string recordName = ((XtField)record.Root.Data["Name"]).Value;

                    if (recordName == name)
                    {
                        return record.Id;
                    }
                }
            }

            return uint.MaxValue;
        }

        public uint VehicleId(string name)
        {
            return this.FindIdFromNameAndType(name, "AmaxVehicle");
        }

        public uint RaceBookVehicleId(string name)
        {
            return this.FindIdFromNameAndType(name, "RaceBookVehicle", "FinalRacebook");
        }

        public uint CityId(string cityName)
        {
            return this.FindIdFromNameAndType(cityName, "City");
        }

        public AmaxVehicle Vehicle(uint id)
        {
            foreach (var v in this.vehicles)
            {
                if (v.Id == id)
                {
                    return v;
                }
            }

            return null;
        }

        public AmaxXtRecord RaceBookVehicle(uint id)
        {
            foreach (var v in this.raceBookVehicles)
            {
                if (v.Id == id)
                {
                    return v;
                }
            }

            return null;
        }

        public AmaxCity City(uint cityId)
        {
            foreach (var city in this.cities)
            {
                if (city.Id == cityId)
                {
                    return city;
                }
            }

            return null;
        }

        public AmaxVehicle Vehicle(string name)
        {
            return this.Vehicle(this.VehicleId(name));
        }

        public AmaxXtRecord RaceBookVehicle(string name)
        {
            return this.RaceBookVehicle(this.RaceBookVehicleId(name));
        }

        public AmaxCity City(string name)
        {
            return this.City(this.CityId(name));
        }

        public uint RouteId(string cityName, int routeIndex)
        {
            return this.RouteId(this.CityId(cityName), routeIndex);
        }
        
        public uint RouteId(uint cityId, int routeIndex)
        {
            foreach (var route in this.routes)
            {
                if ((route.City.Id == cityId) && (route.Index == routeIndex))
                {
                    return route.Id;
                }
            }

            return uint.MaxValue;
        }

        public AmaxRoute Route(uint routeId)
        {
            foreach (var route in this.routes)
            {
                if (route.Id == routeId)
                {
                    return route;
                }
            }

            return null;
        }

        public AmaxRoute Route(uint cityId, int routeIndex)
        {
            return this.Route(this.RouteId(cityId, routeIndex));
        }

        public AmaxRoute Route(string city, int routeIndex)
        {
            uint cityId = this.City(city).Id;

            return this.Route(this.RouteId(cityId, routeIndex));
        }
    }
}

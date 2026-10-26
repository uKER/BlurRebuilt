using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Runtime.Serialization.Formatters.Binary;

namespace ProfilingApplication
{
    [Serializable]
    public struct ExtraInfo
    {
        public UInt32 iData1, iData2;
    }

    /// A single uncompressed event
    [Serializable]
    public struct Event
    {
        public Event(UInt32 str, UInt32 time, Byte thread, Byte core, Byte end)
        {
            iString = str;
            iTime = time;
            iThread = thread;
            iCore = core;
            iEnd = end;
            iOriginalOrder = 0;
            iExtraInfo = new ExtraInfo[4];
        }

        public UInt32   iString;    // name of the event
        public UInt32	iTime;	    // time the event occurred
        public Byte     iThread;	// thread id of the event
        public Byte	    iCore;	    // processor id for the event
        public Byte 	iEnd;	    // 0 = start of this event, 1 = end of this event
        public ExtraInfo[] iExtraInfo;
        public int iOriginalOrder;
    };

    [Serializable]
    public struct Thread
    {
        public Thread(string name)
        {
            sName = name;
            bIdle = false;
        }

        public string sName;
        [NonSerialized()]
        public bool bIdle;
    };

    [Serializable]
    public struct Core
    {
        public Core(string name)
        {
            sName = name;
        }

        public string sName;
    };

    public struct CoreThreadInfo
    {
        public UInt32 iStart;
        public UInt32 iEnd;
        public int iThread;
        public int iStartEvent;
        public int iStartString;
    };

    public struct CoreBarsInfo
    {
        public UInt32 iStart;
        public UInt32 iEnd;
        public UInt32 iString;
        public int iThread;
    };

    /// <summary>
    ///  Contains a single frame's worth of captured profiling data
    /// </summary>
    [Serializable]
    public class CaptureData : PerfViewFile
    {
        public override void InitialiseUnserialisables()
        {
            m_ThreadEventList = new List<List<Event>>();
            m_CoreEventList = new List<List<CoreThreadInfo>>();
            m_CoreBarsList = new List<List<CoreBarsInfo>>();

            m_EventList.Sort(EventSortByOriginalOrder);

            CalculateTotalFrameTime();

            SetIdleThreadFlag();

            GenerateThreadEventLists();

            FixupEndEventStrings();

            GenerateCoreEventLists();

            GenerateCoreBarsList();

            m_EventList.Sort(EventSortByCore);

			m_bSaved = true;
        }

        public CaptureData()
        {
            m_EventList = new List<Event>();
            m_StringList = new List<string>();
            m_ThreadList = new List<Thread>();
            m_CoreList = new List<Core>();
            InitialiseUnserialisables();
        }

        private static int EventSortByOriginalOrder(Event a, Event b)
        {
            return a.iOriginalOrder - b.iOriginalOrder;
        }

        private static int EventSortByCore(Event a, Event b)
        {
            if (a.iCore != b.iCore)
                return a.iCore - b.iCore;
            if (a.iTime != b.iTime)
                return (int)a.iTime - (int)b.iTime;
            return a.iOriginalOrder - b.iOriginalOrder;
        }

        private static int EventSortByThread(Event a, Event b)
        {
            if (a.iThread != b.iThread)
                return a.iThread - b.iThread;
            if (a.iTime != b.iTime)
                return (int)a.iTime - (int)b.iTime;
            return a.iOriginalOrder - b.iOriginalOrder;
        }

        static int SortThreadListByTime(CoreThreadInfo a, CoreThreadInfo b)
        {
            return (int)a.iStart - (int)b.iStart;
        }

        private void AddThreadEventListData(int iStart, int iEnd)
        {
            m_ThreadEventList[m_EventList[iStart].iThread].AddRange(m_EventList.GetRange(iStart, iEnd-iStart));
        }

        // find the time of the last event in the capture
        private void CalculateTotalFrameTime()
        {
            UInt32 iMaxTime = 0;
            foreach(Event ev in m_EventList)
                if (ev.iTime > iMaxTime)
                    iMaxTime = ev.iTime;

            m_iTotalFrameTime = iMaxTime;
        }

        // set idle flag on thread list
        private void SetIdleThreadFlag()
        {
            for (int i = 0; i < m_ThreadList.Count; i++)
            {
                Thread thread = m_ThreadList[i];
                string s = thread.sName;
                if (s == "_SYSTEM IDLE 0" ||
                    s == "_SYSTEM IDLE 1" ||
                    s == "[Idle]")
                {
                    thread.sName = "[Idle]";
                    thread.bIdle = true;
                    m_ThreadList[i] = thread;
                }
            }
        }

        // generate a separate list of events for each thread, to be displayed in timing capture view
        private void GenerateThreadEventLists()
        {
            m_EventList.Sort(EventSortByThread);

            m_ThreadEventList.Clear();
            for (int i = 0; i < m_ThreadList.Count; i++)
                m_ThreadEventList.Add(new List<Event>());

            if (m_EventList.Count == 0)
                return;
            int iCurrentThread = m_EventList[0].iThread;
            int iStartEvent = 0;
            int iEvent;
            for (iEvent=1; iEvent < m_EventList.Count; iEvent++)
            {
                if (m_EventList[iEvent].iThread != iCurrentThread)
                {
                    AddThreadEventListData(iStartEvent, iEvent);
                    iStartEvent = iEvent;
                    iCurrentThread = m_EventList[iEvent].iThread;
                }
            }

            if (iEvent > iStartEvent)
                AddThreadEventListData(iStartEvent, iEvent);
        }
        
        // for each 'end' event, specify the parent perf string as the string instead of 0
        private void FixupEndEventStrings()
        {
            UInt32[] stack = new UInt32[32];
            foreach (List<Event> eventList in m_ThreadEventList)
            {
                int iStackCount = 0;
                stack[0] = 0xfffffffe;

                for (int i = 0; i < eventList.Count; i++)
                {
                    Event ev = eventList[i];
                    if (ev.iString == 0xffffffff)
                        continue;

                    if (ev.iEnd == 0)
                    {
						if (iStackCount<32-1)
						{
							iStackCount++;
							stack[iStackCount] = ev.iString;
						}
						else
						{
							// TODO: display a dialog box warning the user of which event is causing suspected badness
						}
                    }
                    else
                    {
                        if (iStackCount > 0)
                        {
                            iStackCount--;
                            ev.iString = stack[iStackCount];
                        }
                        else
                            ev.iString = 0xfffffffe;
                        eventList[i] = ev;
                    }
                }
            }
        }

        // add coloured bar info from a particular thread section into m_CoreBarsList
        private void AddCoreBarsFromThreadSection(int iCore, CoreThreadInfo section)
        {
            UInt32 iCurrentTime = section.iStart;
            UInt32 iCurrentString = (UInt32)section.iStartString;
            int iEvent = section.iStartEvent;
            if (m_ThreadList[section.iThread].bIdle)
                return;

            while (iEvent < m_ThreadEventList[section.iThread].Count)
            {
                Event ev = m_ThreadEventList[section.iThread][iEvent];

                if (ev.iTime>=section.iEnd)
                    break;

                // ignore thread start/stop events
                if (ev.iString != 0xffffffff)
                {
                    CoreBarsInfo barInfo;
                    barInfo.iStart = iCurrentTime;
                    barInfo.iEnd = iCurrentTime = ev.iTime;
                    barInfo.iString = iCurrentString;
                    iCurrentString = ev.iString;
                    barInfo.iThread = section.iThread;
                    m_CoreBarsList[iCore].Add(barInfo);
                }

                iEvent++;
            }

            {
                CoreBarsInfo barInfo;
                barInfo.iStart = iCurrentTime;
                barInfo.iEnd = section.iEnd;
                barInfo.iString = iCurrentString;
                barInfo.iThread = section.iThread;
                m_CoreBarsList[iCore].Add(barInfo);
            }
        }

        // generate info on what coloured bars to draw for each core in the bar-view
        private void GenerateCoreBarsList()
        {
            m_CoreBarsList.Clear();

            // for each core
            for (int iCore=0; iCore<m_CoreList.Count; iCore++)
            {
                m_CoreBarsList.Add(new List<CoreBarsInfo>());

                // walk through all thread sections and add coloured blocks
                foreach(CoreThreadInfo x in m_CoreEventList[iCore])
                    AddCoreBarsFromThreadSection(iCore, x);
            }            
        }

        // emit a core thread section
        private void FinishCoreEventListSection(Event ev, UInt32 iStartTime, UInt32 iEndTime, int iStartEvent, int iEndEvent, int iCurrentCore)
        {
            CoreThreadInfo info;
            info.iStart = (UInt32)iStartTime;
            info.iEnd = iEndTime;
            info.iThread = ev.iThread;
            info.iStartEvent = iStartEvent;

            System.Diagnostics.Debug.Assert(info.iStart<info.iEnd);

            // if there is an event before the start event, then use that for iStartString
            UInt32 iCurrentString = 0xfffffffe;
            int i = info.iStartEvent;
            while (i > 0)
            {
                i--;
                Event evx = m_ThreadEventList[info.iThread][i];
                if (evx.iString != 0xffffffff)
                {
                    iCurrentString = evx.iString;
                    break;
                }
            }
            info.iStartString = (int)iCurrentString;
            m_CoreEventList[ev.iCore].Add(info);

//            System.Diagnostics.Debug.Assert(ev.iCore == iCurrentCore);

            // PS3: correct PPU perf event core indices for PS3 using thread start/stop events
            for (i = iStartEvent; i < iEndEvent; i++)
            {
                Event tmp = m_ThreadEventList[ev.iThread][i];
                if (tmp.iString == 0xffffffff)
                    continue;

                if (iCurrentCore == 1 || iCurrentCore == 2)
                {
                    System.Diagnostics.Debug.Assert(tmp.iCore < 3);
                }

                tmp.iCore = (Byte)iCurrentCore;
                m_ThreadEventList[ev.iThread][i] = tmp;
            }
        }

        // generate a list for each core to say which threads ran on it in which order
        private void GenerateCoreEventLists()
        {
            // relies on events being sorted by thread
            m_EventList.Sort(EventSortByThread);

            m_CoreEventList.Clear();
            int i;
            for (i=0; i<m_CoreList.Count; i++)
                m_CoreEventList.Add(new List<CoreThreadInfo>());

            foreach(List<Event> eventList in m_ThreadEventList)
            {
                if (eventList.Count == 0)
                    continue;

                int iCurrentCore = eventList[0].iCore;
                int iStartEvent = 0;
                UInt32 iStartTime = 0;
                for (i = 0; i < eventList.Count; i++)
                {
                    Event ev = eventList[i];
                    if (ev.iString != 0xffffffff)
                        continue;

                    if (ev.iEnd == 0)
                    {
                        iStartTime = ev.iTime;
                        iCurrentCore = ev.iCore;
                        iStartEvent = i;
                    }
                    else
                    {
                        FinishCoreEventListSection(ev, iStartTime, ev.iTime, iStartEvent, i, iCurrentCore);
                    }
                }

                if (iStartEvent < i)
                {
                    FinishCoreEventListSection(eventList[i-1], iStartTime, m_iTotalFrameTime, iStartEvent, i-1, iCurrentCore);
                }
            }

            for (i = 0; i < m_CoreEventList.Count; i++)
            {
                // sort sections by time
                m_CoreEventList[i].Sort(SortThreadListByTime);

                // fix any overlaps. these shouldn't happen but sometimes do (corrupt PS3 trace data?)
                for(int iEvent=0; iEvent<m_CoreEventList[i].Count-1; iEvent++)
                {
                    CoreThreadInfo info = m_CoreEventList[i][iEvent];
                    if (info.iEnd>m_CoreEventList[i][iEvent+1].iStart)
                    {
                        info.iEnd = m_CoreEventList[i][iEvent+1].iStart;
                        m_CoreEventList[i][iEvent] = info;
                    }
                }                
            }
 
        }

        public void Read(Bizarre.Artery.MessageReader msgReader)
        {
            // receive events
            int nEvents;
            int iOriginalOrder = 0;
            msgReader.Read(out nEvents);
            m_EventList.Clear();
            for (int i = 0; i < nEvents; i++)
            {
                Event ev = new Event();
                ev.iExtraInfo = new ExtraInfo[4];
                msgReader.Read(out ev.iString);
                if (ev.iString != 0xfffffffe)
                {
                    msgReader.Read(out ev.iTime);
                    msgReader.Read(out ev.iThread);
                    msgReader.Read(out ev.iCore);
                    msgReader.Read(out ev.iEnd);
                    ev.iOriginalOrder = iOriginalOrder;
                    iOriginalOrder++;

                    m_EventList.Add(ev);
                }
                else
                {
                    // add extra info to the last event
                    int iType;
                    UInt32 iData1, iData2;
                    msgReader.Read(out iType);
                    msgReader.Read(out iData1);
                    msgReader.Read(out iData2);

                    m_EventList[m_EventList.Count - 1].iExtraInfo[iType].iData1 = iData1;
                    m_EventList[m_EventList.Count - 1].iExtraInfo[iType].iData2 = iData2;
                }
            }

            int iSize = (16*nEvents)/1024;

            System.Diagnostics.Debug.Write("message size = " + iSize.ToString() +"K\n");

            // receive list of strings
            int nStrings;
            msgReader.Read(out nStrings);
            m_StringList.Clear();
            for (int i = 0; i < nStrings; i++)
            {
                string s;
                msgReader.Read(out s);
                m_StringList.Add(s);
            }

            // receive list of core names
            int nCores;
            m_CoreList.Clear();
            msgReader.Read(out nCores);
            for (int i = 0; i < nCores; i++)
            {
                Core s;
                msgReader.Read(out s.sName);
                m_CoreList.Add(s);
            }

            // receive list of thread names
            int nThreads;
            m_ThreadList.Clear();
            msgReader.Read(out nThreads);
            for (int i = 0; i < nThreads; i++)
            {
                Thread s;
                msgReader.Read(out s.sName);
                s.bIdle = false;
                m_ThreadList.Add(s);
            }

			// add all cache stalls together
			for (int iCore=0; iCore<10; iCore++)
			{
				UInt32 x = 0;
				for (int iEv=0; iEv<m_EventList.Count; iEv++)
				{
					if (m_EventList[iEv].iCore != iCore)
						continue;

					x += m_EventList[iEv].iExtraInfo[0].iData1 * 13;
					m_EventList[iEv].iExtraInfo[0].iData1 = x;
				}
			}

            InitialiseUnserialisables();

			m_bSaved = false;
        }
        public string ReadString(BinaryReader binaryReader)
        {
            int length;

			length = binaryReader.ReadInt32();

			char[] charBuffer = binaryReader.ReadChars(length);
            string s = new string(charBuffer);

            return s.Substring(0, length - 1);

        }
        public void Read(Stream stream)
        {
            BinaryReader binaryReader = new BinaryReader(stream);
            // receive events
            string eventList = ReadString(binaryReader);
            int nEvents;
            int iOriginalOrder = 0;
            nEvents = binaryReader.ReadInt32();
            m_EventList.Clear();
            for (int i = 0; i < nEvents; i++)
            {
                Event ev = new Event();
                ev.iString = binaryReader.ReadUInt32();
                ev.iTime = binaryReader.ReadUInt32();
                ev.iThread = binaryReader.ReadByte();
                ev.iCore = binaryReader.ReadByte();
                ev.iEnd = binaryReader.ReadByte();
                ev.iOriginalOrder = iOriginalOrder;
                iOriginalOrder++;

                m_EventList.Add(ev);
            }

            int iSize = (16 * nEvents) / 1024;

            System.Diagnostics.Debug.Write("message size = " + iSize.ToString() + "K\n");

            // receive list of strings
            int nStrings;
            nStrings = binaryReader.ReadInt32();
            m_StringList.Clear();
            for (int i = 0; i < nStrings; i++)
            {
                string s;
                s = ReadString(binaryReader);
                m_StringList.Add(s);
            }

            // receive list of core names
            int nCores;
            m_CoreList.Clear();
            nCores = binaryReader.ReadInt32();
            for (int i = 0; i < nCores; i++)
            {
                Core s;
                s.sName = ReadString(binaryReader);
                m_CoreList.Add(s);
            }

            // receive list of thread names
            int nThreads;
            m_ThreadList.Clear();
            nThreads = binaryReader.ReadInt32();
            for (int i = 0; i < nThreads; i++)
            {
                Thread s;
                s.sName = ReadString(binaryReader);
                s.bIdle = false;
                m_ThreadList.Add(s);
            }

            InitialiseUnserialisables();

            m_bSaved = false;
        }

        // list of events ordered by core, then by time
        public List<Event> m_EventList;
        // a separate list of events for each thread, ordered by time
        [NonSerialized()]
        public List<List<Event>> m_ThreadEventList;
        // a separate list of thread start-end events for each core
        [NonSerialized()]
        public List<List<CoreThreadInfo>> m_CoreEventList;
        // a list of bars to draw for each core
        [NonSerialized()]
        public List<List<CoreBarsInfo>> m_CoreBarsList;
        // list of names of performance events
        public List<string> m_StringList;
        // list of names of threads
        public List<Thread> m_ThreadList;
        // list of names of cores
        public List<Core>   m_CoreList;
        // time of the last event in the frame
        [NonSerialized()]
        public UInt32       m_iTotalFrameTime;
    }
}

using System;
using System.Collections.Generic;
using System.Text;

using System.Windows;
using System.Windows.Controls;

using System.Collections.ObjectModel;

using System.IO;

using Bizarre.Artery;

using XDevkit;

namespace LuaDebugger
{
    public class Debugger
    {
        public static Debugger debugger = new Debugger();

        const int VERSION = 2;

        protected enum LuaTypes
        {
            None = -1,
            Nil = 0,
            Boolean,
            LightUserData,
            Number,
            String,
            Table,
            Function,
            UserData,
            Thread,
        };

        //const int LUA_TNIL = 0;
        //const int LUA_TBOOLEAN = 1;
        //const int LUA_TLIGHTUSERDATA = 2;
        //const int LUA_TNUMBER = 3;
        //const int LUA_TSTRING = 4;
        //const int LUA_TTABLE = 5;
        //const int LUA_TFUNCTION = 6;
        //const int LUA_TUSERDATA = 7;
        //const int LUA_TTHREAD = 8;

        delegate void MyLaunchDelegate();

        public event EventHandler NotifyBreakpointChange;
        public event EventHandler NotifyFileFocusChange;
        
        public event EventHandler NotifyVariableChange;

        public class LuaEntry
        {
            public string Path { get; set; }
            public string Name { get; set; }
        }

        public class LuaPath : LuaEntry
        {
            public LuaPath()
            {
                Children = new ObservableCollection<LuaEntry>();
            }

            public ObservableCollection<LuaEntry> Children { get; set; }
        }

        public class LuaRoot : LuaPath
        {
        }

        public class LuaFile : LuaEntry
        {
            public object Children { get { return null; } }
        }

        public class Stack
        {
            public string Function { get; set; }
            public string Filename { get; set; }
            public int Line { get; set; }
        };

        public ObservableCollection<Stack> callstack = new ObservableCollection<Stack>();

        public ObservableCollection<LuaPath> fileTree = new ObservableCollection<LuaPath>();

        List<TabItem> fileWindows = new List<TabItem>();

        Connection conn = null;

        System.Windows.Window myWindow;

        public class Breakpoint
        {
            public string filename;
            public bool enabled;
            public int line;
        }

        static public string LuaTypeToString(int type)
        {
            switch ((LuaTypes)type)
            {
                case LuaTypes.Nil:
                    return "nil";
                case LuaTypes.Boolean:
                    return "boolean";
                case LuaTypes.LightUserData:
                    return "lightuserdata";
                case LuaTypes.Number:
                    return "number";
                case LuaTypes.String:
                    return "string";
                case LuaTypes.Table:
                    return "table";
                case LuaTypes.Function:
                    return "function";
                case LuaTypes.UserData:
                    return "userdata";
                case LuaTypes.Thread:
                    return "thread";
                default:
                    return "unknown";
            }
        }

        protected string currentExecutionFile = "";
        protected int currentExecutionLine = -1;

        protected string codePath = "";

        List<Breakpoint> breakpoints = new List<Breakpoint>();

        public ObservableCollection<VariableInfo> localVariables = new ObservableCollection<VariableInfo>();

        public List<Breakpoint> Breakpoints
        {
            get
            {
                return breakpoints;
            }
        }

        public string CurrentExecutionFile
        {
            get
            {
                return currentExecutionFile;
            }
        }

        public int CurrentExecutionLine
        {
            get
            {
                return currentExecutionLine;
            }
        }

        public Debugger()
        {
            ArteryManager.Startup(false, "LUA Debugger");

            conn = new Connection();

            LuaRoot root = new LuaRoot() { Name = "Project", Path = codePath };

            fileTree.Add(root);
        }

        public bool ToggleBreakpoint(string shortfilename, int fileLine)
        {
            int line = fileLine + 1; // LUA counts from 1, we count from Zero

            bool found = false;

            for (int iCount = 0; iCount < breakpoints.Count; iCount++)
            {
                Breakpoint bp = breakpoints[iCount];

                if ((bp.filename.ToLower().CompareTo(shortfilename.ToLower()) == 0) && (line == bp.line))
                {
                    breakpoints.RemoveAt(iCount);
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                Breakpoint newBP = new Breakpoint();
                newBP.filename = shortfilename;
                newBP.line = line;
                newBP.enabled = true;

                breakpoints.Add(newBP);
            }

            // Tell game
            if (conn.IsConnected())
            {
                Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();

                b.Write("BREAKPOINT");
                b.Write((int) ((!found) ? 1 : 0));        // True if we're setting
                b.Write(shortfilename);
                b.Write(line);

                Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
                if (b.Build(msg))
                {
                    conn.Send(msg);
                }
            }

            if (NotifyBreakpointChange != null)
            {
                NotifyBreakpointChange(null, EventArgs.Empty);
            }

            return true;
        }

        //public void FileFocusChanged(FileForm ff)
        //{
        //    NotifyFileFocusChange(ff, EventArgs.Empty);
        //}

        public int QueryBreakpoint(string shortname, int fileLine)
        {
            int line = fileLine + 1; // LUA counts from 1, we count from Zero

            foreach (Breakpoint bp in breakpoints)
            {
                if ((bp.filename.ToLower().CompareTo(shortname.ToLower()) == 0) && (line == bp.line))
                {
                    return bp.enabled ? 1 : 2;
                }
            }

            return 0;
        }

        protected void ScanForLUAFiles_Recurse(string path, LuaPath root)
        {
            System.IO.DirectoryInfo di = new System.IO.DirectoryInfo(path);

            foreach (DirectoryInfo di2 in di.GetDirectories())
            {
                LuaPath lp = new LuaPath() { Name = di2.Name, Path=di2.FullName };

                root.Children.Add(lp);

                ScanForLUAFiles_Recurse(di2.FullName, lp);
            }

            foreach (FileInfo fi in di.GetFiles())
            {
                if (fi.Extension.ToLower().CompareTo(".lua") == 0)
                {
                    // tack this on
                    root.Children.Add(new LuaFile() { Path = fi.FullName, Name = fi.Name });
                }
            }
        }

        public void ScanForLUAFiles()
        {
            LuaRoot root = new LuaRoot() { Name = "Project", Path = codePath };

            fileTree.Clear();
            fileTree.Add(root);

            ScanForLUAFiles_Recurse(codePath, root);
        }

        public string CodePath
        {
            set
            {
                codePath = value;
                ScanForLUAFiles();
            }
            get
            {
                return codePath;
            }
        }

        void Reset()
        {
            breakpoints.Clear();
            codePath = "";
        }

        public bool LoadData(string filename)
        {
            FileStream fsDebugger = new FileStream(filename, FileMode.Open);
            BinaryReader brDebugger = new BinaryReader(fsDebugger);

            int fileVersion = brDebugger.ReadInt32();

            codePath = brDebugger.ReadString();
            ScanForLUAFiles();

            // Write out breakpoints
            {
                int numBreakpoints = brDebugger.ReadInt32();

                for (int iCount=0; iCount < numBreakpoints; iCount++)
                {
                    Breakpoint bp = new Breakpoint();

                    bp.filename = brDebugger.ReadString();
                    if (fileVersion >= 2)
                    {
                        bp.enabled = brDebugger.ReadBoolean();
                    }
                    bp.line = brDebugger.ReadInt32();

                    breakpoints.Add(bp);
                }
            }

            if (NotifyBreakpointChange != null)
            {
                NotifyBreakpointChange(null, EventArgs.Empty);
            }

            fsDebugger.Dispose();

            return true;
        }

        public bool SaveData(string filename)
        {
            FileStream fsDebugger = new FileStream(filename, FileMode.Create);
            BinaryWriter bwDebugger = new BinaryWriter(fsDebugger);

            bwDebugger.Write(VERSION);

            bwDebugger.Write(codePath);

            // Write out breakpoints
            {
                bwDebugger.Write(breakpoints.Count);

                foreach (Breakpoint bp in breakpoints)
                {
                    bwDebugger.Write(bp.filename);
                    bwDebugger.Write(bp.enabled);
                    bwDebugger.Write(bp.line);
                }
            }

            return true;
        }

        protected LuaTypes ReadVariableFromMessage(Bizarre.Artery.MessageReader msgReader, out string typeName, out string value)
        {
            int type;

            msgReader.Read(out type);

            if ((LuaTypes)type == LuaTypes.None)
            {
                typeName = "";
                value = "";
                return LuaTypes.None;
            }

            typeName = LuaTypeToString(type);

            switch ((LuaTypes)type)
            {
                case LuaTypes.Nil:
                    value = "nil";
                    break;

                case LuaTypes.Boolean:
                    {
                        int val;
                        msgReader.Read(out val);

                        value = (val == 1) ? "true" : "false";
                        break;
                    }

                case LuaTypes.Number:
                    {
                        float valf;
                        msgReader.Read(out valf);

                        value = valf.ToString();
                        break;
                    }

                case LuaTypes.String:
                    {
                        string vals;
                        msgReader.Read(out vals);

                        value = vals;
                        break;
                    }

                case LuaTypes.Function:
                    {
                        string vals;
                        msgReader.Read(out vals);

                        value = vals;
                        break;
                    }

                case LuaTypes.Table:
                    {
                        value = "";
                        break;
                    }

                default:
                    System.Diagnostics.Debug.Assert(false);
                    typeName = "unknown";
                    value = "unknown";
                    break;
            }

            return (LuaTypes)type;
        }

        protected void HandleTable(Bizarre.Artery.MessageReader msgReader, VariableInfo variable)
        {
            while (true)
            {
                string keyType, keyValue;
                string valueType, valueValue;

                if (ReadVariableFromMessage(msgReader, out keyType, out keyValue) == LuaTypes.None)
                    break;

                LuaTypes lt = ReadVariableFromMessage(msgReader, out valueType, out valueValue);

                VariableInfo child = new VariableInfo() { VariableName = keyValue, Type = valueType, Value = valueValue };

                if (lt == LuaTypes.Table)
                {
                    HandleTable(msgReader, child);
                }

                variable.Items.Add(child);
            }
        }

        protected void ProcessDebuggerMessagesCallback(Bizarre.Artery.Connection con, System.Object context)
        {
            System.Diagnostics.Debug.WriteLine("Hello"); // > " + poo);

            while (con.IsConnected())
            {
                Bizarre.Artery.Message msg = new Bizarre.Artery.Message();

                if (con.Receive(msg))
                {
                    Bizarre.Artery.MessageReader msgReader = new Bizarre.Artery.MessageReader(msg);

                    String msgString;

                    if (!msgReader.Read(out msgString))
                        continue;

                    switch (msgString)
                    {
                        case "BreakPos":                        // Game has broken - this is where we are
                            string filename;
                            int line;

                            msgReader.Read(out filename);
                            msgReader.Read(out line);

                            currentExecutionFile = filename;
                            currentExecutionLine = line - 1;

                            OpenFile(currentExecutionFile, currentExecutionLine);

                            break;

                        case "Stack":
                            
                            int versionS;

                            msgReader.Read(out versionS);

                            MyLaunchDelegate myLaunchDelegate = delegate
                            {
                                callstack.Clear();

                                string function;

                                while (msgReader.Read(out function))
                                {
                                    string filename2;
                                    int line2;

                                    msgReader.Read(out filename2);
                                    msgReader.Read(out line2);

                                    callstack.Add(new Stack() { Filename = filename2, Function = function, Line = line2 });
                                }
                            };

                            myWindow.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, myLaunchDelegate);

                            break;

                        case "Locals":

                            int version;

                            msgReader.Read(out version);

                            MyLaunchDelegate myLocalsDelegate = delegate
                        {

                            localVariables.Clear();

                            string varname;

                            while (msgReader.Read(out varname))
                            {
                                VariableInfo variable = new VariableInfo();

                                variable.VariableName = varname;

                                int type;

                                msgReader.Read(out type);

                                variable.Type = LuaTypeToString(type);

                                switch ((LuaTypes)type)
                                {
                                    case LuaTypes.Nil:
                                        variable.Value = "nil";
                                        break;

                                    case LuaTypes.Boolean:
                                        {
                                            int val;
                                            msgReader.Read(out val);

                                            variable.Value = (val == 1) ? "true" : "false";
                                            break;
                                        }

                                    case LuaTypes.Number:
                                        {
                                            float valf;
                                            msgReader.Read(out valf);

                                            variable.Value = valf.ToString();
                                            break;
                                        }

                                    case LuaTypes.String:
                                        {
                                            string vals;
                                            msgReader.Read(out vals);

                                            variable.Value = vals;
                                            break;
                                        }

                                    case LuaTypes.Function:
                                        {
                                            string vals;
                                            msgReader.Read(out vals);

                                            variable.Value = vals;
                                            break;
                                        }

                                    case LuaTypes.Table:
                                        {
                                            HandleTable(msgReader, variable);
                                            break;
                                        }

                                    default:
                                        break;
                                }

                                localVariables.Add(variable);
                            }

                            if (NotifyVariableChange != null)
                            {
                                NotifyVariableChange(null, EventArgs.Empty);
                            }
                        };

                            myWindow.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, myLocalsDelegate);

                            break;

                        //case "Automation":
                        //    HandleAutomationResponse(msgReader);
                        //    break;

                        //case "STOP":
                        //    String stopString;

                        //    if (msgReader.Read(out stopString))
                        //    {
                        //        if (StopEvent != null)
                        //        {
                        //            // Call our stop handler to deal with a stop message
                        //            StopEvent(stopString);
                        //        }
                        //    }

                        //    break;

                        default:
                            System.Diagnostics.Debug.WriteLine("msgString: " + msgString);
                            break;
                    }
                }
            }
        }

        public bool SendCommandViaArtery(string command)
        {
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();

            b.Write(command);

            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                conn.Send(msg);

                return true;
            }

            return false;
        }

        public void Connect(System.Windows.Window w)
        {
            myWindow = w;

            if (!conn.IsConnected())
            {
                XboxManager xboxManager = new XboxManager();
                XboxConsole xboxConsole = xboxManager.OpenConsole(xboxManager.DefaultConsole);

                SockAddr target = new Bizarre.Artery.SockAddr(xboxConsole.IPAddressTitle, 6317);

                ArteryManager.Connect(conn, target, "LUA Debugger", ProcessDebuggerMessagesCallback, 0);
            }
        }

        protected void ClearCurrentFile()
        {
            string oldFile = currentExecutionFile;

            currentExecutionFile = "";

            MyLaunchDelegate myLaunchDelegate = delegate
            {
                (myWindow as Window1).ClearCurrentFile(oldFile);
            };

            myWindow.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, myLaunchDelegate);
        }

        public void BreakAll()
        {
            if (conn.IsConnected())
            {
                SendCommandViaArtery("BREAK");
            }
        }

        public void StepInto()
        {
            ClearCurrentFile();

            if (conn.IsConnected())
            {
                SendCommandViaArtery("STEPINTO");
            }
        }

        public void StepOver()
        {
            ClearCurrentFile();

            if (conn.IsConnected())
            {
                SendCommandViaArtery("STEPOVER");
            }
        }

        public void StepOut()
        {
            ClearCurrentFile();

            if (conn.IsConnected())
            {
                SendCommandViaArtery("STEPOUT");
            }
        }

        public void Go()
        {
            ClearCurrentFile();

            if (conn.IsConnected())
            {
                SendCommandViaArtery("GO");
            }
        }
        
        public void Test(string testString)
        {
            if (conn.IsConnected())
            {
                SendCommandViaArtery(testString);
            }
        }

        //public delegate void showWindowDelegate(FileForm eff);
        //public delegate void showWindowLineDelegate(FileForm eff, int line);
        //public delegate void openFileDelegate(string shortfilename);
        //public delegate void openFileLineDelegate(string shortfilename, int line);

        //FileForm TryAndOpenFileForm(string shortfilename)
        //{
        //    string fullfilename = Form1.debugger.CodePath + "\\" + shortfilename;

        //    if (System.IO.File.Exists(fullfilename))
        //    {
        //        return new FileForm(shortfilename);
        //    }

        //    System.Diagnostics.Debug.WriteLine("ERROR: Unable to find file '" + fullfilename + "' to open.");
        //    return null;
        //}

        //protected void ShowWindowFromOtherThread(FileForm eff)
        //{
        //    eff.Show();
        //    eff.textView.Invalidate();
        //}

        //protected void ShowWindowLineFromOtherThread(FileForm eff, int line)
        //{
        //    eff.Show();
        //    eff.GotoLine(line);
        //    eff.textView.Invalidate();
        //}

        //protected void OpenFileFromOtherThread(string shortfilename)
        //{
        //    FileForm ff = TryAndOpenFileForm(shortfilename);
        //    if (ff != null)
        //    {
        //        ff.Show(Program.mainForm.GetDockPanel());

        //        fileWindows.Add(ff);
        //    }
        //}
        
        //protected void OpenFileLineFromOtherThread(string shortfilename, int line)
        //{
        //    FileForm ff = TryAndOpenFileForm(shortfilename);
        //    if (ff != null)
        //    {
        //        ff.Show(Program.mainForm.GetDockPanel());

        //        ff.GotoLine(line);

        //        fileWindows.Add(ff);
        //    }
        //}

        public void OpenFile(string shortfilename)
        {
            OpenFile(shortfilename, 0);
        }

        public void OpenFile(string shortfilename, int line)
        {
            MyLaunchDelegate myLaunchDelegate = delegate
            {
                (myWindow as Window1).OpenFile(shortfilename, line);
            };

            myWindow.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, myLaunchDelegate);
        }

        //public void CloseWindow(string shortfilename)
        //{
        //    foreach (FileForm eff in fileWindows)
        //    {
        //        if (eff.ShortFilename.ToLower().CompareTo(shortfilename.ToLower()) == 0)
        //        {
        //            fileWindows.Remove(eff);
        //            return;
        //        }
        //    }
        //}
    }
}

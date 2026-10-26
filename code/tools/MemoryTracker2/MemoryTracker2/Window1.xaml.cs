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
using System.IO;
using System.Reflection;
using System.Text.RegularExpressions;

using System.Net;

using System.Globalization;

using System.Windows.Threading;

using System.Collections;
using System.Collections.Specialized;

using System.ComponentModel;
using System.Collections.ObjectModel;

using System.Runtime.InteropServices; // DllImport

using XDevkit;

namespace MemoryTracker2
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        private TrackerState trackerState = new TrackerState();

        static public ObservableCollection<ContextNode> contextNodeList;
        static public List<ContextNode> contextNodeList2;
        static public List<int> contextNodeSortList;

        static public List<string> heapNames = new List<string>();

        static private ObservableCollection<MemAllocItem> tempMemoryAllocList;

        static private ObservableCollection<MemAllocItem> memoryAllocList;

        private Bizarre.Artery.Connection consoleConnection;

        private System.Threading.Thread maintainConnectionThread;
        private bool connected;

        private System.Threading.ManualResetEvent shuttingDown = new System.Threading.ManualResetEvent(false);

        private uint m_customIP;
        private uint m_customPort;
        private uint m_currentIP;
        private uint m_currentPort;

        private uint m_selection_MemoryNeeded, m_selection_MemoryWasted;

        public Window1()
        {
            InitializeComponent();

            contextNodeList = new ObservableCollection<ContextNode>();
            contextNodeList2 = new List<ContextNode>();
            contextNodeSortList = new List<int>();

            memoryAllocList = new ObservableCollection<MemAllocItem>();

            ContextTree.DataContext = contextNodeList;
            CurrentTagLabel.DataContext = trackerState;

            AllocListView.DataContext = memoryAllocList;

            int retVal = DbgHelp.Initialise();
            if (retVal != 0)
            {
                MessageBox.Show("DbgHelp.Initialise failed with error(" + retVal + ")", "DbgHelp.Initialise", MessageBoxButton.OK, MessageBoxImage.Error);
            }

            m_selection_MemoryNeeded = m_selection_MemoryWasted = 0;

            m_customIP = 0;
            m_customPort = 0;

            //maintainConnectionThread = new System.Threading.Thread(WorkerThread_MaintainConnection);
            Connect();

            AddSortBinding();
        }

        public void Connect()
        {
            if (maintainConnectionThread != null)
            {
                if (maintainConnectionThread.IsAlive)
                    maintainConnectionThread.Abort();

                while (maintainConnectionThread.IsAlive) ;

                maintainConnectionThread = null;
            }

            XboxManagerClass xboxManager = new XboxManagerClass();
            XboxConsole xboxConsole = xboxManager.OpenConsole(xboxManager.DefaultConsole);

            this.Title = "MemAnalyse v2";
            if (m_customIP == 0)
            {
                this.Title += " (" + xboxManager.DefaultConsole + ")";
                try
                {
                    m_currentIP = xboxConsole.IPAddressTitle;
                }
                catch
                {
                    System.Diagnostics.Debug.WriteLine("Connect: Could not find console '" + xboxManager.DefaultConsole + "' so couldn't work out IP.");
                    m_currentIP = 0;
                }
                m_currentPort = 6317;
            }
            else
            {
                this.Title += Convert.ToString(" Custom IP ");
                m_currentIP = m_customIP;
                m_currentPort = m_customPort;
            }

            maintainConnectionThread = new System.Threading.Thread(WorkerThread_MaintainConnection);
            // Kick off a thread to manage our connection to the console
            maintainConnectionThread.Start();
        }

        public delegate void MyDelegateType();

        private void RebuildContextNodeTree()
        {
            ContextNode cnTreeRoot = new ContextNode();
            cnTreeRoot.ShortName = "Root";
            cnTreeRoot.ParentIndex = -1;
            cnTreeRoot.Expanded = true;
            contextNodeList2.Add(cnTreeRoot);

            for (int i = 0; i < contextNodeSortList.Count; i++)
            {
                int sortedIndex = (int)contextNodeSortList[i];
                ContextNode node = (ContextNode)contextNodeList2[sortedIndex];

                if (node.ParentIndex != -1)
                {
                    node.ShortName = node.FullName.Substring(contextNodeList2[node.ParentIndex].FullName.Length + 1);
                    contextNodeList2[node.ParentIndex].AddChild(node);
                }
                else
                {
                    node.ShortName = node.FullName;
                    cnTreeRoot.AddChild(node);
                    node.ParentIndex = contextNodeList2.Count - 1;
                }

                node.Expanded = false;
            }

            MyDelegateType myDelegateAdd = delegate
            {
                contextNodeList.Clear();
                contextNodeList.Add(cnTreeRoot);
            };
            this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, myDelegateAdd);
        }

        private void Callback_UserMsg(Bizarre.Artery.Message msg)
        {
            Bizarre.Artery.MessageReader msgReader = new Bizarre.Artery.MessageReader(msg);

            String text;
            msgReader.Read(out text);

            if (text == "Tag")
            {
                int tag;
                msgReader.Read(out tag);

                trackerState.TagValue = tag;
            }
            else if (text == "AddModule")
            {
                uint moduleBaseAddr;
                uint moduleSize;

                msgReader.Read(out moduleBaseAddr);
                msgReader.Read(out moduleSize);

                String moduleName;
                msgReader.Read(out moduleName);

                System.Diagnostics.Debug.WriteLine("AddModule - " + moduleName);

                Int32 pdbInfoSize;
                msgReader.Read(out pdbInfoSize);

                byte[] buffer = new byte[pdbInfoSize];

                msgReader.Read(buffer, pdbInfoSize);

                bool loadOk = DbgHelp.LoadSymbolsForModule(moduleBaseAddr, moduleSize, buffer, pdbInfoSize);
                //if (!loadOk)
                //    MessageBox.Show("DbgHelp.LoadSymbolsForModule failed", "AddModule 1", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            else if (text == "BeginMemDump")
            {
                heapNames.Clear();
                heapNames.Add("Main");
            }
            else if (text == "Heap")
            {
                int index;
                string name;

                msgReader.Read(out index);
                msgReader.Read(out name);

                heapNames.Add(name);

                MyDelegateType myAddNameToHeaps = delegate
                {
                    HeapCombo.Items.Add(name);
                };
                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, myAddNameToHeaps);
            }
            else if (text == "ContextNodes")
            {
                int nodeCount;

                msgReader.Read(out nodeCount);

                contextNodeList2.Clear();
                contextNodeSortList.Clear();

                for (int iCount = 0; iCount < nodeCount; iCount++)
                {
                    ContextNode node = new ContextNode();

                    string fullname;
                    int parentIndex;

                    msgReader.Read(out fullname);
                    msgReader.Read(out parentIndex);

                    node.FullName = fullname;
                    node.ParentIndex = parentIndex;

                    contextNodeList2.Add(node);
                }

                // We now get a list of indices which is the context node's sorted into name order
                for (int iCount = 0; iCount < nodeCount; iCount++)
                {
                    int val;
                    msgReader.Read(out val);
                    contextNodeSortList.Add(val);
                }

                RebuildContextNodeTree();
            }
            else if (text == "AllocStart")
            {
                int flags;
                msgReader.Read(out flags);

                int numItems;
                msgReader.Read(out numItems);

                MyDelegateType mySetProgressMax = delegate
                {
                    StatusProgressBar.Maximum = numItems;
                    StatusProgressBar.Value = 0;

                    StatusProgressBar.Visibility = Visibility.Visible;

                    StatusLabel.Text = "0 / " + numItems.ToString();
                };
                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, mySetProgressMax);

                System.Diagnostics.Trace.WriteLine("About to receive information on " + numItems.ToString() + " allocations.");

                tempMemoryAllocList = new ObservableCollection<MemAllocItem>();
            }
            else if (text == "AllocStop")
            {
                if (!DbgHelp.bInitialised)
                {
                    DbgHelp.bInitialised = true;
                }

                MyDelegateType myDelAddAllocItem = delegate
                {
                    memoryAllocList = tempMemoryAllocList;
                    tempMemoryAllocList = null;

                    AllocListView.DataContext = memoryAllocList;

                    StatusLabel.Text = "Ready";
                    StatusProgressBar.Visibility = Visibility.Collapsed;

                    UpdateMemoryStats();
                };
                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, myDelAddAllocItem);
            }
            else if (text == "Alloc")
            {
                Int32 allocCount;
                Int32 version;

                msgReader.Read(out allocCount);

                msgReader.Read(out version);

                while (true)
                {
                    MemAllocItem info = new MemAllocItem();

                    int heapId;

                    if (!msgReader.Read(out heapId))
                        break;

                    int id;

                    if (!msgReader.Read(out id))
                        break;

                    int allocSize, waste, attribs, tag, csdepth;
                    uint address;
                    byte isInSmallBlock;

                    msgReader.Read(out allocSize);

                    if (version >= 2)
                    {
                        msgReader.Read(out address);
                    }
                    else
                    {
                        address = 0;
                    }

                    msgReader.Read(out waste);
                    msgReader.Read(out attribs);
                    msgReader.Read(out tag);

                    if (version >= 2)
                    {
                        msgReader.Read(out isInSmallBlock);
                    }
                    else
                    {
                        isInSmallBlock = 0;
                    }

                    msgReader.Read(out csdepth);

                    ContextNode cn = contextNodeList2[id];
                    cn.AllocationCount++;
                    if (cn.ParentIndex != -1)
                    {
                        do
                        {
                            cn = contextNodeList2[cn.ParentIndex];
                            cn.SubAllocationCount++;
                        } while (cn.ParentIndex != -1);
                    }

                    info.ContextID = id;
                    info.AllocationSize = allocSize;
                    info.AllocAddress = address;
                    info.AllocationWaste = waste;
                    info.Attribs = attribs;
                    info.IsInSmallBlock = (isInSmallBlock != 0);
                    info.Tag = tag;
                    info.Heap = heapId;
                    info.CallStackDepth = csdepth;

                    if (csdepth > 0)
                    {
                        NameResolutionItem[] cstack = new NameResolutionItem[csdepth];
                        byte[] byteStack = new byte[csdepth * 4];

                        msgReader.Read(byteStack, csdepth * 4);

                        for (int iStack = 0; iStack < csdepth; iStack++)
                        {
                            cstack[iStack] = Bizarre2.Helper.GetNRIFromCode((uint)IPAddress.NetworkToHostOrder(BitConverter.ToInt32(byteStack, iStack * 4)));
                        }

                        info.CallStackNRI = cstack;
                    }
                    else
                    {
                        info.CallStackNRI = null;
                    }

                    tempMemoryAllocList.Add(info);
                }

                if (tempMemoryAllocList.Count % 40 == 0)
                {
                    MyDelegateType myUpdateProgress = delegate
                    {
                        StatusProgressBar.Value = Math.Min(tempMemoryAllocList.Count, StatusProgressBar.Maximum);
                        StatusLabel.Text = tempMemoryAllocList.Count.ToString() + " / " + StatusProgressBar.Maximum.ToString();
                    };
                    this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, myUpdateProgress);
                }
            }
        }

        public void MyConnectCallback(Bizarre.Artery.Connection con, System.Object context)
        {
            try
            {
                if (con.IsConnected())
                {
                    connected = true;
                    //mainForm.Invoke(new UpdateConnectedStatusDelegate(UpdateConnectedStatus), new object[] { true });

                    while (con.IsConnected())
                    {
                        Bizarre.Artery.Message msg = new Bizarre.Artery.Message();

                        if (con.Receive(msg))
                        {
                            Callback_UserMsg(msg);
                        }
                    }
                }

                connected = false;
                //mainForm.Invoke(new UpdateConnectedStatusDelegate(UpdateConnectedStatus), new object[] { false });
            }
            catch
            {
            }
        }

        delegate void UpdateStatusDelegate(bool connected);

        protected void UpdateStatus(bool connected)
        {
            if (connected)
            {
                ConnectStatusText.Text = "Connected To 360Sausage";
                ConnectStatusIcon.Fill = new SolidColorBrush(Color.FromArgb(255, 0, 255, 0));
            }
            else
            {
                ConnectStatusText.Text = "Connecting To 360Sausage";
                ConnectStatusIcon.Fill = new SolidColorBrush(Color.FromArgb(255, 255, 0, 0));
            }
        }

        protected void WorkerThread_MaintainConnection()
        {
            try
            {
                connected = false;

                Bizarre.Artery.ArteryManager.Startup(false, "MemoryTrackerPC");

                consoleConnection = new Bizarre.Artery.Connection();

                Bizarre.Artery.SockAddr target = new Bizarre.Artery.SockAddr(m_currentIP, m_currentPort);

                Bizarre.Artery.ArteryManager.Connect(consoleConnection, target, "MemoryTracker", MyConnectCallback, 0);

                while (true)
                {
                    if (shuttingDown.WaitOne(0, true))
                        break;

                    this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, new UpdateStatusDelegate(UpdateStatus), connected);

                    if (connected)
                    {
                        while (consoleConnection.IsConnected())
                        {
                            if (shuttingDown.WaitOne(0, true))
                                break;

                            System.Threading.Thread.Sleep(1000);

                            {
                                Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();
                                b.Write("IDLE");
                                Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
                                if (b.Build(msg))
                                {
                                    consoleConnection.Send(msg);
                                }
                            }
                        }
                    }
                    else
                    {
                        System.Threading.Thread.Sleep(2000);
                        Bizarre.Artery.ArteryManager.Connect(consoleConnection, target, "MemoryTracker", MyConnectCallback, 0);
                    }
                }
            }
            catch
            {
            }
        }

        private void GetAllocations_Click(object sender, RoutedEventArgs e)
        {
            DbgHelp.Reset();

            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();

            b.Write("Allocations2");
            b.Write(2);

            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                consoleConnection.Send(msg);
            }
        }

        private void IncrementTag_Click(object sender, RoutedEventArgs e)
        {
            Bizarre.Artery.MessageBuilder b = new Bizarre.Artery.MessageBuilder();

            b.Write("IncrementTag");

            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                consoleConnection.Send(msg);
            }
        }

        private void CallstackList_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                try
                {
                    // Try and find an instance of VS2008
                    Object obj = System.Runtime.InteropServices.Marshal.GetActiveObject("VisualStudio.DTE.9.0");

                    // If we didn't find 2008, how about 2005?
                    if (obj == null)
                    {
                        obj = System.Runtime.InteropServices.Marshal.GetActiveObject("VisualStudio.DTE.8.0");
                    }

                    if (obj != null)
                    {
                        ListBox lb = sender as ListBox;

                        if (lb.SelectedItem != null)
                        {
                            NameResolutionItem nri = lb.SelectedItem as NameResolutionItem;

                            bool success = true;

                            string fileToOpen = Bizarre2.Helper.GetRemappedFilename(nri.Filename);

                            if (File.Exists(fileToOpen) == false)
                            {
                                MessageBox.Show("File " + nri.Filename + " does not exist (" + fileToOpen + ")");
                                return;
                            }

                            int maxRetries = 5;

                            for (int tryCount = 0; success && tryCount < maxRetries; tryCount++)
                            {
                                try
                                {
                                    obj.GetType().InvokeMember("ExecuteCommand", BindingFlags.InvokeMethod, null, obj,
                                        new object[]
                                    {
                                        "File.OpenFile",
                                        "\"" + fileToOpen + "\""
                                    });
                                    success = true;
                                    break;
                                }
                                catch (Exception)
                                {
                                }
                                success = false;
                                System.Threading.Thread.Sleep(10);
                            }

                            if (success)
                            {
                                for (int tryCount = 0; tryCount < maxRetries; tryCount++)
                                {
                                    try
                                    {
                                        obj.GetType().InvokeMember("ExecuteCommand", BindingFlags.InvokeMethod, null, obj,
                                            new object[]
                                    {
                                        "Edit.GoTo",
                                        nri.Line
                                    });
                                        success = true;
                                        break;
                                    }
                                    catch (Exception)
                                    {
                                    }
                                    success = false;
                                    System.Threading.Thread.Sleep(10);
                                }
                            }

                            if (success)
                            {
                                for (int tryCount = 0; tryCount < maxRetries; tryCount++)
                                {
                                    try
                                    {
                                        Object window = obj.GetType().InvokeMember("MainWindow", BindingFlags.GetProperty,
                                            null, obj, new object[] { });
                                        window.GetType().InvokeMember("Activate", BindingFlags.InvokeMethod, null, window, new object[] { });
                                        success = true;
                                        break;
                                    }
                                    catch (Exception)
                                    {
                                    }
                                    success = false;
                                    System.Threading.Thread.Sleep(10);
                                }
                            }

                            if (!success)
                            {
                                MessageBox.Show("No instance of VS.NET 2008 or 2005 found");
                            }
                        }
                    }
                    else
                    {
                        MessageBox.Show("No instance of VS.NET 2008 or 2005 found");
                        return;
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("No instance of VS.NET 2008 or 2005 found (" + ex.Message + ")");
                    return;
                }
            }
            else if (e.ChangedButton == MouseButton.Right)
            {
                ListBox lb = sender as ListBox;

                if (lb.SelectedItem != null)
                {
                    NameResolutionItem nri = lb.SelectedItem as NameResolutionItem;

                    PickFromCS.DataContext = nri;
                }
            }
        }

        private string SizeToString(int size)
        {
            return String.Format("{0} bytes ({1:0.0} k) ({2:0.00} Mb)", size, (size / 1024.0), (size / (1024.0 * 1024.0)));
        }

        public void UpdateView()
        {
            AllocListView.Items.Filter = new Predicate<object>(MyListFilter);
            UpdateMemoryStats();
        }

        private void UpdateMemoryStats()
        {
            int total = 0, phys = 0, virt = 0, waste = 0;

            foreach (MemAllocItem mai in AllocListView.Items)
            {
                if (MyListFilter(mai))
                {
                    if (mai.IsPhysical)
                        phys += mai.AllocationSize;
                    else
                        virt += mai.AllocationSize;

                    total += mai.AllocationSize;
                    waste += mai.AllocationWaste;
                }
            }

            PhysicalMemoryInfo.Text = SizeToString(phys);
            VirtualMemoryInfo.Text = SizeToString(virt);
            TotalMemoryInfo.Text = SizeToString(total);
            WastedMemoryInfo.Text = SizeToString(waste);
        }

        private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (IsInitialized)
            {
                AllocListView.Items.Filter = new Predicate<object>(MyListFilter);

                UpdateMemoryStats();
            }
        }

        private void HeapCombo_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (IsInitialized)
            {
                AllocListView.Items.Filter = new Predicate<object>(MyListFilter);

                UpdateMemoryStats();
            }
        }

        private void CopyToClipboard_Click(object sender, RoutedEventArgs e)
        {
            string outputBuffer = "";
            int longestString = 0;

            foreach (NameResolutionItem nri in CallstackList.Items)
            {
                if (nri.Name.Length > longestString)
                    longestString = nri.Name.Length;
            }

            foreach (NameResolutionItem nri in CallstackList.Items)
            {
                outputBuffer += nri.Name;

                int padBy = longestString - nri.Name.Length;

                for (int iSpace = 0; iSpace < padBy + 5; iSpace++)
                    outputBuffer += " ";

                outputBuffer += "[" + nri.Filename + " @ " + nri.Line.ToString() + "]";

                outputBuffer += "\n";
            }

            Clipboard.SetText(outputBuffer);
        }

        public string SizeToString(uint size)
        {
            return String.Format("{0} bytes ({1:0.0} k) ({2:0.00} Mb)", size, (size / 1024.0), (size / (1024.0 * 1024.0)));
        }

        private void AllocListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            foreach (MemAllocItem mai in e.RemovedItems)
            {
                m_selection_MemoryWasted -= (uint)mai.AllocationWaste;
                m_selection_MemoryNeeded -= (uint)mai.AllocationSize;
            }

            foreach (MemAllocItem mai in e.AddedItems)
            {
                m_selection_MemoryWasted += (uint)mai.AllocationWaste;
                m_selection_MemoryNeeded += (uint)mai.AllocationSize;
            }

            ListView lv = sender as ListView;

            if (lv.SelectedItems.Count > 0)
                StatusCurrentInfo.Text = String.Format("{0} Allocations using {1} of memory - Wasted {2}", lv.SelectedItems.Count, SizeToString(m_selection_MemoryNeeded), SizeToString(m_selection_MemoryWasted));
            else
                StatusCurrentInfo.Text = "";
        }

        private void Window_Closing(object sender, CancelEventArgs e)
        {
            shuttingDown.Set();
        }

        void SetChildren(ObservableCollection<ContextNode> list, bool state)
        {
            foreach (ContextNode cn in list)
            {
                cn.Show = state;

                if (cn.ChildrenList.Count > 0)
                    SetChildren(cn.ChildrenList, state);
            }
        }

        public bool MyListFilter(object item)
        {
            MemAllocItem mai = item as MemAllocItem;

            if (!Window1.contextNodeList2[mai.ContextID].Show)
                return false;

            if (ShowOnlyTag.IsChecked == true)
            {
                Int32 tagVal = (OnlyTag.Text.Length > 0) ? Int32.Parse(OnlyTag.Text) : 0;

                if (mai.Tag != tagVal)
                    return false;
            }

            if (ShowPickFromCS.IsChecked == true)
            {
                bool found = false;

                if (mai.CallStackNRI != null)
                {
                    foreach (NameResolutionItem nri in mai.CallStackNRI)
                    {
                        if (nri.Equals(PickFromCS.DataContext))
                        {
                            found = true;
                            break;
                        }
                    }
                }

                if (!found)
                    return false;
            }

            if (MemTypeCombo.SelectedIndex > 0)
            {
                if (MemTypeCombo.SelectedIndex == 1 && mai.IsPhysical)
                    return false;
                else if (MemTypeCombo.SelectedIndex == 2 && !mai.IsPhysical)
                    return false;
                else if (MemTypeCombo.SelectedIndex == 3 && !mai.IsInSmallBlock)
                    return false;
            }

            if (HeapCombo.SelectedIndex > 0)
            {
                if (HeapCombo.SelectedIndex-1 != mai.Heap)
                    return false;
            }

            return true;
        }

        private void CheckBox_Click(object sender, RoutedEventArgs e)
        {
            AllocListView.Items.Filter = new Predicate<object>(MyListFilter);

            UpdateMemoryStats();
        }

        //GridViewColumnHeader _lastHeaderClicked = null;
        //ListSortDirection _lastDirection = ListSortDirection.Ascending;

        //void AllocListView_Click(object sender, RoutedEventArgs e)
        //{
        //    GridViewColumnHeader headerClicked = e.OriginalSource as GridViewColumnHeader;
        //    ListSortDirection direction = ListSortDirection.Ascending;

        //    if (headerClicked != null)
        //    {
        //        string header = headerClicked.Column.Header as string;
                
        //        switch (header)
        //        {
        //            case "Size":
        //                header = "AllocationSize";
        //                direction = ListSortDirection.Descending;
        //                break;
        //            case "Waste":
        //                header = "AllocationWaste";
        //                direction = ListSortDirection.Descending;
        //                break;
        //            //case "Tag":
        //            //    break;
        //            //case "Type":
        //            //    break;
        //            case "System":
        //                header = "ContextID";
        //                break;
        //            case "Function Name":
        //                header = "TopFunctionName";
        //                break;
        //        }

        //        if (headerClicked == _lastHeaderClicked)
        //        {
        //            if (_lastDirection == ListSortDirection.Ascending)
        //            {
        //                direction = ListSortDirection.Descending;
        //            }
        //            else
        //            {
        //                direction = ListSortDirection.Ascending;
        //            }
        //        }

        //        Sort(header, direction);
        //        _lastHeaderClicked = headerClicked;
        //        _lastDirection = direction;
        //    }
        //}

        internal class ItemClassSorter : ListViewCustomComparer
        {
            private int SortVals(ListSortDirection lsd, int v1, int v2)
            {
                if (lsd == ListSortDirection.Ascending)
                {
                    return (v1 < v2) ? -1 : (v1 > v2) ? 1 : 0;
                }
                else
                {
                    return (v1 < v2) ? 1 : (v1 > v2) ? -1 : 0;
                }
            }

            private int SortVals(ListSortDirection lsd, uint v1, uint v2)
            {
                if (lsd == ListSortDirection.Ascending)
                {
                    return (v1 < v2) ? -1 : (v1 > v2) ? 1 : 0;
                }
                else
                {
                    return (v1 < v2) ? 1 : (v1 > v2) ? -1 : 0;
                }
            }

            private int SortVals(ListSortDirection lsd, string v1, string v2)
            {
                if (lsd == ListSortDirection.Ascending)
                {
                    return String.Compare(v1, v2);
                }
                else
                {
                    return (-1) * String.Compare(v1, v2);
                }
            }

            public override int Compare(object x, object y)
            {
                try
                {
                    MemAllocItem xc = (MemAllocItem)x;
                    MemAllocItem yc = (MemAllocItem)y;

                    int result = 0;

                    foreach (string sortColumn in GetSortColumnList())
                    {
                        switch (sortColumn)
                        {
                                // Size
                            case "Col1":
                                result = SortVals(sortColumns[sortColumn], xc.AllocationSize, yc.AllocationSize);
                                break;

                                // Waste
                            case "Col2":
                                result = SortVals(sortColumns[sortColumn], xc.AllocationWaste, yc.AllocationWaste);
                                break;

                            // Tag
                            case "Col3":
                                result = SortVals(sortColumns[sortColumn], xc.Tag, yc.Tag);
                                break;

                            // Type
                            case "Col4":
                                result = SortVals(sortColumns[sortColumn], xc.Type, yc.Type);
                                break;
                            
                            // System
                            case "Col5":
                                result = SortVals(sortColumns[sortColumn], xc.ContextID, yc.ContextID);
                                break;

                            // Function Name
                            case "Col6":
                                result = SortVals(sortColumns[sortColumn], xc.TopFunctionName, yc.TopFunctionName);
                                break;

                                // Address
                            case "Col7":
                                result = SortVals(sortColumns[sortColumn], xc.AllocAddress, yc.AllocAddress);
                                break;

                            default:
                                break;
                        }

                        if (result != 0)
                            break;
                        else
                            continue;
                    }

                    return result;
                }
                catch (Exception)
                {
                    return 0;
                }
            }
        }

        private void AddSortBinding()
        {
            GridView gv = (GridView)AllocListView.View;

            for (int i = 1; i <= gv.Columns.Count; i++)
            {
                GridViewColumn col = gv.Columns[i - 1];
                ListViewSorter.SetSortBindingMember(col, new Binding("Col" + i.ToString()));
            }

            ListViewSorter.SetCustomSorter(AllocListView, new ItemClassSorter());
        }

        internal static class ArgumentParser
        {
            // Regex to split args; splits on word boundaries and "quoted phrases"
            private const string NonQuotedWord = @"([^"" \n\r\t\f]+)";
            private const string QuotedPhrase = @"(""[^""]+"")";
            private static Regex regex = new Regex(NonQuotedWord + "|" + QuotedPhrase, RegexOptions.Compiled);

            /// <summary>
            /// Gets the regex used for splitting arguments.
            /// </summary>
            internal static Regex Regex
            {
                get { return regex; }
            }
        }

        public class GameEventArgs : EventArgs
        {
            private string eventName;
            private string[] args;

            /// <summary>
            /// Gets the event name.
            /// </summary>
            public string EventName
            {
                get { return eventName; }
            }

            /// <summary>
            /// Indexer for getting args.
            /// </summary>
            /// <param name="index">Index of argument.</param>
            /// <returns>Selected argument.</returns>
            public string this[int index]
            {
                get { return args[index]; }
            }

            /// <summary>
            /// Gets the # of arguments.
            /// </summary>
            public int Length
            {
                get { return args.Length; }
            }

            /// <summary>
            /// Creates a GameEventArgs, parsing a string for args.
            /// </summary>
            /// <param name="argsString">String containing full, unparsed arguments.</param>
            public GameEventArgs(string argsString)
            {
                // Split into words and quoted phrases
                MatchCollection matches = ArgumentParser.Regex.Matches(argsString);
                System.Diagnostics.Debug.Assert(matches.Count >= 1);

                // 1st arg is event name
                eventName = matches[0].Value;

                // Copy rest to args array; remove quotes from quoted phrases
                args = new string[matches.Count - 1];
                for (int index = 1; index < matches.Count; ++index)
                {
                    args[index - 1] = matches[index].Value.Replace(@"""", string.Empty);
                }
            }
        }

        private void LoadMemoryInformationFromFile(string filename)
        {
            DbgHelp.Reset();

            using (TextReader streamReader = new StreamReader(filename))
            {
                try
                {
                    //AddModule 80040000 160000
                    //xboxkrnl.exe
                    //280
                    //26 160 160 253 14 255 137 70 163 180 237 47 237 226 210 183 1 0 0 0 100 58 92 120 101 110 111 110 92 115 112 114 105 110 103 48 55 92 99 111 114 101 92 112 114 105 118 97 116 101 92 110 116 111 115 92 105 110 105 116 92 100 101 118 107 105 116 92 110 111 114 109 97 108 92 111 98 106 92 120 98 111 120 92 120 98 111 120 107 114 110 108 100 46 112 100 98 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0


                    while (true)
                    {
                        string line = streamReader.ReadLine();

                        GameEventArgs e = new GameEventArgs(line);

                        if (e.EventName == "AddModule")
                        {
                            UInt32 moduleBaseAddr = UInt32.Parse(e[0], System.Globalization.NumberStyles.HexNumber);
                            UInt32 moduleSize = UInt32.Parse(e[1], System.Globalization.NumberStyles.HexNumber);

                            string moduleName = streamReader.ReadLine();
                            Int32 signatureSize = Int32.Parse(streamReader.ReadLine());
                            string signature = streamReader.ReadLine();

                            byte[] buffer = new byte[signatureSize];

                            MatchCollection m = ArgumentParser.Regex.Matches(signature);

                            for (int im = 0; im < m.Count; im++)
                            {
                                buffer[im] = byte.Parse(m[im].Value);
                            }

                            //Bizarre.RawBuffer raw = new Bizarre.RawBuffer(buffer);

                            bool loadOk = DbgHelp.LoadSymbolsForModule(moduleBaseAddr, moduleSize, buffer, signatureSize);

                            //if (!loadOk)
                            //    MessageBox.Show("DbgHelp.LoadSymbolsForModule failed", "AddModule 1", MessageBoxButton.OK, MessageBoxImage.Error);
                        }
                        else if (e.EventName == "BeginMemDump")
                        {
                            heapNames.Clear();
                            heapNames.Add("Main");
                        }
                        else if (e.EventName == "ContextNodes")
                        {
                            int nodeCount = int.Parse(e[0]);

                            contextNodeList2.Clear();
                            contextNodeSortList.Clear();

                            for (int iNode = 0; iNode < nodeCount; iNode++)
                            {
                                ContextNode node = new ContextNode();

                                node.FullName = streamReader.ReadLine();

                                MatchCollection m = ArgumentParser.Regex.Matches(streamReader.ReadLine());

                                node.ParentIndex = int.Parse(m[0].Value);
                                contextNodeList2.Add(node);
                                contextNodeSortList.Add(int.Parse(m[1].Value));
                            }

                            RebuildContextNodeTree();
                        }
                        else if (e.EventName == "Heap")
                        {
                            int index = int.Parse(e[0]);

                            string name = streamReader.ReadLine();

                            heapNames.Add(name);

                            MyDelegateType myAddNameToHeaps = delegate
                            {
                                HeapCombo.Items.Add(name);
                            };
                            this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, myAddNameToHeaps);
                        }
                        else if (e.EventName == "AllocStart")
                        {
                            int flags = int.Parse(e[0]);
                            int numItems = int.Parse(e[1]);

                            MyDelegateType mySetProgressMax = delegate
                            {
                                StatusProgressBar.Maximum = numItems;
                                StatusProgressBar.Value = 0;

                                StatusProgressBar.Visibility = Visibility.Visible;

                                StatusLabel.Text = "0 / " + numItems.ToString();
                            };
                            this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, mySetProgressMax);

                            System.Diagnostics.Trace.WriteLine("About to receive information on " + numItems.ToString() + " allocations.");

                            tempMemoryAllocList = new ObservableCollection<MemAllocItem>();
                        }
                        else if (e.EventName == "Alloc")
                        {
                            // Context + Size + Waste + Type + Tag + Callstack Depth

                            Int32 version = Int32.Parse(e[0]);

                            MemAllocItem info = new MemAllocItem();

                            info.ContextID = int.Parse(e[1]);
                            info.AllocationSize = int.Parse(e[2]);
                            info.AllocationWaste = int.Parse(e[3]);
                            info.Attribs = int.Parse(e[4]);
                            info.Tag = int.Parse(e[5]);
                            info.CallStackDepth = int.Parse(e[6]);

                            if (version >= 2)
                            {
                                info.Heap = int.Parse(e[7]);
                            }

                            NameResolutionItem[] cstack = new NameResolutionItem[info.CallStackDepth];
                            byte[] byteStack = new byte[info.CallStackDepth * 4];

                            MatchCollection m = ArgumentParser.Regex.Matches(streamReader.ReadLine());

                            Bizarre2.RawBuffer raw = new Bizarre2.RawBuffer(byteStack);

                            for (int iStack = 0; iStack < info.CallStackDepth; iStack++)
                            {
                                UInt32 v = UInt32.Parse(m[iStack].Value, System.Globalization.NumberStyles.HexNumber);

                                cstack[iStack] = Bizarre2.Helper.GetNRIFromCode((uint)v);
                            }

                            info.CallStackNRI = cstack;

                            ContextNode cn = contextNodeList2[info.ContextID];
                            cn.AllocationCount++;
                            if (cn.ParentIndex != -1)
                            {
                                do
                                {
                                    cn = contextNodeList2[cn.ParentIndex];
                                    cn.SubAllocationCount++;
                                } while (cn.ParentIndex != -1);
                            }

                            tempMemoryAllocList.Add(info);

                            if (tempMemoryAllocList.Count % 40 == 0)
                            {
                                MyDelegateType myUpdateProgress = delegate
                                {
                                    StatusProgressBar.Value = Math.Min(tempMemoryAllocList.Count, StatusProgressBar.Maximum);
                                    StatusLabel.Text = tempMemoryAllocList.Count.ToString() + " / " + StatusProgressBar.Maximum.ToString();
                                };
                                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, myUpdateProgress);
                            }
                        }
                        else if (e.EventName == "AllocStop")
                        {
                            if (!DbgHelp.bInitialised)
                            {
                                DbgHelp.bInitialised = true;
                            }

                            MyDelegateType myDelAddAllocItem = delegate
                            {
                                memoryAllocList = tempMemoryAllocList;
                                tempMemoryAllocList = null;

                                AllocListView.DataContext = memoryAllocList;

                                StatusLabel.Text = "Ready";
                                StatusProgressBar.Visibility = Visibility.Collapsed;

                                UpdateMemoryStats();
                            };
                            this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, myDelAddAllocItem);

                            break;
                        }
                    }
                }
                catch // (Exception ex)
                {
                    //int mrf = 10;
                }
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog openFileDialog1 = new Microsoft.Win32.OpenFileDialog();
            openFileDialog1.InitialDirectory = @"C:\";
            openFileDialog1.Title = "Select a File";

            openFileDialog1.FileName = "OutOfMemory.mtrack";

            openFileDialog1.Filter = "Memory Tracker File|*.mtrack";
            openFileDialog1.FilterIndex = 1;

            if (openFileDialog1.ShowDialog() == true)
            {
                LoadMemoryInformationFromFile(openFileDialog1.FileName);
            }
        }

        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            //int mrf = 10;
        }

        private void ShowPickFromCS_Click(object sender, RoutedEventArgs e)
        {
            UpdateView();
        }

        private void OnlyTag_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (ShowOnlyTag.IsChecked == true)
            {
                UpdateView();
            }
        }
    }

    public class DbgHelp
    {
        public DbgHelp()
        {
            bInitialised = false;
        }

        [DllImport("AddrToNameDLL.dll")]
        public static extern int Initialise();

        [DllImport("AddrToNameDLL.dll")]
        public static extern void Shutdown();

        [DllImport("AddrToNameDLL.dll")]
        public static extern void Reset();

        [DllImport("AddrToNameDLL.dll")]
        public static extern bool LoadSymbolsForModule(UInt32 baseAddress, UInt32 size, byte[] buffer, Int32 bufferSize);

        [DllImport("AddrToNameDLL.dll")]
        public static extern void End();

        [DllImport("AddrToNameDLL.dll")]
        public static extern bool GetName(uint addr, StringBuilder buf, int nMaxCount);

        [DllImport("AddrToNameDLL.dll")]
        public static extern int GetFilenamePosition(uint addr, StringBuilder buf, int nMaxCount, ref int linePos);

        //public static extern void SetWindowText(int h, String s);
        public static bool bInitialised;
    }

    namespace Bizarre2
    {
        class Helper
        {
            static private Dictionary<UInt32, NameResolutionItem> _cache = new Dictionary<UInt32, NameResolutionItem>();

            static private List<string> _remap = new List<string>();

            static private string localPathRoot = null;

            static public string GetNameFromCode(uint addr)
            {
                return GetNRIFromCode(addr).Name;
            }

            static public string GetRemappedFilename(string filename)
            {
                foreach (var p in _remap)
                {
                    if (filename.StartsWith(p, StringComparison.InvariantCultureIgnoreCase))
                    {
                        return localPathRoot + filename.Substring(p.Length);
                    }
                }

                return filename;
            }

            static public NameResolutionItem GetNRIFromCode(uint addr)
            {
                if (_cache.ContainsKey(addr))
                    return _cache[addr];

                NameResolutionItem nri = new NameResolutionItem() { Address = addr };

                uint rva = addr; //  -0x82000000;// - 0x400000;

                StringBuilder sbSymbolName = new StringBuilder(8192);
                StringBuilder sbFileName = new StringBuilder(8192);
                int fileLineNumber = 0;

                bool result = DbgHelp.GetName(rva, sbSymbolName, sbSymbolName.Capacity - 1);
                int result2 = DbgHelp.GetFilenamePosition(rva, sbFileName, sbFileName.Capacity - 1, ref fileLineNumber);

                //System.Diagnostics.Trace.WriteLine(String.Format("rva={0} - SymbolName={1}", rva, sbSymbolName));

                string name = sbSymbolName.ToString();

                if (name.CompareTo("<unknown>") == 0)
                {
                    nri.Name = String.Format("0x{0:x}", rva);
                }
                else
                {
                    nri.Name = name;
                }

                nri.Filename = sbFileName.ToString();
                nri.Line = fileLineNumber;

                _cache.Add(addr, nri);

                // We wait for a folder that exists which matches code//mercury//library and use that to remap other files
                //string filenameLower = nri.Filename.ToLower();

                if (System.IO.File.Exists(nri.Filename))
                {
                    if (localPathRoot == null)
                    {
                        int offset = nri.Filename.IndexOf(@"code\mercury\library\", StringComparison.InvariantCultureIgnoreCase);

                        if (offset == -1)
                        {
                            offset = nri.Filename.IndexOf(@"code\vendor\", StringComparison.InvariantCultureIgnoreCase);
                        }

                        if (offset != -1)
                        {
                            // This is what we swap into other paths to find the correct file
                            localPathRoot = nri.Filename.Substring(0, offset);
                        }
                    }
                }
                else
                {
                    int offset = nri.Filename.IndexOf(@"code\mercury\library\", StringComparison.InvariantCultureIgnoreCase);

                    if (offset == -1)
                    {
                        offset = nri.Filename.IndexOf(@"code\vendor\", StringComparison.InvariantCultureIgnoreCase);
                    }

                    if (offset != -1)
                    {
                        string pathRoot = nri.Filename.Substring(0, offset);

                        if (!_remap.Contains(pathRoot))
                        {
                            _remap.Add(pathRoot);
                            System.Diagnostics.Debug.WriteLine("REMAP PATH: " + pathRoot);
                        }
                    }
                    //else
                    //{
                    //    System.Diagnostics.Debug.WriteLine("BAD PATH: " + nri.Filename);
                    //}
                }

                return nri;
            }
        }

        /// <summary>
        /// Summary description for BufferBuilder.
        /// </summary>
        public class RawBuffer
        {
            protected byte[] buffer;
            protected int offset;
            protected int bufferSize;
            protected bool reading;

            public RawBuffer(ref byte[] bufferIn, int bufferSizeIn)
            {
                buffer = bufferIn;
                offset = 0;
                bufferSize = bufferSizeIn;
                reading = false;
            }

            public RawBuffer(byte[] bufferIn)
            {
                buffer = bufferIn;
                offset = 0;
                reading = true;
            }

            public string GetString()
            {
                int length = GetByte();

                string val = Encoding.ASCII.GetString(buffer, offset, length);
                offset += length;
                return val;
            }

            public void WriteStringNullInt(string val)
            {
                Write((int)val.Length + 1);

                Encoding.ASCII.GetBytes(val, 0, val.Length, buffer, offset);
                offset += val.Length;

                Write((byte)0);
            }

            public void WriteStringInt(string val)
            {
                Write((int)val.Length);

                Encoding.ASCII.GetBytes(val, 0, val.Length, buffer, offset);
                offset += val.Length;
            }

            public void WriteString(string val)
            {
                Write((byte)val.Length);

                Encoding.ASCII.GetBytes(val, 0, val.Length, buffer, offset);
                offset += val.Length;
            }

            public Int32 GetInt()
            {
                Int32 val = IPAddress.NetworkToHostOrder(BitConverter.ToInt32(buffer, offset));
                offset += 4;
                return val;
            }

            public UInt32 GetUInt()
            {
                Int32 netVal = BitConverter.ToInt32(buffer, offset);
                uint val = (uint)IPAddress.NetworkToHostOrder(netVal);
                offset += 4;
                return val;
            }

            public void Write(int val)
            {
                BitConverter.GetBytes(IPAddress.HostToNetworkOrder(val)).CopyTo(buffer, offset);
                offset += 4;
            }

            public void Write(uint val2)
            {
                int val = (int)val2;

                BitConverter.GetBytes(IPAddress.HostToNetworkOrder(val)).CopyTo(buffer, offset);
                offset += 4;
            }

            public void Write(long val)
            {
                BitConverter.GetBytes(IPAddress.HostToNetworkOrder(val)).CopyTo(buffer, offset);
                offset += 8;
            }

            public byte GetByte()
            {
                byte val = buffer[offset];
                offset++;
                return val;
            }

            public void Write(byte val)
            {
                buffer[offset] = val;
                offset++;
            }

            public int Offset()
            {
                return offset;
            }

            public int Skip(int value)
            {
                offset += value;
                return offset;
            }
        }
    }

    public class TrackerState : INotifyPropertyChanged
    {
        #region Member variables

        private Int32 tagValue = 0;

        #endregion

        public Int32 TagValue
        {
            get
            {
                return tagValue;
            }
            
            set
            {
                if (tagValue != value)
                {
                    tagValue = value;
                    Notify("TagValue");
                }
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected void Notify(string propname)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propname));
            }
        }
    }

    public class ValueToHexConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (targetType != typeof(string))
                return null;

            return String.Format("0x{0:x}", (uint)value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    [ValueConversion(/*sourceType*/ typeof(int), /*destType*/ typeof(string))]
    public class ContextIDToSystemConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (targetType != typeof(string))
                return null;

            try
            {
                return Window1.contextNodeList2[(Int32)value].FullName;
            }
            catch
            {
                return "<Error>";
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    [ValueConversion(/*sourceType*/ typeof(NameResolutionItem[]), /*destType*/ typeof(string))]
    public class CallstackToFunctionConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (targetType != typeof(string) || value == null)
                return null;

            NameResolutionItem[] stack = value as NameResolutionItem[];

            if (stack.Count() == 0)
                return "?";

            // The parameter determines which element of the array we return
            //raw.Skip(Int32.TryParse(parameter));

            int offset;
            Int32.TryParse(parameter as string, out offset);

            bool useOffset = (offset >= 0);

            if (stack[useOffset ? offset : 0].Address != 0)
            {
                string name;

                if (!useOffset && stack.GetLength(0) > 1)
                    name = stack[1].Name;
                else
                    name = stack[0].Name;

                if (!useOffset && (name == "operator new[]"))
                {
                    name = stack[1].Name;
                }

                return name;
            }
            else
            {
                return "?";
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public abstract class CallstackToFunctionMarkupExtension : System.Windows.Markup.MarkupExtension
    {
        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            //if (targetType != typeof(string))
            //    return null;

            //NameResolutionItem[] stack = value as NameResolutionItem[];

            //// The parameter determines which element of the array we return
            ////raw.Skip(Int32.TryParse(parameter));

            //int offset;
            //Int32.TryParse(parameter as string, out offset);

            //bool useOffset = (offset >= 0);

            //if (stack[useOffset ? offset : 0].Address != 0)
            //{
            //    string name;

            //    if (!useOffset && stack.GetLength(0) > 1)
            //        name = stack[1].Name;
            //    else
            //        name = stack[0].Name;

            //    if (!useOffset && (name == "operator new[]"))
            //    {
            //        name = stack[1].Name;
            //    }

            //    return name;
            //}
            //else
            //{
            //    return "?";
            //}
            
            throw new NotImplementedException();
        }
    }

    [ValueConversion(/*sourceType*/ typeof(NameResolutionItem), /*destType*/ typeof(string))]
    public class AddressToFunctionConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value == null)
                return null;

            if (targetType != typeof(string))
                return null;

            if (value is NameResolutionItem)
            {
                return (value as NameResolutionItem).Name;
            }

            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public static class MyCommands
    {
        private static Dispatcher _dispatcher;
        private static RoutedUICommand clearChildrenCommand = new RoutedUICommand("ClearChildren", "clearChildren", typeof(MyCommands));
        private static RoutedUICommand selectChildrenCommand = new RoutedUICommand("SelectChildren", "selectChildren", typeof(MyCommands));

        public static RoutedUICommand ClearChildren
        {
            get
            {
                return clearChildrenCommand;
            }
        }

        public static RoutedUICommand SelectChildren
        {
            get
            {
                return selectChildrenCommand;
            }
        }

        public static void CanExecuteClearChildrenCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            // Handler to provide mechanism for determining when the add command can be executed.
            e.Handled = true;
            e.CanExecute = ((e.Parameter as ContextNode).Children.Count > 0);
        }

        public static void CanExecuteSelectChildrenCommand(object sender, CanExecuteRoutedEventArgs e)
        {
            // Handler to provide mechanism for determining when the add command can be executed.
            e.Handled = true;
            e.CanExecute = ((e.Parameter as ContextNode).Children.Count > 0);
        }

        public static void ExecuteClearChildrenCommand(object sender, ExecutedRoutedEventArgs e)
        {
            //add your code here to handle

            foreach (ContextNode cn in (e.Parameter as ContextNode).Children)
            {
                cn.Show = false;
            }

            (Application.Current.MainWindow as Window1).UpdateView();
        }

        public static void ExecuteSelectChildrenCommand(object sender, ExecutedRoutedEventArgs e)
        {
            //add your code here to handle

            foreach (ContextNode cn in (e.Parameter as ContextNode).Children)
            {
                cn.Show = true;
            }

            (Application.Current.MainWindow as Window1).UpdateView();
        } 

        static MyCommands()
        {
            _dispatcher = Dispatcher.CurrentDispatcher;

            Application.Current.MainWindow.CommandBindings.Add(new CommandBinding(clearChildrenCommand, ExecuteClearChildrenCommand, CanExecuteClearChildrenCommand));
            Application.Current.MainWindow.CommandBindings.Add(new CommandBinding(selectChildrenCommand, ExecuteSelectChildrenCommand, CanExecuteSelectChildrenCommand));
        }
    }

    public class NumberTextBox : TextBox
    {
        protected override void OnPreviewTextInput(System.Windows.Input.TextCompositionEventArgs e)
        {
            e.Handled = !AreAllValidNumericChars(e.Text);
            base.OnPreviewTextInput(e);
        }

        bool AreAllValidNumericChars(string str)
        {
            bool ret = true;

            int l = str.Length;

            for (int i = 0; i < l; i++)
            {
                char ch = str[i];
                ret &= Char.IsDigit(ch);
            }

            return ret;
        }
    }

    public abstract class ListViewCustomComparer : IComparer
    {
        protected Dictionary<string, ListSortDirection> sortColumns = new Dictionary<string, ListSortDirection>();

        public void AddSort(string sortColumn, ListSortDirection dir)
        {
            //if (sortColumns.ContainsKey(sortColumn))
            //    sortColumns.Remove(sortColumn);

            sortColumns.Clear();
            sortColumns.Add(sortColumn, dir);
        }

        public void ClearSort()
        {
            sortColumns.Clear();
        }

        protected List<string> GetSortColumnList()
        {
            List<string> result = new List<string>();
            Stack<string> temp = new Stack<string>();

            foreach (string col in sortColumns.Keys)
            {
                temp.Push(col);
            }

            while (temp.Count > 0)
            {
                result.Add(temp.Pop());
            }

            return result;
        }

        public abstract int Compare(object x, object y);
    }    
}

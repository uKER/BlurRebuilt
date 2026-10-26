using System;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.InteropServices;
using System.Text;
using System.IO;

using XDevkit;

namespace XBrobocopy
{
  public delegate void DelegateScanUpdate(ulong size);

    public class BizProcessStatus
    {
        public ulong size = 0;
    }

    public class BizSyncWorkList
    {
        ulong size;
        ulong fileCount;

        string previousFolder;

        public ulong Size
        {
            get
            {
                return size;
            }
            set
            {
                size = value;
            }
        }
        public ulong FileCount
        {
            get
            {
                return fileCount;
            }
            set
            {
                fileCount = value;
            }
        }

        public BizSyncWorkList(string groupname)
        {
            groups = new List<Group>();

            Group coreGroup = new Group();
            coreGroup.name = groupname;
            coreGroup.allowedToSkip = true;

            groups.Add(coreGroup);

            Size = 0;
            FileCount = 0;
            previousFolder = "";
        }

        public class BizWorkItem
        {
            public string name;
            public ulong size;
            public BizFileInfo srcFile;
            public bool folder;
            public bool showFolder;
            public bool ignore;
            public bool delete;
            public bool fakeDelete;

            public enum WorkItemStatus
            {
                Unknown,
                NewItem,
                DestOlder,
                DestNewer,
                Extra,
                SizeDiff,
            };

            public WorkItemStatus m_Status = WorkItemStatus.Unknown;

            public BizWorkItem()
            {
                name = "";
                size = 0;
                folder = false;
                showFolder = false;
                fakeDelete = false;
            }

            public BizWorkItem(string name, ulong size, bool folder, bool ignore, bool delete)
            {
                this.name = name;
                this.size = size;
                this.folder = folder;
                this.ignore = ignore;
                this.delete = delete;
            }
        }

        public class Group
        {
            public string name;                    // Identifying name of the group
            public string filter;
            public ulong size;
            public bool allowedToSkip;

            // List of files to copy
            public List<BizWorkItem> items;

            public void AddWorkItem(BizWorkItem newItem)
            {
                //if (newItem.folder && items.Count > 0 && items[items.Count-1].folder)
                //{
                //    items.RemoveAt(items.Count - 1);
                //}

                items.Add(newItem);

                if (newItem.ignore == false || newItem.delete == false)
                    size += newItem.size;
            }

            public void AddFolder(string folderName)
            {
                BizWorkItem bwi = new BizWorkItem();

                bwi.name = folderName;
                bwi.folder = true;
                bwi.showFolder = true;

                items.Add(bwi);
            }

            public Group()
            {
                items = new List<BizWorkItem>();
                size = 0;
                allowedToSkip = false;
            }
        }

        public List<Group> groups;

        public void AddGroup(string groupName, string folderFilter, bool canWeSkip)
        {
            Group newGroup = new Group();
            newGroup.name = groupName;
            newGroup.filter = folderFilter;
            newGroup.allowedToSkip = canWeSkip;

            groups.Add(newGroup);
        }

        public Group FindGroup(string filename)
        {
            Group match = null;

            foreach (Group g in groups)
            {
                if (g.filter != null)
                {
                    if (filename.StartsWith(g.filter, true, null))
                        match = g;
                }
                else
                {
                    match = g;
                }
            }
            return match;
        }

        public void RegisterFolder(string folderName)
        {
            if (previousFolder.CompareTo(folderName) != 0)
            {
                Group itemGroup = FindGroup(folderName);

                itemGroup.AddFolder(folderName);

                previousFolder = folderName;
            }
        }

        public void AddWorkItem(BizWorkItem newItem)
        {
            Group itemGroup = FindGroup(newItem.name);

            itemGroup.AddWorkItem(newItem);
           
            Size += newItem.size;
            FileCount++;
        }

        public void DumpGroupInfo()
        {
            foreach (Group group in groups)
            {
                System.Diagnostics.Debug.WriteLine(String.Format("Group '{0}' - Size {1:,0}", group.name, group.size));
            }
        }
    }

    public class BizFileInfo
    {
        public string name;
        public DateTime dtModified;
        public DateTime dtCreated;
        public long lastModified;
        public long created;
        public bool isFolder;
        public ulong size;

        public ArrayList filesList;
        public ArrayList dirList;

        public BizFileInfo()
        {
            size = 0;
            isFolder = false;
            name = "";
        }

        virtual public BizFileInfo CreateFileInfo()
        {
            return null;
        }

        public ulong CalculateSize()
        {
            ulong folderSize = size;

            if (filesList != null)
            {
                foreach (BizFileInfo bfi in filesList)
                {
                    folderSize += bfi.CalculateSize();
                }
            }

            if (dirList != null)
            {
                foreach (BizFileInfo bfi in dirList)
                {
                    folderSize += bfi.CalculateSize();
                }
            }

            return folderSize;
        }

        public virtual void ProcessFolder(string directoryName, BizProcessStatus bps, DelegateScanUpdate usd)
        {
        }

        public virtual void ProcessFolder(string directoryName, BizProcessStatus bps)
        {
            ProcessFolder(directoryName, bps, null);
        }

        public class FileListNameComparer : IComparer
        {
            public FileListNameComparer()
            {
            }

            public int Compare(object left, object right)
            {
                BizFileInfo bfiLeft = (BizFileInfo)left;
                BizFileInfo bfiRight = (BizFileInfo)right;

                return bfiLeft.name.CompareTo(bfiRight.name);
            }
        }

        public static void TraverseFolderToCopyToDest
        (
            SyncType        syncType, 
            BizSyncWorkList workList, 
            BizFileInfo     left,
            XBSubOptions    subOptions,
            string          rootPath,
            string          dirPath,
            List<string>    fileSpec,
            List<string>    dirSpec
        )
        {
            // Files
            {
                IEnumerator iLeft = left.filesList.GetEnumerator();

                while (iLeft.MoveNext())
                {
                    BizFileInfo fiLeft = iLeft.Current as BizFileInfo;

                    // Check item is not on exclude list.
                    bool ignore = false;

                    // File ignore list.
                    if (fiLeft != null)
                    {
                        System.Diagnostics.Debug.Assert(fiLeft.isFolder == false);

                        for (int i = 0; i < fileSpec.Count; i++)
                        {
                            string ignorefile = fileSpec[i].ToLower().Replace('/', '\\');
                            if (fiLeft.name.Contains(ignorefile))
                            {
                                ignore = true;
                                break;
                            }
                        }
                    }

                    if (!ignore)
                    {
                        BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, false, false);
                        newWorkItem.m_Status = BizSyncWorkList.BizWorkItem.WorkItemStatus.NewItem;
                        newWorkItem.srcFile = fiLeft;

                        workList.AddWorkItem(newWorkItem);
                    }
                    else
                    {
                        BizSyncWorkList.BizWorkItem newWorkItem =
                            new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, true, false);

                        newWorkItem.srcFile = fiLeft;
                        workList.AddWorkItem(newWorkItem);
                    }
                }
            }

            // Directories
            {
                IEnumerator iLeft = left.dirList.GetEnumerator();

                while (iLeft.MoveNext())
                {
                    BizFileInfo fiLeft = iLeft.Current as BizFileInfo;

                    // Check item is not on exclude list.
                    bool ignore = false;

                    if (fiLeft != null)
                    {
                        System.Diagnostics.Debug.Assert(fiLeft.isFolder == true);

                        // Folder ignore list.
                        for (int i = 0; i < dirSpec.Count; i++)
                        {
                            string fullname = rootPath.ToLower() + dirPath.ToLower() + fiLeft.name.ToLower();
                            string ignoredir = dirSpec[i].ToLower().Replace('/', '\\');
                            if (fullname.CompareTo(ignoredir) == 0)
                            {
                                ignore = true;
                                break;
                            }
                        }
                    }

                    if (!ignore)
                    {
                        BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, false, false);
                        newWorkItem.m_Status = BizSyncWorkList.BizWorkItem.WorkItemStatus.NewItem;
                        newWorkItem.srcFile = fiLeft;

                        System.Diagnostics.Debug.Assert(fiLeft.isFolder == true);

                        if (fiLeft.filesList.Count > 0 || fiLeft.dirList.Count > 0)
                        {
                            workList.AddWorkItem(newWorkItem);
                            TraverseFolderToCopyToDest(syncType, workList, fiLeft, subOptions, rootPath, dirPath + fiLeft.name + "\\", fileSpec, dirSpec);
                        }
                        else if (subOptions == XBSubOptions.CopyEmptySub)
                        {
                            workList.AddWorkItem(newWorkItem);
                        }
                    }
                    else
                    {
                        BizSyncWorkList.BizWorkItem newWorkItem =
                            new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, fiLeft.isFolder, true, false);
                        
                        newWorkItem.srcFile = fiLeft;
                        workList.AddWorkItem(newWorkItem);
                    }
                }
            }
        }

        public static void TraverseFolderToRemoveFromDest
        (
            SyncType syncType,
            BizSyncWorkList workList,
            BizFileInfo left,
            XBSubOptions subOptions,
            string dirPath,
            List<string> fileSpec,
            List<string> dirSpec
        )
        {
            // Files
            {
                IEnumerator iLeft = left.filesList.GetEnumerator();

                while (iLeft.MoveNext())
                {
                    BizFileInfo fiLeft = iLeft.Current as BizFileInfo;
                    System.Diagnostics.Debug.Assert(fiLeft.isFolder == false);

                    BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, false, false, true);
                    newWorkItem.srcFile = fiLeft;

                    workList.AddWorkItem(newWorkItem);
                }
            }

            // Directories
            {
                IEnumerator iLeft = left.dirList.GetEnumerator();

                while (iLeft.MoveNext())
                {
                    BizFileInfo fiLeft = iLeft.Current as BizFileInfo;
                    System.Diagnostics.Debug.Assert(fiLeft.isFolder == true);

                    // Fake work item to just display we're deleting a folder
                    {
                        BizSyncWorkList.BizWorkItem newWorkItemFake = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, true, false, true);
                        newWorkItemFake.srcFile = fiLeft;
                        newWorkItemFake.fakeDelete = true;

                        workList.AddWorkItem(newWorkItemFake);
                    }

                    BizSyncWorkList.BizWorkItem newWorkItem = new BizSyncWorkList.BizWorkItem(dirPath + fiLeft.name, fiLeft.size, true, false, true);
                    newWorkItem.srcFile = fiLeft;

                    if (fiLeft.filesList.Count > 0 || fiLeft.dirList.Count > 0)
                    {
                        TraverseFolderToRemoveFromDest(syncType, workList, fiLeft, subOptions, dirPath + fiLeft.name + "\\", fileSpec, dirSpec);
                        workList.AddWorkItem(newWorkItem);
                    }
                    else if (subOptions == XBSubOptions.CopyEmptySub)
                    {
                        workList.AddWorkItem(newWorkItem);
                    }
                }
            }
        }
    };

    public class BizPCFileInfo : BizFileInfo
    {
        override public BizFileInfo CreateFileInfo()
        {
            return new BizPCFileInfo();
        }

        public override void ProcessFolder(string directoryName, BizProcessStatus bps, DelegateScanUpdate usd)
        {
            filesList = new ArrayList();
            dirList = new ArrayList();

            DirectoryInfo dirInfo = new DirectoryInfo(directoryName);

            FileInfo[] fileInfos = dirInfo.GetFiles();

            foreach (FileInfo fi in fileInfos)
            {
                BizPCFileInfo pcfi = new BizPCFileInfo();

                pcfi.name = fi.Name;
                pcfi.size = (ulong)fi.Length;

                bps.size += pcfi.size;
                if (usd != null)
                    usd(bps.size);

                pcfi.dtModified = fi.LastWriteTime;
                pcfi.dtCreated = fi.CreationTime;

                pcfi.created = fi.CreationTime.ToUniversalTime().Ticks;
                pcfi.lastModified = fi.LastWriteTime.ToUniversalTime().Ticks;

                //if (!fi.LastWriteTime.IsDaylightSavingTime())
                //    pcfi.lastModified += 1 * System.TimeSpan.TicksPerHour;

                //if ((fi.Name.ToLower().CompareTo("environment.dds") == 0))
                //{
                //    System.Console.WriteLine("PC   : {0} - ToFileTime              - {1}", fi.Name, fi.LastWriteTime.ToFileTime().ToString());
                //    System.Console.WriteLine("PC   : {0} - ToFileTimeUtc           - {1}", fi.Name, fi.LastWriteTime.ToFileTimeUtc().ToString());
                //    System.Console.WriteLine("PC   : {0} - ToLocalTime().Ticks     - {1}", fi.Name, fi.LastWriteTime.ToLocalTime().Ticks.ToString());
                //    System.Console.WriteLine("PC   : {0} - ToLocalTime()           - {1}", fi.Name, fi.LastWriteTime.ToLocalTime().ToString());
                //    System.Console.WriteLine("PC   : {0} - ToUniversalTime().Ticks - {1}", fi.Name, fi.LastWriteTime.ToUniversalTime().Ticks.ToString());

                //    System.Console.WriteLine("PC   : DS - " + fi.LastWriteTime.ToLocalTime().IsDaylightSavingTime());
                //}

                //int nodeIndex = treeNode.Nodes.Add(new TreeNode(fsi.Name));
                int nPos = filesList.BinarySearch(pcfi, new FileListNameComparer());

                if (nPos >= 0) filesList.Insert(nPos, pcfi);
                else filesList.Insert(~nPos, pcfi);
            }

            DirectoryInfo[] directoryInfos = dirInfo.GetDirectories();

            foreach (DirectoryInfo di in directoryInfos)
            {
                BizPCFileInfo pcfi = new BizPCFileInfo();

                pcfi.name = di.Name;

                int nPos = dirList.BinarySearch(pcfi, new FileListNameComparer());

                if (nPos >= 0) dirList.Insert(nPos, pcfi);
                else dirList.Insert(~nPos, pcfi);

                pcfi.isFolder = true;
                pcfi.ProcessFolder(di.FullName, bps, usd);
            }
        }
    }

    public class BizXenonFileInfo : BizFileInfo
    {
        static XboxConsole xboxConsole;

        static public bool Init( XboxConsole inXboxConsole )
        {
            bool success = true;

            xboxConsole = inXboxConsole;

            try
            {
                xboxConsole.FindConsole(1, 3000);
            }
            catch (COMException ex)
            {
                string errorString = xboxConsole.XboxManager.TranslateError(ex.ErrorCode);
                System.Console.WriteLine("Eek: " + ex.Message + "(" + errorString + ")");
                success = false;
            }

            return success;
        }

        override public BizFileInfo CreateFileInfo()
        {
            return new BizXenonFileInfo();
        }

        public override void ProcessFolder(string directoryName, BizProcessStatus bps, DelegateScanUpdate usd)
        {
            filesList = new ArrayList();
            dirList = new ArrayList();

            try
            {
                IXboxFiles xbf = xboxConsole.DirectoryFiles(directoryName);

                foreach (IXboxFile f in xbf)
                {
                    string fileName = f.Name.Substring(directoryName.Length + 1);

                    //int nodeIndex = treeNode.Nodes.Add(new TreeNode(fileName));

                    BizXenonFileInfo pcfi = new BizXenonFileInfo();

                    pcfi.name = f.Name.Substring(directoryName.Length + 1);
                    pcfi.size = f.Size;

                    if (bps != null)
                    {
                        bps.size += pcfi.size;
                        if (usd != null)
                            usd(bps.size);
                    }

                    pcfi.dtCreated = ((System.DateTime)f.CreationTime);
                    pcfi.dtModified = ((System.DateTime)f.ChangeTime);

                    pcfi.created = ((System.DateTime)f.CreationTime).ToUniversalTime().Ticks;
                    pcfi.lastModified = ((System.DateTime)f.ChangeTime).ToUniversalTime().Ticks;

                    //if (((System.DateTime)f.CreationTime).IsDaylightSavingTime())
                    //   pcfi.created += System.TimeSpan.TicksPerHour;

                    //if (((System.DateTime)f.ChangeTime).IsDaylightSavingTime())
                    //   pcfi.lastModified += System.TimeSpan.TicksPerHour;

                    ////if ((pcfi.name.ToLower().CompareTo("a.wmv") == 0) || (pcfi.name.ToLower().CompareTo("pgr3yearout.wmv") == 0))
                    //if ((pcfi.name.ToLower().CompareTo("environment.dds") == 0))
                    //{
                    //    IXboxFile xbfile = xboxConsole.GetFileObject(f.Name);

                    //    long ft1 = ((System.DateTime)xbfile.ChangeTime).ToFileTime();
                    //    long ft2 = ((System.DateTime)xbfile.ChangeTime).ToFileTimeUtc();

                    //    IXboxFile xbfile2 = xboxConsole.GetFileObject(f.Name);

                    //    long ft12 = ((System.DateTime)xbfile2.ChangeTime).ToFileTime();
                    //    long ft22 = ((System.DateTime)xbfile2.ChangeTime).ToFileTimeUtc();

                    //    //System.Console.WriteLine("Xenon: a.wmv - " + (pcfi.lastModified / 10000).ToString());
                    //    System.Console.WriteLine("Xenon: {0} - ToFileTime    - {1}", pcfi.name, ((System.DateTime)f.ChangeTime).ToFileTime().ToString());
                    //    System.Console.WriteLine("Xenon: {0} - ToFileTimeUtc - {1}", pcfi.name, ((System.DateTime)f.ChangeTime).ToFileTimeUtc().ToString());
                    //    System.Console.WriteLine("Xenon: {0} - ToLocalTime   - {1}", pcfi.name, ((System.DateTime)f.ChangeTime).ToLocalTime().ToString());
                    //    System.Console.WriteLine("Xenon: {0} - Ticks         - {1}", pcfi.name, ((System.DateTime)f.ChangeTime).Ticks.ToString());
                    //    System.Console.WriteLine("Xenon: {0} - UTicks        - {1}", pcfi.name, ((System.DateTime)f.ChangeTime).ToUniversalTime().Ticks.ToString());
                    //}

                    if (!f.IsDirectory)
                    {
                        int nPos = filesList.BinarySearch(pcfi, new FileListNameComparer());

                        if (nPos >= 0) filesList.Insert(nPos, pcfi);
                        else filesList.Insert(~nPos, pcfi);
                    }
                    else
                    {
                        int nPos = dirList.BinarySearch(pcfi, new FileListNameComparer());

                        if (nPos >= 0) dirList.Insert(nPos, pcfi);
                        else dirList.Insert(~nPos, pcfi);

                        pcfi.isFolder = true;
                        pcfi.ProcessFolder(f.Name, bps, usd);
                    }
                }
            }
            catch (COMException ex)
            {
                string errorString = xboxConsole.XboxManager.TranslateError(ex.ErrorCode);
                System.Console.WriteLine("Eek: " + ex.Message + "(" + errorString + ")");
            }
        }
    }
}

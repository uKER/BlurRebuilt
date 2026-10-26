using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.IO;

using XBrobocopy;

namespace SyncTool
{
    class Program
    {
        public static C_XBrobocopy xbRoboCopy = new C_XBrobocopy();

        delegate void CloseDelegate();
        delegate void RebuildAllocListDelegate(ulong size);
        delegate void MrTT(int size);

        static System.Threading.AutoResetEvent srcARE;
        static System.Threading.AutoResetEvent dstARE;

        static BizFileInfo bfiSrc;
        static BizFileInfo bfiDst;

        static InitialScan initialForm;

        static string pathSrc, pathDst;

        static void CloseMe()
        {
            initialForm.DialogResult = DialogResult.OK;
            initialForm.Close();
        }

        static void SetTextSrc(ulong size)
        {
            initialForm.SetSrcStatus(size);
        }

        static void SetTextDst(ulong size)
        {
            initialForm.SetDstStatus(size);
        }

        static void UpdateStatusCallbackSrc(ulong size)
        {
            initialForm.Invoke(new RebuildAllocListDelegate(SetTextSrc), size);
        }
        
        static void UpdateStatusCallbackDst(ulong size)
        {
            initialForm.Invoke(new RebuildAllocListDelegate(SetTextDst), size);
        }

        static void ScanFuncSrc()
        {
            try
            {
                BizProcessStatus bps = new BizProcessStatus();
                xbRoboCopy.SetScanDelegate(UpdateStatusCallbackSrc);
                bfiSrc = xbRoboCopy.ScanSrcDir(pathSrc, bps);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
            srcARE.Set();
        }

        static void ScanFuncDst()
        {
            try
            {
                BizProcessStatus bps = new BizProcessStatus();
                xbRoboCopy.SetScanDelegate(UpdateStatusCallbackDst);
                bfiDst = xbRoboCopy.ScanDstDir(pathDst, bps);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
            dstARE.Set();
        }

        static void ScanFunc()
        {
            srcARE = new System.Threading.AutoResetEvent(false);
            dstARE = new System.Threading.AutoResetEvent(false);

            System.Threading.Thread myTSrc = new System.Threading.Thread(ScanFuncSrc);
            System.Threading.Thread myTDst = new System.Threading.Thread(ScanFuncDst);

            myTSrc.IsBackground = true;
            myTDst.IsBackground = true;
            
            myTSrc.Start();
            myTDst.Start();

            System.Threading.AutoResetEvent[] areEvents = new System.Threading.AutoResetEvent[2];
            areEvents[0] = srcARE;
            areEvents[1] = dstARE;

            System.Threading.WaitHandle.WaitAll(areEvents);

            initialForm.Invoke(new CloseDelegate(CloseMe), null);

            // Wait for them both to finish
        }

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            pathSrc = args[0];
            pathDst = args[1];

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            initialForm = new InitialScan();
 
            // Connect to the xbox.
            if (!xbRoboCopy.ConnectToXbox(""))
                return;

            System.Threading.Thread myT = new System.Threading.Thread(ScanFunc);
            myT.Start();

            DialogResult dr = initialForm.ShowDialog();

            FileInfo src = new FileInfo(pathSrc);
            List<string> FileSpec = new List<string>();
            List<string> DirSpec = new List<string>();
            BizSyncWorkList workList = new BizSyncWorkList(src.Name);

            Statistics statistics = new Statistics();

            // Now we need to work out what the difference is between the two sets
            xbRoboCopy.CompareFolders(SyncType.CopyIfNewer, workList, bfiSrc, bfiDst, XBSubOptions.CopyEmptySub, pathSrc, "", FileSpec, DirSpec, ref statistics);

            if (workList.FileCount > 0)
            {
                workList.DumpGroupInfo();

                SyncProgress syncForm = new SyncProgress();

                syncForm.AddGroup("Total", workList.Size, false);

                foreach (BizSyncWorkList.Group group in workList.groups)
                {
                    syncForm.AddGroup(group.name, group.size, group.allowedToSkip);
                }

                syncForm.pathSrc = pathSrc;
                syncForm.pathDst = pathDst;

                syncForm.workList = workList;

                dr = syncForm.ShowDialog();
            }
            else
            {
                MessageBox.Show("There were no files found which you need.", "Bizarre Sync Tool", MessageBoxButtons.OK);
            }
        }
    }
}
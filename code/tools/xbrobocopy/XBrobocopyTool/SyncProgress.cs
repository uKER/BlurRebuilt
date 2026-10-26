using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

using XBrobocopy;

namespace SyncTool
{
    public partial class SyncProgress : Form
    {
        public BizSyncWorkList workList;
        public string pathSrc;
        public string pathDst;

        protected int xPos = 15;
        protected int xPos2 = 100;
        protected int xPos3 = 320;
        protected int xPos4 = 450;
        protected int yPos = 10;
        protected int yStep = 20;
        protected int groupIndex = 1;

        public class Group
        {
            public System.Windows.Forms.Label label;
            public System.Windows.Forms.Label label2;
            public System.Windows.Forms.ProgressBar progressBar;
            public System.Windows.Forms.CheckBox checkBox;
            public ulong size;
            public ulong totalSize;
            public bool enabled;
       }

        List<Group> groups;

        public string SizeString(ulong leftVal, ulong rightVal)
        {
            if (rightVal < 1024)
                return String.Format("{0:,0} / {1:,0} Bytes", leftVal, rightVal);
            else if (rightVal < 1024*1024)
                return String.Format("{0:,0.0} / {1:,0.0} Kb", leftVal / 1024.0, rightVal / 1024.0);
            else
                return String.Format("{0:,0.0} / {1:,0.0} Mb", leftVal / (1024.0 * 1024.0), rightVal / (1024.0 * 1024.0));
        }

        public void AddGroup(string name, ulong newSize, bool allowDisable)
        {
            Group newGroup = new Group();

            newGroup.size = 0;
            newGroup.totalSize = newSize;

            if (newSize > 0)
            {
                newGroup.enabled = true;

                newGroup.label = new System.Windows.Forms.Label();
                newGroup.label.Text = name;
                newGroup.label.AutoSize = true;
                newGroup.label.Location = new System.Drawing.Point(xPos, yPos);
                newGroup.label.Name = name;

                newGroup.progressBar = new System.Windows.Forms.ProgressBar();
                newGroup.progressBar.Location = new System.Drawing.Point(xPos2, yPos);
                newGroup.progressBar.Name = name + "_pb";
                newGroup.progressBar.Size = new System.Drawing.Size(213, 17);
                newGroup.progressBar.Value = 0;

                newGroup.label2 = new System.Windows.Forms.Label();
                newGroup.label2.Text = SizeString(0, newGroup.totalSize);
                newGroup.label2.AutoSize = true;
                newGroup.label2.Location = new System.Drawing.Point(xPos3, yPos);
                newGroup.label2.Name = name;

                Controls.Add(newGroup.label);
                Controls.Add(newGroup.progressBar);
                Controls.Add(newGroup.label2);

                if (allowDisable)
                    Controls.Add(newGroup.checkBox);

                yPos += yStep;
            }
            else
            {
                newGroup.enabled = false;
            }

            groups.Add(newGroup);
            

            this.listView1.Location = new System.Drawing.Point(12, yPos + 10);
            this.ClientSize = new System.Drawing.Size(536, yPos + 10 + this.listView1.Size.Height + 10);
        }

        public SyncProgress()
        {
            InitializeComponent();

            groups = new List<Group>();
        }

        public void UpdateInfoDele(string name, string directory, ulong size, XBStatus status, BizSyncWorkList.BizWorkItem.WorkItemStatus wiStatus)
        {
            if (status == XBStatus.Trying)
            {
                this.Invoke(new UpdateInfoDelegate(UpdateInfo1), new object[] { name, size, status });
            }
            else
            {
                this.Invoke(new UpdateInfoDelegate(UpdateInfo2), new object[] { name, size, status });
            }
        }

        public void UpdateInfo1( string name, ulong size, XBStatus status)
        {
            this.listView1.BeginUpdate();
            ListViewItem item = this.listView1.Items.Add(name);
            this.listView1.Items[this.listView1.Items.Count - 1].EnsureVisible();
            this.listView1.EndUpdate();

            item.SubItems.Add(String.Format("{0:,0}", size));
            item.SubItems.Add("Trying");
        }

        public void UpdateInfo2( string name, ulong size, XBStatus status)
        {
            ListViewItem item = this.listView1.Items[this.listView1.Items.Count - 1];

            if (status == XBStatus.Success)
                item.SubItems[2].Text = "Success";
            else
                item.SubItems[2].Text = "Failed";

            Group grpT = this.groups[0];
            Group grp = this.groups[groupIndex];

            grpT.size += size;
            grp.size += size;

            grpT.label2.Text = SizeString(grpT.size, grpT.totalSize);
            grpT.progressBar.Value = (int)(100.0 * grpT.size / grpT.totalSize);
            grp.label2.Text = SizeString(grp.size, grp.totalSize);
            grp.progressBar.Value = (int)(100.0 * grp.size / grp.totalSize);

        }

        delegate void UpdateInfoDelegate(string name, ulong size, XBStatus status);

        void MySyncWorker()
        {
            int groupIndex = 1;

            Program.xbRoboCopy.SetCopyDelegate(UpdateInfoDele);

            // Need to create an intial directory.
            Program.xbRoboCopy.CreateFolder(pathDst);     
           
            foreach (BizSyncWorkList.Group group in workList.groups)
            {
                if (groups[groupIndex].enabled)
                {
                    foreach (BizSyncWorkList.BizWorkItem item in group.items)
                    {
                        System.Diagnostics.Debug.WriteLine(String.Format("MySyncWorker - item({0}) size({1})", item.name, item.size));

                        string filenameSrc = pathSrc + "\\" + item.name;
                        string filenameDst = pathDst + "\\" + item.name;

                        if (!item.folder)
                        {
                            Program.xbRoboCopy.CopyFile(filenameSrc, filenameDst, item.m_Status);
                        }
                        else
                        {
                            Program.xbRoboCopy.CreateFolder(pathDst + "\\" + item.name);
                        }
                    }
                }
                groupIndex++;  
            }
            this.Invoke((MethodInvoker)delegate() { this.buttonGo.Text = "Complete!"; });
        }

        private void buttonGo_Click(object sender, EventArgs e)
        {
            System.Threading.Thread myWorker = new System.Threading.Thread(MySyncWorker);

            myWorker.IsBackground = true;

            myWorker.Start();

            buttonGo.Text = "Running!";
            buttonGo.Enabled = false;
        }
    }
}
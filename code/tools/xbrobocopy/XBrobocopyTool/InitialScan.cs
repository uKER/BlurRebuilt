using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace SyncTool
{
    public partial class InitialScan : Form
    {
        public bool iWasClosed;

        class MyInfo
        {
            public ulong size;

            public MyInfo()
            {
                size = 0;
            }
        }

        public void SetSrcStatus(ulong size)
        {
            ulong val = ((MyInfo)labelSrc.Tag).size;

            if (size > (val + 10 * 1024 * 1024))
            {
                labelSrc.Text = String.Format("Scanning source folder... {0:0,0.0}Mb", size / (1024.0 * 1024.0));

                ((MyInfo)labelSrc.Tag).size = size;
            }            
        }

        public void SetDstStatus(ulong size)
        {
            ulong val = ((MyInfo)labelDst.Tag).size;

            if (size > (val + 10 * 1024 * 1024))
            {
                labelDst.Text = String.Format("Scanning destination folder... {0:0,0.0}Mb", size / (1024.0 * 1024.0));

                ((MyInfo)labelDst.Tag).size = size;
            }
        }

        public InitialScan()
        {
            InitializeComponent();

            labelSrc.Tag = new MyInfo();
            labelDst.Tag = new MyInfo();

            iWasClosed = false;
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            iWasClosed = true;
        }
    }
}
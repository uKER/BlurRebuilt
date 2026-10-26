using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace ProfilerTool.Controls
{
    public partial class Log : UserControl
    {
        public enum Types
        {
        }

        public Log(string strItem)
        {
            InitializeComponent();

            ColumnHeader frame = new ColumnHeader();
            frame.Text = "Frame";
            frame.Width = 75;
            lvLog.Columns.Add(frame);

            ColumnHeader type = new ColumnHeader();
            type.Text = "Type";
            type.Width = 75;
            lvLog.Columns.Add(type);

            ColumnHeader msg = new ColumnHeader();
            msg.Text = "Message";
            msg.Width = 175;
            lvLog.Columns.Add(msg);

            ColumnHeader com = new ColumnHeader();
            com.Text = strItem;
            com.Width = 150;
            lvLog.Columns.Add(com);
        }

        public void AddLine(UInt32 frame, string msg, string par1, string par2, Color textCol, Color backCol)
        {
            if (!cbxPause.Checked)
            {
                ListViewItem lvItem = new ListViewItem("[" + Convert.ToString(frame) + "]");
                lvItem.SubItems.Add(msg);
                lvItem.SubItems.Add(par2);
                lvItem.SubItems.Add(par1);
                lvItem.ForeColor = textCol;
                lvItem.BackColor = backCol;
                lvLog.Items.Add(lvItem);
                lvLog.EnsureVisible(lvItem.Index);
            }
        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            lvLog.Items.Clear();
        }
    }
}

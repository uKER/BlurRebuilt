using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Global;
using System.Xml;
using System.Collections;
using System.Diagnostics;
using System.Media;

namespace MenuSystemLibrary
{
	public partial class Container_CustomList : MenuSystemLibrary.Container_Base
	{
		private Container_CustomList()
		{
			InitializeComponent();
		}

		public Container_CustomList(MenuSystem owner, XmlElement xContainer, bool bCreateChildren)
			: base(owner, xContainer, bCreateChildren)
		{
			InitializeComponent();

			// Put any more constructor logic here.
		}

// Ignore these unless you want your control to have children.
//		protected override Control ChildContainer { get { return null; } }
//		protected override ArrayList Children { get { return null; } }

		/// <summary>
		/// This is called by the system to set the name of this control.
		/// </summary>
		override internal string ObjectName
		{
		    set { this.labelControlName.Text = value; }
		}

		/// <summary>
		/// If you override this function then it is up to you to make your control 'readonly'.
		/// The simplest way is to set the Enabled flag on the control. This is default behaviour,
		/// however you may wish to do something a little more elegant.
		/// </summary>
		/// <param name="bReadonly"></param>
		override protected void MakeReadonly(bool bReadonly)
		{
			//this.Enabled = !bReadonly;		// No need for this override if this is all you do.

            but_add.Enabled = !bReadonly;
            but_remove.Enabled = !bReadonly;
            textBoxContainerValue.ReadOnly = bReadonly;

            if ( bReadonly )
                listBox.BackColor = SystemColors.Control;
            else
                listBox.BackColor = SystemColors.Window;
		}

		/// <summary>
		/// This is called by the system to get or set the value for this control.
		/// </summary>
		override internal string ObjectValue
		{
			get
            {
                string strResult = "";

                foreach (Object lbo in this.listBox.Items)
                {
                    if (strResult != "")
                        strResult += "\n";

                    strResult += lbo.ToString();
                }
                return strResult;
            }
			set	
            {
                this.listBox.Items.Clear();
                string[] strings = value.Split(new char[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);

                foreach (string strValue in strings)
                {
                    this.listBox.Items.Add(strValue);
                }
                label_target.Text = "Targets: " + listBox.Items.Count;
            }   
		}

		/// <summary>
		/// Optional. Returns the control that any tooltip text is associated with.
		/// </summary>
		internal override Control ObjectTooltipTarget
		{
			get
			{
				return this.textBoxContainerValue;
			}
		}

		/// <summary>
		/// This is called when the textbox changes.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
        //private void textBoxContainerValue_TextChanged(object sender, EventArgs e)
        //{
        //    // You must call this when you want to update the value of your container.
        //    ValueChanged("value", this.textBoxContainerValue.Text, false);

        //    // Optionally do other stuff.
        //}

        private void but_add_Click(object sender, EventArgs e)
        {
            if (textBoxContainerValue.TextLength > 0)
            {
                listBox.Items.Add( textBoxContainerValue.Text );
                textBoxContainerValue.Clear();
                ValueChanged("value", ObjectValue, false);
                label_target.Text = "Targets: " + listBox.Items.Count;
            }
        }

        private void but_remove_Click(object sender, EventArgs e)
        {
            int size = listBox.SelectedIndices.Count;

            for (int i = 0; i < size; i++)
            {
                listBox.Items.RemoveAt(listBox.SelectedIndices[0]);
                ValueChanged("value", ObjectValue, false);
                label_target.Text = "Targets: " + listBox.Items.Count;
            }
        }

        private void label2_Click(object sender, EventArgs e)
        {

        }

        private void listBox_ValueMemberChanged(object sender, EventArgs e)
        {
            
        }

        private void textBoxContainerValue_KeyUp(object sender, KeyEventArgs e)
        {
            if ( e.KeyCode == Keys.Enter )
            {
                if (textBoxContainerValue.TextLength > 0)
                {
                    listBox.Items.Add( textBoxContainerValue.Text );
                    textBoxContainerValue.Clear();
                    ValueChanged("value", ObjectValue, false);
                    label_target.Text = "Targets: " + listBox.Items.Count;
                }
            }
        }

        private void listBox_PasteClip(object sender, EventArgs e)
        {
            IDataObject clipObj = Clipboard.GetDataObject();

            string[] strings = Clipboard.GetText().Split(new char[] { '\n', '\r' }, StringSplitOptions.RemoveEmptyEntries);

            foreach (string strValue in strings)
            {
                listBox.Items.Add(strValue);
                ValueChanged("value", ObjectValue, false);
                label_target.Text = "Targets: " + listBox.Items.Count;
            }
        }

        private void listBox_SelectAll(object sender, EventArgs e)
        {
            for (int i = 0; i < listBox.Items.Count; i++)
            {
                listBox.SetSelected(i, true);
            }
        }


        private void listBox_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                ContextMenu cm = new ContextMenu();

                cm.MenuItems.Add("Paste", listBox_PasteClip);
                cm.MenuItems.Add("Select All", listBox_SelectAll);

                cm.Show(this.listBox, new Point(0, e.Y));
            }
        }
	}
}


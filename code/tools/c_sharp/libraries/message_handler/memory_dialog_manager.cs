using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using Global;
using SuperControl;
using System.Diagnostics;

namespace message_handler
{
	public partial class MemoryDialogForm : Form
	{
		internal MemoryDialogForm()
		{
			InitializeComponent();
		}

		/// <summary>
		/// This is called via a delegate either when the comboBox needs populating or when
		/// the comboBox selection has been made.
		/// In the former case the comboBox should be populated. In the latter the comboBox can
		/// be interrogated for any newly added data.
		/// </summary>
		/// <param name="owner">The listview item that owns the control.</param>
		/// <param name="control">The control that needs to be populated.</param>
		/// <param name="set">True if the control needs populating, false if it is simply reporting what is in it.
		/// This is useful for controls that can have new items added, if the code wants to remember them for next time.</param>
		/// <returns></returns>
		static bool PopulateSuperListView_Action(object myObject, Control control, ListViewItem.ListViewSubItem lvsi, bool set)
		{
			ComboBox cb = control as ComboBox;
			if(set == true)
			{
				string strType = myObject as string;

				cb.Items.Clear();
				switch(strType)
				{
					case "AbortRetryIgnore":
						{
							cb.Items.Add("Abort");
							cb.Items.Add("Retry");
							cb.Items.Add("Ignore");
						}
						break;
					case "OK":
						{
							cb.Items.Add("OK");
						}
						break;
					case "OKCancel":
						{
							cb.Items.Add("OK");
							cb.Items.Add("Cancel");
						}
						break;
					case "RetryCancel":
						{
							cb.Items.Add("Retry");
							cb.Items.Add("Cancel");
						}
						break;
					case "YesNo":
						{
							cb.Items.Add("Yes");
							cb.Items.Add("No");
						}
						break;
					case "YesNoCancel":
						{
							cb.Items.Add("Yes");
							cb.Items.Add("No");
							cb.Items.Add("Cancel");
						}
						break;

					default:
						{
							cb.Items.Add("*ERROR*");
							return false;
						}
				}
				cb.Items.Add("Ask");
				cb.Text = lvsi.Text;
			}
			else
			{
				lvsi.Text = cb.Text;
				Debug.WriteLine("ComboBox changed.");
			}

			return true;
		}

		static bool PopulateSuperListView_Button(object myObject, Control control, ListViewItem.ListViewSubItem lvsi, bool set)
		{
			Button button = control as Button;
			if (set == true)
			{
				button.Text = lvsi.Text;
			}
			else
			{
				control.Parent.Focus();					// Remove focus from the button so it makes itself invisible.
//				MessageBox.Show(control.Parent, "You have pressed:\n" + button.Text, "Executer test", MessageBoxButtons.OK, MessageBoxIcon.Information);
			}

			return true;
		}

		static bool PopulateSuperListView_DateTimePicker(object myObject, Control control, ListViewItem.ListViewSubItem lvsi, bool set)
		{
			DateTimePicker dtp = control as DateTimePicker;
			if (set == true)
			{
				try
				{
					dtp.Text = lvsi.Text;									/// This will cause an exception if the string is not a valid date.
				}
				catch
				{
				}
			}
			else
			{
				lvsi.Text = dtp.Text;
			}

			return true;
		}

		public static DialogResult Manage(IWin32Window owner)
		{
			MemoryDialogForm messageHandlerForm = new MemoryDialogForm();

			//XmlElement xMemory = DialogMemory.GetMemoryElement();
			XmlElement xMemory = MemoryDialog.GetMemoryRoot();

			messageHandlerForm.superListView.Columns.Add("Question", 400);
			//			messageHandlerForm.superListView.Columns.Add("Type", 120);
			messageHandlerForm.superListView.Columns.Add("Action", 90);

			foreach(XmlNode xNode in xMemory)
			{
				if(xNode.NodeType == XmlNodeType.Element)
				{
					XmlElement xElement = xNode as XmlElement;

					string strType = XmlHelper.GetKeyValue(xElement, "type");			// May or may not exist.
					string strQuestion = XmlHelper.GetKeyValue(xElement, "question");	// May or may not exist.
					string strAnswer = XmlHelper.GetKeyValue(xElement, "answer");		// May or may not exist.

					if(strQuestion != null && strQuestion != "")
					{
						ListViewItem lvi = new ListViewItem(strQuestion.Replace('\n', ' '));
						ListViewItem.ListViewSubItem lviAction = lvi.SubItems.Add(strAnswer);

						messageHandlerForm.superListView.AddItemControl(lviAction, SuperListViewItemControl.ControlType.ComboBox, new SuperListView.DelegateManageControl(PopulateSuperListView_Action), strType);
						messageHandlerForm.superListView.AddItemControl(lvi.SubItems[0], SuperListViewItemControl.ControlType.Button, new SuperListView.DelegateManageControl(PopulateSuperListView_Button), null);
						//messageHandlerForm.superListView.AddItemControl(lvi.SubItems[0], SuperListViewItemControl.ControlType.DateTimePicker, new SuperListView.DelegateManageControl(PopulateSuperListView_DateTimePicker), null);

						messageHandlerForm.superListView.Items.Add(lvi);
					}
				}
			}
			messageHandlerForm.ShowDialog(owner);

			return DialogResult.OK;
		}

		#region Events.
		private void buttonOK_Click(object sender, EventArgs e)
		{
			buttonApply_Click(sender, e);
			this.DialogResult = DialogResult.OK;
		}

		private void buttonCancel_Click(object sender, EventArgs e)
		{
			this.DialogResult = DialogResult.Cancel;
		}

		private void buttonApply_Click(object sender, EventArgs e)
		{
			ApplyChanges();
		}

		#endregion Events.

		/// <summary>
		/// Apply any changes that have been made in the SuperListView.
		/// This function is reliant on the fact that the listView contents correlate directly with
		/// the xMemory contents.
		/// </summary>
		private void ApplyChanges()
		{
			//XmlElement xMemory = DialogMemory.GetMemoryElement();
			XmlElement xMemory = MemoryDialog.GetMemoryRoot();

			int itemCounter = 0;

			ListViewItem lvi;

			foreach(XmlNode xNode in xMemory)
			{
				if(xNode.NodeType == XmlNodeType.Element)
				{
					string strQuestion = XmlHelper.GetKeyValue(xNode, "question");
					if(strQuestion != null && strQuestion != "")
					{
						lvi = this.superListView.Items[itemCounter++];

						XmlElement xElement = xNode as XmlElement;
						XmlHelper.AddAttribute(xElement, "answer", lvi.SubItems[1].Text);
					}
				}
			}
		}
	}
}
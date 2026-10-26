using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using Windows;

// TODO: Stop using SubItem.Tag. Overload Subitem to SuperSubItem and use SuperSubItem.SuperListViewItemControl. This is not trivial.
// Alternatively maintain a map in the SuperListView that links the subitems to an object.
// TODO: ??? Allow addition of more than one control to a subitem. If there is more than one then clicking produces a context menu to choose which one to use.

// TODO: In SuperListViewItemControl, add a reference to a draw type, e.g. a progress bar, along with an 'evaluate' callback. Intercept the SubItem OnPaint method and
// draw the type with the correct values. The evaluate method can use the SubItem.Text entry or external data.

namespace SuperControl
{
	public partial class SuperListView : ListView
	{
		private ListViewItem lvItem = null;

		/// <summary>
		/// This is the delegate to be called when the control needs populating or when
		/// the control selection has been made.
		/// In the former case the control should be populated. In the latter the control can
		/// be interrogated for any newly added data.
		/// </summary>
		/// <param name="owner">The listview item that owns the control.</param>
		/// <param name="control">The control that needs to be managed.</param>
		/// <param name="set">True if the control needs populating, false if it is simply reporting what is in it.
		/// This is useful for controls that can have new items added, if the code wants to remember them for next time.</param>
		/// <returns></returns>
		public delegate bool DelegateManageControl(object myObject, Control control, ListViewItem.ListViewSubItem lvsi, bool set);
		SuperListViewItemControl m_slvic;				/// The container for the control that's currently active.
		string m_strOriginalValue;
//		Control m_currentControl = null;
		
		public SuperListView()
		{
			InitializeComponent();
		}

		ListViewHitTestInfo hitTestInfo;
//111
		[System.Runtime.InteropServices.DllImport("user32.dll")]
		private static extern IntPtr SendMessage(IntPtr hWnd, int msg, int wParam, int[]lParam);

		private void SuperListView_MouseUp(object sender, MouseEventArgs e)
		{
			// Get the item on the row that is clicked.
			//			lvItem = this.GetItemAt(e.X, e.Y);

			hitTestInfo = this.HitTest(e.X, e.Y);

			// Make sure that an item is clicked.
			
			if(hitTestInfo != null)
			{
				lvItem = hitTestInfo.Item;
				if(lvItem != null && hitTestInfo.SubItem != null)
				{
					if(hitTestInfo.SubItem.Tag is SuperListViewItemControl)
					{
						m_slvic = hitTestInfo.SubItem.Tag as SuperListViewItemControl;

						m_strOriginalValue = hitTestInfo.SubItem.Text;

						/// Find the correct control, populate it and set the starting value.
						Control control = m_slvic.PrepareControl(this, hitTestInfo.SubItem);

						// Get the bounds of the item that is clicked.
						Rectangle ClickedItem = hitTestInfo.SubItem.Bounds;

						#region Get the x-coordinate for the control.

						int columnCount = Columns.Count;
						int[] columnOrder = new int[columnCount];

						Rectangle itemRect = lvItem.GetBounds(ItemBoundsPortion.Entire);
						SendMessage(this.Handle, (int) Windows.Message.LVM.GetColumnOrderArray, columnCount, columnOrder);

						int x = itemRect.Left;

						for(int i = 0; i < columnCount; i++)
						{
							if(hitTestInfo.SubItem == lvItem.SubItems[columnOrder[i]])
								break;
							x += Columns[columnOrder[i]].Width;
						}
						#endregion Get the x-coordinate for the control.

						ClickedItem.X = x;

						if(lvItem.SubItems[0] == hitTestInfo.SubItem)														/// HACK.
						{
							ClickedItem.Width = this.Columns[0].Width;
						}
						
						control.Bounds = ClickedItem;

						// Display the control, and make sure that it is on top with focus.
						control.Parent = this;
						control.Visible = true;
						control.BringToFront();
						control.Focus();

						/// Custom control modification.
						if(control == this.comboBox)
							this.comboBox.DroppedDown = true;
					}
				}
			}
		}

		/// <summary>
		/// This adds a control to a listview subitem.
		/// </summary>
		/// <param name="lvSubItem">The subitem to add the control to.</param>
		/// <param name="controlType">The type of the control, e.g. ControlType.ComboBox</param>
		/// <param name="DelegateManageControl">The delegate to call when the control needs populating or has updated itself.</param>
		/// <param name="myObject">A user defined object to pass to the delegate.</param>
		public void AddItemControl(ListViewItem.ListViewSubItem lvSubItem, SuperListViewItemControl.ControlType controlType, DelegateManageControl delegateManageControl, object myObject)
		{
			lvSubItem.Tag = new SuperListViewItemControl(this, controlType, myObject, delegateManageControl);			
		}

		private void SuperListView_ColumnClick(object sender, ColumnClickEventArgs e)
		{
//			this.Focus();
		}

		private void SuperListView_ColumnReordered(object sender, ColumnReorderedEventArgs e)
		{
			this.Focus();
		}

		private void SuperListView_ColumnWidthChanged(object sender, ColumnWidthChangedEventArgs e)
		{

		}

		private void SuperListView_ColumnWidthChanging(object sender, ColumnWidthChangingEventArgs e)
		{
//			Debug.WriteLine(e.NewWidth.ToString());
			if (e.NewWidth < 50 || this.Columns[e.ColumnIndex].Width < 50)
			{
				e.Cancel = true;
				this.Columns[e.ColumnIndex].Width = 50;
				e.NewWidth = 50;
			}

			if(m_slvic != null)
				if (m_slvic.m_control.Visible == true)
					m_slvic.m_control.Width = e.NewWidth;

//			if (m_currentControl != null)
//				if (m_currentControl.Visible == true)
//					m_currentControl.Width = e.NewWidth;
		}

		#region Control events.
		private void control_SelectedValueChanged(object sender, EventArgs e)
		{
			m_slvic = hitTestInfo.SubItem.Tag as SuperListViewItemControl;
			m_slvic.ManageControl(m_slvic.m_object, sender as Control, hitTestInfo.SubItem, false);
		}

		private void control_Leave(object sender, EventArgs e)
		{
			m_slvic = hitTestInfo.SubItem.Tag as SuperListViewItemControl;
			m_slvic.ManageControl(m_slvic.m_object, sender as Control, hitTestInfo.SubItem, false);
			(sender as Control).Visible = false;
		}

		private void control_KeyPress(object sender, KeyPressEventArgs e)
		{
			/// Some keyboard actions need to be dealt with.
			switch (e.KeyChar)
			{
				case (char)(int)Keys.Escape:
					{
						// Reset the original text value, and then hide the control.
						m_slvic = hitTestInfo.SubItem.Tag as SuperListViewItemControl;
//						hitTestInfo.SubItem.Text = m_strOriginalValue;				/// No good because the 'Leave' event fires after this.
						try
						{
							(sender as Control).Text = m_strOriginalValue;			/// try ... catch since some controls are picky about what you try and put in them.
						}
						catch { }

						(sender as Control).Visible = false;
						break;
					}

				case (char)(int)Keys.Enter:
					{
						m_slvic = hitTestInfo.SubItem.Tag as SuperListViewItemControl;
//						m_slvic.ManageControl(m_slvic.m_object, sender as Control, hitTestInfo.SubItem, false);		// The 'Leave' event does this.
						(sender as Control).Visible = false;
						break;
					}
			}
		}

		#endregion ComboBox events.

		#region Control events.
		private void comboBox_SelectedValueChanged(object sender, EventArgs e)
		{
			control_SelectedValueChanged(sender, e);
		}

		private void comboBox_Leave(object sender, EventArgs e)
		{
			control_Leave(sender, e);
		}

		private void comboBox_KeyPress(object sender, KeyPressEventArgs e)
		{
			control_KeyPress(sender, e);
		}

		#endregion ComboBox events.

		#region DateTimePicker events.
		private void dateTimePicker_ValueChanged(object sender, EventArgs e)
		{
			control_SelectedValueChanged(sender, e);
		}

		private void dateTimePicker_Leave(object sender, EventArgs e)
		{
			control_Leave(sender, e);
		}

		private void dateTimePicker_KeyPress(object sender, KeyPressEventArgs e)
		{
			control_KeyPress(sender, e);
		}
		#endregion DateTimePicker events.

		#region Button events.
		private void button_Leave(object sender, EventArgs e)
		{
//			control_Leave(sender, e);
			(sender as Control).Visible = false;
		}

		private void button_KeyPress(object sender, KeyPressEventArgs e)
		{
		}

		private void button_Click(object sender, EventArgs e)
		{
			/// Buttons are special. They do not pass values back to the SuperListView however they
			/// need to tell their SuperListView that they have been clicked.
			control_SelectedValueChanged(sender, e);
		}

		private void button_Enter(object sender, EventArgs e)
		{
			/// The button autosizes itself. This ensures that it does not start out too big.
			Button button = sender as Button;
//			button.Width = 50;
		}
		#endregion Button events.



	}
}
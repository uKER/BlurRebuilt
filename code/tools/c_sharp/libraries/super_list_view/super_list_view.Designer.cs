using System.Windows.Forms;
namespace SuperControl
{
	partial class SuperListView
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if(disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Component Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.comboBox = new System.Windows.Forms.ComboBox();
			this.dateTimePicker = new System.Windows.Forms.DateTimePicker();
			this.button = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// comboBox
			// 
			this.comboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBox.FormattingEnabled = true;
			this.comboBox.Items.AddRange(new object[] {
            "hi",
            "this",
            "is",
            "a",
            "test"});
			this.comboBox.Location = new System.Drawing.Point(0, 0);
			this.comboBox.MaxDropDownItems = 20;
			this.comboBox.Name = "comboBox";
			this.comboBox.Size = new System.Drawing.Size(121, 21);
			this.comboBox.TabIndex = 0;
			this.comboBox.Visible = false;
			this.comboBox.Leave += new System.EventHandler(this.comboBox_Leave);
			this.comboBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.comboBox_KeyPress);
			this.comboBox.SelectedValueChanged += new System.EventHandler(this.comboBox_SelectedValueChanged);
			// 
			// dateTimePicker
			// 
			this.dateTimePicker.Location = new System.Drawing.Point(0, 0);
			this.dateTimePicker.Name = "dateTimePicker";
			this.dateTimePicker.Size = new System.Drawing.Size(200, 20);
			this.dateTimePicker.TabIndex = 0;
			this.dateTimePicker.Visible = false;
			this.dateTimePicker.ValueChanged += new System.EventHandler(this.dateTimePicker_ValueChanged);
			this.dateTimePicker.Leave += new System.EventHandler(this.dateTimePicker_Leave);
			this.dateTimePicker.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.dateTimePicker_KeyPress);
			// 
			// button
			// 
			this.button.AutoSize = true;
			this.button.Location = new System.Drawing.Point(0, 0);
			this.button.MinimumSize = new System.Drawing.Size(0, 23);
			this.button.Name = "button";
			this.button.Size = new System.Drawing.Size(75, 23);
			this.button.TabIndex = 0;
			this.button.Text = "button1";
			this.button.UseVisualStyleBackColor = true;
			this.button.Enter += new System.EventHandler(this.button_Enter);
			this.button.Click += new System.EventHandler(this.button_Click);
			this.button.Leave += new System.EventHandler(this.button_Leave);
			this.button.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.button_KeyPress);
			// 
			// SuperListView
			// 
			this.Controls.Add(this.comboBox);
			this.ColumnWidthChanged += new System.Windows.Forms.ColumnWidthChangedEventHandler(this.SuperListView_ColumnWidthChanged);
			this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.SuperListView_MouseUp);
			this.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(this.SuperListView_ColumnClick);
			this.ColumnReordered += new System.Windows.Forms.ColumnReorderedEventHandler(this.SuperListView_ColumnReordered);
			this.ColumnWidthChanging += new System.Windows.Forms.ColumnWidthChangingEventHandler(this.SuperListView_ColumnWidthChanging);
			this.ResumeLayout(false);

		}

		#endregion

		private const int WM_HSCROLL = 0x114;
		private const int WM_VSCROLL = 0x115;

		protected override void WndProc(ref Message msg)
		{
			// Look for the WM_VSCROLL or the WM_HSCROLL messages.
			if((msg.Msg == WM_VSCROLL) || (msg.Msg == WM_HSCROLL))
			{
				// Move focus to the ListView to cause control to lose focus.
				this.Focus();
			}
/* TODO:
			if(msg.Msg == WM_PAINT)
			{
				// Call a function to position all the embedded controls...
			}
*/
			// Pass message to default handler.
			base.WndProc(ref msg);
		}
		internal DateTimePicker dateTimePicker;
		internal ComboBox comboBox;
		internal Button button;
	}
}

using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.Xml;
using System.Diagnostics;

namespace ActorSystem
{
	/// <summary>
	/// Summary description for ActorDropbox.
	/// </summary>
	public class ActorDropbox : ActorParameter
	{
		private System.Windows.Forms.GroupBox MainGroupBox;
		internal System.Windows.Forms.ComboBox comboBox;
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		internal ActorDropbox()		// For the designer only.
		{
			InitializeComponent();		
		}

		internal ActorDropbox(Control owner, XmlNode valueNode, ArrayList strEntries) : base(owner, valueNode)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// Add any initialization after the InitializeComponent call
			foreach(string strEntry in strEntries)
				this.comboBox.Items.Add(strEntry);
		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
//			Debug.WriteLine("Disposed dropbox.");
		}


		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.MainGroupBox = new System.Windows.Forms.GroupBox();
			this.comboBox = new System.Windows.Forms.ComboBox();
			this.MainGroupBox.SuspendLayout();
			this.SuspendLayout();
			// 
			// MainGroupBox
			// 
			this.MainGroupBox.Controls.Add(this.comboBox);
			this.MainGroupBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.MainGroupBox.Location = new System.Drawing.Point(0, 0);
			this.MainGroupBox.Name = "MainGroupBox";
			this.MainGroupBox.Size = new System.Drawing.Size(152, 40);
			this.MainGroupBox.TabIndex = 0;
			this.MainGroupBox.TabStop = false;
			this.MainGroupBox.Text = "Put name here";
			// 
			// comboBox
			// 
			this.comboBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.comboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBox.Location = new System.Drawing.Point(8, 16);
			this.comboBox.Name = "comboBox";
			this.comboBox.Size = new System.Drawing.Size(136, 21);
			this.comboBox.TabIndex = 0;
			this.comboBox.SelectedIndexChanged += new System.EventHandler(this.comboBox_SelectedIndexChanged);
			// 
			// ActorDropbox
			// 
			this.Controls.Add(this.MainGroupBox);
			this.Name = "ActorDropbox";
			this.Size = new System.Drawing.Size(152, 40);
			this.Load += new System.EventHandler(this.ActorDropbox_Load);
			this.MainGroupBox.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void comboBox_SelectedIndexChanged(object sender, System.EventArgs e)
		{
				CallParameterChanged(this);
		}

		private void ActorDropbox_Load(object sender, System.EventArgs e)
		{
		
		}

		#region IActorParameter Members

		public override string _ActorName
		{
			get
			{
				return this.MainGroupBox.Text;
			}
			set
			{
				this.MainGroupBox.Text = value;
			}
		}

		public override string _ActorValue
		{
			get
			{
				return this.comboBox.SelectedItem as string;
			}
			set
			{
				this.comboBox.SelectedItem = value;
			}
		}



		#endregion
	}
}

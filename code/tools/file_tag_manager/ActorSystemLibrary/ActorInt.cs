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
	/// Summary description for ActorInt.
	/// </summary>
	public class ActorInt : ActorParameter, IActorParameter
	{
		private System.Windows.Forms.TextBox editBox;
		private System.Windows.Forms.GroupBox MainGroupBox;
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ActorInt(Control owner, XmlNode valueNode) : base(owner, valueNode)
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitializeComponent call

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
//			Debug.WriteLine("Disposed Int.");

		}

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.MainGroupBox = new System.Windows.Forms.GroupBox();
			this.editBox = new System.Windows.Forms.TextBox();
			this.MainGroupBox.SuspendLayout();
			this.SuspendLayout();
			// 
			// MainGroupBox
			// 
			this.MainGroupBox.Controls.Add(this.editBox);
			this.MainGroupBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.MainGroupBox.Location = new System.Drawing.Point(0, 0);
			this.MainGroupBox.Name = "MainGroupBox";
			this.MainGroupBox.Size = new System.Drawing.Size(152, 40);
			this.MainGroupBox.TabIndex = 0;
			this.MainGroupBox.TabStop = false;
			this.MainGroupBox.Text = "Put name here";
			// 
			// editBox
			// 
			this.editBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.editBox.HideSelection = false;
			this.editBox.Location = new System.Drawing.Point(8, 16);
			this.editBox.Name = "editBox";
			this.editBox.Size = new System.Drawing.Size(140, 20);
			this.editBox.TabIndex = 0;
			this.editBox.Text = "empty";
			this.editBox.TextChanged += new System.EventHandler(this.editBox_TextChanged);
			// 
			// ActorInt
			// 
			this.Controls.Add(this.MainGroupBox);
			this.Name = "ActorInt";
			this.Size = new System.Drawing.Size(152, 40);
			this.MainGroupBox.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void editBox_TextChanged(object sender, System.EventArgs e)
		{
			CallParameterChanged(this);
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
				// TODO:  Add ActorInt.ActorValue getter implementation
				return this.editBox.Text;
			}
			set
			{
				// TODO:  Add ActorInt.ActorValue setter implementation
				this.editBox.Text = value;
			}
		}

		#endregion
	}
}

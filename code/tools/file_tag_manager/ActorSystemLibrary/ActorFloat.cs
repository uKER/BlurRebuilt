using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.Xml;

namespace ActorSystem
{
	public class ActorFloat : ActorParameter, IActorParameter
	{
		private System.Windows.Forms.TextBox editBox;
		private System.Windows.Forms.GroupBox MainGroupBox;
		private System.ComponentModel.IContainer components = null;

		public ActorFloat(Form owner, XmlNode valueNode) : base(owner, valueNode)

		{
			// This call is required by the Windows Form Designer.
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
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}


		#region Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.editBox = new System.Windows.Forms.TextBox();
			this.MainGroupBox = new System.Windows.Forms.GroupBox();
			this.MainGroupBox.SuspendLayout();
			this.SuspendLayout();
			// 
			// editBox
			// 
			this.editBox.Location = new System.Drawing.Point(8, 16);
			this.editBox.Name = "editBox";
			this.editBox.Size = new System.Drawing.Size(136, 20);
			this.editBox.TabIndex = 0;
			this.editBox.Text = "";
			this.editBox.TextChanged += new System.EventHandler(this.editBox_TextChanged);
			// 
			// MainGroupBox
			// 
			this.MainGroupBox.Controls.Add(this.editBox);
			this.MainGroupBox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.MainGroupBox.Location = new System.Drawing.Point(0, 0);
			this.MainGroupBox.Name = "MainGroupBox";
			this.MainGroupBox.Size = new System.Drawing.Size(152, 40);
			this.MainGroupBox.TabIndex = 1;
			this.MainGroupBox.TabStop = false;
			this.MainGroupBox.Text = "Put name here";
			// 
			// ActorFloat
			// 
			this.Controls.Add(this.MainGroupBox);
			this.Name = "ActorFloat";
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
				return this.MainGroupBox.Name;
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
				return this.editBox.Text;
			}
			set
			{
				this.editBox.Text = value;
			}
		}

		#endregion
	}
}


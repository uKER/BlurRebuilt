namespace BizarreCreations.JamIntegration
{
	partial class JamPropertiesDlg
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

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.ok_button = new System.Windows.Forms.Button();
			this.cancel_button = new System.Windows.Forms.Button();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.label3 = new System.Windows.Forms.Label();
			this.mCustomOptions = new System.Windows.Forms.TextBox();
			this.mDontRunActions = new System.Windows.Forms.CheckBox();
			this.mBuildNewestFirst = new System.Windows.Forms.CheckBox();
			this.mQuitEarly = new System.Windows.Forms.CheckBox();
			this.label1 = new System.Windows.Forms.Label();
			this.mNumProcessors = new System.Windows.Forms.NumericUpDown();
			this.label2 = new System.Windows.Forms.Label();
			this.mDebuggingLevel = new System.Windows.Forms.NumericUpDown();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.mShowDependencies = new System.Windows.Forms.CheckBox();
			this.mShowCommands = new System.Windows.Forms.CheckBox();
			this.mShowCauses = new System.Windows.Forms.CheckBox();
			this.mShowMakeTree = new System.Windows.Forms.CheckBox();
			this.mShowActions = new System.Windows.Forms.CheckBox();
			this.mShowIncludes = new System.Windows.Forms.CheckBox();
			this.mEnableNetworkCaching = new System.Windows.Forms.CheckBox();
			this.groupBox1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.mNumProcessors)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.mDebuggingLevel)).BeginInit();
			this.groupBox2.SuspendLayout();
			this.SuspendLayout();
			// 
			// ok_button
			// 
			this.ok_button.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.ok_button.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.ok_button.Location = new System.Drawing.Point(298, 210);
			this.ok_button.Name = "ok_button";
			this.ok_button.Size = new System.Drawing.Size(75, 23);
			this.ok_button.TabIndex = 0;
			this.ok_button.Text = "&OK";
			this.ok_button.UseVisualStyleBackColor = true;
			// 
			// cancel_button
			// 
			this.cancel_button.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.cancel_button.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancel_button.Location = new System.Drawing.Point(379, 210);
			this.cancel_button.Name = "cancel_button";
			this.cancel_button.Size = new System.Drawing.Size(75, 23);
			this.cancel_button.TabIndex = 1;
			this.cancel_button.Text = "&Cancel";
			this.cancel_button.UseVisualStyleBackColor = true;
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.mEnableNetworkCaching);
			this.groupBox1.Controls.Add(this.label3);
			this.groupBox1.Controls.Add(this.mCustomOptions);
			this.groupBox1.Controls.Add(this.mDontRunActions);
			this.groupBox1.Controls.Add(this.mBuildNewestFirst);
			this.groupBox1.Controls.Add(this.mQuitEarly);
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.mNumProcessors);
			this.groupBox1.Location = new System.Drawing.Point(12, 12);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(210, 189);
			this.groupBox1.TabIndex = 2;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Build options";
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(9, 141);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(82, 13);
			this.label3.TabIndex = 8;
			this.label3.Text = "Custom options:";
			// 
			// mCustomOptions
			// 
			this.mCustomOptions.Location = new System.Drawing.Point(12, 157);
			this.mCustomOptions.Name = "mCustomOptions";
			this.mCustomOptions.Size = new System.Drawing.Size(183, 20);
			this.mCustomOptions.TabIndex = 7;
			// 
			// mDontRunActions
			// 
			this.mDontRunActions.AutoSize = true;
			this.mDontRunActions.Location = new System.Drawing.Point(12, 91);
			this.mDontRunActions.Name = "mDontRunActions";
			this.mDontRunActions.Size = new System.Drawing.Size(129, 17);
			this.mDontRunActions.TabIndex = 6;
			this.mDontRunActions.Text = "Don\'t execute actions";
			this.mDontRunActions.UseVisualStyleBackColor = true;
			// 
			// mBuildNewestFirst
			// 
			this.mBuildNewestFirst.AutoSize = true;
			this.mBuildNewestFirst.Location = new System.Drawing.Point(12, 68);
			this.mBuildNewestFirst.Name = "mBuildNewestFirst";
			this.mBuildNewestFirst.Size = new System.Drawing.Size(168, 17);
			this.mBuildNewestFirst.TabIndex = 5;
			this.mBuildNewestFirst.Text = "Build from newest sources first";
			this.mBuildNewestFirst.UseVisualStyleBackColor = true;
			// 
			// mQuitEarly
			// 
			this.mQuitEarly.AutoSize = true;
			this.mQuitEarly.Location = new System.Drawing.Point(12, 45);
			this.mQuitEarly.Name = "mQuitEarly";
			this.mQuitEarly.Size = new System.Drawing.Size(159, 17);
			this.mQuitEarly.TabIndex = 4;
			this.mQuitEarly.Text = "Quit as soon as a target fails";
			this.mQuitEarly.UseVisualStyleBackColor = true;
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(6, 21);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(113, 13);
			this.label1.TabIndex = 1;
			this.label1.Text = "Number of processors:";
			// 
			// mNumProcessors
			// 
			this.mNumProcessors.Location = new System.Drawing.Point(152, 19);
			this.mNumProcessors.Name = "mNumProcessors";
			this.mNumProcessors.Size = new System.Drawing.Size(43, 20);
			this.mNumProcessors.TabIndex = 0;
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(6, 21);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(87, 13);
			this.label2.TabIndex = 2;
			this.label2.Text = "Debugging level:";
			// 
			// mDebuggingLevel
			// 
			this.mDebuggingLevel.Location = new System.Drawing.Point(151, 19);
			this.mDebuggingLevel.Name = "mDebuggingLevel";
			this.mDebuggingLevel.Size = new System.Drawing.Size(43, 20);
			this.mDebuggingLevel.TabIndex = 3;
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.mShowIncludes);
			this.groupBox2.Controls.Add(this.mShowDependencies);
			this.groupBox2.Controls.Add(this.mShowCommands);
			this.groupBox2.Controls.Add(this.mShowCauses);
			this.groupBox2.Controls.Add(this.mShowMakeTree);
			this.groupBox2.Controls.Add(this.mShowActions);
			this.groupBox2.Controls.Add(this.label2);
			this.groupBox2.Controls.Add(this.mDebuggingLevel);
			this.groupBox2.Location = new System.Drawing.Point(228, 12);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(226, 189);
			this.groupBox2.TabIndex = 3;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Debug output options";
			// 
			// mShowDependencies
			// 
			this.mShowDependencies.AutoSize = true;
			this.mShowDependencies.Location = new System.Drawing.Point(9, 91);
			this.mShowDependencies.Name = "mShowDependencies";
			this.mShowDependencies.Size = new System.Drawing.Size(123, 17);
			this.mShowDependencies.TabIndex = 9;
			this.mShowDependencies.Text = "Show dependencies";
			this.mShowDependencies.UseVisualStyleBackColor = true;
			// 
			// mShowCommands
			// 
			this.mShowCommands.AutoSize = true;
			this.mShowCommands.Location = new System.Drawing.Point(9, 137);
			this.mShowCommands.Name = "mShowCommands";
			this.mShowCommands.Size = new System.Drawing.Size(114, 17);
			this.mShowCommands.TabIndex = 8;
			this.mShowCommands.Text = "Display commands";
			this.mShowCommands.UseVisualStyleBackColor = true;
			// 
			// mShowCauses
			// 
			this.mShowCauses.AutoSize = true;
			this.mShowCauses.Location = new System.Drawing.Point(9, 68);
			this.mShowCauses.Name = "mShowCauses";
			this.mShowCauses.Size = new System.Drawing.Size(90, 17);
			this.mShowCauses.TabIndex = 7;
			this.mShowCauses.Text = "Show causes";
			this.mShowCauses.UseVisualStyleBackColor = true;
			// 
			// mShowMakeTree
			// 
			this.mShowMakeTree.AutoSize = true;
			this.mShowMakeTree.Location = new System.Drawing.Point(9, 114);
			this.mShowMakeTree.Name = "mShowMakeTree";
			this.mShowMakeTree.Size = new System.Drawing.Size(110, 17);
			this.mShowMakeTree.TabIndex = 6;
			this.mShowMakeTree.Text = "Display make tree";
			this.mShowMakeTree.UseVisualStyleBackColor = true;
			// 
			// mShowActions
			// 
			this.mShowActions.AutoSize = true;
			this.mShowActions.Location = new System.Drawing.Point(9, 45);
			this.mShowActions.Name = "mShowActions";
			this.mShowActions.Size = new System.Drawing.Size(90, 17);
			this.mShowActions.TabIndex = 5;
			this.mShowActions.Text = "Show actions";
			this.mShowActions.UseVisualStyleBackColor = true;
			// 
			// mShowIncludes
			// 
			this.mShowIncludes.AutoSize = true;
			this.mShowIncludes.Location = new System.Drawing.Point(9, 160);
			this.mShowIncludes.Name = "mShowIncludes";
			this.mShowIncludes.Size = new System.Drawing.Size(96, 17);
			this.mShowIncludes.TabIndex = 10;
			this.mShowIncludes.Text = "Show Includes";
			this.mShowIncludes.UseVisualStyleBackColor = true;
			// 
			// mEnableNetworkCaching
			// 
			this.mEnableNetworkCaching.AutoSize = true;
			this.mEnableNetworkCaching.Location = new System.Drawing.Point(12, 114);
			this.mEnableNetworkCaching.Name = "mEnableNetworkCaching";
			this.mEnableNetworkCaching.Size = new System.Drawing.Size(144, 17);
			this.mEnableNetworkCaching.TabIndex = 9;
			this.mEnableNetworkCaching.Text = "Enable Network Caching";
			this.mEnableNetworkCaching.UseVisualStyleBackColor = true;
			// 
			// JamPropertiesDlg
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(466, 245);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.cancel_button);
			this.Controls.Add(this.ok_button);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "JamPropertiesDlg";
			this.ShowInTaskbar = false;
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
			this.Text = "Jam Build Properties";
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.mNumProcessors)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.mDebuggingLevel)).EndInit();
			this.groupBox2.ResumeLayout(false);
			this.groupBox2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Button ok_button;
		private System.Windows.Forms.Button cancel_button;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.NumericUpDown mNumProcessors;
		private System.Windows.Forms.CheckBox mDontRunActions;
		private System.Windows.Forms.CheckBox mBuildNewestFirst;
		private System.Windows.Forms.CheckBox mQuitEarly;
		private System.Windows.Forms.NumericUpDown mDebuggingLevel;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.CheckBox mShowCommands;
		private System.Windows.Forms.CheckBox mShowCauses;
		private System.Windows.Forms.CheckBox mShowMakeTree;
		private System.Windows.Forms.CheckBox mShowActions;

		public void SetSettings(JamProperties prop)
		{
			mNumProcessors.Value = prop.mNumProcessors;
			mDontRunActions.Checked = prop.mDontRunActions;
			mBuildNewestFirst.Checked = prop.mBuildNewestFirst;
			mQuitEarly.Checked = prop.mQuitEarly;
			mDebuggingLevel.Value = prop.mDebuggingLevel;
			mShowCommands.Checked = prop.mShowCommands;
			mShowCauses.Checked = prop.mShowCauses;
			mShowMakeTree.Checked = prop.mShowMakeTree;
			mShowActions.Checked = prop.mShowActions;
			mShowDependencies.Checked = prop.mShowDependencies;
			mShowIncludes.Checked = prop.mShowIncludes;
			mEnableNetworkCaching.Checked = prop.mEnableNetworkCaching;
			mCustomOptions.Text = prop.mCustomOptions;
		}

		public JamProperties RetrieveSettings()
		{
			JamProperties prop = new JamProperties();

			prop.mNumProcessors = (int)mNumProcessors.Value;
			prop.mDontRunActions = mDontRunActions.Checked;
			prop.mBuildNewestFirst = mBuildNewestFirst.Checked;
			prop.mQuitEarly = mQuitEarly.Checked;
			prop.mDebuggingLevel = (int)mDebuggingLevel.Value;
			prop.mShowCommands = mShowCommands.Checked;
			prop.mShowCauses = mShowCauses.Checked;
			prop.mShowMakeTree = mShowMakeTree.Checked;
			prop.mShowActions = mShowActions.Checked;
			prop.mShowDependencies = mShowDependencies.Checked;
			prop.mCustomOptions = mCustomOptions.Text;
			prop.mEnableNetworkCaching = mEnableNetworkCaching.Checked;
			prop.mShowIncludes = mShowIncludes.Checked;
			return prop;
		}

		private System.Windows.Forms.CheckBox mShowDependencies;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.TextBox mCustomOptions;
		private System.Windows.Forms.CheckBox mEnableNetworkCaching;
		private System.Windows.Forms.CheckBox mShowIncludes;
	}
}
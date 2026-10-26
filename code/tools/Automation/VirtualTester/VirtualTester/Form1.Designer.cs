namespace VirtualTester
{
    partial class Form1
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
            if (disposing && (components != null))
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
			this.components = new System.ComponentModel.Container();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.buttonRT = new System.Windows.Forms.Button();
			this.buttonLT = new System.Windows.Forms.Button();
			this.buttonRS = new System.Windows.Forms.Button();
			this.buttonLS = new System.Windows.Forms.Button();
			this.buttonGuide = new System.Windows.Forms.Button();
			this.buttonStart = new System.Windows.Forms.Button();
			this.buttonBack = new System.Windows.Forms.Button();
			this.buttonB = new System.Windows.Forms.Button();
			this.buttonA = new System.Windows.Forms.Button();
			this.buttonX = new System.Windows.Forms.Button();
			this.buttonY = new System.Windows.Forms.Button();
			this.buttonDR = new System.Windows.Forms.Button();
			this.buttonDD = new System.Windows.Forms.Button();
			this.buttonDL = new System.Windows.Forms.Button();
			this.buttonDU = new System.Windows.Forms.Button();
			this.shapeContainer1 = new Microsoft.VisualBasic.PowerPacks.ShapeContainer();
			this.ovalShapeRA = new Microsoft.VisualBasic.PowerPacks.OvalShape();
			this.rectangle_RA = new Microsoft.VisualBasic.PowerPacks.RectangleShape();
			this.ovalShapeLA = new Microsoft.VisualBasic.PowerPacks.OvalShape();
			this.rectangle_LA = new Microsoft.VisualBasic.PowerPacks.RectangleShape();
			this.comboBoxProject = new System.Windows.Forms.ComboBox();
			this.comboBoxPlatform = new System.Windows.Forms.ComboBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.label8 = new System.Windows.Forms.Label();
			this.totalTestTimeMinutes = new System.Windows.Forms.NumericUpDown();
			this.label3 = new System.Windows.Forms.Label();
			this.timeBetweenInputsMS = new System.Windows.Forms.NumericUpDown();
			this.comboBoxBuild = new System.Windows.Forms.ComboBox();
			this.label4 = new System.Windows.Forms.Label();
			this.contextMenuDevkitTab = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.toolStripMenuItemScreenCapture = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItemReboot = new System.Windows.Forms.ToolStripMenuItem();
			this.menuStrip1 = new System.Windows.Forms.MenuStrip();
			this.logFolderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.openLogFolderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.actionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.resetJoysticksToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.optionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripStartStopTesting = new System.Windows.Forms.ToolStripMenuItem();
			this.textExecutable = new System.Windows.Forms.TextBox();
			this.label5 = new System.Windows.Forms.Label();
			this.label6 = new System.Windows.Forms.Label();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.labelCurrentScreen = new System.Windows.Forms.Label();
			this.label9 = new System.Windows.Forms.Label();
			this.buttonGoToCurrentLog = new System.Windows.Forms.Button();
			this.labelStatus = new System.Windows.Forms.Label();
			this.label7 = new System.Windows.Forms.Label();
			this.labelRunningTime = new System.Windows.Forms.Label();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.totalTestTimeMinutes)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.timeBetweenInputsMS)).BeginInit();
			this.contextMenuDevkitTab.SuspendLayout();
			this.menuStrip1.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.buttonRT);
			this.groupBox1.Controls.Add(this.buttonLT);
			this.groupBox1.Controls.Add(this.buttonRS);
			this.groupBox1.Controls.Add(this.buttonLS);
			this.groupBox1.Controls.Add(this.buttonGuide);
			this.groupBox1.Controls.Add(this.buttonStart);
			this.groupBox1.Controls.Add(this.buttonBack);
			this.groupBox1.Controls.Add(this.buttonB);
			this.groupBox1.Controls.Add(this.buttonA);
			this.groupBox1.Controls.Add(this.buttonX);
			this.groupBox1.Controls.Add(this.buttonY);
			this.groupBox1.Controls.Add(this.buttonDR);
			this.groupBox1.Controls.Add(this.buttonDD);
			this.groupBox1.Controls.Add(this.buttonDL);
			this.groupBox1.Controls.Add(this.buttonDU);
			this.groupBox1.Controls.Add(this.shapeContainer1);
			this.groupBox1.Location = new System.Drawing.Point(12, 120);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(326, 196);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Manual Override";
			// 
			// buttonRT
			// 
			this.buttonRT.Location = new System.Drawing.Point(279, 19);
			this.buttonRT.Name = "buttonRT";
			this.buttonRT.Size = new System.Drawing.Size(33, 27);
			this.buttonRT.TabIndex = 15;
			this.buttonRT.Text = "RT";
			this.buttonRT.UseVisualStyleBackColor = true;
			this.buttonRT.Click += new System.EventHandler(this.buttonRT_Click);
			// 
			// buttonLT
			// 
			this.buttonLT.Location = new System.Drawing.Point(13, 19);
			this.buttonLT.Name = "buttonLT";
			this.buttonLT.Size = new System.Drawing.Size(33, 27);
			this.buttonLT.TabIndex = 14;
			this.buttonLT.Text = "LT";
			this.buttonLT.UseVisualStyleBackColor = true;
			this.buttonLT.Click += new System.EventHandler(this.buttonLT_Click);
			// 
			// buttonRS
			// 
			this.buttonRS.Location = new System.Drawing.Point(279, 48);
			this.buttonRS.Name = "buttonRS";
			this.buttonRS.Size = new System.Drawing.Size(33, 27);
			this.buttonRS.TabIndex = 13;
			this.buttonRS.Text = "RS";
			this.buttonRS.UseVisualStyleBackColor = true;
			this.buttonRS.Click += new System.EventHandler(this.buttonRS_Click);
			// 
			// buttonLS
			// 
			this.buttonLS.Location = new System.Drawing.Point(13, 48);
			this.buttonLS.Name = "buttonLS";
			this.buttonLS.Size = new System.Drawing.Size(33, 27);
			this.buttonLS.TabIndex = 12;
			this.buttonLS.Text = "LS";
			this.buttonLS.UseVisualStyleBackColor = true;
			this.buttonLS.Click += new System.EventHandler(this.buttonLS_Click);
			// 
			// buttonGuide
			// 
			this.buttonGuide.Location = new System.Drawing.Point(125, 23);
			this.buttonGuide.Name = "buttonGuide";
			this.buttonGuide.Size = new System.Drawing.Size(75, 23);
			this.buttonGuide.TabIndex = 11;
			this.buttonGuide.Text = "Guide";
			this.buttonGuide.UseVisualStyleBackColor = true;
			this.buttonGuide.Click += new System.EventHandler(this.buttonGuide_Click);
			// 
			// buttonStart
			// 
			this.buttonStart.Location = new System.Drawing.Point(167, 52);
			this.buttonStart.Name = "buttonStart";
			this.buttonStart.Size = new System.Drawing.Size(75, 23);
			this.buttonStart.TabIndex = 10;
			this.buttonStart.Text = "Start";
			this.buttonStart.UseVisualStyleBackColor = true;
			this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
			// 
			// buttonBack
			// 
			this.buttonBack.Location = new System.Drawing.Point(83, 52);
			this.buttonBack.Name = "buttonBack";
			this.buttonBack.Size = new System.Drawing.Size(75, 23);
			this.buttonBack.TabIndex = 9;
			this.buttonBack.Text = "Back";
			this.buttonBack.UseVisualStyleBackColor = true;
			this.buttonBack.Click += new System.EventHandler(this.buttonBack_Click);
			// 
			// buttonB
			// 
			this.buttonB.Location = new System.Drawing.Point(279, 113);
			this.buttonB.Name = "buttonB";
			this.buttonB.Size = new System.Drawing.Size(33, 27);
			this.buttonB.TabIndex = 8;
			this.buttonB.Text = "B";
			this.buttonB.UseVisualStyleBackColor = true;
			this.buttonB.Click += new System.EventHandler(this.buttonB_Click);
			// 
			// buttonA
			// 
			this.buttonA.Location = new System.Drawing.Point(244, 142);
			this.buttonA.Name = "buttonA";
			this.buttonA.Size = new System.Drawing.Size(33, 27);
			this.buttonA.TabIndex = 7;
			this.buttonA.Text = "A";
			this.buttonA.UseVisualStyleBackColor = true;
			this.buttonA.Click += new System.EventHandler(this.buttonA_Click);
			// 
			// buttonX
			// 
			this.buttonX.Location = new System.Drawing.Point(209, 113);
			this.buttonX.Name = "buttonX";
			this.buttonX.Size = new System.Drawing.Size(33, 27);
			this.buttonX.TabIndex = 6;
			this.buttonX.Text = "X";
			this.buttonX.UseVisualStyleBackColor = true;
			this.buttonX.Click += new System.EventHandler(this.buttonX_Click);
			// 
			// buttonY
			// 
			this.buttonY.Location = new System.Drawing.Point(244, 84);
			this.buttonY.Name = "buttonY";
			this.buttonY.Size = new System.Drawing.Size(33, 27);
			this.buttonY.TabIndex = 5;
			this.buttonY.Text = "Y";
			this.buttonY.UseVisualStyleBackColor = true;
			this.buttonY.Click += new System.EventHandler(this.buttonY_Click);
			// 
			// buttonDR
			// 
			this.buttonDR.Location = new System.Drawing.Point(83, 113);
			this.buttonDR.Name = "buttonDR";
			this.buttonDR.Size = new System.Drawing.Size(33, 27);
			this.buttonDR.TabIndex = 4;
			this.buttonDR.Text = "DR";
			this.buttonDR.UseVisualStyleBackColor = true;
			this.buttonDR.Click += new System.EventHandler(this.buttonDR_Click);
			// 
			// buttonDD
			// 
			this.buttonDD.Location = new System.Drawing.Point(48, 142);
			this.buttonDD.Name = "buttonDD";
			this.buttonDD.Size = new System.Drawing.Size(33, 27);
			this.buttonDD.TabIndex = 3;
			this.buttonDD.Text = "DD";
			this.buttonDD.UseVisualStyleBackColor = true;
			this.buttonDD.Click += new System.EventHandler(this.buttonDD_Click);
			// 
			// buttonDL
			// 
			this.buttonDL.Location = new System.Drawing.Point(13, 113);
			this.buttonDL.Name = "buttonDL";
			this.buttonDL.Size = new System.Drawing.Size(33, 27);
			this.buttonDL.TabIndex = 2;
			this.buttonDL.Text = "DL";
			this.buttonDL.UseVisualStyleBackColor = true;
			this.buttonDL.Click += new System.EventHandler(this.buttonDL_Click);
			// 
			// buttonDU
			// 
			this.buttonDU.Location = new System.Drawing.Point(48, 84);
			this.buttonDU.Name = "buttonDU";
			this.buttonDU.Size = new System.Drawing.Size(33, 27);
			this.buttonDU.TabIndex = 1;
			this.buttonDU.Text = "DU";
			this.buttonDU.UseVisualStyleBackColor = true;
			this.buttonDU.Click += new System.EventHandler(this.buttonDU_Click);
			// 
			// shapeContainer1
			// 
			this.shapeContainer1.Location = new System.Drawing.Point(3, 16);
			this.shapeContainer1.Margin = new System.Windows.Forms.Padding(0);
			this.shapeContainer1.Name = "shapeContainer1";
			this.shapeContainer1.Shapes.AddRange(new Microsoft.VisualBasic.PowerPacks.Shape[] {
            this.ovalShapeRA,
            this.rectangle_RA,
            this.ovalShapeLA,
            this.rectangle_LA});
			this.shapeContainer1.Size = new System.Drawing.Size(320, 177);
			this.shapeContainer1.TabIndex = 16;
			this.shapeContainer1.TabStop = false;
			// 
			// ovalShapeRA
			// 
			this.ovalShapeRA.Location = new System.Drawing.Point(209, 150);
			this.ovalShapeRA.Name = "ovalShapeRA";
			this.ovalShapeRA.Size = new System.Drawing.Size(5, 5);
			// 
			// rectangle_RA
			// 
			this.rectangle_RA.Location = new System.Drawing.Point(191, 133);
			this.rectangle_RA.Name = "rectangle_RA";
			this.rectangle_RA.Size = new System.Drawing.Size(40, 40);
			this.rectangle_RA.MouseMove += new System.Windows.Forms.MouseEventHandler(this.MouseDown_RA);
			// 
			// ovalShapeLA
			// 
			this.ovalShapeLA.Location = new System.Drawing.Point(106, 150);
			this.ovalShapeLA.Name = "ovalShapeLA";
			this.ovalShapeLA.Size = new System.Drawing.Size(5, 5);
			// 
			// rectangle_LA
			// 
			this.rectangle_LA.Location = new System.Drawing.Point(89, 133);
			this.rectangle_LA.Name = "rectangle_LA";
			this.rectangle_LA.Size = new System.Drawing.Size(40, 40);
			this.rectangle_LA.MouseMove += new System.Windows.Forms.MouseEventHandler(this.MouseDown_LA);
			// 
			// comboBoxProject
			// 
			this.comboBoxProject.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxProject.FormattingEnabled = true;
			this.comboBoxProject.Location = new System.Drawing.Point(12, 53);
			this.comboBoxProject.Name = "comboBoxProject";
			this.comboBoxProject.Size = new System.Drawing.Size(97, 21);
			this.comboBoxProject.TabIndex = 2;
			this.comboBoxProject.SelectionChangeCommitted += new System.EventHandler(this.comboBoxProject_SelectionChangeCommitted);
			// 
			// comboBoxPlatform
			// 
			this.comboBoxPlatform.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxPlatform.FormattingEnabled = true;
			this.comboBoxPlatform.Items.AddRange(new object[] {
            "XBox 360",
            "PS3",
            "PC"});
			this.comboBoxPlatform.Location = new System.Drawing.Point(115, 53);
			this.comboBoxPlatform.Name = "comboBoxPlatform";
			this.comboBoxPlatform.Size = new System.Drawing.Size(81, 21);
			this.comboBoxPlatform.TabIndex = 3;
			this.comboBoxPlatform.SelectionChangeCommitted += new System.EventHandler(this.comboBoxPlatform_SelectionChangeCommitted);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(9, 37);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(43, 13);
			this.label1.TabIndex = 4;
			this.label1.Text = "Project:";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(112, 37);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(48, 13);
			this.label2.TabIndex = 5;
			this.label2.Text = "Platform:";
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.label8);
			this.groupBox2.Controls.Add(this.totalTestTimeMinutes);
			this.groupBox2.Controls.Add(this.label3);
			this.groupBox2.Controls.Add(this.timeBetweenInputsMS);
			this.groupBox2.Location = new System.Drawing.Point(345, 37);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(232, 76);
			this.groupBox2.TabIndex = 6;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Configuration";
			// 
			// label8
			// 
			this.label8.AutoSize = true;
			this.label8.Location = new System.Drawing.Point(86, 48);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(94, 13);
			this.label8.TabIndex = 4;
			this.label8.Text = "Minutes to run test";
			// 
			// totalTestTimeMinutes
			// 
			this.totalTestTimeMinutes.Location = new System.Drawing.Point(5, 45);
			this.totalTestTimeMinutes.Maximum = new decimal(new int[] {
            86400,
            0,
            0,
            0});
			this.totalTestTimeMinutes.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.totalTestTimeMinutes.Name = "totalTestTimeMinutes";
			this.totalTestTimeMinutes.Size = new System.Drawing.Size(76, 20);
			this.totalTestTimeMinutes.TabIndex = 3;
			this.totalTestTimeMinutes.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(87, 21);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(139, 13);
			this.label3.TabIndex = 2;
			this.label3.Text = "Milliseconds between inputs";
			// 
			// timeBetweenInputsMS
			// 
			this.timeBetweenInputsMS.Location = new System.Drawing.Point(6, 19);
			this.timeBetweenInputsMS.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
			this.timeBetweenInputsMS.Name = "timeBetweenInputsMS";
			this.timeBetweenInputsMS.Size = new System.Drawing.Size(75, 20);
			this.timeBetweenInputsMS.TabIndex = 1;
			this.timeBetweenInputsMS.Value = new decimal(new int[] {
            500,
            0,
            0,
            0});
			// 
			// comboBoxBuild
			// 
			this.comboBoxBuild.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxBuild.FormattingEnabled = true;
			this.comboBoxBuild.Items.AddRange(new object[] {
            "Release",
            "Master",
            "Speed",
            "Debug"});
			this.comboBoxBuild.Location = new System.Drawing.Point(202, 53);
			this.comboBoxBuild.Name = "comboBoxBuild";
			this.comboBoxBuild.Size = new System.Drawing.Size(136, 21);
			this.comboBoxBuild.TabIndex = 9;
			this.comboBoxBuild.SelectedIndexChanged += new System.EventHandler(this.comboBoxBuild_SelectedIndexChanged);
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(199, 37);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(33, 13);
			this.label4.TabIndex = 10;
			this.label4.Text = "Build:";
			// 
			// contextMenuDevkitTab
			// 
			this.contextMenuDevkitTab.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemScreenCapture,
            this.toolStripMenuItemReboot});
			this.contextMenuDevkitTab.Name = "contextMenuDevkitTab";
			this.contextMenuDevkitTab.Size = new System.Drawing.Size(161, 48);
			// 
			// toolStripMenuItemScreenCapture
			// 
			this.toolStripMenuItemScreenCapture.Name = "toolStripMenuItemScreenCapture";
			this.toolStripMenuItemScreenCapture.Size = new System.Drawing.Size(160, 22);
			this.toolStripMenuItemScreenCapture.Text = "Screen Capture";
			// 
			// toolStripMenuItemReboot
			// 
			this.toolStripMenuItemReboot.Name = "toolStripMenuItemReboot";
			this.toolStripMenuItemReboot.Size = new System.Drawing.Size(160, 22);
			this.toolStripMenuItemReboot.Text = "Reboot";
			// 
			// menuStrip1
			// 
			this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.logFolderToolStripMenuItem,
            this.actionsToolStripMenuItem,
            this.optionsToolStripMenuItem,
            this.helpToolStripMenuItem,
            this.toolStripStartStopTesting});
			this.menuStrip1.Location = new System.Drawing.Point(0, 0);
			this.menuStrip1.Name = "menuStrip1";
			this.menuStrip1.Size = new System.Drawing.Size(588, 24);
			this.menuStrip1.TabIndex = 13;
			this.menuStrip1.Text = "menuStrip1";
			// 
			// logFolderToolStripMenuItem
			// 
			this.logFolderToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openLogFolderToolStripMenuItem,
            this.exitToolStripMenuItem});
			this.logFolderToolStripMenuItem.Name = "logFolderToolStripMenuItem";
			this.logFolderToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
			this.logFolderToolStripMenuItem.Text = "File";
			// 
			// openLogFolderToolStripMenuItem
			// 
			this.openLogFolderToolStripMenuItem.Name = "openLogFolderToolStripMenuItem";
			this.openLogFolderToolStripMenuItem.Size = new System.Drawing.Size(164, 22);
			this.openLogFolderToolStripMenuItem.Text = "Open Log Folder";
			this.openLogFolderToolStripMenuItem.Click += new System.EventHandler(this.openLogFolderToolStripMenuItem_Click);
			// 
			// exitToolStripMenuItem
			// 
			this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			this.exitToolStripMenuItem.Size = new System.Drawing.Size(164, 22);
			this.exitToolStripMenuItem.Text = "Exit";
			this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
			// 
			// actionsToolStripMenuItem
			// 
			this.actionsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.resetJoysticksToolStripMenuItem});
			this.actionsToolStripMenuItem.Name = "actionsToolStripMenuItem";
			this.actionsToolStripMenuItem.Size = new System.Drawing.Size(54, 20);
			this.actionsToolStripMenuItem.Text = "Actions";
			// 
			// resetJoysticksToolStripMenuItem
			// 
			this.resetJoysticksToolStripMenuItem.Name = "resetJoysticksToolStripMenuItem";
			this.resetJoysticksToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
			this.resetJoysticksToolStripMenuItem.Text = "Reset Joysticks";
			this.resetJoysticksToolStripMenuItem.Click += new System.EventHandler(this.resetJoysticksToolStripMenuItem_Click);
			// 
			// optionsToolStripMenuItem
			// 
			this.optionsToolStripMenuItem.Enabled = false;
			this.optionsToolStripMenuItem.Name = "optionsToolStripMenuItem";
			this.optionsToolStripMenuItem.Size = new System.Drawing.Size(56, 20);
			this.optionsToolStripMenuItem.Text = "Options";
			// 
			// helpToolStripMenuItem
			// 
			this.helpToolStripMenuItem.Enabled = false;
			this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
			this.helpToolStripMenuItem.Size = new System.Drawing.Size(40, 20);
			this.helpToolStripMenuItem.Text = "Help";
			// 
			// toolStripStartStopTesting
			// 
			this.toolStripStartStopTesting.Alignment = System.Windows.Forms.ToolStripItemAlignment.Right;
			this.toolStripStartStopTesting.Name = "toolStripStartStopTesting";
			this.toolStripStartStopTesting.Size = new System.Drawing.Size(190, 20);
			this.toolStripStartStopTesting.Text = "<<< START VIRTUAL TESTER >>>";
			this.toolStripStartStopTesting.Click += new System.EventHandler(this.toolStripStartStopTesting_Click);
			// 
			// textExecutable
			// 
			this.textExecutable.Location = new System.Drawing.Point(12, 93);
			this.textExecutable.Name = "textExecutable";
			this.textExecutable.Size = new System.Drawing.Size(326, 20);
			this.textExecutable.TabIndex = 14;
			this.textExecutable.TextChanged += new System.EventHandler(this.textExecutable_TextChanged);
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(12, 77);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(63, 13);
			this.label5.TabIndex = 15;
			this.label5.Text = "Executable:";
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(6, 75);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(76, 13);
			this.label6.TabIndex = 16;
			this.label6.Text = "Running Time:";
			// 
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.labelCurrentScreen);
			this.groupBox3.Controls.Add(this.label9);
			this.groupBox3.Controls.Add(this.buttonGoToCurrentLog);
			this.groupBox3.Controls.Add(this.labelStatus);
			this.groupBox3.Controls.Add(this.label7);
			this.groupBox3.Controls.Add(this.labelRunningTime);
			this.groupBox3.Controls.Add(this.label6);
			this.groupBox3.Location = new System.Drawing.Point(344, 120);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(232, 196);
			this.groupBox3.TabIndex = 17;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "Current Test Run";
			// 
			// labelCurrentScreen
			// 
			this.labelCurrentScreen.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.labelCurrentScreen.Location = new System.Drawing.Point(7, 140);
			this.labelCurrentScreen.Name = "labelCurrentScreen";
			this.labelCurrentScreen.Size = new System.Drawing.Size(220, 20);
			this.labelCurrentScreen.TabIndex = 23;
			// 
			// label9
			// 
			this.label9.AutoSize = true;
			this.label9.Location = new System.Drawing.Point(8, 127);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(81, 13);
			this.label9.TabIndex = 22;
			this.label9.Text = "Current Screen:";
			// 
			// buttonGoToCurrentLog
			// 
			this.buttonGoToCurrentLog.Enabled = false;
			this.buttonGoToCurrentLog.Location = new System.Drawing.Point(6, 167);
			this.buttonGoToCurrentLog.Name = "buttonGoToCurrentLog";
			this.buttonGoToCurrentLog.Size = new System.Drawing.Size(220, 23);
			this.buttonGoToCurrentLog.TabIndex = 21;
			this.buttonGoToCurrentLog.Text = "Go to this log";
			this.buttonGoToCurrentLog.UseVisualStyleBackColor = true;
			this.buttonGoToCurrentLog.Click += new System.EventHandler(this.button1_Click);
			// 
			// labelStatus
			// 
			this.labelStatus.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.labelStatus.Location = new System.Drawing.Point(6, 39);
			this.labelStatus.Name = "labelStatus";
			this.labelStatus.Size = new System.Drawing.Size(220, 33);
			this.labelStatus.TabIndex = 20;
			// 
			// label7
			// 
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(6, 21);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(40, 13);
			this.label7.TabIndex = 19;
			this.label7.Text = "Status:";
			// 
			// labelRunningTime
			// 
			this.labelRunningTime.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.labelRunningTime.Location = new System.Drawing.Point(6, 92);
			this.labelRunningTime.Name = "labelRunningTime";
			this.labelRunningTime.Size = new System.Drawing.Size(220, 33);
			this.labelRunningTime.TabIndex = 18;
			// 
			// Form1
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.BackColor = System.Drawing.SystemColors.Control;
			this.ClientSize = new System.Drawing.Size(588, 330);
			this.Controls.Add(this.label5);
			this.Controls.Add(this.textExecutable);
			this.Controls.Add(this.menuStrip1);
			this.Controls.Add(this.groupBox3);
			this.Controls.Add(this.comboBoxBuild);
			this.Controls.Add(this.label4);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.comboBoxProject);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.comboBoxPlatform);
			this.MainMenuStrip = this.menuStrip1;
			this.Name = "Form1";
			this.Text = "Virtual Tester";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
			this.groupBox1.ResumeLayout(false);
			this.groupBox2.ResumeLayout(false);
			this.groupBox2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.totalTestTimeMinutes)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.timeBetweenInputsMS)).EndInit();
			this.contextMenuDevkitTab.ResumeLayout(false);
			this.menuStrip1.ResumeLayout(false);
			this.menuStrip1.PerformLayout();
			this.groupBox3.ResumeLayout(false);
			this.groupBox3.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button buttonB;
        private System.Windows.Forms.Button buttonA;
        private System.Windows.Forms.Button buttonX;
        private System.Windows.Forms.Button buttonY;
        private System.Windows.Forms.Button buttonDR;
        private System.Windows.Forms.Button buttonDD;
        private System.Windows.Forms.Button buttonDL;
        private System.Windows.Forms.Button buttonDU;
        private System.Windows.Forms.Button buttonGuide;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Button buttonBack;
        private System.Windows.Forms.ComboBox comboBoxProject;
        private System.Windows.Forms.ComboBox comboBoxPlatform;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown timeBetweenInputsMS;
        private System.Windows.Forms.ComboBox comboBoxBuild;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ContextMenuStrip contextMenuDevkitTab;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemScreenCapture;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemReboot;
        private System.Windows.Forms.Button buttonRS;
        private System.Windows.Forms.Button buttonLS;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem logFolderToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem optionsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem toolStripStartStopTesting;
        private System.Windows.Forms.TextBox textExecutable;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Label labelRunningTime;
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button buttonGoToCurrentLog;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.NumericUpDown totalTestTimeMinutes;
		private System.Windows.Forms.Label labelCurrentScreen;
		private System.Windows.Forms.Label label9;
		private System.Windows.Forms.Button buttonRT;
		private System.Windows.Forms.Button buttonLT;
		private Microsoft.VisualBasic.PowerPacks.ShapeContainer shapeContainer1;
		private Microsoft.VisualBasic.PowerPacks.RectangleShape rectangle_LA;
		private Microsoft.VisualBasic.PowerPacks.OvalShape ovalShapeLA;
		private Microsoft.VisualBasic.PowerPacks.RectangleShape rectangle_RA;
		private Microsoft.VisualBasic.PowerPacks.OvalShape ovalShapeRA;
		private System.Windows.Forms.ToolStripMenuItem openLogFolderToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem actionsToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem resetJoysticksToolStripMenuItem;
    }
}


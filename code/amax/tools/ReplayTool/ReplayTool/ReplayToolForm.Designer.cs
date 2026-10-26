namespace ReplayTool
{
    partial class ReplayToolForm
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
            this.Box_ReplayInformation = new System.Windows.Forms.GroupBox();
            this.Button_SetSaveFilename = new System.Windows.Forms.Button();
            this.Label_ReplaySaveFilename_Extension = new System.Windows.Forms.Label();
            this.TextBox_ReplaySaveFilename = new System.Windows.Forms.TextBox();
            this.Button_Refresh = new System.Windows.Forms.Button();
            this.Button_LoadReplay = new System.Windows.Forms.Button();
            this.TextBox_ReplayLoadFilename = new System.Windows.Forms.TextBox();
            this.Box_ReplayInformation_Statistics = new System.Windows.Forms.GroupBox();
            this.Label_NumScenes_Value = new System.Windows.Forms.Label();
            this.Label_CurrentScene_Value = new System.Windows.Forms.Label();
            this.Label_ReplayLength_Value = new System.Windows.Forms.Label();
            this.Label_CurrentTime_Value = new System.Windows.Forms.Label();
            this.Label_NumScenes = new System.Windows.Forms.Label();
            this.Label_CurrentScene = new System.Windows.Forms.Label();
            this.Label_ReplayLength = new System.Windows.Forms.Label();
            this.Label_CurrentTime = new System.Windows.Forms.Label();
            this.CheckBox_RecordEveryFrame = new System.Windows.Forms.CheckBox();
            this.Box_PlaybackControls = new System.Windows.Forms.GroupBox();
            this.Button_SkipFrame = new System.Windows.Forms.Button();
            this.CheckBox_ShowUI = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.TextBox_GoToTime_Seconds = new System.Windows.Forms.MaskedTextBox();
            this.TextBox_GoToTime_Minutes = new System.Windows.Forms.MaskedTextBox();
            this.Button_GoToTime = new System.Windows.Forms.Button();
            this.CheckBox_Loop = new System.Windows.Forms.CheckBox();
            this.Box_PlaybackControls_SceneSkip = new System.Windows.Forms.GroupBox();
            this.Button_NextScene = new System.Windows.Forms.Button();
            this.Button_PreviousScene = new System.Windows.Forms.Button();
            this.Button_Restart = new System.Windows.Forms.Button();
            this.Button_Pause = new System.Windows.Forms.Button();
            this.ToolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.TextBox_SceneLength = new System.Windows.Forms.MaskedTextBox();
            this.Label_SceneLength = new System.Windows.Forms.Label();
            this.OpenReplayFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.Box_Cameras = new System.Windows.Forms.GroupBox();
            this.Box_Cameras_LookAt = new System.Windows.Forms.GroupBox();
            this.Label_LookAtFoV_Value = new System.Windows.Forms.Label();
            this.Label_LookAtZ_Value = new System.Windows.Forms.Label();
            this.Label_LookAtY_Value = new System.Windows.Forms.Label();
            this.Label_LookAtX_Value = new System.Windows.Forms.Label();
            this.Label_LookAtFoV = new System.Windows.Forms.Label();
            this.Label_LookAtZ = new System.Windows.Forms.Label();
            this.Label_LookAtY = new System.Windows.Forms.Label();
            this.Label_LookAtX = new System.Windows.Forms.Label();
            this.TrackBar_LookAtFoV = new System.Windows.Forms.TrackBar();
            this.TrackBar_LookAtZ = new System.Windows.Forms.TrackBar();
            this.TrackBar_LookAtY = new System.Windows.Forms.TrackBar();
            this.TrackBar_LookAtX = new System.Windows.Forms.TrackBar();
            this.Label_LookAtTarget_Name = new System.Windows.Forms.Label();
            this.Label_LookAtTarget = new System.Windows.Forms.Label();
            this.Label_LookAtTarget_Number = new System.Windows.Forms.Label();
            this.TrackBar_LookAtTarget = new System.Windows.Forms.TrackBar();
            this.Box_Cameras_Shake = new System.Windows.Forms.GroupBox();
            this.Label_ShakeFrequency_Value = new System.Windows.Forms.Label();
            this.Label_ShakeZ_Value = new System.Windows.Forms.Label();
            this.Label_ShakeY_Value = new System.Windows.Forms.Label();
            this.Label_ShakeX_Value = new System.Windows.Forms.Label();
            this.Label_ShakeFrequency = new System.Windows.Forms.Label();
            this.Label_ShakeZ = new System.Windows.Forms.Label();
            this.Label_ShakeY = new System.Windows.Forms.Label();
            this.Label_ShakeX = new System.Windows.Forms.Label();
            this.TrackBar_ShakeFrequency = new System.Windows.Forms.TrackBar();
            this.TrackBar_ShakeZ = new System.Windows.Forms.TrackBar();
            this.TrackBar_ShakeY = new System.Windows.Forms.TrackBar();
            this.TrackBar_ShakeX = new System.Windows.Forms.TrackBar();
            this.CheckBox_ShakeOverride = new System.Windows.Forms.CheckBox();
            this.Label_CurrentFocusVehicle_Name = new System.Windows.Forms.Label();
            this.CheckBox_CameraLookAt = new System.Windows.Forms.CheckBox();
            this.Label_CurrentCameraType = new System.Windows.Forms.Label();
            this.ComboBox_CameraType = new System.Windows.Forms.ComboBox();
            this.Label_CurrentFocusVehicle = new System.Windows.Forms.Label();
            this.Label_FocusVehicle_Value = new System.Windows.Forms.Label();
            this.TrackBar_FocusVehicle = new System.Windows.Forms.TrackBar();
            this.Box_RecordingControls = new System.Windows.Forms.GroupBox();
            this.Box_Perks = new System.Windows.Forms.GroupBox();
            this.Numeric_PerkLevel = new System.Windows.Forms.NumericUpDown();
            this.Button_FirePerk = new System.Windows.Forms.Button();
            this.ComboBox_PerkType = new System.Windows.Forms.ComboBox();
            this.Box_CutsceneEditing = new System.Windows.Forms.GroupBox();
            this.Button_TakeControl = new System.Windows.Forms.Button();
            this.Box_ReplayInformation.SuspendLayout();
            this.Box_ReplayInformation_Statistics.SuspendLayout();
            this.Box_PlaybackControls.SuspendLayout();
            this.Box_PlaybackControls_SceneSkip.SuspendLayout();
            this.Box_Cameras.SuspendLayout();
            this.Box_Cameras_LookAt.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtFoV)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtTarget)).BeginInit();
            this.Box_Cameras_Shake.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_ShakeFrequency)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_ShakeZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_ShakeY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_ShakeX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_FocusVehicle)).BeginInit();
            this.Box_RecordingControls.SuspendLayout();
            this.Box_Perks.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.Numeric_PerkLevel)).BeginInit();
            this.Box_CutsceneEditing.SuspendLayout();
            this.SuspendLayout();
            // 
            // Box_ReplayInformation
            // 
            this.Box_ReplayInformation.Controls.Add(this.Button_SetSaveFilename);
            this.Box_ReplayInformation.Controls.Add(this.Label_ReplaySaveFilename_Extension);
            this.Box_ReplayInformation.Controls.Add(this.TextBox_ReplaySaveFilename);
            this.Box_ReplayInformation.Controls.Add(this.Button_Refresh);
            this.Box_ReplayInformation.Controls.Add(this.Button_LoadReplay);
            this.Box_ReplayInformation.Controls.Add(this.TextBox_ReplayLoadFilename);
            this.Box_ReplayInformation.Controls.Add(this.Box_ReplayInformation_Statistics);
            this.Box_ReplayInformation.Location = new System.Drawing.Point(12, 12);
            this.Box_ReplayInformation.Name = "Box_ReplayInformation";
            this.Box_ReplayInformation.Size = new System.Drawing.Size(256, 241);
            this.Box_ReplayInformation.TabIndex = 0;
            this.Box_ReplayInformation.TabStop = false;
            this.Box_ReplayInformation.Text = "Replay Information";
            // 
            // Button_SetSaveFilename
            // 
            this.Button_SetSaveFilename.Location = new System.Drawing.Point(17, 45);
            this.Button_SetSaveFilename.Name = "Button_SetSaveFilename";
            this.Button_SetSaveFilename.Size = new System.Drawing.Size(106, 24);
            this.Button_SetSaveFilename.TabIndex = 9;
            this.Button_SetSaveFilename.Text = "Set Save Filename";
            this.ToolTip1.SetToolTip(this.Button_SetSaveFilename, "Save this filename as the one that should be associated with the next replay crea" +
                    "ted");
            this.Button_SetSaveFilename.UseVisualStyleBackColor = true;
            this.Button_SetSaveFilename.Click += new System.EventHandler(this.Button_SetSaveFilename_Click);
            // 
            // Label_ReplaySaveFilename_Extension
            // 
            this.Label_ReplaySaveFilename_Extension.AutoSize = true;
            this.Label_ReplaySaveFilename_Extension.Location = new System.Drawing.Point(210, 22);
            this.Label_ReplaySaveFilename_Extension.Name = "Label_ReplaySaveFilename_Extension";
            this.Label_ReplaySaveFilename_Extension.Size = new System.Drawing.Size(33, 13);
            this.Label_ReplaySaveFilename_Extension.TabIndex = 8;
            this.Label_ReplaySaveFilename_Extension.Text = ".amrp";
            // 
            // TextBox_ReplaySaveFilename
            // 
            this.TextBox_ReplaySaveFilename.Location = new System.Drawing.Point(17, 19);
            this.TextBox_ReplaySaveFilename.Name = "TextBox_ReplaySaveFilename";
            this.TextBox_ReplaySaveFilename.Size = new System.Drawing.Size(193, 20);
            this.TextBox_ReplaySaveFilename.TabIndex = 7;
            this.TextBox_ReplaySaveFilename.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.ToolTip1.SetToolTip(this.TextBox_ReplaySaveFilename, "The filename to give the next replay (if a file with this name already exists one" +
                    " will be constructed from the system time)");
            // 
            // Button_Refresh
            // 
            this.Button_Refresh.Location = new System.Drawing.Point(17, 145);
            this.Button_Refresh.Name = "Button_Refresh";
            this.Button_Refresh.Size = new System.Drawing.Size(60, 44);
            this.Button_Refresh.TabIndex = 6;
            this.Button_Refresh.Text = "Refresh";
            this.ToolTip1.SetToolTip(this.Button_Refresh, "Refresh constant replay info");
            this.Button_Refresh.UseVisualStyleBackColor = true;
            this.Button_Refresh.Click += new System.EventHandler(this.Button_Refresh_Click);
            // 
            // Button_LoadReplay
            // 
            this.Button_LoadReplay.Location = new System.Drawing.Point(17, 75);
            this.Button_LoadReplay.Name = "Button_LoadReplay";
            this.Button_LoadReplay.Size = new System.Drawing.Size(86, 24);
            this.Button_LoadReplay.TabIndex = 4;
            this.Button_LoadReplay.Text = "Load Replay...";
            this.ToolTip1.SetToolTip(this.Button_LoadReplay, "Load an existing Amax Replay file");
            this.Button_LoadReplay.UseVisualStyleBackColor = true;
            this.Button_LoadReplay.Visible = false;
            this.Button_LoadReplay.Click += new System.EventHandler(this.Button_LoadReplay_Click);
            // 
            // TextBox_ReplayLoadFilename
            // 
            this.TextBox_ReplayLoadFilename.Enabled = false;
            this.TextBox_ReplayLoadFilename.Location = new System.Drawing.Point(17, 105);
            this.TextBox_ReplayLoadFilename.Name = "TextBox_ReplayLoadFilename";
            this.TextBox_ReplayLoadFilename.ReadOnly = true;
            this.TextBox_ReplayLoadFilename.Size = new System.Drawing.Size(215, 20);
            this.TextBox_ReplayLoadFilename.TabIndex = 2;
            this.TextBox_ReplayLoadFilename.Text = ".amrp";
            this.ToolTip1.SetToolTip(this.TextBox_ReplayLoadFilename, "Path and filename of the last replay loaded");
            // 
            // Box_ReplayInformation_Statistics
            // 
            this.Box_ReplayInformation_Statistics.Controls.Add(this.Label_NumScenes_Value);
            this.Box_ReplayInformation_Statistics.Controls.Add(this.Label_CurrentScene_Value);
            this.Box_ReplayInformation_Statistics.Controls.Add(this.Label_ReplayLength_Value);
            this.Box_ReplayInformation_Statistics.Controls.Add(this.Label_CurrentTime_Value);
            this.Box_ReplayInformation_Statistics.Controls.Add(this.Label_NumScenes);
            this.Box_ReplayInformation_Statistics.Controls.Add(this.Label_CurrentScene);
            this.Box_ReplayInformation_Statistics.Controls.Add(this.Label_ReplayLength);
            this.Box_ReplayInformation_Statistics.Controls.Add(this.Label_CurrentTime);
            this.Box_ReplayInformation_Statistics.Location = new System.Drawing.Point(94, 137);
            this.Box_ReplayInformation_Statistics.Name = "Box_ReplayInformation_Statistics";
            this.Box_ReplayInformation_Statistics.Size = new System.Drawing.Size(154, 98);
            this.Box_ReplayInformation_Statistics.TabIndex = 1;
            this.Box_ReplayInformation_Statistics.TabStop = false;
            this.Box_ReplayInformation_Statistics.Text = "Replay Stats";
            // 
            // Label_NumScenes_Value
            // 
            this.Label_NumScenes_Value.AutoSize = true;
            this.Label_NumScenes_Value.Location = new System.Drawing.Point(117, 77);
            this.Label_NumScenes_Value.Name = "Label_NumScenes_Value";
            this.Label_NumScenes_Value.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.Label_NumScenes_Value.Size = new System.Drawing.Size(10, 13);
            this.Label_NumScenes_Value.TabIndex = 6;
            this.Label_NumScenes_Value.Text = "-";
            // 
            // Label_CurrentScene_Value
            // 
            this.Label_CurrentScene_Value.AutoSize = true;
            this.Label_CurrentScene_Value.Location = new System.Drawing.Point(117, 62);
            this.Label_CurrentScene_Value.Name = "Label_CurrentScene_Value";
            this.Label_CurrentScene_Value.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.Label_CurrentScene_Value.Size = new System.Drawing.Size(10, 13);
            this.Label_CurrentScene_Value.TabIndex = 6;
            this.Label_CurrentScene_Value.Text = "-";
            // 
            // Label_ReplayLength_Value
            // 
            this.Label_ReplayLength_Value.AutoSize = true;
            this.Label_ReplayLength_Value.Location = new System.Drawing.Point(111, 39);
            this.Label_ReplayLength_Value.Name = "Label_ReplayLength_Value";
            this.Label_ReplayLength_Value.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.Label_ReplayLength_Value.Size = new System.Drawing.Size(22, 13);
            this.Label_ReplayLength_Value.TabIndex = 5;
            this.Label_ReplayLength_Value.Text = "--:--";
            // 
            // Label_CurrentTime_Value
            // 
            this.Label_CurrentTime_Value.AutoSize = true;
            this.Label_CurrentTime_Value.Location = new System.Drawing.Point(111, 22);
            this.Label_CurrentTime_Value.Name = "Label_CurrentTime_Value";
            this.Label_CurrentTime_Value.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.Label_CurrentTime_Value.Size = new System.Drawing.Size(22, 13);
            this.Label_CurrentTime_Value.TabIndex = 4;
            this.Label_CurrentTime_Value.Text = "--:--";
            // 
            // Label_NumScenes
            // 
            this.Label_NumScenes.AutoSize = true;
            this.Label_NumScenes.Location = new System.Drawing.Point(6, 77);
            this.Label_NumScenes.Name = "Label_NumScenes";
            this.Label_NumScenes.Size = new System.Drawing.Size(71, 13);
            this.Label_NumScenes.TabIndex = 3;
            this.Label_NumScenes.Text = "Num Scenes:";
            // 
            // Label_CurrentScene
            // 
            this.Label_CurrentScene.AutoSize = true;
            this.Label_CurrentScene.Location = new System.Drawing.Point(6, 62);
            this.Label_CurrentScene.Name = "Label_CurrentScene";
            this.Label_CurrentScene.Size = new System.Drawing.Size(78, 13);
            this.Label_CurrentScene.TabIndex = 2;
            this.Label_CurrentScene.Text = "Current Scene:";
            // 
            // Label_ReplayLength
            // 
            this.Label_ReplayLength.AutoSize = true;
            this.Label_ReplayLength.Location = new System.Drawing.Point(6, 39);
            this.Label_ReplayLength.Name = "Label_ReplayLength";
            this.Label_ReplayLength.Size = new System.Drawing.Size(79, 13);
            this.Label_ReplayLength.TabIndex = 1;
            this.Label_ReplayLength.Text = "Replay Length:";
            // 
            // Label_CurrentTime
            // 
            this.Label_CurrentTime.Location = new System.Drawing.Point(6, 22);
            this.Label_CurrentTime.Name = "Label_CurrentTime";
            this.Label_CurrentTime.Size = new System.Drawing.Size(78, 13);
            this.Label_CurrentTime.TabIndex = 0;
            this.Label_CurrentTime.Text = "Current Time:";
            // 
            // CheckBox_RecordEveryFrame
            // 
            this.CheckBox_RecordEveryFrame.AutoSize = true;
            this.CheckBox_RecordEveryFrame.Location = new System.Drawing.Point(128, 19);
            this.CheckBox_RecordEveryFrame.Name = "CheckBox_RecordEveryFrame";
            this.CheckBox_RecordEveryFrame.Size = new System.Drawing.Size(81, 30);
            this.CheckBox_RecordEveryFrame.TabIndex = 5;
            this.CheckBox_RecordEveryFrame.Text = "Record\r\nevery frame";
            this.ToolTip1.SetToolTip(this.CheckBox_RecordEveryFrame, "Checking this will force the system to record vehicle positions every frame");
            this.CheckBox_RecordEveryFrame.UseVisualStyleBackColor = true;
            this.CheckBox_RecordEveryFrame.CheckedChanged += new System.EventHandler(this.CheckBox_RecordEveryFrame_CheckedChanged);
            // 
            // Box_PlaybackControls
            // 
            this.Box_PlaybackControls.Controls.Add(this.Button_SkipFrame);
            this.Box_PlaybackControls.Controls.Add(this.CheckBox_ShowUI);
            this.Box_PlaybackControls.Controls.Add(this.label1);
            this.Box_PlaybackControls.Controls.Add(this.TextBox_GoToTime_Seconds);
            this.Box_PlaybackControls.Controls.Add(this.TextBox_GoToTime_Minutes);
            this.Box_PlaybackControls.Controls.Add(this.Button_GoToTime);
            this.Box_PlaybackControls.Controls.Add(this.CheckBox_Loop);
            this.Box_PlaybackControls.Controls.Add(this.Box_PlaybackControls_SceneSkip);
            this.Box_PlaybackControls.Controls.Add(this.Button_Restart);
            this.Box_PlaybackControls.Controls.Add(this.Button_Pause);
            this.Box_PlaybackControls.Location = new System.Drawing.Point(274, 12);
            this.Box_PlaybackControls.Name = "Box_PlaybackControls";
            this.Box_PlaybackControls.Size = new System.Drawing.Size(264, 180);
            this.Box_PlaybackControls.TabIndex = 1;
            this.Box_PlaybackControls.TabStop = false;
            this.Box_PlaybackControls.Text = "Playback Controls";
            // 
            // Button_SkipFrame
            // 
            this.Button_SkipFrame.Location = new System.Drawing.Point(175, 75);
            this.Button_SkipFrame.Name = "Button_SkipFrame";
            this.Button_SkipFrame.Size = new System.Drawing.Size(82, 35);
            this.Button_SkipFrame.TabIndex = 10;
            this.Button_SkipFrame.Text = "Next Frame";
            this.Button_SkipFrame.UseVisualStyleBackColor = true;
            this.Button_SkipFrame.Click += new System.EventHandler(this.Button_SkipFrame_Click);
            // 
            // CheckBox_ShowUI
            // 
            this.CheckBox_ShowUI.AutoSize = true;
            this.CheckBox_ShowUI.Location = new System.Drawing.Point(121, 53);
            this.CheckBox_ShowUI.Name = "CheckBox_ShowUI";
            this.CheckBox_ShowUI.Size = new System.Drawing.Size(67, 17);
            this.CheckBox_ShowUI.TabIndex = 9;
            this.CheckBox_ShowUI.Text = "Show UI";
            this.ToolTip1.SetToolTip(this.CheckBox_ShowUI, "Show the Game UI");
            this.CheckBox_ShowUI.UseVisualStyleBackColor = true;
            this.CheckBox_ShowUI.CheckedChanged += new System.EventHandler(this.CheckBox_ShowUI_CheckedChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(140, 86);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(10, 13);
            this.label1.TabIndex = 8;
            this.label1.Text = ":";
            // 
            // TextBox_GoToTime_Seconds
            // 
            this.TextBox_GoToTime_Seconds.Location = new System.Drawing.Point(150, 84);
            this.TextBox_GoToTime_Seconds.Mask = "00";
            this.TextBox_GoToTime_Seconds.Name = "TextBox_GoToTime_Seconds";
            this.TextBox_GoToTime_Seconds.Size = new System.Drawing.Size(19, 20);
            this.TextBox_GoToTime_Seconds.TabIndex = 7;
            this.TextBox_GoToTime_Seconds.Text = "00";
            // 
            // TextBox_GoToTime_Minutes
            // 
            this.TextBox_GoToTime_Minutes.Location = new System.Drawing.Point(121, 84);
            this.TextBox_GoToTime_Minutes.Mask = "00";
            this.TextBox_GoToTime_Minutes.Name = "TextBox_GoToTime_Minutes";
            this.TextBox_GoToTime_Minutes.Size = new System.Drawing.Size(19, 20);
            this.TextBox_GoToTime_Minutes.TabIndex = 6;
            this.TextBox_GoToTime_Minutes.Text = "00";
            // 
            // Button_GoToTime
            // 
            this.Button_GoToTime.Location = new System.Drawing.Point(25, 76);
            this.Button_GoToTime.Name = "Button_GoToTime";
            this.Button_GoToTime.Size = new System.Drawing.Size(82, 35);
            this.Button_GoToTime.TabIndex = 5;
            this.Button_GoToTime.Text = "Go To Time";
            this.ToolTip1.SetToolTip(this.Button_GoToTime, "Jump to the given time in the current replay ( minutes : seconds )");
            this.Button_GoToTime.UseVisualStyleBackColor = true;
            this.Button_GoToTime.Click += new System.EventHandler(this.Button_GoToTime_Click);
            // 
            // CheckBox_Loop
            // 
            this.CheckBox_Loop.AutoSize = true;
            this.CheckBox_Loop.Location = new System.Drawing.Point(25, 53);
            this.CheckBox_Loop.Name = "CheckBox_Loop";
            this.CheckBox_Loop.Size = new System.Drawing.Size(50, 17);
            this.CheckBox_Loop.TabIndex = 3;
            this.CheckBox_Loop.Text = "Loop";
            this.ToolTip1.SetToolTip(this.CheckBox_Loop, "Loop replays when playback reaches the end");
            this.CheckBox_Loop.UseVisualStyleBackColor = true;
            this.CheckBox_Loop.CheckedChanged += new System.EventHandler(this.CheckBox_Loop_CheckedChanged);
            // 
            // Box_PlaybackControls_SceneSkip
            // 
            this.Box_PlaybackControls_SceneSkip.Controls.Add(this.Button_NextScene);
            this.Box_PlaybackControls_SceneSkip.Controls.Add(this.Button_PreviousScene);
            this.Box_PlaybackControls_SceneSkip.Location = new System.Drawing.Point(6, 125);
            this.Box_PlaybackControls_SceneSkip.Name = "Box_PlaybackControls_SceneSkip";
            this.Box_PlaybackControls_SceneSkip.Size = new System.Drawing.Size(203, 49);
            this.Box_PlaybackControls_SceneSkip.TabIndex = 2;
            this.Box_PlaybackControls_SceneSkip.TabStop = false;
            this.Box_PlaybackControls_SceneSkip.Text = "Scene Skip";
            // 
            // Button_NextScene
            // 
            this.Button_NextScene.Location = new System.Drawing.Point(117, 19);
            this.Button_NextScene.Name = "Button_NextScene";
            this.Button_NextScene.Size = new System.Drawing.Size(77, 22);
            this.Button_NextScene.TabIndex = 1;
            this.Button_NextScene.Text = "Next";
            this.ToolTip1.SetToolTip(this.Button_NextScene, "Skip to the next scene");
            this.Button_NextScene.UseVisualStyleBackColor = true;
            this.Button_NextScene.Click += new System.EventHandler(this.Button_NextScene_Click);
            // 
            // Button_PreviousScene
            // 
            this.Button_PreviousScene.Location = new System.Drawing.Point(12, 19);
            this.Button_PreviousScene.Name = "Button_PreviousScene";
            this.Button_PreviousScene.Size = new System.Drawing.Size(77, 22);
            this.Button_PreviousScene.TabIndex = 0;
            this.Button_PreviousScene.Text = "Previous";
            this.ToolTip1.SetToolTip(this.Button_PreviousScene, "Skip to the previous scene");
            this.Button_PreviousScene.UseVisualStyleBackColor = true;
            this.Button_PreviousScene.Click += new System.EventHandler(this.Button_PreviousScene_Click);
            // 
            // Button_Restart
            // 
            this.Button_Restart.Location = new System.Drawing.Point(121, 19);
            this.Button_Restart.Name = "Button_Restart";
            this.Button_Restart.Size = new System.Drawing.Size(79, 28);
            this.Button_Restart.TabIndex = 1;
            this.Button_Restart.Text = "Restart";
            this.ToolTip1.SetToolTip(this.Button_Restart, "Reset playback to the start of the replay");
            this.Button_Restart.UseVisualStyleBackColor = true;
            this.Button_Restart.Click += new System.EventHandler(this.Button_Restart_Click);
            // 
            // Button_Pause
            // 
            this.Button_Pause.Location = new System.Drawing.Point(18, 19);
            this.Button_Pause.Name = "Button_Pause";
            this.Button_Pause.Size = new System.Drawing.Size(79, 28);
            this.Button_Pause.TabIndex = 0;
            this.Button_Pause.Text = "Pause";
            this.ToolTip1.SetToolTip(this.Button_Pause, "Pause playback");
            this.Button_Pause.UseVisualStyleBackColor = true;
            this.Button_Pause.Click += new System.EventHandler(this.Button_Pause_Click);
            // 
            // TextBox_SceneLength
            // 
            this.TextBox_SceneLength.Location = new System.Drawing.Point(89, 25);
            this.TextBox_SceneLength.Mask = "0";
            this.TextBox_SceneLength.Name = "TextBox_SceneLength";
            this.TextBox_SceneLength.Size = new System.Drawing.Size(18, 20);
            this.TextBox_SceneLength.TabIndex = 6;
            this.TextBox_SceneLength.Text = "5";
            this.ToolTip1.SetToolTip(this.TextBox_SceneLength, "The length of each replay \"scene\", i.e. how often the system dumps to file. This " +
                    "is also the size of Scene Skips");
            this.TextBox_SceneLength.TextChanged += new System.EventHandler(this.TextBox_SceneLength_TextChanged);
            // 
            // Label_SceneLength
            // 
            this.Label_SceneLength.AutoSize = true;
            this.Label_SceneLength.Location = new System.Drawing.Point(6, 19);
            this.Label_SceneLength.Name = "Label_SceneLength";
            this.Label_SceneLength.Size = new System.Drawing.Size(77, 26);
            this.Label_SceneLength.TabIndex = 7;
            this.Label_SceneLength.Text = "Scene Length:\r\n(seconds)";
            this.ToolTip1.SetToolTip(this.Label_SceneLength, "The length of each replay \"scene\", i.e. how often the system dumps to file. This " +
                    "is also the size of Scene Skips");
            // 
            // OpenReplayFileDialog
            // 
            this.OpenReplayFileDialog.DefaultExt = "amrp";
            this.OpenReplayFileDialog.FileName = ".amrp";
            this.OpenReplayFileDialog.Filter = "Amax Replay Files (*.amrp)|*.amrp";
            this.OpenReplayFileDialog.Title = "Open Replay File";
            this.OpenReplayFileDialog.FileOk += new System.ComponentModel.CancelEventHandler(this.OpenReplayFileDialog_FileOk);
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // Box_Cameras
            // 
            this.Box_Cameras.Controls.Add(this.Box_Cameras_LookAt);
            this.Box_Cameras.Controls.Add(this.Box_Cameras_Shake);
            this.Box_Cameras.Controls.Add(this.CheckBox_ShakeOverride);
            this.Box_Cameras.Controls.Add(this.Label_CurrentFocusVehicle_Name);
            this.Box_Cameras.Controls.Add(this.CheckBox_CameraLookAt);
            this.Box_Cameras.Controls.Add(this.Label_CurrentCameraType);
            this.Box_Cameras.Controls.Add(this.ComboBox_CameraType);
            this.Box_Cameras.Controls.Add(this.Label_CurrentFocusVehicle);
            this.Box_Cameras.Controls.Add(this.Label_FocusVehicle_Value);
            this.Box_Cameras.Controls.Add(this.TrackBar_FocusVehicle);
            this.Box_Cameras.Location = new System.Drawing.Point(12, 259);
            this.Box_Cameras.Name = "Box_Cameras";
            this.Box_Cameras.Size = new System.Drawing.Size(526, 293);
            this.Box_Cameras.TabIndex = 2;
            this.Box_Cameras.TabStop = false;
            this.Box_Cameras.Text = "Camera Control";
            // 
            // Box_Cameras_LookAt
            // 
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtFoV_Value);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtZ_Value);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtY_Value);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtX_Value);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtFoV);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtZ);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtY);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtX);
            this.Box_Cameras_LookAt.Controls.Add(this.TrackBar_LookAtFoV);
            this.Box_Cameras_LookAt.Controls.Add(this.TrackBar_LookAtZ);
            this.Box_Cameras_LookAt.Controls.Add(this.TrackBar_LookAtY);
            this.Box_Cameras_LookAt.Controls.Add(this.TrackBar_LookAtX);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtTarget_Name);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtTarget);
            this.Box_Cameras_LookAt.Controls.Add(this.Label_LookAtTarget_Number);
            this.Box_Cameras_LookAt.Controls.Add(this.TrackBar_LookAtTarget);
            this.Box_Cameras_LookAt.Location = new System.Drawing.Point(258, 68);
            this.Box_Cameras_LookAt.Name = "Box_Cameras_LookAt";
            this.Box_Cameras_LookAt.Size = new System.Drawing.Size(262, 219);
            this.Box_Cameras_LookAt.TabIndex = 8;
            this.Box_Cameras_LookAt.TabStop = false;
            this.Box_Cameras_LookAt.Text = "Look At";
            // 
            // Label_LookAtFoV_Value
            // 
            this.Label_LookAtFoV_Value.AutoSize = true;
            this.Label_LookAtFoV_Value.Location = new System.Drawing.Point(19, 185);
            this.Label_LookAtFoV_Value.Name = "Label_LookAtFoV_Value";
            this.Label_LookAtFoV_Value.Size = new System.Drawing.Size(35, 13);
            this.Label_LookAtFoV_Value.TabIndex = 15;
            this.Label_LookAtFoV_Value.Text = "<null>";
            // 
            // Label_LookAtZ_Value
            // 
            this.Label_LookAtZ_Value.AutoSize = true;
            this.Label_LookAtZ_Value.Location = new System.Drawing.Point(19, 151);
            this.Label_LookAtZ_Value.Name = "Label_LookAtZ_Value";
            this.Label_LookAtZ_Value.Size = new System.Drawing.Size(35, 13);
            this.Label_LookAtZ_Value.TabIndex = 14;
            this.Label_LookAtZ_Value.Text = "<null>";
            // 
            // Label_LookAtY_Value
            // 
            this.Label_LookAtY_Value.AutoSize = true;
            this.Label_LookAtY_Value.Location = new System.Drawing.Point(19, 117);
            this.Label_LookAtY_Value.Name = "Label_LookAtY_Value";
            this.Label_LookAtY_Value.Size = new System.Drawing.Size(35, 13);
            this.Label_LookAtY_Value.TabIndex = 13;
            this.Label_LookAtY_Value.Text = "<null>";
            // 
            // Label_LookAtX_Value
            // 
            this.Label_LookAtX_Value.AutoSize = true;
            this.Label_LookAtX_Value.Location = new System.Drawing.Point(19, 83);
            this.Label_LookAtX_Value.Name = "Label_LookAtX_Value";
            this.Label_LookAtX_Value.Size = new System.Drawing.Size(35, 13);
            this.Label_LookAtX_Value.TabIndex = 12;
            this.Label_LookAtX_Value.Text = "<null>";
            // 
            // Label_LookAtFoV
            // 
            this.Label_LookAtFoV.AutoSize = true;
            this.Label_LookAtFoV.Location = new System.Drawing.Point(3, 172);
            this.Label_LookAtFoV.Name = "Label_LookAtFoV";
            this.Label_LookAtFoV.Size = new System.Drawing.Size(29, 13);
            this.Label_LookAtFoV.TabIndex = 11;
            this.Label_LookAtFoV.Text = "FoV:";
            // 
            // Label_LookAtZ
            // 
            this.Label_LookAtZ.AutoSize = true;
            this.Label_LookAtZ.Location = new System.Drawing.Point(3, 138);
            this.Label_LookAtZ.Name = "Label_LookAtZ";
            this.Label_LookAtZ.Size = new System.Drawing.Size(48, 13);
            this.Label_LookAtZ.TabIndex = 10;
            this.Label_LookAtZ.Text = "Z Offset:";
            // 
            // Label_LookAtY
            // 
            this.Label_LookAtY.AutoSize = true;
            this.Label_LookAtY.Location = new System.Drawing.Point(3, 104);
            this.Label_LookAtY.Name = "Label_LookAtY";
            this.Label_LookAtY.Size = new System.Drawing.Size(48, 13);
            this.Label_LookAtY.TabIndex = 9;
            this.Label_LookAtY.Text = "Y Offset:";
            // 
            // Label_LookAtX
            // 
            this.Label_LookAtX.AutoSize = true;
            this.Label_LookAtX.Location = new System.Drawing.Point(3, 70);
            this.Label_LookAtX.Name = "Label_LookAtX";
            this.Label_LookAtX.Size = new System.Drawing.Size(48, 13);
            this.Label_LookAtX.TabIndex = 8;
            this.Label_LookAtX.Text = "X Offset:";
            // 
            // TrackBar_LookAtFoV
            // 
            this.TrackBar_LookAtFoV.Location = new System.Drawing.Point(57, 170);
            this.TrackBar_LookAtFoV.Maximum = 90;
            this.TrackBar_LookAtFoV.Name = "TrackBar_LookAtFoV";
            this.TrackBar_LookAtFoV.Size = new System.Drawing.Size(199, 45);
            this.TrackBar_LookAtFoV.TabIndex = 4;
            this.TrackBar_LookAtFoV.Value = 30;
            this.TrackBar_LookAtFoV.Scroll += new System.EventHandler(this.TrackBar_LookAtFoV_Scroll);
            // 
            // TrackBar_LookAtZ
            // 
            this.TrackBar_LookAtZ.Location = new System.Drawing.Point(57, 136);
            this.TrackBar_LookAtZ.Maximum = 40;
            this.TrackBar_LookAtZ.Minimum = -40;
            this.TrackBar_LookAtZ.Name = "TrackBar_LookAtZ";
            this.TrackBar_LookAtZ.Size = new System.Drawing.Size(199, 45);
            this.TrackBar_LookAtZ.TabIndex = 7;
            this.TrackBar_LookAtZ.Scroll += new System.EventHandler(this.TrackBar_LookAtZ_Scroll);
            // 
            // TrackBar_LookAtY
            // 
            this.TrackBar_LookAtY.Location = new System.Drawing.Point(57, 102);
            this.TrackBar_LookAtY.Maximum = 40;
            this.TrackBar_LookAtY.Minimum = -40;
            this.TrackBar_LookAtY.Name = "TrackBar_LookAtY";
            this.TrackBar_LookAtY.Size = new System.Drawing.Size(199, 45);
            this.TrackBar_LookAtY.TabIndex = 6;
            this.TrackBar_LookAtY.Scroll += new System.EventHandler(this.TrackBar_LookAtY_Scroll);
            // 
            // TrackBar_LookAtX
            // 
            this.TrackBar_LookAtX.Location = new System.Drawing.Point(57, 68);
            this.TrackBar_LookAtX.Maximum = 40;
            this.TrackBar_LookAtX.Minimum = -40;
            this.TrackBar_LookAtX.Name = "TrackBar_LookAtX";
            this.TrackBar_LookAtX.Size = new System.Drawing.Size(199, 45);
            this.TrackBar_LookAtX.TabIndex = 5;
            this.TrackBar_LookAtX.Scroll += new System.EventHandler(this.TrackBar_LookAtX_Scroll);
            // 
            // Label_LookAtTarget_Name
            // 
            this.Label_LookAtTarget_Name.AutoSize = true;
            this.Label_LookAtTarget_Name.Location = new System.Drawing.Point(163, 51);
            this.Label_LookAtTarget_Name.Name = "Label_LookAtTarget_Name";
            this.Label_LookAtTarget_Name.Size = new System.Drawing.Size(35, 13);
            this.Label_LookAtTarget_Name.TabIndex = 3;
            this.Label_LookAtTarget_Name.Text = "<null>";
            // 
            // Label_LookAtTarget
            // 
            this.Label_LookAtTarget.AutoSize = true;
            this.Label_LookAtTarget.Location = new System.Drawing.Point(31, 51);
            this.Label_LookAtTarget.Name = "Label_LookAtTarget";
            this.Label_LookAtTarget.Size = new System.Drawing.Size(119, 13);
            this.Label_LookAtTarget.TabIndex = 2;
            this.Label_LookAtTarget.Text = "Look At Target Vehicle:";
            // 
            // Label_LookAtTarget_Number
            // 
            this.Label_LookAtTarget_Number.AutoSize = true;
            this.Label_LookAtTarget_Number.Location = new System.Drawing.Point(240, 19);
            this.Label_LookAtTarget_Number.Name = "Label_LookAtTarget_Number";
            this.Label_LookAtTarget_Number.Size = new System.Drawing.Size(10, 13);
            this.Label_LookAtTarget_Number.TabIndex = 1;
            this.Label_LookAtTarget_Number.Text = "-";
            // 
            // TrackBar_LookAtTarget
            // 
            this.TrackBar_LookAtTarget.Location = new System.Drawing.Point(6, 19);
            this.TrackBar_LookAtTarget.Maximum = 20;
            this.TrackBar_LookAtTarget.Name = "TrackBar_LookAtTarget";
            this.TrackBar_LookAtTarget.Size = new System.Drawing.Size(228, 45);
            this.TrackBar_LookAtTarget.TabIndex = 0;
            this.TrackBar_LookAtTarget.Scroll += new System.EventHandler(this.TrackBar_LookAtTarget_Scroll);
            // 
            // Box_Cameras_Shake
            // 
            this.Box_Cameras_Shake.Controls.Add(this.Label_ShakeFrequency_Value);
            this.Box_Cameras_Shake.Controls.Add(this.Label_ShakeZ_Value);
            this.Box_Cameras_Shake.Controls.Add(this.Label_ShakeY_Value);
            this.Box_Cameras_Shake.Controls.Add(this.Label_ShakeX_Value);
            this.Box_Cameras_Shake.Controls.Add(this.Label_ShakeFrequency);
            this.Box_Cameras_Shake.Controls.Add(this.Label_ShakeZ);
            this.Box_Cameras_Shake.Controls.Add(this.Label_ShakeY);
            this.Box_Cameras_Shake.Controls.Add(this.Label_ShakeX);
            this.Box_Cameras_Shake.Controls.Add(this.TrackBar_ShakeFrequency);
            this.Box_Cameras_Shake.Controls.Add(this.TrackBar_ShakeZ);
            this.Box_Cameras_Shake.Controls.Add(this.TrackBar_ShakeY);
            this.Box_Cameras_Shake.Controls.Add(this.TrackBar_ShakeX);
            this.Box_Cameras_Shake.Location = new System.Drawing.Point(6, 122);
            this.Box_Cameras_Shake.Name = "Box_Cameras_Shake";
            this.Box_Cameras_Shake.Size = new System.Drawing.Size(244, 165);
            this.Box_Cameras_Shake.TabIndex = 7;
            this.Box_Cameras_Shake.TabStop = false;
            this.Box_Cameras_Shake.Text = "Camera Shake";
            // 
            // Label_ShakeFrequency_Value
            // 
            this.Label_ShakeFrequency_Value.AutoSize = true;
            this.Label_ShakeFrequency_Value.Location = new System.Drawing.Point(12, 131);
            this.Label_ShakeFrequency_Value.Name = "Label_ShakeFrequency_Value";
            this.Label_ShakeFrequency_Value.Size = new System.Drawing.Size(35, 13);
            this.Label_ShakeFrequency_Value.TabIndex = 11;
            this.Label_ShakeFrequency_Value.Text = "<null>";
            // 
            // Label_ShakeZ_Value
            // 
            this.Label_ShakeZ_Value.AutoSize = true;
            this.Label_ShakeZ_Value.Location = new System.Drawing.Point(12, 97);
            this.Label_ShakeZ_Value.Name = "Label_ShakeZ_Value";
            this.Label_ShakeZ_Value.Size = new System.Drawing.Size(35, 13);
            this.Label_ShakeZ_Value.TabIndex = 10;
            this.Label_ShakeZ_Value.Text = "<null>";
            // 
            // Label_ShakeY_Value
            // 
            this.Label_ShakeY_Value.AutoSize = true;
            this.Label_ShakeY_Value.Location = new System.Drawing.Point(12, 63);
            this.Label_ShakeY_Value.Name = "Label_ShakeY_Value";
            this.Label_ShakeY_Value.Size = new System.Drawing.Size(35, 13);
            this.Label_ShakeY_Value.TabIndex = 9;
            this.Label_ShakeY_Value.Text = "<null>";
            // 
            // Label_ShakeX_Value
            // 
            this.Label_ShakeX_Value.AutoSize = true;
            this.Label_ShakeX_Value.Location = new System.Drawing.Point(12, 29);
            this.Label_ShakeX_Value.Name = "Label_ShakeX_Value";
            this.Label_ShakeX_Value.Size = new System.Drawing.Size(35, 13);
            this.Label_ShakeX_Value.TabIndex = 8;
            this.Label_ShakeX_Value.Text = "<null>";
            // 
            // Label_ShakeFrequency
            // 
            this.Label_ShakeFrequency.AutoSize = true;
            this.Label_ShakeFrequency.Location = new System.Drawing.Point(3, 118);
            this.Label_ShakeFrequency.Name = "Label_ShakeFrequency";
            this.Label_ShakeFrequency.Size = new System.Drawing.Size(31, 13);
            this.Label_ShakeFrequency.TabIndex = 7;
            this.Label_ShakeFrequency.Text = "Freq:";
            // 
            // Label_ShakeZ
            // 
            this.Label_ShakeZ.AutoSize = true;
            this.Label_ShakeZ.Location = new System.Drawing.Point(3, 84);
            this.Label_ShakeZ.Name = "Label_ShakeZ";
            this.Label_ShakeZ.Size = new System.Drawing.Size(17, 13);
            this.Label_ShakeZ.TabIndex = 6;
            this.Label_ShakeZ.Text = "Z:";
            // 
            // Label_ShakeY
            // 
            this.Label_ShakeY.AutoSize = true;
            this.Label_ShakeY.Location = new System.Drawing.Point(3, 50);
            this.Label_ShakeY.Name = "Label_ShakeY";
            this.Label_ShakeY.Size = new System.Drawing.Size(17, 13);
            this.Label_ShakeY.TabIndex = 5;
            this.Label_ShakeY.Text = "Y:";
            // 
            // Label_ShakeX
            // 
            this.Label_ShakeX.AutoSize = true;
            this.Label_ShakeX.Location = new System.Drawing.Point(3, 16);
            this.Label_ShakeX.Name = "Label_ShakeX";
            this.Label_ShakeX.Size = new System.Drawing.Size(17, 13);
            this.Label_ShakeX.TabIndex = 4;
            this.Label_ShakeX.Text = "X:";
            // 
            // TrackBar_ShakeFrequency
            // 
            this.TrackBar_ShakeFrequency.Location = new System.Drawing.Point(43, 116);
            this.TrackBar_ShakeFrequency.Maximum = 2000;
            this.TrackBar_ShakeFrequency.Name = "TrackBar_ShakeFrequency";
            this.TrackBar_ShakeFrequency.Size = new System.Drawing.Size(199, 45);
            this.TrackBar_ShakeFrequency.TabIndex = 3;
            this.TrackBar_ShakeFrequency.TickFrequency = 10;
            this.TrackBar_ShakeFrequency.Value = 500;
            this.TrackBar_ShakeFrequency.Scroll += new System.EventHandler(this.TrackBar_ShakeFrequency_Scroll);
            // 
            // TrackBar_ShakeZ
            // 
            this.TrackBar_ShakeZ.Location = new System.Drawing.Point(43, 82);
            this.TrackBar_ShakeZ.Maximum = 500;
            this.TrackBar_ShakeZ.Name = "TrackBar_ShakeZ";
            this.TrackBar_ShakeZ.Size = new System.Drawing.Size(199, 45);
            this.TrackBar_ShakeZ.TabIndex = 2;
            this.TrackBar_ShakeZ.TickFrequency = 10;
            this.TrackBar_ShakeZ.Value = 100;
            this.TrackBar_ShakeZ.Scroll += new System.EventHandler(this.TrackBar_ShakeZ_Scroll);
            // 
            // TrackBar_ShakeY
            // 
            this.TrackBar_ShakeY.Location = new System.Drawing.Point(43, 48);
            this.TrackBar_ShakeY.Maximum = 500;
            this.TrackBar_ShakeY.Name = "TrackBar_ShakeY";
            this.TrackBar_ShakeY.Size = new System.Drawing.Size(199, 45);
            this.TrackBar_ShakeY.TabIndex = 1;
            this.TrackBar_ShakeY.TickFrequency = 10;
            this.TrackBar_ShakeY.Value = 100;
            this.TrackBar_ShakeY.Scroll += new System.EventHandler(this.TrackBar_ShakeY_Scroll);
            // 
            // TrackBar_ShakeX
            // 
            this.TrackBar_ShakeX.Location = new System.Drawing.Point(43, 14);
            this.TrackBar_ShakeX.Maximum = 500;
            this.TrackBar_ShakeX.Name = "TrackBar_ShakeX";
            this.TrackBar_ShakeX.Size = new System.Drawing.Size(199, 45);
            this.TrackBar_ShakeX.TabIndex = 0;
            this.TrackBar_ShakeX.TickFrequency = 10;
            this.TrackBar_ShakeX.Value = 100;
            this.TrackBar_ShakeX.Scroll += new System.EventHandler(this.TrackBar_ShakeX_Scroll);
            // 
            // CheckBox_ShakeOverride
            // 
            this.CheckBox_ShakeOverride.AutoSize = true;
            this.CheckBox_ShakeOverride.CheckAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.CheckBox_ShakeOverride.Location = new System.Drawing.Point(148, 105);
            this.CheckBox_ShakeOverride.Name = "CheckBox_ShakeOverride";
            this.CheckBox_ShakeOverride.Size = new System.Drawing.Size(100, 17);
            this.CheckBox_ShakeOverride.TabIndex = 6;
            this.CheckBox_ShakeOverride.Text = "Shake Override";
            this.CheckBox_ShakeOverride.UseVisualStyleBackColor = true;
            this.CheckBox_ShakeOverride.CheckedChanged += new System.EventHandler(this.CheckBox_ShakeOverride_CheckedChanged);
            // 
            // Label_CurrentFocusVehicle_Name
            // 
            this.Label_CurrentFocusVehicle_Name.AutoSize = true;
            this.Label_CurrentFocusVehicle_Name.Location = new System.Drawing.Point(123, 54);
            this.Label_CurrentFocusVehicle_Name.Name = "Label_CurrentFocusVehicle_Name";
            this.Label_CurrentFocusVehicle_Name.Size = new System.Drawing.Size(35, 13);
            this.Label_CurrentFocusVehicle_Name.TabIndex = 5;
            this.Label_CurrentFocusVehicle_Name.Text = "<null>";
            // 
            // CheckBox_CameraLookAt
            // 
            this.CheckBox_CameraLookAt.AutoSize = true;
            this.CheckBox_CameraLookAt.CheckAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.CheckBox_CameraLookAt.Location = new System.Drawing.Point(185, 82);
            this.CheckBox_CameraLookAt.Name = "CheckBox_CameraLookAt";
            this.CheckBox_CameraLookAt.Size = new System.Drawing.Size(63, 17);
            this.CheckBox_CameraLookAt.TabIndex = 3;
            this.CheckBox_CameraLookAt.Text = "Look At";
            this.CheckBox_CameraLookAt.UseVisualStyleBackColor = true;
            this.CheckBox_CameraLookAt.CheckedChanged += new System.EventHandler(this.CheckBox_CameraLookAt_CheckedChanged);
            // 
            // Label_CurrentCameraType
            // 
            this.Label_CurrentCameraType.AutoSize = true;
            this.Label_CurrentCameraType.Location = new System.Drawing.Point(18, 79);
            this.Label_CurrentCameraType.Name = "Label_CurrentCameraType";
            this.Label_CurrentCameraType.Size = new System.Drawing.Size(107, 13);
            this.Label_CurrentCameraType.TabIndex = 4;
            this.Label_CurrentCameraType.Text = "Current Camera Type";
            // 
            // ComboBox_CameraType
            // 
            this.ComboBox_CameraType.FormattingEnabled = true;
            this.ComboBox_CameraType.Items.AddRange(new object[] {
            "Game Feed",
            "Fly",
            "Relative Fly",
            "Fixed",
            "Relative",
            "Script Cam"});
            this.ComboBox_CameraType.Location = new System.Drawing.Point(6, 95);
            this.ComboBox_CameraType.Name = "ComboBox_CameraType";
            this.ComboBox_CameraType.Size = new System.Drawing.Size(137, 21);
            this.ComboBox_CameraType.TabIndex = 3;
            this.ComboBox_CameraType.SelectedIndexChanged += new System.EventHandler(this.ComboBox_CameraType_SelectedIndexChanged);
            // 
            // Label_CurrentFocusVehicle
            // 
            this.Label_CurrentFocusVehicle.AutoSize = true;
            this.Label_CurrentFocusVehicle.Location = new System.Drawing.Point(3, 54);
            this.Label_CurrentFocusVehicle.Name = "Label_CurrentFocusVehicle";
            this.Label_CurrentFocusVehicle.Size = new System.Drawing.Size(114, 13);
            this.Label_CurrentFocusVehicle.TabIndex = 2;
            this.Label_CurrentFocusVehicle.Text = "Current Focus Vehicle:";
            // 
            // Label_FocusVehicle_Value
            // 
            this.Label_FocusVehicle_Value.AutoSize = true;
            this.Label_FocusVehicle_Value.Location = new System.Drawing.Point(234, 21);
            this.Label_FocusVehicle_Value.Name = "Label_FocusVehicle_Value";
            this.Label_FocusVehicle_Value.Size = new System.Drawing.Size(10, 13);
            this.Label_FocusVehicle_Value.TabIndex = 1;
            this.Label_FocusVehicle_Value.Text = "-";
            // 
            // TrackBar_FocusVehicle
            // 
            this.TrackBar_FocusVehicle.Location = new System.Drawing.Point(6, 20);
            this.TrackBar_FocusVehicle.Maximum = 20;
            this.TrackBar_FocusVehicle.Name = "TrackBar_FocusVehicle";
            this.TrackBar_FocusVehicle.Size = new System.Drawing.Size(228, 45);
            this.TrackBar_FocusVehicle.TabIndex = 0;
            this.TrackBar_FocusVehicle.Scroll += new System.EventHandler(this.TrackBar_FocusVehicle_Scroll);
            // 
            // Box_RecordingControls
            // 
            this.Box_RecordingControls.Controls.Add(this.Label_SceneLength);
            this.Box_RecordingControls.Controls.Add(this.TextBox_SceneLength);
            this.Box_RecordingControls.Controls.Add(this.CheckBox_RecordEveryFrame);
            this.Box_RecordingControls.Location = new System.Drawing.Point(274, 198);
            this.Box_RecordingControls.Name = "Box_RecordingControls";
            this.Box_RecordingControls.Size = new System.Drawing.Size(215, 55);
            this.Box_RecordingControls.TabIndex = 3;
            this.Box_RecordingControls.TabStop = false;
            this.Box_RecordingControls.Text = "Recording Controls";
            // 
            // Box_Perks
            // 
            this.Box_Perks.Controls.Add(this.Numeric_PerkLevel);
            this.Box_Perks.Controls.Add(this.Button_FirePerk);
            this.Box_Perks.Controls.Add(this.ComboBox_PerkType);
            this.Box_Perks.Location = new System.Drawing.Point(544, 499);
            this.Box_Perks.Name = "Box_Perks";
            this.Box_Perks.Size = new System.Drawing.Size(246, 52);
            this.Box_Perks.TabIndex = 4;
            this.Box_Perks.TabStop = false;
            this.Box_Perks.Text = "Perks";
            // 
            // Numeric_PerkLevel
            // 
            this.Numeric_PerkLevel.Location = new System.Drawing.Point(105, 20);
            this.Numeric_PerkLevel.Maximum = new decimal(new int[] {
            3,
            0,
            0,
            0});
            this.Numeric_PerkLevel.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.Numeric_PerkLevel.Name = "Numeric_PerkLevel";
            this.Numeric_PerkLevel.Size = new System.Drawing.Size(41, 20);
            this.Numeric_PerkLevel.TabIndex = 2;
            this.Numeric_PerkLevel.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // Button_FirePerk
            // 
            this.Button_FirePerk.BackColor = System.Drawing.Color.White;
            this.Button_FirePerk.ForeColor = System.Drawing.Color.Red;
            this.Button_FirePerk.Location = new System.Drawing.Point(163, 17);
            this.Button_FirePerk.Name = "Button_FirePerk";
            this.Button_FirePerk.Size = new System.Drawing.Size(66, 24);
            this.Button_FirePerk.TabIndex = 1;
            this.Button_FirePerk.Text = "FIRE!";
            this.Button_FirePerk.UseVisualStyleBackColor = false;
            this.Button_FirePerk.Click += new System.EventHandler(this.Button_FirePerk_Click);
            // 
            // ComboBox_PerkType
            // 
            this.ComboBox_PerkType.FormattingEnabled = true;
            this.ComboBox_PerkType.Items.AddRange(new object[] {
            "Barge",
            "Flash",
            "Nitrous",
            "Shock",
            "Shunt"});
            this.ComboBox_PerkType.Location = new System.Drawing.Point(7, 20);
            this.ComboBox_PerkType.Name = "ComboBox_PerkType";
            this.ComboBox_PerkType.Size = new System.Drawing.Size(79, 21);
            this.ComboBox_PerkType.TabIndex = 0;
            // 
            // Box_CutsceneEditing
            // 
            this.Box_CutsceneEditing.Controls.Add(this.Button_TakeControl);
            this.Box_CutsceneEditing.Location = new System.Drawing.Point(544, 12);
            this.Box_CutsceneEditing.Name = "Box_CutsceneEditing";
            this.Box_CutsceneEditing.Size = new System.Drawing.Size(245, 84);
            this.Box_CutsceneEditing.TabIndex = 5;
            this.Box_CutsceneEditing.TabStop = false;
            this.Box_CutsceneEditing.Text = "Cutscene Editing";
            // 
            // Button_TakeControl
            // 
            this.Button_TakeControl.Location = new System.Drawing.Point(66, 25);
            this.Button_TakeControl.Name = "Button_TakeControl";
            this.Button_TakeControl.Size = new System.Drawing.Size(116, 44);
            this.Button_TakeControl.TabIndex = 0;
            this.Button_TakeControl.Text = "Take Control of Focus Vehicle";
            this.Button_TakeControl.UseVisualStyleBackColor = true;
            this.Button_TakeControl.Click += new System.EventHandler(this.Button_TakeControl_Click);
            // 
            // ReplayToolForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(797, 556);
            this.Controls.Add(this.Box_CutsceneEditing);
            this.Controls.Add(this.Box_Perks);
            this.Controls.Add(this.Box_RecordingControls);
            this.Controls.Add(this.Box_Cameras);
            this.Controls.Add(this.Box_PlaybackControls);
            this.Controls.Add(this.Box_ReplayInformation);
            this.Name = "ReplayToolForm";
            this.Text = "Bizarre Creations - Amax - Replay Tool";
            this.Load += new System.EventHandler(this.ReplayToolForm_Load);
            this.Box_ReplayInformation.ResumeLayout(false);
            this.Box_ReplayInformation.PerformLayout();
            this.Box_ReplayInformation_Statistics.ResumeLayout(false);
            this.Box_ReplayInformation_Statistics.PerformLayout();
            this.Box_PlaybackControls.ResumeLayout(false);
            this.Box_PlaybackControls.PerformLayout();
            this.Box_PlaybackControls_SceneSkip.ResumeLayout(false);
            this.Box_Cameras.ResumeLayout(false);
            this.Box_Cameras.PerformLayout();
            this.Box_Cameras_LookAt.ResumeLayout(false);
            this.Box_Cameras_LookAt.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtFoV)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_LookAtTarget)).EndInit();
            this.Box_Cameras_Shake.ResumeLayout(false);
            this.Box_Cameras_Shake.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_ShakeFrequency)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_ShakeZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_ShakeY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_ShakeX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.TrackBar_FocusVehicle)).EndInit();
            this.Box_RecordingControls.ResumeLayout(false);
            this.Box_RecordingControls.PerformLayout();
            this.Box_Perks.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.Numeric_PerkLevel)).EndInit();
            this.Box_CutsceneEditing.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox Box_ReplayInformation;
        private System.Windows.Forms.GroupBox Box_ReplayInformation_Statistics;
        private System.Windows.Forms.Label Label_CurrentTime;
        private System.Windows.Forms.Label Label_ReplayLength;
        private System.Windows.Forms.Label Label_CurrentScene;
        private System.Windows.Forms.Label Label_NumScenes;
        private System.Windows.Forms.Label Label_CurrentTime_Value;
        private System.Windows.Forms.Label Label_ReplayLength_Value;
        private System.Windows.Forms.Label Label_NumScenes_Value;
        private System.Windows.Forms.Label Label_CurrentScene_Value;
        private System.Windows.Forms.GroupBox Box_PlaybackControls;
        private System.Windows.Forms.Button Button_Pause;
        private System.Windows.Forms.Button Button_Restart;
        private System.Windows.Forms.GroupBox Box_PlaybackControls_SceneSkip;
        private System.Windows.Forms.Button Button_NextScene;
        private System.Windows.Forms.Button Button_PreviousScene;
        private System.Windows.Forms.CheckBox CheckBox_Loop;
        private System.Windows.Forms.Button Button_GoToTime;
        private System.Windows.Forms.TextBox TextBox_ReplayLoadFilename;
        private System.Windows.Forms.ToolTip ToolTip1;
        private System.Windows.Forms.Button Button_LoadReplay;
        private System.Windows.Forms.OpenFileDialog OpenReplayFileDialog;
        private System.Windows.Forms.CheckBox CheckBox_RecordEveryFrame;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.MaskedTextBox TextBox_GoToTime_Minutes;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.MaskedTextBox TextBox_GoToTime_Seconds;
        private System.Windows.Forms.GroupBox Box_Cameras;
        private System.Windows.Forms.TrackBar TrackBar_FocusVehicle;
        private System.Windows.Forms.Label Label_FocusVehicle_Value;
        private System.Windows.Forms.Label Label_CurrentFocusVehicle;
        private System.Windows.Forms.ComboBox ComboBox_CameraType;
        private System.Windows.Forms.Label Label_CurrentCameraType;
        private System.Windows.Forms.CheckBox CheckBox_CameraLookAt;
        private System.Windows.Forms.Button Button_Refresh;
        private System.Windows.Forms.GroupBox Box_RecordingControls;
        private System.Windows.Forms.MaskedTextBox TextBox_SceneLength;
        private System.Windows.Forms.Label Label_CurrentFocusVehicle_Name;
        private System.Windows.Forms.Label Label_SceneLength;
        private System.Windows.Forms.TextBox TextBox_ReplaySaveFilename;
        private System.Windows.Forms.Label Label_ReplaySaveFilename_Extension;
        private System.Windows.Forms.Button Button_SetSaveFilename;
        private System.Windows.Forms.CheckBox CheckBox_ShakeOverride;
        private System.Windows.Forms.GroupBox Box_Cameras_Shake;
        private System.Windows.Forms.TrackBar TrackBar_ShakeX;
        private System.Windows.Forms.TrackBar TrackBar_ShakeY;
        private System.Windows.Forms.TrackBar TrackBar_ShakeZ;
        private System.Windows.Forms.TrackBar TrackBar_ShakeFrequency;
        private System.Windows.Forms.Label Label_ShakeFrequency;
        private System.Windows.Forms.Label Label_ShakeZ;
        private System.Windows.Forms.Label Label_ShakeY;
        private System.Windows.Forms.Label Label_ShakeX;
        private System.Windows.Forms.Label Label_ShakeY_Value;
        private System.Windows.Forms.Label Label_ShakeX_Value;
        private System.Windows.Forms.Label Label_ShakeFrequency_Value;
        private System.Windows.Forms.Label Label_ShakeZ_Value;
        private System.Windows.Forms.CheckBox CheckBox_ShowUI;
        private System.Windows.Forms.GroupBox Box_Cameras_LookAt;
        private System.Windows.Forms.Label Label_LookAtTarget_Number;
        private System.Windows.Forms.TrackBar TrackBar_LookAtTarget;
        private System.Windows.Forms.Label Label_LookAtTarget_Name;
        private System.Windows.Forms.Label Label_LookAtTarget;
        private System.Windows.Forms.TrackBar TrackBar_LookAtFoV;
        private System.Windows.Forms.TrackBar TrackBar_LookAtX;
        private System.Windows.Forms.TrackBar TrackBar_LookAtY;
        private System.Windows.Forms.TrackBar TrackBar_LookAtZ;
        private System.Windows.Forms.Label Label_LookAtFoV_Value;
        private System.Windows.Forms.Label Label_LookAtZ_Value;
        private System.Windows.Forms.Label Label_LookAtY_Value;
        private System.Windows.Forms.Label Label_LookAtX_Value;
        private System.Windows.Forms.Label Label_LookAtFoV;
        private System.Windows.Forms.Label Label_LookAtZ;
        private System.Windows.Forms.Label Label_LookAtY;
        private System.Windows.Forms.Label Label_LookAtX;
        private System.Windows.Forms.GroupBox Box_Perks;
        private System.Windows.Forms.ComboBox ComboBox_PerkType;
        private System.Windows.Forms.Button Button_FirePerk;
        private System.Windows.Forms.NumericUpDown Numeric_PerkLevel;
        private System.Windows.Forms.Button Button_SkipFrame;
        private System.Windows.Forms.GroupBox Box_CutsceneEditing;
        private System.Windows.Forms.Button Button_TakeControl;
    }
}


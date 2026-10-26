using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using XDevkit;

using Bizarre.Artery;

namespace ReplayTool
{
    public partial class ReplayToolForm : Form
    {
        bool m_ConsoleConnected = false;
        Connection m_ConsoleConnection;

        int m_TickCount = 0;
        int m_UpdateFrequency = 1;

        static float m_CurrentTime;
        static float m_ReplayLength;
        static Int32 m_CurrentScene;
        static Int32 m_NumScenes;
        static Int32 m_NumCars;
        static string m_CurrentReplayFilename = ".amrp";
        static string m_CurrentFocusVehicleName = "<null>";
        static string m_LookAtTargetVehicleName = "<null>";

        bool m_Paused = false;

        public ReplayToolForm()
        {
            InitializeComponent();

            XboxManagerClass xboxManager = new XboxManagerClass();
            XboxConsole xboxConsole;
            try
            {
                xboxConsole = xboxManager.OpenConsole(xboxManager.DefaultConsole);
            }
            catch
            {
                MessageBox.Show("No default Xbox 360 found", "Error");
                this.Close();
                Application.Exit();
                return;
            }

            if (ArteryManager.Startup(false, "ReplayToolPC"))
            {
                m_ConsoleConnection = new Connection();
                SockAddr target = new SockAddr(xboxConsole.IPAddressTitle, 6317);

                bool result = ArteryManager.Connect(m_ConsoleConnection, target, "Replay", MyConnectCallback, 0);
            }
        }

        private void ReplayToolForm_Load(object sender, EventArgs e)
        {
            SendMessage("REPLAY_CONSTANT_INFO_REQUEST");
        }

        private string ConvertSecondsToClockString(float p_Seconds)
        {
            string result;
            int minutes, seconds, hundredths;
            minutes = (int)p_Seconds / 60;
            seconds = (int)p_Seconds % 60;
            hundredths = (int)((p_Seconds - (int)p_Seconds) * 100);

            //  Minutes
            if (minutes < 10)
            {
                result = "0" + minutes + ":";
            }
            else
            {
                result = minutes + ":";
            }

            //  Seconds
            if (seconds < 10)
            {
                result += "0" + seconds + ".";
            }
            else
            {
                result += seconds + ".";
            }

            //Hundredths of a second
            if(hundredths < 10)
            {
                result += "0" + hundredths;
            }
            else
            {
                result += hundredths;
            }

            return result;
        }

        private void UpdateReplayToolForm()
        {
            TextBox_ReplayLoadFilename.Text = m_CurrentReplayFilename;
            Label_CurrentTime_Value.Text = ConvertSecondsToClockString(m_CurrentTime);
            Label_ReplayLength_Value.Text = ConvertSecondsToClockString(m_ReplayLength) ;
            Label_CurrentScene_Value.Text = m_CurrentScene.ToString();
            Label_NumScenes_Value.Text = m_NumScenes.ToString();
            TrackBar_FocusVehicle.Maximum = m_NumCars - 1;
            TrackBar_FocusVehicle.Minimum = 0;
            Label_FocusVehicle_Value.Text = TrackBar_FocusVehicle.Value.ToString();
            Label_CurrentFocusVehicle_Name.Text = m_CurrentFocusVehicleName;

            Label_ShakeX_Value.Text = "" + (float)(Convert.ToInt32(TrackBar_ShakeX.Value)) / 100;
            Label_ShakeY_Value.Text = "" + (float)(Convert.ToInt32(TrackBar_ShakeY.Value)) / 100;
            Label_ShakeZ_Value.Text = "" + (float)(Convert.ToInt32(TrackBar_ShakeZ.Value)) / 100;
            Label_ShakeFrequency_Value.Text = "" + (float)(Convert.ToInt32(TrackBar_ShakeFrequency.Value)) / 100;

            TrackBar_LookAtTarget.Maximum = m_NumCars - 1;
            TrackBar_LookAtTarget.Minimum = 0;
            Label_LookAtTarget_Number.Text = TrackBar_LookAtTarget.Value.ToString();
            Label_LookAtTarget_Name.Text = m_LookAtTargetVehicleName;

            Label_LookAtX_Value.Text = "" + (float)(Convert.ToInt32(TrackBar_LookAtX.Value)) / 10;
            Label_LookAtY_Value.Text = "" + (float)(Convert.ToInt32(TrackBar_LookAtY.Value)) / 10;
            Label_LookAtZ_Value.Text = "" + (float)(Convert.ToInt32(TrackBar_LookAtZ.Value)) / 10;
            Label_LookAtFoV_Value.Text = "" + (float)(Convert.ToInt32(TrackBar_LookAtFoV.Value));
        }

        private void MyConnectCallback(Connection p_Connection, object p_Context)
        {
            if (p_Connection.IsConnected())
            {
                m_ConsoleConnected = true;

                while (p_Connection.IsConnected())
                {
                    Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
                    if (p_Connection.Receive(msg))
                    {
                        Callback_UserMsg(msg);
                    }
                }

                m_ConsoleConnected = false;
            }
        }

        private void Callback_UserMsg(Bizarre.Artery.Message msg)
        {
            Bizarre.Artery.MessageReader msgReader = new Bizarre.Artery.MessageReader(msg);

            String text;
            msgReader.Read(out text);

            if (text == "REPLAY_UPDATE_TOOL")
            {
                msgReader.Read(out m_CurrentTime);
                msgReader.Read(out m_CurrentScene);
                msgReader.Read(out m_CurrentFocusVehicleName);
                msgReader.Read(out m_LookAtTargetVehicleName);
            }
            else if (text == "REPLAY_CONSTANT_INFO")
            {
                msgReader.Read(out m_CurrentReplayFilename);
                msgReader.Read(out m_ReplayLength);
                msgReader.Read(out m_NumScenes);
                msgReader.Read(out m_NumCars);
            }
        }

        private bool SendMessage(String messageText)
        {
            if (m_ConsoleConnected)
            {
                // Send the message
                MessageBuilder b = new MessageBuilder();

                b.Write(messageText);

                Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
                if (b.Build(msg))
                {
                    return m_ConsoleConnection.Send(msg);
                }
            }
            return false;
        }

        private bool SendMessage(String messageText, int value)
        {
            if (m_ConsoleConnected)
            {
                // Send the message
                MessageBuilder b = new MessageBuilder();

                b.Write(messageText);
                b.Write(value);

                Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
                if (b.Build(msg))
                {
                    return m_ConsoleConnection.Send(msg);
                }
            }
            return false;
        }

        private bool SendMessage(String messageText, float value)
        {
            if (m_ConsoleConnected)
            {
                // Send the message
                MessageBuilder b = new MessageBuilder();

                b.Write(messageText);
                b.Write(value);

                Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
                if (b.Build(msg))
                {
                    return m_ConsoleConnection.Send(msg);
                }
            }
            return false;
        }

        private bool SendMessage(String messageText, String text)
        {
            if (m_ConsoleConnected)
            {
                // Send the message
                MessageBuilder b = new MessageBuilder();

                b.Write(messageText);
                b.Write(text);

                Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
                if (b.Build(msg))
                {
                    return m_ConsoleConnection.Send(msg);
                }
            }
            return false;
        }

        private void Button_Pause_Click(object sender, EventArgs e)
        {
            m_Paused = !m_Paused;
            if (m_Paused)
            {
                this.Button_Pause.Text = "Play";
                ToolTip1.SetToolTip(this.Button_Pause, "Resume playback");
                SendMessage("REPLAY_PAUSE_TOGGLE", 1);
            }
            else
            {
                this.Button_Pause.Text = "Pause";
                ToolTip1.SetToolTip(this.Button_Pause, "Pause playback");
                SendMessage("REPLAY_PAUSE_TOGGLE", 0);
            }
        }

        private void OpenReplayFileDialog_FileOk(object sender, CancelEventArgs e)
        {
            //  TO DO: This is where to actually start the loading of the replay file
            SendMessage("REPLAY_DEBUG_LOAD_FILE", OpenReplayFileDialog.FileName);
        }

        private void Button_LoadReplay_Click(object sender, EventArgs e)
        {
            OpenReplayFileDialog.ShowDialog(this);
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            m_TickCount++;

            if (m_TickCount > m_UpdateFrequency)
            {
                SendMessage("REPLAY_UPDATE_TOOL_REQUEST");
                m_TickCount = 0;
            }
            UpdateReplayToolForm();
        }

        private void Button_GoToTime_Click(object sender, EventArgs e)
        {
            int minutes = Convert.ToInt32(TextBox_GoToTime_Minutes.Text);
            int seconds = Convert.ToInt32(TextBox_GoToTime_Seconds.Text);
            int hundredths = Convert.ToInt32(TextBox_GoToTime_Hundredths.Text);

            float goToTime = (float)(minutes * 60) + (float)seconds + (((float)hundredths) / 100);
            SendMessage("REPLAY_GOTO_TIME", goToTime);
        }

        private void TextBox_SceneLength_TextChanged(object sender, EventArgs e)
        {
            try
            {
                SendMessage("REPLAY_CHANGE_SCENE_LENGTH", Convert.ToInt32(TextBox_SceneLength.Text));
            }
            catch
            {
            }
        }

        private void Button_Restart_Click(object sender, EventArgs e)
        {
            SendMessage("REPLAY_RESTART");
        }

        private void CheckBox_Loop_CheckedChanged(object sender, EventArgs e)
        {
            if (CheckBox_Loop.Checked)
                SendMessage("REPLAY_LOOP_TOGGLE", 1);
            else
                SendMessage("REPLAY_LOOP_TOGGLE", 0);
        }

        private void Button_PreviousScene_Click(object sender, EventArgs e)
        {
            SendMessage("REPLAY_PREVIOUS_SCENE");
        }

        private void Button_NextScene_Click(object sender, EventArgs e)
        {
            SendMessage("REPLAY_NEXT_SCENE");
        }

        private void TrackBar_FocusVehicle_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_CHANGE_FOCUS_VEHICLE", TrackBar_FocusVehicle.Value);
        }

        private void Button_Refresh_Click(object sender, EventArgs e)
        {
            SendMessage("REPLAY_CONSTANT_INFO_REQUEST");
        }

        private void ComboBox_CameraType_SelectedIndexChanged(object sender, EventArgs e)
        {
            SendMessage("REPLAY_CHANGE_CAMERA", ComboBox_CameraType.Text);
        }

        private void CheckBox_RecordEveryFrame_CheckedChanged(object sender, EventArgs e)
        {
            if (CheckBox_RecordEveryFrame.Checked)
                SendMessage("REPLAY_RECORD_EVERY_FRAME_TOGGLE", 1);
            else
                SendMessage("REPLAY_RECORD_EVERY_FRAME_TOGGLE", 0);
        }

        private void Button_SetSaveFilename_Click(object sender, EventArgs e)
        {
            SendMessage("REPLAY_SET_SAVE_FILENAME", TextBox_ReplaySaveFilename.Text);
        }

        private void CheckBox_CameraLookAt_CheckedChanged(object sender, EventArgs e)
        {
            if(CheckBox_CameraLookAt.Checked)
                SendMessage("REPLAY_LOOKAT_TOGGLE", 1);
            else
                SendMessage("REPLAY_LOOKAT_TOGGLE", 0);
        }

        private void CheckBox_ShakeOverride_CheckedChanged(object sender, EventArgs e)
        {
            if (CheckBox_ShakeOverride.Checked)
                SendMessage("REPLAY_SHAKEOVERRIDE_TOGGLE", 1);
            else
                SendMessage("REPLAY_SHAKEOVERRIDE_TOGGLE", 0);
        }

        private void TrackBar_ShakeX_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_SHAKEX", (float)(Convert.ToInt32(TrackBar_ShakeX.Value)) / 100);
        }

        private void TrackBar_ShakeY_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_SHAKEY", (float)(Convert.ToInt32(TrackBar_ShakeY.Value)) / 100);
        }

        private void TrackBar_ShakeZ_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_SHAKEZ", (float)(Convert.ToInt32(TrackBar_ShakeZ.Value)) / 100);
        }

        private void TrackBar_ShakeFrequency_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_SHAKEFREQUENCY", (float)(Convert.ToInt32(TrackBar_ShakeFrequency.Value)) / 100);
        }

        private void CheckBox_ShowUI_CheckedChanged(object sender, EventArgs e)
        {
            if(CheckBox_ShowUI.Checked)
                SendMessage("REPLAY_SHOWUI_TOGGLE", 1);
            else
                SendMessage("REPLAY_SHOWUI_TOGGLE", 0);
        }

        private void TrackBar_LookAtTarget_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_CHANGE_LOOKAT_TARGET", TrackBar_LookAtTarget.Value);
        }

        private void TrackBar_LookAtX_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_LOOKATX", (float)(Convert.ToInt32(TrackBar_LookAtX.Value)) / 10);
        }

        private void TrackBar_LookAtY_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_LOOKATY", (float)(Convert.ToInt32(TrackBar_LookAtY.Value)) / 10);
        }

        private void TrackBar_LookAtZ_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_LOOKATZ", (float)(Convert.ToInt32(TrackBar_LookAtZ.Value)) / 10);
        }

        private void TrackBar_LookAtFoV_Scroll(object sender, EventArgs e)
        {
            SendMessage("REPLAY_LOOKAT_FOV", (float)(Convert.ToInt32(TrackBar_LookAtFoV.Value)));
        }

        private void Button_FirePerk_Click(object sender, EventArgs e)
        {
            MessageBuilder b = new MessageBuilder();

            b.Write("REPLAY_FIREPERK");
            b.Write(ComboBox_PerkType.Text);
            b.Write(Convert.ToInt32(Numeric_PerkLevel.Value));
            b.Write(TrackBar_FocusVehicle.Value);

            Bizarre.Artery.Message msg = new Bizarre.Artery.Message();
            if (b.Build(msg))
            {
                m_ConsoleConnection.Send(msg);
            }
        }

        private void Button_SkipFrame_Click(object sender, EventArgs e)
        {
            SendMessage("REPLAY_SKIP_FRAME");
        }

        private void Button_TakeControl_Click(object sender, EventArgs e)
        {
            SendMessage("REPLAY_TAKE_CONTROL", TrackBar_FocusVehicle.Value);
        }
    }
}
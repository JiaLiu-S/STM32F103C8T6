using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Media;
using System.Text;
using System.Windows.Forms;

namespace VillaSecurityMonitor
{
    static class Program
    {
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MonitorForm());
        }
    }

    public sealed class MonitorForm : Form
    {
        private readonly Color ink = Color.FromArgb(32, 45, 51);
        private readonly Color muted = Color.FromArgb(96, 110, 116);
        private readonly Color green = Color.FromArgb(35, 127, 79);
        private readonly Color red = Color.FromArgb(190, 47, 46);
        private readonly ComboBox ports = new ComboBox();
        private readonly Button connect = new Button();
        private readonly Button refresh = new Button();
        private readonly Button export = new Button();
        private readonly CheckBox sound = new CheckBox();
        private readonly Label banner = new Label();
        private readonly Label connection = new Label();
        private Label mode;
        private Label entry;
        private Label temp;
        private Label received;
        private readonly ListView events = new ListView();
        private readonly NotifyIcon tray = new NotifyIcon();
        private readonly Timer timer = new Timer();
        private readonly List<string[]> records = new List<string[]>();
        private readonly StringBuilder input = new StringBuilder();
        private SerialPort serial;
        private DateTime lastFrame = DateTime.MinValue;
        private StatusFrame previous;
        private bool stale;

        public MonitorForm()
        {
            Text = "别墅安防 · 串口报警监控";
            Font = new Font("Microsoft YaHei UI", 10F);
            BackColor = Color.FromArgb(248, 250, 249);
            ForeColor = ink;
            MinimumSize = new Size(720, 560);
            Size = new Size(820, 650);
            StartPosition = FormStartPosition.CenterScreen;
            BuildUi();
            tray.Icon = SystemIcons.Warning;
            tray.Text = "别墅安防串口监控";
            tray.Visible = true;
            timer.Interval = 1000;
            timer.Tick += TimerTick;
            timer.Start();
            RefreshPorts();
            ShowUnavailable("等待连接", "选择 COM 口后点击连接");
        }

        private Label NewLabel(string value, int x, int y, int width, int height, float size, bool bold)
        {
            Label label = new Label();
            label.Text = value;
            label.Location = new Point(x, y);
            label.Size = new Size(width, height);
            label.Font = new Font(Font.FontFamily, size, bold ? FontStyle.Bold : FontStyle.Regular);
            label.TextAlign = ContentAlignment.MiddleLeft;
            Controls.Add(label);
            return label;
        }

        private void BuildUi()
        {
            NewLabel("安防状态", 24, 15, 340, 35, 18, true);
            connection.Location = new Point(24, 51);
            connection.Size = new Size(700, 27);
            connection.ForeColor = muted;
            connection.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            Controls.Add(connection);

            ports.Location = new Point(24, 92);
            ports.Size = new Size(170, 31);
            ports.DropDownStyle = ComboBoxStyle.DropDownList;
            Controls.Add(ports);
            refresh.Text = "刷新串口";
            refresh.Location = new Point(205, 91);
            refresh.Size = new Size(105, 34);
            refresh.Click += delegate { RefreshPorts(); };
            Controls.Add(refresh);
            connect.Text = "连接";
            connect.Location = new Point(322, 91);
            connect.Size = new Size(105, 34);
            connect.Click += ConnectClick;
            Controls.Add(connect);
            sound.Text = "报警声音";
            sound.Checked = true;
            sound.Location = new Point(449, 94);
            sound.Size = new Size(120, 28);
            Controls.Add(sound);

            banner.Location = new Point(24, 144);
            banner.Size = new Size(756, 91);
            banner.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            banner.TextAlign = ContentAlignment.MiddleCenter;
            banner.Font = new Font(Font.FontFamily, 25, FontStyle.Bold);
            Controls.Add(banner);

            mode = NewLabel("布防：--", 24, 249, 240, 34, 13, true);
            entry = NewLabel("红外：--", 284, 249, 240, 34, 13, true);
            temp = NewLabel("热敏：--", 544, 249, 230, 34, 13, true);
            entry.Anchor = AnchorStyles.Top | AnchorStyles.Left;
            received = NewLabel("最后接收：--", 24, 290, 760, 27, 9, false);
            received.ForeColor = muted;
            received.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;

            NewLabel("事件记录", 24, 330, 300, 30, 13, true);
            export.Text = "导出 CSV";
            export.Location = new Point(663, 328);
            export.Size = new Size(117, 33);
            export.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            export.Click += ExportClick;
            Controls.Add(export);
            events.Location = new Point(24, 372);
            events.Size = new Size(756, 222);
            events.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            events.View = View.Details;
            events.FullRowSelect = true;
            events.GridLines = true;
            events.Columns.Add("电脑时间", 175);
            events.Columns.Add("事件", 230);
            events.Columns.Add("状态", 315);
            Controls.Add(events);
        }

        private void RefreshPorts()
        {
            string selected = ports.SelectedItem as string;
            string[] names;
            try { names = SerialPort.GetPortNames().OrderBy(p => p).ToArray(); }
            catch (Exception ex) { connection.Text = "读取串口失败：" + ex.Message; return; }
            ports.Items.Clear();
            ports.Items.AddRange(names);
            if (selected != null && names.Contains(selected)) ports.SelectedItem = selected;
            else if (names.Length > 0) ports.SelectedIndex = 0;
            if (serial == null && names.Length == 0)
                connection.Text = "未发现 COM 口，请插入 CH340 并检查驱动";
        }

        private void ConnectClick(object sender, EventArgs e)
        {
            if (serial != null) { Disconnect("已断开", false); return; }
            string name = ports.SelectedItem as string;
            if (name == null) { MessageBox.Show("未找到串口。请先插入 CH340 USB 转 TTL 模块。", "连接", MessageBoxButtons.OK, MessageBoxIcon.Information); return; }
            try {
                serial = new SerialPort(name, 115200, Parity.None, 8, StopBits.One);
                serial.Encoding = Encoding.ASCII;
                serial.DataReceived += SerialReceived;
                serial.ErrorReceived += SerialError;
                serial.Open();
                input.Clear();
                previous = null;
                stale = false;
                lastFrame = DateTime.UtcNow;
                connect.Text = "断开";
                ports.Enabled = false;
                connection.Text = name + " 已连接 · 115200 8N1，等待 STM32 数据";
                ShowUnavailable("等待数据", "已连接，但尚未收到有效状态");
                AddEvent("串口已连接", name);
            } catch (Exception ex) {
                if (serial != null) { serial.Dispose(); serial = null; }
                MessageBox.Show("无法打开 " + name + "：" + ex.Message, "连接失败", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void SerialReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try {
                string chunk = ((SerialPort)sender).ReadExisting();
                if (IsDisposed || !IsHandleCreated) return;
                BeginInvoke((Action)(delegate { ReceiveChunk(chunk); }));
            } catch (Exception ex) {
                if (!IsDisposed && IsHandleCreated)
                    try { BeginInvoke((Action)(delegate { Disconnect("串口读取失败：" + ex.Message, true); })); }
                    catch (InvalidOperationException) { }
            }
        }

        private void SerialError(object sender, SerialErrorReceivedEventArgs e)
        {
            if (!IsDisposed && IsHandleCreated)
                try { BeginInvoke((Action)(delegate { Disconnect("串口通信错误", true); })); }
                catch (InvalidOperationException) { }
        }

        private void ReceiveChunk(string chunk)
        {
            if (serial == null) return;
            foreach (char ch in chunk) {
                if (ch == '\n') {
                    string line = input.ToString();
                    input.Clear();
                    StatusFrame frame;
                    if (StatusFrame.TryParse(line, out frame)) ApplyFrame(frame);
                } else if (ch != '\r') {
                    if (input.Length >= 100) input.Clear();
                    else input.Append(ch);
                }
            }
        }

        private void ApplyFrame(StatusFrame frame)
        {
            bool wasAlarm = previous != null && previous.Alarm;
            bool wasArmed = previous != null && previous.Armed;
            bool recovered = stale;
            lastFrame = DateTime.UtcNow;
            stale = false;
            if (frame.Alarm) {
                banner.Text = "报警中";
                banner.BackColor = Color.FromArgb(254, 226, 223);
                banner.ForeColor = red;
            } else {
                banner.Text = frame.Armed ? "已布防 · 正常" : "已撤防";
                banner.BackColor = frame.Armed ? Color.FromArgb(222, 242, 228) : Color.FromArgb(231, 236, 238);
                banner.ForeColor = frame.Armed ? green : ink;
            }
            connection.Text = serial.PortName + " 已连接 · 数据正常";
            mode.Text = "布防：" + (frame.Armed ? "已布防" : "已撤防");
            entry.Text = "红外：" + (frame.Entry ? "检测到" : "正常");
            temp.Text = "热敏：" + (frame.Hot ? "达到阈值" : "正常");
            received.Text = "最后接收：" + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + "    设备运行：" + (frame.UptimeMs / 1000u) + " 秒";
            if (recovered) AddEvent("通信恢复", Describe(frame));
            if (previous != null && wasArmed != frame.Armed) AddEvent(frame.Armed ? "已布防" : "已撤防", Describe(frame));
            if (frame.Alarm && !wasAlarm) {
                string cause = frame.Entry && frame.Hot ? "红外和热敏" : frame.Entry ? "红外" : "热敏";
                AddEvent("报警开始：" + cause, Describe(frame));
                tray.BalloonTipTitle = "别墅安防报警";
                tray.BalloonTipText = cause + "触发报警";
                tray.ShowBalloonTip(5000);
                if (sound.Checked) SystemSounds.Exclamation.Play();
            } else if (!frame.Alarm && wasAlarm) AddEvent("报警解除", Describe(frame));
            previous = frame;
        }

        private static string Describe(StatusFrame frame)
        {
            return (frame.Armed ? "布防" : "撤防") + " / 红外" + (frame.Entry ? "触发" : "正常") + " / 热敏" + (frame.Hot ? "触发" : "正常");
        }

        private void TimerTick(object sender, EventArgs e)
        {
            if (serial == null) { RefreshPorts(); return; }
            if (!serial.IsOpen) { Disconnect("串口已断开", true); return; }
            if (!stale && (DateTime.UtcNow - lastFrame).TotalSeconds > 3.0) {
                stale = true;
                ShowUnavailable("通信中断", "超过 3 秒未收到有效状态，请检查接线和供电");
                AddEvent("通信中断", serial.PortName);
                tray.BalloonTipTitle = "别墅安防通信中断";
                tray.BalloonTipText = "无法确认设备当前状态";
                tray.ShowBalloonTip(5000);
                if (sound.Checked) SystemSounds.Hand.Play();
            }
        }

        private void ShowUnavailable(string title, string detail)
        {
            banner.Text = title;
            banner.BackColor = Color.FromArgb(255, 239, 214);
            banner.ForeColor = Color.FromArgb(135, 80, 20);
            connection.Text = detail;
            mode.Text = "布防：--";
            entry.Text = "红外：--";
            temp.Text = "热敏：--";
            received.Text = "最后接收：--";
        }

        private void Disconnect(string reason, bool error)
        {
            SerialPort old = serial;
            serial = null;
            if (old != null) {
                old.DataReceived -= SerialReceived;
                old.ErrorReceived -= SerialError;
                try { old.Close(); } catch (IOException) { } catch (InvalidOperationException) { }
                old.Dispose();
            }
            previous = null;
            ports.Enabled = true;
            connect.Text = "连接";
            ShowUnavailable(error ? "连接断开" : "等待连接", reason);
            AddEvent(error ? "连接异常" : "串口已断开", reason);
            RefreshPorts();
        }

        private void AddEvent(string name, string detail)
        {
            string[] row = { DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"), name, detail };
            records.Add(row);
            ListViewItem item = new ListViewItem(row);
            events.Items.Insert(0, item);
            if (events.Items.Count > 500) events.Items.RemoveAt(events.Items.Count - 1);
        }

        private void ExportClick(object sender, EventArgs e)
        {
            if (records.Count == 0) { MessageBox.Show("当前没有可导出的事件。", "导出 CSV"); return; }
            using (SaveFileDialog dialog = new SaveFileDialog()) {
                dialog.Filter = "CSV 文件 (*.csv)|*.csv";
                dialog.FileName = "安防事件_" + DateTime.Now.ToString("yyyyMMdd_HHmmss") + ".csv";
                if (dialog.ShowDialog(this) != DialogResult.OK) return;
                using (StreamWriter writer = new StreamWriter(dialog.FileName, false, new UTF8Encoding(true))) {
                    writer.WriteLine("电脑时间,事件,状态");
                    foreach (string[] row in records)
                        writer.WriteLine(string.Join(",", row.Select(CsvField).ToArray()));
                }
            }
        }

        private static string CsvField(string value)
        {
            return "\"" + value.Replace("\"", "\"\"") + "\"";
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            timer.Stop();
            if (serial != null) Disconnect("程序关闭", false);
            tray.Visible = false;
            tray.Dispose();
            base.OnFormClosed(e);
        }
    }
}

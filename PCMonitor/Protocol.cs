using System;
using System.Globalization;

namespace VillaSecurityMonitor
{
    public sealed class StatusFrame
    {
        public uint UptimeMs;
        public bool Armed;
        public bool Entry;
        public bool Hot;
        public bool Alarm;

        public static bool TryParse(string line, out StatusFrame frame)
        {
            frame = null;
            if (line == null) return false;
            string[] parts = line.Trim().Split(',');
            if (parts.Length != 6 || parts[0] != "VS1") return false;
            uint uptime;
            if (!uint.TryParse(parts[1], NumberStyles.None, CultureInfo.InvariantCulture, out uptime)) return false;
            for (int i = 2; i < 6; i++)
                if (parts[i] != "0" && parts[i] != "1") return false;
            frame = new StatusFrame {
                UptimeMs = uptime,
                Armed = parts[2] == "1",
                Entry = parts[3] == "1",
                Hot = parts[4] == "1",
                Alarm = parts[5] == "1"
            };
            // 报警字段必须与固件的布防和传感器状态一致。
            return frame.Alarm == (frame.Armed && (frame.Entry || frame.Hot));
        }
    }
}

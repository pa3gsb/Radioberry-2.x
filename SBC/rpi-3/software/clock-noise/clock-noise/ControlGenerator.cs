using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.IO.Ports;

namespace ClockNoise
{
    class ControlGenerator
    {
        SerialPort comPort;

        //Constructor
        public  ControlGenerator()
        {
            comPort = new SerialPort();

            comPort.PortName = "COM1";
            comPort.BaudRate = Convert.ToInt32("57600");
            comPort.DataBits = Convert.ToInt16("8");
            comPort.StopBits = (StopBits)Enum.Parse(typeof(StopBits), "1");
            comPort.Parity = (Parity)Enum.Parse(typeof(Parity), "None");
            comPort.Open();
        }

        ~ControlGenerator()
        {
            comPort.Close();
        }

        public void stop()
        {
            comPort.Close();
        }

        public void setFrequency(int frequency)
        {
            if (comPort.IsOpen){
                String freq9 = String.Format("{0:000000000}", frequency);
         
                byte[] value = new byte[11];
                value[0] = (byte)0x8F;      // commmand init
                value[1] = (byte)0x66;      // f

                for (int i = 0; i < 9; i++)
                {
                    value[i + 2] = (byte)Int32.Parse(freq9.Substring(i, 1));
                }

                comPort.Write(value, 0, 11);
                System.Threading.Thread.Sleep(2000);
            }
        }
    }
}

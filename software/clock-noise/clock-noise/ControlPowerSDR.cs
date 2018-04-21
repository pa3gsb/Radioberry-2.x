using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.IO.Ports;

namespace ClockNoise
{
    class ControlPowerSDR
    {

        SerialPort comPort;

        SDRCATCommand sdrCATCommand = new SDRCATCommand();

        //Constructor
        public  ControlPowerSDR()
        {
            comPort = new SerialPort();

            comPort.PortName = "COM2";
            comPort.BaudRate = Convert.ToInt32("4800");
            comPort.DataBits = Convert.ToInt16("8");
            comPort.StopBits = (StopBits)Enum.Parse(typeof(StopBits), "1");
            comPort.Parity = (Parity)Enum.Parse(typeof(Parity), "None");
            comPort.Open();
        }

        ~ControlPowerSDR()
        {
            comPort.Close();
        }

        public void stop()
        {
            comPort.Close();
        }

        public void setCommand(string command)
        {
            if (comPort.IsOpen)
            {
                comPort.Write(command);
                System.Threading.Thread.Sleep(100);

                String result  = comPort.ReadExisting();
            }

        }

        public String getCommand(string command)
        {
            String result = "";
            if (comPort.IsOpen)
            {
                comPort.Write(command);
                System.Threading.Thread.Sleep(100);

                result = comPort.ReadExisting();
            }

            return result;

        }

 

    }
}

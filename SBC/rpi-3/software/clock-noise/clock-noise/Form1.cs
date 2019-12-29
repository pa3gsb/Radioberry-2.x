using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;

namespace ClockNoise
{
    public partial class Form1 : Form
    {

        private Thread thread;
        SerialPort comPort = new SerialPort();

        private Process objProcess;
        private int stepf;

        

        public Form1()
        {
            InitializeComponent();
            comboBox1.SelectedIndex = 4;    // Default
        }

        //Start without signal generator
        private void button2_Click(object sender, EventArgs e)
        {
            button2.Enabled = false;
            if (objProcess == null)
            {
                objProcess = new Process(Convert.ToInt32(textBox1.Text), Convert.ToInt32(textBox2.Text), stepf);

                thread = new Thread(new ThreadStart(startNFMeasurement));
                thread.Name = "NF Measurement Loop";
                thread.Priority = ThreadPriority.Highest;
                thread.Start();
            }
        }

        //Start with signal generator
        private void button3_Click(object sender, EventArgs e)
        {
            if (objProcess != null)
            {
                button2.Enabled = false;
                objProcess.startWithSignal();
            }

        }

        private void stopProcess()
        {
            objProcess.stop();
            thread.Abort();
            if (thread != null) thread = null;
            objProcess = null;

            button2.Enabled = true;
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            stepf = Convert.ToInt32((string) comboBox1.SelectedItem);
        }

        private void startNFMeasurement()
        {
            objProcess.ProgressStatus += new Process.ProgressEventHandler(UpdateFormWithProgressData);

            objProcess.startWithoutSignal();
        }

        delegate void SetValuesCallBack(ProgressEventArgs e);

        void UpdateFormWithProgressData(ProgressEventArgs e)
        {
            SetValuesCallBack values = new SetValuesCallBack(ShowProgress);
            this.Invoke(values, new object[] { e });
        }

        private void ShowProgress(ProgressEventArgs e)
        {
            if ("nextStep".Equals(e.Message))
            {
                label9.Text = "Ready measuring with signal";
                progressBar.Value = 0;
                return;
            }

            if (objProcess.getIsStopped())
            {
                label9.Text = "Result of measurement can be found in c:\\temp\\measurement.txt";
                stopProcess();
                progressBar.Value = 0;
                return;
            }
            progressBar.Value = (progressBar.Value + 1) % 100;

            if ( e.Message.Length > 0 ) label9.Text = e.Message;
        }

       
    }
}

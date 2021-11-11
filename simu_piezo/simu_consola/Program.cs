using System;
using System.Globalization;
using System.IO;
using System.IO.Ports;
using System.Threading;

namespace simula_piezo
{
    class MainClass
    {
        public static void Main(string[] args)
        {

            string[] lines;

            Console.WriteLine("Simulador de la señal del piezo");
            try
            {
                lines = File.ReadAllLines(args[0]);
            }
            catch
            {
                Console.Write("Debe indicar un archivo .CSV"); 
                return;
            }

            int sampling_size =int.Parse(lines[0].Split(',')[1]);
            short[] data = new short[sampling_size];

            float T_sampling = float.Parse(lines[10].Split(',')[1], CultureInfo.InvariantCulture.NumberFormat);

            const int START_DATA = 13;
            for (int i = 0; i < sampling_size; i++)
                data[i] = short.Parse(lines[START_DATA + i].Split(',')[0]);

            byte[] dato = new byte[2];
            SerialPort port;
            try
            {
                port = new SerialPort("/dev/ttyUSB0", 115200);
                port.Open();
                port.DiscardInBuffer();

            }
            catch 
            {
                Console.WriteLine("No se puede conectar con el Arduino\n");
                return;
            }

            Console.WriteLine("Enviando datos");

            for (int i = 0; i < data.Length; i++)
            {
                dato[0] = BitConverter.GetBytes(data[i])[0];
                dato[1] = BitConverter.GetBytes(data[i])[1];

                port.Write(dato, 0, 2);
                if (port.BytesToRead > 0)
                    Console.WriteLine(port.ReadLine());
                Thread.Sleep(1);

            }

            Console.WriteLine("Fin de la Transmision");

            port.Close();
        }
    }
}

﻿using System;
using Gtk;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Globalization;
using OxyPlot;
using OxyPlot.Series;
using OxyPlot.GtkSharp;

public partial class MainWindow : Gtk.Window
{

    private PlotView plotView;
    private SerialPort port;
    private List<short> datos_signal = new List<short>();

    public MainWindow() : base(Gtk.WindowType.Toplevel)
    {
        Build();

        // Agrego el widget por codigo por bugs en el paquete de oxyplot
        plotView = new PlotView();
        plotView.SetSizeRequest(fixed1.Allocation.Width, fixed1.Allocation.Height);
        fixed1.Add(plotView);
        plotView.ShowAll();

        var plotModel = new PlotModel();

        var datos_plot = new LineSeries();
        datos_plot.Points.Add(new DataPoint(0, 0));
        plotModel.Series.Add(datos_plot);
        plotView.Model = plotModel;

        //Busca si esta el arduino esta conectado
        var port_names = Directory.GetFiles("/dev/", "ttyUSB*");
        foreach(string port_name in port_names)
            combobox1.AppendText(port_name);

        port_names = Directory.GetFiles("/dev/", "ttyACM*");
        foreach (string port_name in port_names)
            combobox1.AppendText(port_name);

    }

    protected void OnDeleteEvent(object sender, DeleteEventArgs a)
    {
        Application.Quit();
        a.RetVal = true;
        if (port.IsOpen)
            port.Close();
    }

    protected void OnOpenSignal(object sender, EventArgs e)
    {
        var OpenFileDialog = new FileChooserDialog("Seleccione un archivo CSV", this, 
                                         FileChooserAction.Open, "Cancel", 
                                         ResponseType.Cancel, "Ok", 
                                         ResponseType.Ok);

        int resp = OpenFileDialog.Run();

        if (resp == (int)ResponseType.Ok)
        {

            var lines = File.ReadAllLines(OpenFileDialog.Filename);

            int sampling_size = int.Parse(lines[0].Split(',')[1]);

            float T_sampling = float.Parse(lines[10].Split(',')[1], CultureInfo.InvariantCulture.NumberFormat);
//            Console.WriteLine("Muestreo a: " + T_sampling.ToString());

            label1.Text = string.Format("Multiplicador: {0}", (int)(1e-3 / T_sampling));

            const int START_DATA = 13;
            var plotModel = new PlotModel();
            var datos_plot = new LineSeries();

            datos_signal.Clear();
            for (int i = 0; i < sampling_size; i++)
            {
                //Cargo los datos del archivo CSV 
                datos_signal.Add(short.Parse(lines[START_DATA + i].Split(',')[0]));
                datos_plot.Points.Add(new DataPoint(i*T_sampling, datos_signal[i]));
            }

            plotModel.Series.Add(datos_plot);
            plotView.Model = plotModel;

        }

        OpenFileDialog.Destroy();

    }

    protected void OnPortChange(object sender, EventArgs e)
    {
        conectar.Sensitive = true;
        port = new SerialPort(combobox1.ActiveText, 115200);
    }

    protected void OnConectar(object sender, EventArgs e)
    {
        if (!port.IsOpen)
        {
            port.Open();
            enviar.Sensitive = true;
        }
    }

    protected void OnEnviar(object sender, EventArgs e)
    {
        port.DiscardInBuffer();

        for (int i = 0; i < datos_signal.Count; i++)
        {
            var dato = BitConverter.GetBytes(datos_signal[i]);
            port.Write(dato, 0, 2);
            if (port.BytesToRead > 0)
                Console.WriteLine(i.ToString() + " : " + port.ReadLine());

            Thread.Sleep(1);
        }

    }

    protected void OnBuscarPuertos(object o, EventArgs args)
    {

        combobox1.RemoveText(0);

        conectar.Sensitive = false;
        enviar.Sensitive = false;
        var port_names = Directory.GetFiles("/dev/", "ttyUSB*");
        foreach (string port_name in port_names)
            combobox1.AppendText(port_name);

        port_names = Directory.GetFiles("/dev/", "ttyACM*");
        foreach (string port_name in port_names)
            combobox1.AppendText(port_name);

    }
}

using System;
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
    private List<short> datos = new List<short>();

    public MainWindow() : base(Gtk.WindowType.Toplevel)
    {
        Build();


        plotView = new PlotView();
        plotView.SetSizeRequest(fixed1.Allocation.Width, fixed1.Allocation.Height);
        fixed1.Add(plotView);
        plotView.ShowAll();

        var plotModel = new PlotModel();

        var puntos = new LineSeries();
        puntos.Points.Add(new DataPoint(0, 0));
        plotModel.Series.Add(puntos);
        plotView.Model = plotModel;

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

            const int START_DATA = 13;
            var plotModel = new PlotModel();
            var puntos = new LineSeries();

            datos.Clear();
            for (int i = 0; i < sampling_size; i++)
            {
                datos.Add(short.Parse(lines[START_DATA + i].Split(',')[0]));
                puntos.Points.Add(new DataPoint(i*T_sampling, datos[i]));
            }

            plotModel.Series.Add(puntos);
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
        for (int i = 0; i < datos.Count; i++)
        {
            var dato = BitConverter.GetBytes(datos[i]);
            port.Write(dato, 0, 2);
            Thread.Sleep(1);
        }
    }
}

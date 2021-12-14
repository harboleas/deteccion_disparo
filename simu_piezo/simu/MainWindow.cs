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
    private List<float> datos_signal = new List<float>();
    private float escala_tension;
    private float T_sampling;
    private bool file_abierto = false;
    private bool puerto_abierto = false;
    private short multiplicador;

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

            T_sampling = float.Parse(lines[10].Split(',')[1], CultureInfo.InvariantCulture.NumberFormat);
            //            Console.WriteLine("Muestreo a: " + T_sampling.ToString());
            escala_tension = float.Parse(lines[4].Split(',')[1], CultureInfo.InvariantCulture.NumberFormat);

            multiplicador = (short) (1e-3 / T_sampling);
            label1.Text = string.Format("Multiplicador: {0}", multiplicador);

            const int START_DATA = 13;
            var plotModel = new PlotModel(); 
            var datos_plot = new LineSeries();

            datos_signal.Clear();
            for (int i = 0; i < sampling_size; i++)
            {
                // Cargo los datos del archivo CSV 
                var val = short.Parse(lines[START_DATA + i].Split(',')[0]);
                // El osciloscopio usado para las mediciones es de 8 bits,
                // para obtener el valor real de tension el rango maximo son 10 
                // divisiones (cuadraditos del osci)
                var val_real = (escala_tension * 10 / 256) * val;
                datos_signal.Add(val_real);
                datos_plot.Points.Add(new DataPoint(i*T_sampling, datos_signal[i]));
            }

            plotModel.Series.Add(datos_plot);
            plotView.Model = plotModel;

            file_abierto = true;
            if (puerto_abierto)
                setMult.Sensitive = true;
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
            // Reseteo el arduino //
//            port.DtrEnable = false;
//            Thread.Sleep(1);
//            port.DiscardInBuffer();
//            port.DtrEnable = true;
            ////////////////////////
            puerto_abierto = true;
            if (file_abierto)
                setMult.Sensitive = true;
        }
    }

    protected void OnEnviar(object sender, EventArgs e)
    {
        port.DiscardInBuffer();

        var plotModel = plotView.Model;
        var datos_plot2 = new LineSeries();
        datos_plot2.Points.Add(new DataPoint(0, 0));

        for (int i = 0; i < datos_signal.Count; i++)
        {
            // simulo la conversion de 10 bits del arduino para enviar los datos
            short val = (short)(datos_signal[i] * 1024 / 5);
            ////////////////////////////////////////////////////////////////////

            var dato = BitConverter.GetBytes(val);
            port.Write(dato, 0, 2);

            // Para visualizar cuando se producen los cambios de estado en el Arduino 
            if (port.BytesToRead > 0)
            {
                datos_plot2.Points.Add(new DataPoint((i - 1 - 3) * T_sampling, 0));
                datos_plot2.Points.Add(new DataPoint((i - 3) * T_sampling, 5));
                Console.WriteLine(((i-3)* T_sampling).ToString() + " : Estado " + port.ReadLine());
                datos_plot2.Points.Add(new DataPoint((i + 1 - 3) * T_sampling, 0));

            }
            //////////////////////////////////////////////////////////////////////////
             
            Thread.Sleep(1);
        }


        plotModel.Series.Add(datos_plot2);
        plotModel.InvalidatePlot(true);


    }

    protected void OnBuscarPuertos(object o, EventArgs args)
    {

        combobox1.RemoveText(0);
        puerto_abierto = false;
        setMult.Sensitive = false;
        conectar.Sensitive = false;
        enviar.Sensitive = false;
        var port_names = Directory.GetFiles("/dev/", "ttyUSB*");
        foreach (string port_name in port_names)
            combobox1.AppendText(port_name);

        port_names = Directory.GetFiles("/dev/", "ttyACM*");
        foreach (string port_name in port_names)
            combobox1.AppendText(port_name);

    }

    protected void OnConfigMult(object sender, EventArgs e)
    {
        // Reseteo el arduino //
        port.DtrEnable = false;
        Thread.Sleep(1);
        port.DiscardInBuffer();
        port.DtrEnable = true;
        ////////////////////////

        Thread.Sleep(1500); // Espera hasta que el arduino resetee

        port.Write(BitConverter.GetBytes(multiplicador), 0, 2);
        enviar.Sensitive = true;
        Console.WriteLine("Mult seteado en: " + port.ReadLine());
    }
}

using System;
using Gtk;
using System.IO;
using System.Threading;
using System.Globalization;
using OxyPlot;
using OxyPlot.Series;
using OxyPlot.GtkSharp;

public partial class MainWindow : Gtk.Window
{

    private PlotView plotView;

    public MainWindow() : base(Gtk.WindowType.Toplevel)
    {
        Build();


        plotView = new PlotView();
        plotView.SetSizeRequest(fixed1.Allocation.Width, fixed1.Allocation.Height);
        fixed1.Add(plotView);
        plotView.ShowAll();

        var plotModel = new PlotModel();

        var datos = new LineSeries();
        datos.Points.Add(new DataPoint(0, 0));
        plotModel.Series.Add(datos);
        plotView.Model = plotModel;

    }

    protected void OnDeleteEvent(object sender, DeleteEventArgs a)
    {
        Application.Quit();
        a.RetVal = true;
    }

    protected void OnOpenSignal(object sender, EventArgs e)
    {
        var OpenFileDialog = new FileChooserDialog("Open CSV", this, 
                                         FileChooserAction.Open, "Cancel", 
                                         ResponseType.Cancel, "Ok", 
                                         ResponseType.Ok);

        int resp = OpenFileDialog.Run();

        if (resp == (int)ResponseType.Ok)
        {

            var lines = File.ReadAllLines(OpenFileDialog.Filename);

            int sampling_size = int.Parse(lines[0].Split(',')[1]);
            short[] data = new short[sampling_size];

            float T_sampling = float.Parse(lines[10].Split(',')[1], CultureInfo.InvariantCulture.NumberFormat);

            const int START_DATA = 13;
            var plotModel = new PlotModel();
            var datos = new LineSeries();

            for (int i = 0; i < sampling_size; i++)
            {
                data[i] = short.Parse(lines[START_DATA + i].Split(',')[0]);
                datos.Points.Add(new DataPoint(i*T_sampling, data[i]));
            }

            plotModel.Series.Add(datos);
            plotView.Model = plotModel;

        }

        OpenFileDialog.Destroy();

    }
}

package com.example.neonintrinsicssamples;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;
import android.widget.TextView;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.LegendRenderer;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Get references to controls
        textViewProcessingTime = findViewById(R.id.textViewProcessingTime);
        checkBoxUseNeon = findViewById(R.id.checkboxUseNeon);

        // Configure graph
        configureGraph();
    }

    /**
     Native methods from the native-lib.cpp
     */
    public native byte[] generateSignal();
    public native byte[] truncate(boolean useNeon);
    public native byte[] convolution(boolean useNeon);

    private native double getProcessingTime();
    private native int getSignalLength();

    // Fields
    GraphView graph;
    TextView textViewProcessingTime;
    CheckBox checkBoxUseNeon;

    // Series
    private LineGraphSeries<DataPoint> signalSeries = new LineGraphSeries<>();
    private LineGraphSeries<DataPoint> signalTruncateSeries = new LineGraphSeries<>();
    private LineGraphSeries<DataPoint> signalAfterConvolutionSeries = new LineGraphSeries<>();

    // Event handlers
    public void buttonGenerateSignalClicked(View v) {
        signalSeries.resetData(getDataPoints(generateSignal()));
    }

    public void buttonTruncateClicked(View v) {
        signalTruncateSeries.resetData(getDataPoints(truncate(useNeon())));

        displayProcessingTime();
    }

    public void buttonConvolutionClicked(View v) {
        signalAfterConvolutionSeries.resetData(getDataPoints(convolution(useNeon())));

        displayProcessingTime();
    }

    // Helpers
    private DataPoint[] getDataPoints(byte[] array) {
        int count = array.length;
        DataPoint[] dataPoints = new DataPoint[count];

        for(int i = 0; i < count; i++) {
            dataPoints[i] = new DataPoint(i, array[i]);
        }

        return dataPoints;
    }

    private void configureGraph(){
        // Initialize graph
        graph = (GraphView) findViewById(R.id.graph);

        // Set bounds
        graph.getViewport().setXAxisBoundsManual(true);
        graph.getViewport().setMaxX(getSignalLength());

        graph.getViewport().setYAxisBoundsManual(true);
        graph.getViewport().setMinY(-150);
        graph.getViewport().setMaxY(150);

        // Configure series
        int thickness = 4;
        signalSeries.setTitle("Signal");
        signalSeries.setThickness(thickness);
        signalSeries.setColor(Color.BLUE);

        signalTruncateSeries.setTitle("Truncate");
        signalTruncateSeries.setThickness(thickness);
        signalTruncateSeries.setColor(Color.GREEN);

        signalAfterConvolutionSeries.setTitle("Convolution");
        signalAfterConvolutionSeries.setThickness(thickness);
        signalAfterConvolutionSeries.setColor(Color.RED);

        // Add series
        graph.addSeries(signalSeries);
        graph.addSeries(signalTruncateSeries);
        graph.addSeries(signalAfterConvolutionSeries);

        // Add legend
        graph.getLegendRenderer().setVisible(true);
        graph.getLegendRenderer().setAlign(LegendRenderer.LegendAlign.TOP);
    }

    private boolean useNeon() {
        return checkBoxUseNeon.isChecked();
    }

    private void displayProcessingTime() {
        String strProcessingTime = String.format("%s = %.2f us", "Processing time", getProcessingTime());

        textViewProcessingTime.setText(strProcessingTime );
    }
}

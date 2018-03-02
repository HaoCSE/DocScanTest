package ch.abacus.test.docscantest;

import android.app.NativeActivity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Core;
import org.opencv.core.CvException;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfFloat;
import org.opencv.core.Point;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    static {
        try {
            System.loadLibrary("DocScanner");
            System.loadLibrary("scanner");
            System.loadLibrary("opencv_java3");

            if (OpenCVLoader.initDebug()) {
                DocScannerHelper.logMessage("OpenCV Loaded");
            } else {
                DocScannerHelper.logMessage("OpenCV Not Loaded");
            }
        } catch (Exception e) {
            DocScannerHelper.logMessage(e.getMessage());
        }
    }

    public static final String TAG = "MainActivity";
    private ImageView testImage;
    private DocScannerHelper h;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        testImage = (ImageView) findViewById(R.id.testImage);
        testImage.setOnClickListener(this);
        h = new DocScannerHelper(this);

        processEdges();

    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.testImage) {

        }
    }

    private void processEdges () {
        Mat edgeImage = h.bitmapToMat(BitmapFactory.decodeResource(getResources(), R.drawable.foobar2));
        String path = h.getModelYml();
        if (path != null) {
            Mat responseImg = new Mat();
            returnEdges(path, edgeImage.getNativeObjAddr(), responseImg.getNativeObjAddr());
            Bitmap result = h.matToBitmap(responseImg);
            if (result != null) {
                testImage.setImageBitmap(result);
            } else {
                Toast.makeText(this, "Error!",Toast.LENGTH_LONG).show();
            }
        } else {
            Toast.makeText(this, "Error!",Toast.LENGTH_LONG).show();
        }
    }

    public native void returnEdges(String path, long rawImage, long responseImage);
    public native String test(String msg);
}

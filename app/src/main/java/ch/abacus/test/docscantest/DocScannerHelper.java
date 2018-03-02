package ch.abacus.test.docscantest;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;

import org.opencv.android.Utils;
import org.opencv.core.Core;
import org.opencv.core.CvException;
import org.opencv.core.Mat;
import org.opencv.core.Point;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by benedetti on 3/2/2018.
 */

public class DocScannerHelper {

    private Context context;

    public DocScannerHelper(Context context) {
        this.context = context;
    }

    public Mat bitmapToMat(Bitmap bmp) {
        Mat mat = new Mat();

        Bitmap bmp32 = bmp.copy(Bitmap.Config.ARGB_8888, true);
        Utils.bitmapToMat(bmp32, mat);

        return mat;
    }

    public Bitmap matToBitmap(Mat src) {
//        Mat src_gray        = new Mat();
//        Mat dst             = new Mat();
//        Mat dst_norm        = new Mat();
//        Mat dst_norm_scaled = new Mat();
//
//        // Detector parameters
//        int blockSize = 2;
//        int apertureSize = 3;
//        double k = 0.04;
//        // Filter params
//        int thresh = 200;
//        int max_thresh = 255;
//
//        // Detecting corner
//        logMessage("w:" + src.width() + ", h:" + src.height());
//        Imgproc.cvtColor(src, src_gray, Imgproc.COLOR_GRAY2RGBA);
//        Imgproc.cornerHarris(src_gray, dst, blockSize, apertureSize, k);
//
//        // Normalizing
//        Core.normalize(dst, dst_norm, 0, 255, Core.NORM_MINMAX);
//        Core.convertScaleAbs(dst_norm, dst_norm_scaled);
//
//        // Drawing a circle around corners
//        for (int j = 0; j < dst_norm.rows(); j++) {
//            for (int i = 0; i < dst_norm.cols(); i++) {
//                if (dst_norm.get(j, i)[0] > thresh) {
//                    Imgproc.circle(dst_norm_scaled, new Point(i, j), 5, new Scalar(255));
//                }
//            }
//        }
//
//        // Create bitmap
//        final Bitmap bitmap = Bitmap.createBitmap(dst_norm_scaled.cols(), dst_norm_scaled.rows(), Bitmap.Config.ARGB_8888);
//        Utils.matToBitmap(dst_norm_scaled, bitmap);

        logMessage("Type: " + src.type());
        final Bitmap bitmap = Bitmap.createBitmap(src.cols(), src.rows(), Bitmap.Config.ARGB_8888);
        Utils.matToBitmap(src, bitmap);
        return bitmap;
    }

    public String getModelYml() {
        File modelYml = new File(context.getFilesDir() + "/doc_scanner/model.yml");
        if (modelYml.exists()) {
            return modelYml.getPath();
        } else {
            AssetManager am = context.getAssets();
            try {
                InputStream in = am.open("doc_scanner/model.yml");
                modelYml.getParentFile().mkdirs();
                modelYml.createNewFile();
                OutputStream out = new FileOutputStream(modelYml.getPath());
                byte[] buffer = new byte[1024];
                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }
                in.close();
                out.flush();
                out.close();
                return modelYml.getPath();
            } catch (IOException e) {
                return null;
            }
        }
    }

    public static void logMessage(String msg) {
        System.out.println("[DocScan]" + msg);
    }

}

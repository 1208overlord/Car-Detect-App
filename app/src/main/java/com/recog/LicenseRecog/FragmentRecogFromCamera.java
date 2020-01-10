package com.recog.LicenseRecog;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.recog.CameraViewFragment;
import com.recog.DarknetDao;
import com.recog.LicenseRecog.RecogFromCamera;

import com.recog.MainActivity;
import com.recog.OpenCvCameraView;
import com.recog.R;
import com.recog.Result;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.imgproc.Imgproc;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class FragmentRecogFromCamera extends CameraViewFragment {

    private String IMAGE_PATH = "";
    private AsyncCaller asyncCaller;
    public Mat crop_im = new Mat();
    private Mat proc_frame = new Mat();
    public static boolean is_running=false;
    private DarknetDao darknet;

    public TextView mResultTextView;

    public FragmentRecogFromCamera() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View rootView = inflater.inflate(R.layout.fragment_recog_from_camera, container, false);

        mOpenCvCameraView = (OpenCvCameraView) rootView.findViewById(R.id.CameraView);
        mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
        mOpenCvCameraView.setCvCameraViewListener(this);

        initCameraProperty();
        frag_type = 3;

        darknet = new DarknetDao();
        mResultTextView = rootView.findViewById(R.id.RecogTextView);//new TextView(getActivity());
        //WaitLiveFace();
        return rootView;
    }

    public Mat onCameraFrame(OpenCvCameraView.CvCameraViewFrame inputFrame) {
        Mat frame = super.onCameraFrame(inputFrame);
        if (frame.rows() > frame.cols())
        {
            crop_im = new Mat(frame, new Rect(0, frame.rows() / 2 - frame.cols() / 2, frame.cols(), frame.cols()));
        }
        else if (frame.cols() > frame.rows())
        {
            crop_im = new Mat (frame, new Rect(frame.cols() / 2 - frame.rows() / 2, 0, frame.rows(), frame.rows()));
        }
        else
        {
            crop_im = new Mat(new Size(frame.rows(), frame.cols()), CvType.CV_8UC3);
            frame.copyTo(crop_im);
        }
        if(asyncCaller==null){
            crop_im.copyTo(proc_frame);
            asyncCaller= new AsyncCaller();
            asyncCaller.execute();

        }
        if(asyncCaller!=null) {
            if (asyncCaller.getStatus() == AsyncTask.Status.FINISHED) {
                crop_im.copyTo(proc_frame);
                asyncCaller = new AsyncCaller();
                asyncCaller.execute();
            }
        }
        return frame;
    }

    private class AsyncCaller extends AsyncTask<Void, Void, Void>
    {
        String result_str="";
        private TextView textView;
        @Override
        protected void onPreExecute() {
            super.onPreExecute();

            //this method will be running on UI thread

        }
        @Override
        protected Void doInBackground(Void... params) {
            //this method will be running on background thread so don't update UI frome here
            //do your long running http tasks here,you dont want to pass argument and u can access the parent class' variable url over here
            try {
                Result[] Plate_Results = darknet.detectLicensePlate(crop_im.nativeObj);
                int angle = 0;
                int plate_type = 0;
                int count = 0;
                for (Result r : Plate_Results) {
                    //Mat plate_image = new Mat(resizeMat, new Rect(r.getLeft(), r.getTop(), r.getWidth(), r.getHeight()));
                    Mat plate_image = new Mat(crop_im, new Rect((int)(r.getLeft()), (int)(r.getTop()), (int)((r.getRight() - r.getLeft())), (int)((r.getBot() - r.getTop()))));
                    Imgproc.resize(plate_image, plate_image, new Size(plate_image.cols() / 2, plate_image.rows() / 2));
                    Mat rotImage = new Mat();
                    try {
                        angle = darknet.Rotate(plate_image.nativeObj, rotImage.nativeObj);
                        Log.e("CarLicenseRecog", "JNI : rotangle " + angle);
                    }catch (Exception e){
                        Log.e("CarLicenseRecog", e.getMessage());
                    }
                    if (r.getLabel().equals("CAR")) {
                        Imgproc.resize(rotImage, rotImage, new Size(160, 40));
                        Log.e("CarLicenseRecog", "JNI : rows " + rotImage.rows() + "cols" + rotImage.cols());
                        plate_type = 1;
                    }
                    else if (r.getLabel().equals("MOTORBIKE"))
                    {
                        Imgproc.resize(rotImage, rotImage, new Size(80, 80));
                        Log.e("CarLicenseRecog", "JNI : rows " + rotImage.rows() + "cols" + rotImage.cols());
                        plate_type = 2;
                    }
                    try {
                        Result[] Recog_Results = darknet.Recognition(rotImage.nativeObj, 0.2f, plate_type);
                        for (Result rr : Recog_Results) {
                            Imgproc.rectangle(rotImage, new Point(rr.getLeft(), rr.getTop()), new Point(rr.getRight(), rr.getBot())
                                    , new Scalar(0, 255, 0));
                            result_str += rr.getLabel();
                            count++;
                        }
                    }catch (Exception e) {
                        Log.e("CarLicenseRecog", e.getMessage());
                    }
                }
                Log.e("CarLicenseRecog" , "JNI count is " + count);
            }catch (Exception e) {
                Log.e("CarLicenseRecog", e.getMessage());
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);
            if (result_str != "")
            {
                mResultTextView.setText(result_str);
                //mMiddleTextview.setText("");
                //is_running = true;
            }
        }
    }
}

package com.recog.LicenseRecog;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.content.Intent;
import android.provider.MediaStore;
import android.util.Log;
import android.widget.*;
import android.view.View;

import com.recog.DarknetDao;
import com.recog.R;
import com.recog.Result;

import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Rect;
import org.opencv.core.Point;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.imgcodecs.Imgcodecs;
import org.opencv.android.Utils;
import org.opencv.imgproc.Imgproc;


public class RecogFromFile extends AppCompatActivity {

    private static final int IMAGE = 1;
    private String m_imagePath;
    private Mat m_inputImage = new Mat();
    private Button m_RecognitionStartBtn;
    private ProgressDialog pDialog;
    private TextView licenseView;
    private DarknetDao darknet;

    static {
        try {
            System.loadLibrary("opencv_java3");
            Log.i("CarLicenseRecog", "Successfully load library");
        } catch (UnsatisfiedLinkError e) {
            Log.i("CarLicenseRecog", "Failed to load library error");

        }
    }

    protected void initpDialog() {

        pDialog = new ProgressDialog(this);
        pDialog.setCancelable(false);
    }

    protected void showpDialog() {

        if (!pDialog.isShowing()){
            pDialog.setMessage(getString(R.string.msg_loading));
            pDialog.show();
        }

    }

    protected void showpDialog(String msg) {

        if (!pDialog.isShowing()){
            pDialog.setMessage(msg);
            pDialog.show();
        }

    }

    protected void hidepDialog() {

        if (pDialog.isShowing()) pDialog.dismiss();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recog_from_file);

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setTitle("Recognition from File");

        initpDialog();

        licenseView = (TextView)findViewById(R.id.textView);
        Log.e("CarLicenseRecog", "File Dialog Open Prepare");
        Boolean isSDPresent = android.os.Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED);
        //Boolean isSDSupportedDevice = Environment.isExternalStorageRemovable();
        //if (isSDPresent) {
            Intent intent = new Intent(Intent.ACTION_PICK,
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
            Log.e("CarLicenseRecog", "Media store is " + MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
            startActivityForResult(intent, IMAGE);
        //}
        //else
        //{
        //    Intent intent = new Intent(Intent.ACTION_PICK,
        //            MediaStore.Images.Media.INTERNAL_CONTENT_URI);
        //    Log.e("CarLicenseRecog", "Media store is " + MediaStore.Images.Media.INTERNAL_CONTENT_URI);
        //    startActivityForResult(intent, IMAGE);
        //}*/
        /**
        Intent BrowseAct = new Intent(getBaseContext(),
                FileDialog.class);
        String line =  Environment.getExternalStorageDirectory()+ File.separator + "DCIM" + File.separator+ "Camera" + File.separator;
        int idx = line.lastIndexOf('/');
        String start_path = line.substring(0, idx);

        BrowseAct.putExtra(FileDialog.START_PATH, start_path);

        BrowseAct.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        startActivityForResult(BrowseAct, IMAGE);
        Log.e("CarLicenseRecog", "File Dialog Opened");**/
        m_RecognitionStartBtn = (Button) findViewById(R.id.LicenseRecognitionStart);
        m_RecognitionStartBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (m_inputImage.rows() == 0 || m_inputImage.cols() == 0) return;
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        new StartRecognitionTask(RecogFromFile.this).execute();
                    }
                }).run();
            }
        });

        darknet = new DarknetDao();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == IMAGE && resultCode == Activity.RESULT_OK && data != null) {
            //m_imagePath = data.getStringExtra(FileDialog.RESULT_PATH);

            Uri selectedImage = data.getData();
            String[] filePathColumns = {MediaStore.Images.Media.DATA};
            Cursor c = getContentResolver().query(selectedImage, filePathColumns, null, null, null);
            c.moveToFirst();
            int columnIndex = c.getColumnIndex(filePathColumns[0]);
            m_imagePath = c.getString(columnIndex);
            c.close();

            Log.e("CarLicenseRecog", "Image path is " + m_imagePath);
            m_inputImage = Imgcodecs.imread(m_imagePath);

            Mat rgbFrame = new Mat(new Size(m_inputImage.rows(), m_inputImage.cols()), CvType.CV_8UC3);
            m_inputImage.copyTo(rgbFrame);

            Imgproc.cvtColor(rgbFrame, rgbFrame, Imgproc.COLOR_BGR2RGBA);
            Bitmap resultBitmap = Bitmap.createBitmap(m_inputImage.cols(),  m_inputImage.rows(),Bitmap.Config.ARGB_8888);
            Utils.matToBitmap(rgbFrame, resultBitmap);
            Bitmap mResult = resultBitmap;
            ImageView imgView = (ImageView) findViewById(R.id.CarImage);
            imgView.setImageBitmap(mResult);

        }
    }

    @Override
    public void onBackPressed(){

        finish();
    }

    private class StartRecognitionTask extends AsyncTask<Void, Void, Void>
    {
        private String license;
        private Context mContext;
        private TextView textView;
        private Bitmap mResult;
        private ImageView imgView;

        public StartRecognitionTask(Context context){
            this.mContext=context;
        }
        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            showpDialog("Please wait recognizing license number.");
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            hidepDialog();

            licenseView.setText(license);
        }
        @Override
        protected Void doInBackground(Void... voids) {
            long time = System.currentTimeMillis();
            Handler handler = new Handler(Looper.getMainLooper()) {
                @Override
                public void handleMessage(Message msg) {
                    // Any UI task, example
                    textView = (TextView) (((Activity) mContext).findViewById(R.id.textView));
                    textView.setText("Plate Detecting...");
                }
            };
            Mat crop_im;
            if (m_inputImage.rows() == 0 || m_inputImage.cols() == 0) return null;
            if (m_inputImage.rows() > m_inputImage.cols())
            {
                crop_im = new Mat(m_inputImage, new Rect(0, m_inputImage.rows() / 2 - m_inputImage.cols() / 2, m_inputImage.cols(), m_inputImage.cols()));
            }
            else if (m_inputImage.cols() > m_inputImage.rows())
            {
                crop_im = new Mat (m_inputImage, new Rect(m_inputImage.cols() / 2 - m_inputImage.rows() / 2, 0, m_inputImage.rows(), m_inputImage.rows()));
            }
            else
            {
                crop_im = new Mat(new Size(m_inputImage.rows(), m_inputImage.cols()), CvType.CV_8UC3);
                m_inputImage.copyTo(crop_im);
            }

            handler.sendEmptyMessage(1);
            try {
                Result[] Plate_Results = darknet.detectLicensePlate(crop_im.nativeObj);
                Log.e("CarLicenseRecog", "JNI Plate Detect TIME:dealing " + (System.currentTimeMillis() - time));
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
                    Mat rgbFrame = new Mat(new Size(rotImage.rows(), rotImage.cols()), CvType.CV_8UC3);
                    rotImage.copyTo(rgbFrame);
                    Imgproc.cvtColor(rgbFrame, rgbFrame, Imgproc.COLOR_BGR2RGBA);
                    Bitmap resultBitmap = Bitmap.createBitmap(rotImage.cols(),  rotImage.rows(),Bitmap.Config.ARGB_8888);
                    Utils.matToBitmap(rgbFrame, resultBitmap);
                    mResult = resultBitmap;

                    handler = new Handler(Looper.getMainLooper()) {
                        @Override
                        public void handleMessage(Message msg) {
                            // Any UI task, example
                            imgView = (ImageView) (((Activity) mContext).findViewById(R.id.PlateImage));
                            imgView.setImageBitmap(mResult);
                            textView.setText("Plate Recognizing...");
                        }
                    };
                    handler.sendEmptyMessage(1);
                    try {
                        Result[] Recog_Results = darknet.Recognition(rotImage.nativeObj, 0.2f, plate_type);
                        Log.e("CarLicenseRecog", "JNI Plate Recog TIME:dealing " + (System.currentTimeMillis() - time));
                        license = "License is ";
                        for (Result rr : Recog_Results) {
                            Imgproc.rectangle(rotImage, new Point(rr.getLeft(), rr.getTop()), new Point(rr.getRight(), rr.getBot())
                                    , new Scalar(0, 255, 0));
                            license += rr.getLabel();

                            count++;
                        }
                        rgbFrame = new Mat(new Size(rotImage.rows(), rotImage.cols()), CvType.CV_8UC3);
                        rotImage.copyTo(rgbFrame);
                        Imgproc.cvtColor(rgbFrame, rgbFrame, Imgproc.COLOR_BGR2RGBA);
                        resultBitmap = Bitmap.createBitmap(rotImage.cols(),  rotImage.rows(),Bitmap.Config.ARGB_8888);
                        Utils.matToBitmap(rgbFrame, resultBitmap);
                        mResult = resultBitmap;
                    }catch (Exception e) {
                        Log.e("CarLicenseRecog", e.getMessage());
                    }
                }
                Log.e("CarLicenseRecog" , "JNI count is " + count);
                if (count == 0)
                {
                    license = "No License Recognized!!!";
                    handler = new Handler(Looper.getMainLooper()) {
                        @Override
                        public void handleMessage(Message msg) {
                            textView.setText(license);
                        }
                    };
                    handler.sendEmptyMessage(1);
                }
                else
                {
                    handler = new Handler(Looper.getMainLooper()) {
                        @Override
                        public void handleMessage(Message msg) {
                            imgView.setImageBitmap(mResult);
                            textView.setText(license);
                        }
                    };
                    handler.sendEmptyMessage(1);
                }
            }catch (Exception e) {
                Log.e("CarLicenseRecog", e.getMessage());
            }
            return null;
        }
    }
}

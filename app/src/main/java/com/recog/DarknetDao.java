package com.recog;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;

/**
 * Created by Leon on 2018/7/26.
 */

public class DarknetDao {

    //private static String PATH = Environment.getExternalStorageDirectory()+ File.separator + "CarLicenseRecognition" + File.separator;
    //private static String IMAGE_PATH = Environment.getExternalStorageDirectory()+ File.separator + "DCIM" + File.separator+ "Camera" + File.separator;
    private static String PATH = "";
    private static String IMAGE_PATH = "";
    static {
        try {
            System.loadLibrary("opencv_java3");
            System.loadLibrary("darknet");
            Log.i("CarLiecenseApp", "Successfully load library");
        } catch (UnsatisfiedLinkError e) {
            Log.i("CarLiecenseApp", "Failed to load library error");

        }
    }
    //darknet.exe detector test data/coco.data yolov3.cfg yolov3.weights -i 0 -thresh 0.25 dog.jpg -ext_output
    public static void copyFiles(Context context){
        Log.e("CarLicenseRecog", "JNI : starting copying files");
        Boolean isSDPresent = android.os.Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED);
        //Boolean isSDSupportedDevice = Environment.isExternalStorageRemovable();

        //if(isSDPresent)
        {
            PATH = Environment.getExternalStorageDirectory()+ File.separator + "CarLicenseRecognition" + File.separator;
            IMAGE_PATH = Environment.getExternalStorageDirectory()+ File.separator + "DCIM" + File.separator+ "Camera" + File.separator;
        }
        //else{
        //    PATH = context.getFilesDir().getAbsolutePath()+ File.separator + "CarLicenseRecognition" + File.separator;
        //    IMAGE_PATH = context.getFilesDir().getAbsolutePath()+ File.separator + "DCIM" + File.separator+ "Camera" + File.separator;
        //}
        File file = new File(PATH+"detection.cfg");
        if(file.exists()) {
            UtilFile.delFolderContent(PATH);
            Log.e("CarLicenseRecog", "JNI : Delete all previous files");
            //Log.e("CarLicenseRecog", "JNI : File already exist");
            //return;
        }
        new File(PATH).mkdirs();
        new File(IMAGE_PATH).mkdirs();
        UtilFile.CopyDataToSdcard(context,"car_lp.cfg",PATH+"car_lp.cfg");
        UtilFile.CopyDataToSdcard(context,"car_lp.weights",PATH+"car_lp.weights");
        UtilFile.CopyDataToSdcard(context,"motor_lp.cfg",PATH+"motor_lp.cfg");
        UtilFile.CopyDataToSdcard(context,"motor_lp.weights",PATH+"motor_lp.weights");
        UtilFile.CopyDataToSdcard(context,"detection.cfg",PATH+"detection.cfg");
        UtilFile.CopyDataToSdcard(context,"detection.weights",PATH+"detection.weights");

        int file_num = 11;
        String filename;
        for (int i = 0 ; i < file_num; i ++)
        {
            filename = i + ".jpg";
            UtilFile.CopyDataToSdcard(context,filename,IMAGE_PATH+ filename);
        }
        Log.e("CarLicenseRecog", "JNI : copy success");

    }

    public native Result[] detectLicensePlate(long inputImage);
    public native int Rotate(long inputImage, long outputImage);
    public native Result[] Recognition(long inputImage, float thresh, int plate_type);

    public native boolean load(String detection_cfgfile, String detection_weightfile,
                                String longlp_cfgfile, String longlp_weightfile,
                                String shortlp_cfgfile, String shortlp_weightfile);
    public native boolean unload();
}

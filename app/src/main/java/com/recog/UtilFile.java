package com.recog;

import android.content.Context;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

/**
 * Created by Leon on 2017/8/1.
 */

public class UtilFile {
    public static boolean CopyDataToSdcard(Context context, String fileName, String outFileName) {
        try {
            InputStream is = context.getResources().getAssets().open(fileName);

            int size = is.available();
            byte[] buffer = new byte[size];
            is.read(buffer);
            is.close();
            File out = new File(outFileName);
            //if(!out.exists()){
            //    out.createNewFile();
            //}
            FileOutputStream fos = new FileOutputStream(out);
            fos.write(buffer);
            fos.close();
            return true;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    public static void delFolderContent(String dir_name)
    {
        File dir = new File(dir_name);
        if (dir.isDirectory()==true){
            File [] files = dir.listFiles();
            for (int i=0;i<files.length;i++){
                files[i].delete();
            }
        }
    }
}

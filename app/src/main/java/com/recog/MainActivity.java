package com.recog;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.media.Image;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;

import com.recog.LicenseRecog.RecogFromFile;
import com.recog.LicenseRecog.RecogFromCamera;


import com.recog.R;
import com.recog.logger.LogRecorder;

import java.io.File;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    protected ImageButton btnFromFile;
    protected ImageButton btnFromCamera;

    LogRecorder logRecorder = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_start_);

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setTitle("Recogntion from File");
        getSupportActionBar().hide();

        btnFromFile = (ImageButton) findViewById(R.id.BtnFromFile);
        btnFromFile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.e("CarLicenseRecog", "JNI : Recognition From File Button clicked");
                Intent i = new Intent(MainActivity.this, RecogFromFile.class);
                startActivity(i);
            }
        });
        btnFromCamera = (ImageButton) findViewById(R.id.BtnFromCamera);
        btnFromCamera.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.e("CarLicenseRecog", "JNI : Recognition From Camera Button clicked");
                Intent i = new Intent(MainActivity.this, RecogFromCamera.class);
                startActivity(i);
            }
        });
    }

    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();

    }

    public void onDestroy() {
        super.onDestroy();
    }

}

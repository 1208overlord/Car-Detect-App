package com.recog;

import java.io.File;
import java.io.IOException;

import com.recog.LoadingTask.LoadingTaskFinishedListener;
import com.recog.logger.LogRecorder;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ProgressBar;

public class SplashActivity extends Activity implements LoadingTaskFinishedListener {

	LogRecorder logRecorder = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.act_splash);
		//getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		checkPermissions();

	}

	private void checkPermissions(){
		Log.e("CarLicenseRecog", "JNI : Check Permission Start");
		String [] permissions={Manifest.permission.READ_EXTERNAL_STORAGE,
				Manifest.permission.WRITE_EXTERNAL_STORAGE,
				Manifest.permission.CAMERA

		};
		int n=0;
		for (int i=0;i<permissions.length;i++){
			if (ActivityCompat.checkSelfPermission(this, permissions[i]) != PackageManager.PERMISSION_GRANTED){
				ActivityCompat.shouldShowRequestPermissionRationale((Activity) this,	permissions[i]);
				n++;
			}
		}

		if (n>0){
			Log.e("CarLicenseRecog", "JNI : Check Permission Successed");
			ActivityCompat.requestPermissions((Activity) this, permissions,	10);
		}else{
			CreateLog();
			Log.e("CarLicenseRecog", "JNI : Check Permission Successed");


			DarknetDao.copyFiles(SplashActivity.this);
			// Start your loading
			ProgressBar progressBar = (ProgressBar) findViewById(R.id.progressBar1);
			new LoadingTask(progressBar, this).execute("Loading.."); // Pass in whatever you need a url is just an example we don't use it in this tutorial

			new LoadNetworks().execute(this);
		}

	}
	public void CreateLog(){
		Boolean isSDPresent = android.os.Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED);
		Boolean isSDSupportedDevice = Environment.isExternalStorageRemovable();
		String logPath = "";
		//if(isSDPresent)
		{
			logPath = Environment.getExternalStorageDirectory() + File.separator + "CarLicenseRecognition"+"/";
		}
		//else{
		//	logPath = getApplicationContext().getFilesDir().getAbsolutePath() + File.separator + "CarLicenseRecognition"+"/";
		//}
		Log.e("CarLicenseRecog", "JNI : LogPath is " + logPath);
		try{
			Runtime.getRuntime().exec("chmod -R 777" + logPath);

		}catch (IOException e){
			Log.e("CarLicenseRecog", e.getMessage());

		}

		File root = new File(logPath);

		//File root = new File(App.getInstance().getAppDir()+"/cleanliness1");
		//File root = new File(getApplicationContext().getFilesDir(), APP_TEMP_FOLDER);
		//File root = new File(CameraPictureProvider.CONTENT_URI);

		if (!root.exists()) {

			if(root.mkdirs()){
				//Toast.makeText(this, "Successfully created app folder", Toast.LENGTH_LONG).show();
			}else{
				//Toast.makeText(this, "Failed to create app folder", Toast.LENGTH_LONG).show();
			}
		}else{
			//Toast.makeText(this, "Already exist app folder", Toast.LENGTH_LONG).show();
		}

		String path = root.getAbsolutePath();
		Log.e("CarLicenseRecog", "JNI : LogFolderPath is " + path);
		logRecorder = new LogRecorder.Builder(getApplicationContext())
				.setLogFolderName( "CarLicenseRecognition" )
				.setLogFolderPath( path )
				.setLogFileNameSuffix( "log" )
				.setLogFileSizeLimitation( 255 )
				.setLogLevel( 6 )
				.addLogFilterTag( "CarLicenseRecog" )
				.build();
		logRecorder.start();
	}
	@Override
	public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
		//super.onRequestPermissionsResult(requestCode, permissions, grantResults);

		switch (requestCode) {
			case 10:
				if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
					Intent i = getBaseContext().getPackageManager()
							.getLaunchIntentForPackage( getBaseContext().getPackageName() );
					i.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
					startActivity(i);

                    /*Dialog dlg = new Dialog(this);
                    dlg.setTitle("Restart App");
                    dlg.setOnDismissListener(new DialogInterface.OnDismissListener() {
                        @Override
                        public void onDismiss(DialogInterface dialog) {
                            System.exit(0);
                        }
                    });
                    dlg.show();*/
                    /*RestartDialog dlg = new RestartDialog(this);
                    dlg.setTitle("Restart App");
                    dlg.setOnDismissListener(new OnDismissListener() {
                        @Override
                        public void onDismiss(DialogInterface dialog) {
                            Intent i = getBaseContext().getPackageManager()
                                    .getLaunchIntentForPackage( getBaseContext().getPackageName() );

                            i.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

                            //create a pending intent so the application is restarted after System.exit(0) was called.
                            // We use an AlarmManager to call this intent in 100ms
                            int mPendingIntentId = 223344;
                            PendingIntent mPendingIntent = PendingIntent
                                    .getActivity(SearchActivity.this, mPendingIntentId, i,
                                            PendingIntent.FLAG_CANCEL_CURRENT);
                            AlarmManager mgr = (AlarmManager) SearchActivity.this.getSystemService(Context.ALARM_SERVICE);
                            mgr.set(AlarmManager.RTC, System.currentTimeMillis() + 100, mPendingIntent);
                            //kill the application
                            System.exit(0);
                        }
                    });
                    dlg.show();*/

					//Toast.makeText(this, "Please restart app for permission.", Toast.LENGTH_LONG).show();
				}
				break;
		}

	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		return keyCode == KeyEvent.KEYCODE_BACK ? true : false;
	}

	@Override
	protected void onStop() {
		super.onStop();		
	}
	
	private void startHomeActivity() {
		Intent intent = new Intent(SplashActivity.this, MainActivity.class);
		startActivity(intent);
		finish();
	}

	@Override
	public void onTaskFinished() {
		// TODO Auto-generated method stub
		
	}
}

class LoadNetworks extends AsyncTask<Activity, Integer, Object> {

	private Activity activity;
	private DarknetDao darknet = new DarknetDao();
	private String PATH = "";
	@Override
	protected Object doInBackground(Activity... arg0) {
		Log.e("CarLicenseRecog", "JNI : starting loading");
		activity = arg0[0];

		Boolean isSDPresent = android.os.Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED);
		//Boolean isSDSupportedDevice = Environment.isExternalStorageRemovable();

		//if(isSDPresent)
		{
			PATH = Environment.getExternalStorageDirectory()+ File.separator + "CarLicenseRecognition" + File.separator;
		}
		//else{
		//	PATH = activity.getFilesDir().getAbsolutePath()+ File.separator + "CarLicenseRecognition" + File.separator;
		//}
		Log.e("CarLicenseRecog", "JNI : Data Path is " + PATH);
		darknet.load(PATH+"detection.cfg", PATH+"detection.weights",
				PATH+"car_lp.cfg", PATH+"car_lp.weights",
				PATH+"motor_lp.cfg", PATH+"motor_lp.weights");
		Log.e("CarLicenseRecog", "JNI : load success");
		return null;
	}

    // This is called each time you call publishProgress()
    protected void onProgressUpdate(Integer... progress) {
    }

    // This is called when doInBackground() is finished
    protected void onPostExecute(Object result) {
    	Intent intent = new Intent(activity, MainActivity.class);
    	activity.startActivity(intent);
    	activity.finish();
    }
}
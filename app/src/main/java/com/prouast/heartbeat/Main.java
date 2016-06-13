package com.prouast.heartbeat;

import org.apache.commons.io.FileUtils;
import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.regex.Pattern;

/**
 * The main class.
 */
public class Main extends AppCompatActivity implements CvCameraViewListener2, RPPGMobile.RPPGListener, RPPGNetworkClient.NetworkClientStateListener {

    /* Settings */
    private static final int SAMPLING_FREQUENCY = 1;
    private static final int RESCAN_INTERVAL = 1;
    private static final boolean LOG = false;
    private static final boolean VIDEO = false;
    private static final boolean DRAW = false;
    private static final int VIDEO_BITRATE = 100000;

    /* Constants */
    private static final String TAG = "Heartbeat::Main";
    private static final Pattern PARTIAl_IP_ADDRESS =
            Pattern.compile("^((25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[1-9][0-9]|[0-9])\\.){0,3}"+
                    "((25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[1-9][0-9]|[0-9])){0,1}$");

    private CameraBridgeViewBase mOpenCvCameraView;
    private RPPGMobile rPPG;
    private RPPGResultQueue queue;
    private Mat mRgba;
    private Mat mGray;
    private long now;

    private FFmpegEncoder encoder;
    private File videoFile;

    private RPPGNetworkClient client = null;
    private String serverAddress = null;
    private boolean clientConnected = false;

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("RPPGMobile");

                    rPPG = new RPPGMobile();

                    mOpenCvCameraView.enableView();
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    private String loadCascadeFile(File cascadeDir, int id, String filename) throws IOException {

        InputStream is = getResources().openRawResource(id);
        File cascadeFile = new File(cascadeDir, filename);
        FileOutputStream os = new FileOutputStream(cascadeFile);

        byte[] buffer = new byte[4096];
        int bytesRead;
        while ((bytesRead = is.read(buffer)) != -1) {
            os.write(buffer, 0, bytesRead);
        }
        is.close();
        os.close();

        return cascadeFile.getAbsolutePath();
    }

    static {
        try {
            System.loadLibrary("avformat-55");
            System.loadLibrary("avcodec-55");
            System.loadLibrary("avutil-52");
            System.loadLibrary("swscale-2");
            System.loadLibrary("FFmpegEncoder");
        } catch (UnsatisfiedLinkError e) {
            Log.e("Error importing lib: ", e.toString());
        }
    }

    /* ACTIVITY LIFE CYCLE */

    /**
     * First method that is called when the app is started.
     * @param savedInstanceState saved state
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        // Clear directory from old log files
        try {
            FileUtils.deleteDirectory(getApplicationContext().getExternalFilesDir(null));
        } catch (IOException e) {
            Log.e(TAG, "Exception while clearing directory: " + e);
        }

        // Set the user interface layout for this Activity
        // The layout file is defined in the project res/layout/activity_main.xml file
        setContentView(R.layout.activity_main);

        // Wire up the camera view
        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.fd_activity_surface_view);
        mOpenCvCameraView.setCvCameraViewListener(this);

        // Initialise the Network client, result queue and Heartrate monitor
        client = new RPPGNetworkClient(this);
        queue = RPPGResultQueue.getInstance();

        // Initialise the video file and encoder
        if (VIDEO) {
            videoFile = new File(getApplicationContext().getExternalFilesDir(null), "Android_ffmpeg.mkv");
            encoder = new FFmpegEncoder();
        }

        // Setup network button
        Button networkClientButton = (Button)findViewById(R.id.btnNetworkClient);
        networkClientButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.i(TAG, "Network Button clicked!");
                if (clientConnected) {
                    client.stop();
                } else {
                    getServerAddress();
                }
            }
        });
    }

    /**
     * Called when app is paused.
     */
    @Override
    public void onPause() {
        super.onPause();
        if (mOpenCvCameraView != null) {
            mOpenCvCameraView.disableView();
        }
        //monitor.clearHRM();
    }

    /**
     * Called when app resumes from a pause.
     */
    @Override
    public void onResume() {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_1_0, this, mLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    /* Helper methods */

    /**
     * Generate an alert dialog for IP address input
     * Called when user hits the network button
     */
    private void getServerAddress() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Please enter the IP address of your Brownie client!");
        final EditText input = new EditText(this);
        input.setInputType(InputType.TYPE_CLASS_PHONE);
        input.addTextChangedListener(new TextWatcher() {
            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {}

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            private String mPreviousText = "";

            @Override
            public void afterTextChanged(Editable s) {
                if (PARTIAl_IP_ADDRESS.matcher(s).matches()) {
                    mPreviousText = s.toString();
                } else {
                    s.replace(0, s.length(), mPreviousText);
                }
            }
        });
        if (serverAddress != null) {
            input.setText(serverAddress);
        }
        builder.setView(input);
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                serverAddress = input.getText().toString();
                client.setServerAddress(serverAddress);
                new Thread(client).start();
            }
        });
        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        });
        builder.show();
    }

    /* CvCameraViewListener2 methods */

    /**
     * Called when the camera view starts
     * @param width - the width of the frames that will be delivered
     * @param height - the height of the frames that will be delivered
     */
    public void onCameraViewStarted(int width, int height) {

        // Set up Mats
        mGray = new Mat();
        mRgba = new Mat();

        // Prepare FFmpegEncoder
        if (VIDEO) {
            if (!encoder.openFile(videoFile.getAbsolutePath(), width, height, VIDEO_BITRATE, 30)) {
                Log.e(TAG, "Encoder failed to open");
            } else {
                Log.i(TAG, "Encoder loaded successfully");
            }
        }

        File cascadeDir = getDir("cascade", Context.MODE_PRIVATE);

        // Initialise rPPG

        try {
            rPPG.load(this, width, height, SAMPLING_FREQUENCY, RESCAN_INTERVAL,
                    getApplicationContext().getExternalFilesDir(null).getAbsolutePath(),
                    loadCascadeFile(cascadeDir, R.raw.haarcascade_frontalface_alt, "haarcascade_frontalface_alt.xml"),
                    LOG, DRAW);
            Log.i(TAG, "Loaded rPPG");
        } catch (IOException e) {
            Log.e(TAG, "Failed to load cascade. Exception thrown: " + e);
        }

        cascadeDir.delete();
    }

    /**
     * Called for processing of each camera frame
     * @param inputFrame - the delivered frame
     * @return mRgba
     */
    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {

        // Retrieve timestamp
        // This is where the timestamp for each video frame originates
        now = System.currentTimeMillis();

        mRgba.release();
        mGray.release();

        // Get RGBA and Gray versions
        mRgba = inputFrame.rgba();
        mGray = inputFrame.gray();

        // Write frame to video
        if (VIDEO) {
            encoder.writeFrame(mRgba.dataAddr(), now);
        }

        // Send the frame to rPPG for processing
        // To C++
        rPPG.processFrame(mRgba.getNativeObjAddr(), mGray.getNativeObjAddr(), now);

        return mRgba;
    }

    /**
     * Called when the camera view stops
     */
    public void onCameraViewStopped() {

        // Stop the client
        if (client.isActive) {
            client.stop();
        }

        if (VIDEO) {
            encoder.closeFile();
        }

        // Release resources
        mGray.release();
        mRgba.release();
        rPPG.exit();
    }

    /* PRRGListener methods */

    /**
     * Called when a result from the HRM is delivered
     * Called from C++
     * @param result the RPPGResult
     */
    public void onRPPGResult(RPPGResult result) {

        // Push the result to the queue
        if (client.isActive) {
            queue.push(result);
        }
        Log.i(TAG, "RPPGResult: " + result.getTime() + " – " + result.getMean());
    }

    /* NetworkClientStateListener methods */

    /**
     * Called when the client has connected to the network
     */
    public void onNetworkConnected() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // Update the network button
                Button networkClientButton = (Button) findViewById(R.id.btnNetworkClient);
                networkClientButton.setTextColor(Color.parseColor("#00ff00"));
                networkClientButton.setClickable(true);
            }
        });
        clientConnected = true;
    }

    /**
     * Called when the client has been disconnected from the network
     */
    public void onNetworkDisconnected() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // Update the network button
                Button networkClientButton = (Button) findViewById(R.id.btnNetworkClient);
                networkClientButton.setTextColor(Color.parseColor("#ff0000"));
                networkClientButton.setClickable(true);
            }
        });
        clientConnected = false;

        // Stop requested from Brownie
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    /**
     * Called when there was an exception while connecting to the network
     */
    public void onNetworkConnectException() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // Update the network button
                Button networkClientButton = (Button) findViewById(R.id.btnNetworkClient);
                networkClientButton.setTextColor(Color.parseColor("#ff0000"));
                networkClientButton.setClickable(true);
                // Display an alert
                AlertDialog alertDialog = new AlertDialog.Builder(Main.this).create();
                alertDialog.setTitle("Brownie");
                alertDialog.setMessage("Could not establish a connection to Brownie server. Please check that the server is running and IP address is correct!");
                alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                            }
                        });
                alertDialog.show();
            }
        });
    }

    /**
     * Called when the client has started trying to connect to the network
     */
    public void onNetworkTryingToConnect() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // Update the network button
                Button networkClientButton = (Button) findViewById(R.id.btnNetworkClient);
                networkClientButton.setClickable(false);
            }
        });
    }
}
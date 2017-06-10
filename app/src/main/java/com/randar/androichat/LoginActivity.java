package com.randar.androichat;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.content.Intent;
import android.support.v7.widget.LinearLayoutCompat;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.Toast;

import java.io.Serializable;

import static android.provider.AlarmClock.EXTRA_MESSAGE;

/**
 * Created by Mrz355 on 08.06.17.
 */

public class LoginActivity extends AppCompatActivity {
    public static final String EXTRA_MESSAGE = "com.randar.androichat.MESSAGE";

    private Toast toast;
    private String username;
    private LinearLayout progressIndicator;
    private Button button;

    static {
        System.loadLibrary("msg-receiver");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        button = (Button) findViewById(R.id.loginButton);
    }

    @Override
    public void onResume() { //at the application launch and going back from MainActivity
        super.onResume();
        logout(); // ensure that we're not logged in
    }

    // Invoked when user clicks "Login" button
    public void sendMessage(View view) {
        if(toast != null) {
            toast.cancel();
        }
        EditText editText = (EditText) findViewById(R.id.loginEditText);
        username = editText.getText().toString();
        progressIndicator = (LinearLayout) findViewById(R.id.progress_indicator);
        progressIndicator.setVisibility(View.VISIBLE);
        button.setEnabled(false);
        connectToServer();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if(toast != null) {
            toast.cancel();
        }
    }

    void connectionRefused() {
        if(toast != null)
            toast.cancel();
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                progressIndicator.setVisibility(View.GONE);
                button.setEnabled(true);
                toast = Toast.makeText(getApplicationContext(), "Could not connect to server. Check your internet connection.", Toast.LENGTH_LONG);
                toast.show();
            }
        });

        logout();
        //finish();
    }

    void connectionEstablished() {
        if(login(username) == 0) {
            Intent intent = new Intent(this, MainActivity.class);
            String message = username;
            intent.putExtra(EXTRA_MESSAGE, message);
            startActivity(intent);
        } else {
            new Handler(Looper.getMainLooper()).post(new Runnable() {
                @Override
                public void run() {
                    toast = Toast.makeText(getApplicationContext(), "Username is already taken.", Toast.LENGTH_LONG);
                    toast.show();
                    button.setEnabled(true);
                }
            });
            logout();
        }
    }

    public native int connectToServer();
    public native int login(String username);
    public native int logout(); // TODO: put it int some onStop(), but do not forget to do not able disconnection from server while rotating phone!
}

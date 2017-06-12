package com.randar.androichat;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.text.InputType;
import android.util.Log;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import java.io.Serializable;
import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {

    private String message;
    private TextView tv;
    private AlertDialog alertDialog;
    private EditText messageEditText;
    private ListView list ;
    private ArrayAdapter<String> adapter ;
    static Toast toast;
    private String username;
    private ProgressBar pb;
    ArrayList<String> messageList;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("msg-receiver");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        Intent intent = getIntent();
        username = intent.getStringExtra(LoginActivity.EXTRA_MESSAGE);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
                builder.setTitle("Message");

                // Set up the input
                //final EditText input = new EditText(MainActivity.this);
                messageEditText = new EditText(MainActivity.this);
                // Specify the type of input expected
                messageEditText.setInputType(InputType.TYPE_CLASS_TEXT);
                builder.setView(messageEditText);


                final InputMethodManager imm;
                messageEditText.requestFocus();
                imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, InputMethodManager.HIDE_IMPLICIT_ONLY);

                // Set up the buttons
                builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        message = messageEditText.getText().toString();
                        if(sendMessage(message) == -1) {
                            Log.d("ERROR","cannot send message");
                        }
                    }
                });
                builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.cancel();
                    }
                });

                alertDialog = builder.show();
                alertDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
                    @Override
                    public void onDismiss(DialogInterface dialog) {
                        messageEditText.setFocusable(true);
                        messageEditText.setFocusableInTouchMode(true);
                        imm.toggleSoftInput(InputMethodManager.HIDE_IMPLICIT_ONLY, 0);
                    }
                });
            }
        });

        list = (ListView) findViewById(R.id.listView1);
        messageList = new ArrayList<>();
        listenCoroutine();
    }



    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    void newMessage(final String username, final String message, final String time) {

        Handler mainHandler = new Handler(this.getMainLooper());
        Runnable myRunnable = new Runnable() {
            @Override
            public void run() {
                messageList.add(time+username+message);
                if(adapter == null) {
                    adapter = new ArrayAdapter<>(getApplicationContext(), R.layout.message_row, messageList);
                    list.setAdapter(adapter);
                } else {
                    adapter.notifyDataSetChanged();
                }
                list.smoothScrollToPosition(adapter.getCount() - 1);
            } // This is your code
        };
        mainHandler.post(myRunnable);
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int listenCoroutine();
    public native int sendMessage(String message);
}

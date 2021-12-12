package com.akb825.deepsea.unittests;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import java.io.File;

public class UnitTestActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        File libDir = new File(getApplicationInfo().nativeLibraryDir);
        File[] libFiles = libDir.listFiles();
        for (int i = 0; i < libFiles.length; ++i)
        {
            String fileName = libFiles[i].getName();
            // Strip lib prefix and .so suffix.
            System.loadLibrary(fileName.substring(3, fileName.length() - 3));
        }

        setContentView(R.layout.activity_main);
        TextView view = (TextView)getWindow().findViewById(R.id.outputView);
        view.setHorizontallyScrolling(true);
        view.setText(runTests());
    }

    @Override
    public void onBackPressed() {
        finish();
        super.onBackPressed();
    }

    private native String runTests();
}

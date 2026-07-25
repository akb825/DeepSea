/*
 * Copyright 2019-2021 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.akb825.deepsea.unittests;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.TextView;

import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.OnApplyWindowInsetsListener;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

public class UnitTestActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String mainLibName = null;
        try {
            ApplicationInfo applicationInfo = getPackageManager().getApplicationInfo(
                getPackageName(), PackageManager.GET_META_DATA);
            mainLibName = applicationInfo.metaData.getString("DS_MAIN_LIB_NAME");
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
        if (mainLibName == null)
            mainLibName = "main";

        getOnBackPressedDispatcher().addCallback(new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                UnitTestActivity.this.finish();
            }
        });

        System.loadLibrary(mainLibName);

        setContentView(R.layout.activity_main);
        TextView view = getWindow().findViewById(R.id.outputView);
        view.setHorizontallyScrolling(true);
        view.setText(runTests());

        Window window = getWindow();
        View decorView = window.getDecorView();

        ViewCompat.setOnApplyWindowInsetsListener(
            decorView.findViewById(R.id.outputWindow),
	        (view1, windowInsets) -> {
	            // Apply insets for the system bars to account for edge-to-edge displays.
	            Insets insets = windowInsets.getInsets(WindowInsetsCompat.Type.systemBars());
	            ViewGroup.MarginLayoutParams layoutParams =
	                (ViewGroup.MarginLayoutParams) view1.getLayoutParams();
	            layoutParams.leftMargin = insets.left;
	            layoutParams.topMargin = insets.top;
	            layoutParams.rightMargin = insets.right;
	            layoutParams.bottomMargin = insets.bottom;
	            view1.setLayoutParams(layoutParams);
	            return WindowInsetsCompat.CONSUMED;
	        });

        WindowInsetsControllerCompat insetsController = WindowCompat.getInsetsController(
            window, decorView);
        insetsController.setAppearanceLightStatusBars(true);
        insetsController.setAppearanceLightNavigationBars(true);
    }

    private native String runTests();
}

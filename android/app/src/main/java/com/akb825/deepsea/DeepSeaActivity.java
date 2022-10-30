/*
 * Copyright 2018-2021 Aaron Barany
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

package com.akb825.deepsea;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;

import org.libsdl.app.SDLActivity;

public class DeepSeaActivity extends SDLActivity
{
	protected String[] getLibraries()
	{
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
		return new String[]{"SDL2", mainLibName};
	}
}

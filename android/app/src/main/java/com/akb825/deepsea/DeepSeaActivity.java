package com.akb825.deepsea;

import org.libsdl.app.SDLActivity;
import java.io.File;

public class DeepSeaActivity extends SDLActivity
{
	protected String[] getLibraries()
	{
		File libDir = new File(getApplicationInfo().nativeLibraryDir);
		File[] libFiles = libDir.listFiles();
		String[] libraries = new String[libFiles.length];
		int hidapiIndex = -1;
		int sdlIndex = -1;
		for (int i = 0; i < libFiles.length; ++i)
		{
			String fileName = libFiles[i].getName();
			// Strip lib prefix and .so suffix.
			libraries[i] = fileName.substring(3, fileName.length() - 3);
			if (libraries[i].equals("hidapi"))
				hidapiIndex = i;
			else if (libraries[i].equals("SDL2"))
				sdlIndex = i;
		}

		// Expects HIDAPI to be first, SDL to be second.
		int index = 0;
		if (hidapiIndex >= 0)
		{
			String temp = libraries[index];
			libraries[index] = libraries[hidapiIndex];
			libraries[hidapiIndex] = temp;
			++index;
		}
		if (sdlIndex >= 0)
		{
			String temp = libraries[index];
			libraries[index] = libraries[sdlIndex];
			libraries[sdlIndex] = temp;
		}

		// Expects the "main" library to be last.
		for (int i = 0; i < libraries.length; ++i)
		{
			if (libraries[i].contains("app") || libraries[i].contains("main"))
			{
				int newMainIndex = libraries.length - 1;
				String temp = libraries[i];
				libraries[i] = libraries[newMainIndex];
				libraries[newMainIndex] = temp;
				break;
			}
		}
		return libraries;
	}
};

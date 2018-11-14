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
		int sdlIndex = 0;
		for (int i = 0; i < libFiles.length; ++i)
		{
			String fileName = libFiles[i].getName();
			// Strip lib prefix and .so suffix.
			libraries[i] = fileName.substring(3, fileName.length() - 3);
			if (libraries[i].equals("SDL2"))
				sdlIndex = i;
		}

		String temp = libraries[0];
		libraries[0] = libraries[sdlIndex];
		libraries[sdlIndex] = temp;
		return libraries;
	}
};

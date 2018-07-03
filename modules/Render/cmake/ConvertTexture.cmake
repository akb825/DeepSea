# Copyright 2017 Aaron Barany
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_program(CUTTLEFISH cuttlefish)

# ds_convert_texture(container
#                    OUTPUT output
#                    FORMAT format
#                    [TYPE type]
#                    [DIMENSION dim]
#                    [ALPHA alpha]
#                    [QUALITY quality]
#                    [OUTPUT_FORMAT format]
#                    [IMAGE file]
#                    [ARRAY_IMAGE file1 [file2 ...]]
#                    [NEGX file]
#                    [POSX file]
#                    [NEGY file]
#                    [POSY file]
#                    [NEGZ file]
#                    [POSZ file]
#                    [ARRAY_NEGX file1 [file2 ...]]
#                    [ARRAY_POSX file1 [file2 ...]]
#                    [ARRAY_NEGY file1 [file2 ...]]
#                    [ARRAY_POSY file1 [file2 ...]]
#                    [ARRAY_NEGZ file1 [file2 ...]]
#                    [ARRAY_POSZ file1 [file2 ...]]
#                    [IMAGE_LIST file]
#                    [RESIZE width height [filter]]
#                    [MIPMAP [levels] [filter]]
#                    [FLIPX]
#                    [FLIPY]
#                    [ROTATE angle]
#                    [NORMALMAP [height]]
#                    [GRAYSCALE]
#                    [SWIZZLE swzl]
#                    [SRGB]
#                    [PRE_MULTIPLY]
#                    [WORKING_DIRECTORY dir])
#
# Converts an image, or list of images, to a texture.
#
# OUTPUT - the path of the output.
# FORMAT - the texture format to convert to. See the cuttlefish help output for a list of fromats.
# TYPE - the type stored in each format. May be unorm (default), snorm, uint, int, ufloat, or
#        float.
# DIMENSION - the dimension fo the texture. May be 1, 2 (default), or 3.
# ALPHA - the alpha type. May be none, standard, pre-multiplied, or encoded.
# QUALITY - the quality of the compression. May be lowest, low, normal (default), high, or highest.
# OUTPUT_FORMAT - the file format to output as. May be dds, ktx, or pvr. Defaults to the extension
#                 on the output file.
# IMAGE - path to the image file to convert for a single-image texture.
# ARRAY_IMAGE - list of image files to convert for a texture array.
# NEGX - image file for the negative X face of a cube map.
# POSX - image file for the positive X face of a cube map.
# NEGY - image file for the negative Y face of a cube map.
# POSY - image file for the positive Y face of a cube map.
# NEGZ - image file for the negative Z face of a cube map.
# POSZ - image file for the positive Z face of a cube map.
# ARRAY_NEGX - list of image files for the negative X face of a cube map array.
# ARRAY_POSX - list of image files for the positive X face of a cube map array.
# ARRAY_NEGY - list of image files for the negative Y face of a cube map array.
# ARRAY_POSY - list of image files for the positive Y face of a cube map array.
# ARRAY_NEGZ - list of image files for the negative Z face of a cube map array.
# ARRAY_POSZ - list of image files for the positive Z face of a cube map array.
# IMAGE_LIST - path to a text image list file to describe the image or images to convert.
# RESIZE - resizes the image to a new width and height. Width or height may be nextpo2 or
#          nearestpo2 for the next power of two or nearest power of two. The filter may optionally
#          be specified as box, linear, cubic, or catmull-rom (default)
# MIPMAP - mipmap the image. The number of levels may optionally be specified. The filter may also
#          be optionally specified and may be box, linear, cubic, or catmull-rom (default). If both
#          the number of levels and filter are specified, the number of levels must be first.
# FLIPX - flip the image in the X direction.
# FLIPY - flip the image in the Y direction.
# ROTATE - rotate the image. Must be a multiple of 90 degrees.
# NORMALMAP - generate a normal map from the image. The height of the normal map may optionally be
#             specified.
# GRAYSCALE - convert the image to grayscale based on REC 709.
# SWIZZLE - swizzles the color channels. A combination of 4 values of R, G, B, A, or X.
# SRGB - convert the image from sRGB color space to linear. This will be done in hardware during
#        texture sampling if possible.
# PRE_MULTIPLY - pre-multiply the alpha.
# WORKING_DIRECTORY - the working directory for running cuttlefish.
function(ds_convert_texture container)
	if (NOT CUTTLEFISH)
		message(FATAL_ERROR "Program 'cuttlefish' not found on the path.")
	endif()

	set(options FLIPX FLIPY GRAYSCALE SRGB PRE_MULTIPLY)
	set(oneValueArgs OUTPUT FORMAT TYPE DIMENSION ALPHA QUALITY OUTPUT_FORMAT IMAGE NEGX POSX
		NEGY POSY NEGZ POSZ IMAGE_LIST ROTATE NORMALMAP SWIZZLE WORKING_DIRECTORY)
	set(multiValueArgs ARRAY_IMAGE ARRAY_NEGX ARRAY_POSX ARRAY_NEGY ARRAY_POSY ARRAY_NEGZ
		ARRAY_POSZ MIPMAP)
	cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	if (NOT ARGS_OUTPUT)
		message(FATAL_ERROR "Required option OUTPUT not specified.")
		return()
	endif()
	if (NOT ARGS_FORMAT)
		message(FATAL_ERROR "Required option FORMAT not specified.")
		return()
	endif()
	if (ARGS_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "Unknown arguments: ${ARGS_UNPARSED_ARGUMENTS}")
	endif()

	set(options MIPMAP NORMALMAP)
	cmake_parse_arguments(HAS "${options}" "" "" ${ARGN})

	set(args -o ${ARGS_OUTPUT} -f ${ARGS_FORMAT} -j 1)
	if (ARGS_TYPE)
		list(APPEND args -t ${ARGS_TYPE})
	endif()
	if (ARGS_DIMENSION)
		list(APPEND args -d ${ARGS_DIMENSION})
	endif()
	if (ARGS_ALPHA)
		list(APPEND args --alpha ${ARGS_ALPHA})
	endif()
	if (ARGS_QUALITY)
		list(APPEND args -q ${ARGS_QUALITY})
	endif()
	if (ARGS_OUTPUT_FORMAT)
		list(APPEND args --file-format ${ARGS_OUTPUT_FORMAT})
	endif()
	if (ARGS_RESIZE)
		list(APPEND args -r ${ARGS_RESIZE}) 
	endif()
	if (HAS_MIPMAP)
		list(APPEND args -m)
		if (ARGS_MIPMAP)
			list(APPEND args ${ARGS_MIPMAP})
		endif()
	endif()
	if (ARGS_FLIPX)
		list(APPEND args --flipx)
	endif()
	if (ARGS_FLIPY)
		list(APPEND args --flipy)
	endif()
	if (ARGS_ROTATE)
		list(APPEND args --rotate ${ARGS_ROTATE})
	endif()
	if (HAS_NORMALMAP)
		list(APPEND args -n)
		if (ARGS_NORMALMAP)
			list(APPEND args ${ARGS_NORMALMAP})
		endif()
	endif()
	if (ARGS_GRAYSCALE)
		list(APPEND args -g)
	endif()
	if (ARGS_SWIZZLE)
		list(APPEND args -s ${ARGS_SWIZZLE})
	endif()
	if (ARGS_SRGB)
		list(APPEND args --srgb)
	endif()
	if (ARGS_PRE_MULTIPLY)
		list(APPEND args --pre-multiply)
	endif()

	if (ARGS_IMAGE)
		list(APPEND args -i ${ARGS_IMAGE})
		set(dependencies ${ARGS_IMAGE})
	elseif (ARGS_ARRAY_IMAGE)
		foreach (image ${ARGS_ARRAY_IMAGE})
			list(APPEND args -a ${image})
		endforeach()
		set(dependencies ${ARGS_ARRAY_IMAGE})
	elseif (ARGS_NEGX)
		list(APPEND args -c -x ${ARGS_NEGX} -c +x ${ARGS_POSX})
		list(APPEND args -c -y ${ARGS_NEGY} -c +y ${ARGS_POSY})
		list(APPEND args -c -z ${ARGS_NEGZ} -c +z ${ARGS_POSZ})
		set(dependencies ${ARGS_NEGX} ${ARGS_POSX} ${ARGS_NEGY} ${ARGS_POSY}
			${ARGS_NEGZ} ${ARGS_POSZ})
	elseif (ARGS_ARRAY_NEGX)
		set(index 0)
		foreach (image ${ARGS_ARRAY_NEGX})
			list(APPEND args -C ${index} -x ${image})
			math(EXPR index "${index} + 1")
		endforeach()
		set(index 0)
		foreach (image ${ARGS_ARRAY_POSX})
			list(APPEND args -C ${index} +x ${image})
			math(EXPR index "${index} + 1")
		endforeach()
		set(index 0)
		foreach (image ${ARGS_ARRAY_NEGY})
			list(APPEND args -C ${index} -y ${image})
			math(EXPR index "${index} + 1")
		endforeach()
		set(index 0)
		foreach (image ${ARGS_ARRAY_POSY})
			list(APPEND args -C ${index} +y ${image})
			math(EXPR index "${index} + 1")
		endforeach()
		set(index 0)
		foreach (image ${ARGS_ARRAY_NEGZ})
			list(APPEND args -C ${index} -z ${image})
			math(EXPR index "${index} + 1")
		endforeach()
		set(index 0)
		foreach (image ${ARGS_ARRAY_POSZ})
			list(APPEND args -C ${index} +z ${image})
			math(EXPR index "${index} + 1")
		endforeach()
		set(dependencies ${ARGS_ARRAY_NEGX} ${ARGS_ARRAY_POSX} ${ARGS_ARRAY_NEGY} ${ARGS_ARRAY_POSY}
			${ARGS_ARRAY_NEGZ} ${ARGS_ARRAY_POSZ})
	elseif (ARGS_IMAGE_LIST)
		list(APPEND args -I ${ARGS_IMAGE_LIST})
		set(dependencies ${ARGS_IMAGE_LIST})
	else()
		message(FATAL_ERROR "No image input has been specified.")
		return()
	endif()

	if (ARGS_WORKING_DIRECTORY)
		set(workingDir WORKING_DIRECTORY ${ARGS_WORKING_DIRECTORY})
	else()
		set(workingDir "")
	endif()

	add_custom_command(OUTPUT ${ARGS_OUTPUT}
		COMMAND ${CUTTLEFISH} ARGS ${args}
		DEPENDS ${dependencies} ${CUTTLEFISH}
		${workingDir}
		COMMENT "Converting image to ${ARGS_FORMAT}: ${output}")

	set(${container} ${${container}} ${ARGS_OUTPUT} PARENT_SCOPE)
endfunction()

# ds_convert_textures_target(target container)
#
# Adds a target to convert textures made from previous calls to ds_convert_texture().
#
# target - the name of the target.
# container - the container previously passed to ds_compile_shaders().
# All following arguments are forwarded to add_custom_target().
function(ds_convert_textures_target target container)
	add_custom_target(${target} DEPENDS ${${container}} ${ARGN})
endfunction()

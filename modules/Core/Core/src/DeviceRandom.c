/*
 * Copyright 2022 Aaron Barany
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

#include <DeepSea/Core/DeviceRandom.h>
#include <DeepSea/Core/Error.h>
#include <stdint.h>

#if DS_BSD
#include <sys/param.h>
#endif

// Uses the best method on modern systems, falling back to direct reads on /dev/urandom when a
// function call isn't supported. The only case that should outright fail is Windows before Vista
// SP2, which is well out of support at this time.
#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <bcrypt.h>
#pragma comment(lib, "Bcrypt.lib")
#elif (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 25)) || __ANDROID_API__ >= 28
#define DS_USE_GETRANDOM
#include <sys/random.h>
#elif __MAC_OS_X_VERSION_MIN_REQUIRED >= 101300 || __FreeBSD_version >= 1200000 || OpenBSD >= 201411
#define DS_USE_GETENTROPY
#include <sys/random.h>
#else
#include <DeepSea/Core/Atomic.h>
#include <fcntl.h>
#include <unistd.h>
#endif

bool dsDeviceRandomBytes(void* outData, size_t size)
{
	if (size > 0 && !outData)
	{
		errno = EINVAL;
		return false;
	}

	uint8_t* curBytes = (uint8_t*)outData;

#if DS_WINDOWS

	// ULONG is always 32 bits on Windows.
	ULONG maxSize = (ULONG)-1;
	while (size > 0)
	{
		ULONG curSize = size > maxSize ? maxSize : (ULONG)size;
		NTSTATUS result =
			BCryptGenRandom(NULL, curBytes, curSize, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
		if (!BCRYPT_SUCCESS(result))
		{
			errno = EPERM;
			return false;
		}

		curBytes += curSize;
		size -= curSize;
	}

#elif defined(DS_USE_GETRANDOM)

	while (size > 0)
	{
		ssize_t readSize = getrandom(curBytes, size, 0);
		if (readSize < 0)
		{
			// Try again if it failed with an interrupt.
			if (errno == EINTR)
				continue;
			return false;
		}

		curBytes += readSize;
		size -= readSize;
	}

#elif defined(DS_USE_GETENTROPY)

	// getentropy() only allows 256 bytes.
	const size_t maxSize = 256;
	while (size > 0)
	{
		size_t curSize = size > maxSize ? maxSize : size;
		if (getentropy(curBytes, curSize) != 0)
			return false;

		curBytes += curSize;
		size -= curSize;
	}

#else

	static int32_t urand = -1;

	// Thread-safe opening of /dev/urandom on first call. Allow the OS to close the file on exit.
	// Reads should be thread safe from the same file descriptor, with the kernel performing any
	// synchronization that's needed.
	int32_t fd;
	DS_ATOMIC_LOAD32(&urand, &fd);
	if (fd < 0)
	{
		int32_t newFd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
		if (newFd < 0)
			return false;

		if (DS_ATOMIC_COMPARE_EXCHANGE32(&urand, &fd, &newFd, false))
			fd = newFd;
		else
		{
			// Another thread opened it, fd was updated and need to close newFd.
			close(newFd);
		}
	}

	while (size > 0)
	{
		ssize_t readSize = read(fd, curBytes, size);
		if (readSize < 0)
		{
			// Try again if it failed with an interrupt.
			if (errno == EINTR)
				continue;
			return false;
		}
		else if (readSize == 0)
		{
			errno = EIO;
			return false;
		}

		curBytes += readSize;
		size -= readSize;
	}

#endif

	return true;
}

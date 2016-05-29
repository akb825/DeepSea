/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Export.h>

DS_CORE_EXPORT size_t dsStream_read(dsStream* stream, void* data, size_t size);
DS_CORE_EXPORT size_t dsStream_write(dsStream* stream, const void* data, size_t size);

DS_CORE_EXPORT bool dsStream_seek(dsStream* stream, int64_t offset, dsStreamSeekWay way);
DS_CORE_EXPORT uint64_t dsStream_tell(dsStream* stream);

DS_CORE_EXPORT void dsStream_flush(dsStream* stream);
DS_CORE_EXPORT bool dsStream_close(dsStream* stream);

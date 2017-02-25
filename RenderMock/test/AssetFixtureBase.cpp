#include "AssetFixtureBase.h"
#include <DeepSea/Core/Streams/Path.h>

extern char assetsDir[];

AssetFixtureBase::AssetFixtureBase(const char* dir)
	: m_dir(dir)
{
}

const char* AssetFixtureBase::getPath(const char* fileName)
{
	dsPath_combine(m_buffer, DS_PATH_MAX, assetsDir, m_dir);
	dsPath_combine(m_buffer, DS_PATH_MAX, m_buffer, fileName);
	return m_buffer;
}

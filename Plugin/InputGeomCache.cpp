//! PrecompiledHeader Include.
#include "Plugin/PrecompiledHeader.h"

//! Header Include.
#include "InputGeomCache.h"

InputGeomCache::InputGeomCache(const GeomCacheDesc *desc)
{
	uint32_t count = 0;

	const GeomCacheDesc *currentDesc = desc;
	while (currentDesc->semantic != nullptr)
	{
		++count;
	}

	memcpy(m_Descriptor, desc, sizeof(GeomCacheDesc) * count);
}

InputGeomCache::~InputGeomCache()
{
	auto it = m_Data.begin();
	const auto itEnd = m_Data.end();
	while (it != itEnd)
	{
		void *data = const_cast<void*>(it->second.data);
		free(data);

		++it;
	}

	m_Data.clear();
}

void InputGeomCache::addData(float time, const GeomCacheData* data)
{
	const auto it = std::find_if(m_Data.begin(), m_Data.end(),
						[time] (const FrameDataType& d)
						{
							return d.first == time;
						});

	if (it == m_Data.end())
	{
		void *currentData = nullptr;
		if (data->data)
		{
			const size_t cacheDataSize = getDataSize();

			currentData = malloc(cacheDataSize);
			if (currentData != nullptr)
			{
				memcpy(currentData, data->data, cacheDataSize * data->count);
			}
		}

		// Insert sorted.
		auto dataToInsert = std::make_pair(time, GeomCacheData{ currentData, data->count });
		const auto itInsert = std::lower_bound(m_Data.begin(), m_Data.end(), dataToInsert,
										[](const FrameDataType& lhs, const FrameDataType& rhs)
										{
											return lhs.first < rhs.first;
										});
		m_Data.insert(itInsert, dataToInsert);
	}
}

void InputGeomCache::getDesc(GeomCacheDesc* desc) const
{
	uint32_t count = 0;

	const GeomCacheDesc *currentDesc = m_Descriptor;
	while (currentDesc->semantic != nullptr)
	{
		++count;
	}

	memcpy(desc, m_Descriptor, sizeof(GeomCacheDesc) * count);
}

size_t InputGeomCache::getDataSize() const
{
	size_t cacheDataSize = 0;

	const GeomCacheDesc *currentDesc = m_Descriptor;
	while (currentDesc->semantic != nullptr)
	{
		cacheDataSize += getSizeOfDataFormat(currentDesc->format);
	}

	return cacheDataSize;
}

void InputGeomCache::getData(float time, GeomCacheData* data)
{
	const auto it = std::find_if(m_Data.begin(), m_Data.end(),
							[time](const FrameDataType& d)
							{
								return d.first == time;
							});

	if (it == m_Data.end())
	{
		data->data = it->second.data;
		data->count = it->second.count;
	}
}

size_t InputGeomCache::getDataCount() const
{
	return 0;
}
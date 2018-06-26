#pragma once

//! Local Includes.
#include "IDecompressor.h"
#include "NullTypes.h"

//! Project Includes.
#include "Plugin/GeomCacheData.h"

namespace nvc
{

class NullDecompressor final : public IDecompressor
{
private:
	using FrameDataType = std::pair<float, GeomCacheData>;

	Stream* m_pStream = nullptr;

	null_compression::FileHeader m_Header = {};

	GeomCacheDesc m_Descriptor[GEOM_CACHE_MAX_DESCRIPTOR_COUNT] = {};

	std::vector<uint64_t> m_SeekTable;
	std::vector<float> m_FrameTimeTable;
	std::vector<bool> m_IsFrameLoaded;

	std::vector<FrameDataType> m_LoadedFrames;

	size_t m_FramesOffset = 0;

public:
	NullDecompressor() = default;
	~NullDecompressor();

	void open(Stream* pStream) override;
	void close() override;
	void prefetch(size_t frameIndex, size_t range) override;

	bool getData(size_t frameIndex, float& time, const GeomCacheData& data) override;
	bool getData(float time, const GeomCacheData& data) override;

	//...
	NullDecompressor(const NullDecompressor&) = delete;
	NullDecompressor(NullDecompressor&&) = delete;
	NullDecompressor& operator=(const NullDecompressor&) = delete;
	NullDecompressor& operator=(NullDecompressor&&) = delete;

private:
	void addLoadedData(size_t frameIndex, const GeomCacheData& data);

	size_t getSeekTableIndex(size_t frameIndex) const
	{
		return frameIndex / m_Header.FrameSeekWindowCount;
	}

	size_t getSeekTableOffset(size_t frameIndex) const
	{
		const size_t seekTableIndex = getSeekTableIndex(frameIndex);
		if (seekTableIndex < m_SeekTable.size())
		{
			return m_SeekTable[seekTableIndex];
		}

		assert(false);
		return ~0u;
	}

	size_t getDataSize() const;

	void loadFrame(size_t frameIndex);
};

} // namespace nvc
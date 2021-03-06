#include "Plugin/PrecompiledHeader.h"
#include "Plugin/Foundation/Types.h"
#include "Plugin/Foundation/Pcg.h"
#include "Plugin/AlembicToGeomCache/AlembicToGeomCache.h"
#include "Plugin/Stream/FileStream.h"
#include "Plugin/Compression/NullCompressor.h"
#include "Plugin/Compression/QuantisationCompressor.h"
#include "Plugin/InputGeomCache.h"
#include "Plugin/GeomCache.h"
#include "Plugin/GeomCacheData.h"
#include "Plugin/NativeVertexCacheTest/TestUtil.h"

namespace {
using namespace nvc;
#define USE_QUANTISATION_COMPRESSOR 1
#if defined(USE_QUANTISATION_COMPRESSOR) && (USE_QUANTISATION_COMPRESSOR)
using Compressor = nvc::QuantisationCompressor;
#else
using Compressor = nvc::NullCompressor;
#endif
template<typename T>
void convertDataArrayToFloat2(float2* dst, const T* src, size_t numberOfElements) {
	for(size_t i = 0; i < numberOfElements; ++i) {
		dst[i] = to_float(src[i]);
	}
}

template<typename T>
void convertDataArrayToFloat3(float3* dst, const T* src, size_t numberOfElements) {
	for(size_t i = 0; i < numberOfElements; ++i) {
		dst[i] = to_float(src[i]);
	}
}

template<typename T>
void convertDataArrayToFloat4(float4* dst, const T* src, size_t numberOfElements) {
	for(size_t i = 0; i < numberOfElements; ++i) {
		dst[i] = to_float(src[i]);
	}
}

void convertDataArrayToFloat2(float2* dst, const void* src, size_t numberOfElements, DataFormat dataFormat) {
	switch(dataFormat) {
	default:		assert(false && "DataFormat must have 2 components");		break;
//	case DataFormat::Int2:		convertDataArrayToFloat2(dst, static_cast<const int2*>     (src), numberOfElements);	break;
	case DataFormat::Float2:	convertDataArrayToFloat2(dst, static_cast<const float2*>   (src), numberOfElements);	break;
	case DataFormat::Half2:		convertDataArrayToFloat2(dst, static_cast<const half2*>    (src), numberOfElements);	break;
	case DataFormat::SNorm16x2:	convertDataArrayToFloat2(dst, static_cast<const snorm16x2*>(src), numberOfElements);	break;
	case DataFormat::UNorm16x2:	convertDataArrayToFloat2(dst, static_cast<const unorm16x2*>(src), numberOfElements);	break;
	}
}

void convertDataArrayToFloat3(float3* dst, const void* src, size_t numberOfElements, DataFormat dataFormat) {
	switch(dataFormat) {
	default:		assert(false && "DataFormat must have 3 components");		break;
//	case DataFormat::Int3:		convertDataArrayToFloat3(dst, static_cast<const int3*>     (src), numberOfElements);	break;
	case DataFormat::Float3:	convertDataArrayToFloat3(dst, static_cast<const float3*>   (src), numberOfElements);	break;
	case DataFormat::Half3:		convertDataArrayToFloat3(dst, static_cast<const half3*>    (src), numberOfElements);	break;
	case DataFormat::SNorm16x3:	convertDataArrayToFloat3(dst, static_cast<const snorm16x3*>(src), numberOfElements);	break;
	case DataFormat::UNorm16x3:	convertDataArrayToFloat3(dst, static_cast<const unorm16x3*>(src), numberOfElements);	break;
	}
}

void convertDataArrayToFloat4(float4* dst, const void* src, size_t numberOfElements, DataFormat dataFormat) {
	switch(dataFormat) {
	default:		assert(false && "DataFormat must have 4 components");		break;
//	case DataFormat::Int4:		convertDataArrayToFloat4(dst, static_cast<const int4*>     (src), numberOfElements);	break;
	case DataFormat::Float4:	convertDataArrayToFloat4(dst, static_cast<const float4*>   (src), numberOfElements);	break;
	case DataFormat::Half4:		convertDataArrayToFloat4(dst, static_cast<const half4*>    (src), numberOfElements);	break;
	case DataFormat::SNorm16x4:	convertDataArrayToFloat4(dst, static_cast<const snorm16x4*>(src), numberOfElements);	break;
	case DataFormat::UNorm16x4:	convertDataArrayToFloat4(dst, static_cast<const unorm16x4*>(src), numberOfElements);	break;
	}
}
} // namespace

/*
static void dump(const nvc::InputGeomCache& igc) {
	using namespace nvc;
	using namespace nvcabc;
	const auto nFrame = igc.getDataCount();
	GeomCacheDesc geomCacheDescs[GEOM_CACHE_MAX_DESCRIPTOR_COUNT + 1] {};
	igc.getDesc(&geomCacheDescs[0]);

	const int descIndex_points   = getAttributeIndex(geomCacheDescs, nvcSEMANTIC_POINTS  );
	const int descIndex_normals  = getAttributeIndex(geomCacheDescs, nvcSEMANTIC_NORMALS );
	const int descIndex_tangents = getAttributeIndex(geomCacheDescs, nvcSEMANTIC_TANGENTS);
	const int descIndex_uv0      = getAttributeIndex(geomCacheDescs, nvcSEMANTIC_UV0     );
	const int descIndex_uv1      = getAttributeIndex(geomCacheDescs, nvcSEMANTIC_UV1     );
	const int descIndex_colors   = getAttributeIndex(geomCacheDescs, nvcSEMANTIC_COLORS  );

	printf("GeomCacheDesc.descIndex_points   = %d\n", descIndex_points   );
	printf("GeomCacheDesc.descIndex_normals  = %d\n", descIndex_normals  );
	printf("GeomCacheDesc.descIndex_tangents = %d\n", descIndex_tangents );
	printf("GeomCacheDesc.descIndex_uv0      = %d\n", descIndex_uv0      );
	printf("GeomCacheDesc.descIndex_uv1      = %d\n", descIndex_uv1      );
	printf("GeomCacheDesc.descIndex_colors   = %d\n", descIndex_colors   );

	printf("nFrame(%zd)\n", nFrame);
	for(size_t iFrame = 0; iFrame < nFrame; ++iFrame) {
		printf("frame(%zd), ", iFrame);
		float frameTime = 0.0f;
		GeomCacheData geomCacheData {};
		igc.getData(iFrame, frameTime, &geomCacheData);

		printf("time(%5.3f)", frameTime);
		printf("\n");

		const auto nIndex = geomCacheData.indexCount;
		printf("index[%zd]={", nIndex);
		const int32_t* pIndex = static_cast<const int32_t*>(geomCacheData.indices);
		for(size_t iIndex = 0; iIndex < nIndex; ++iIndex) {
			const int32_t index = pIndex[iIndex];
			printf("%d, ", index);
		}
		printf("}\n");

		if(descIndex_points >= 0 && geomCacheData.vertices != nullptr) {
			std::vector<float3> points(geomCacheData.vertexCount);
			const auto* p = geomCacheData.vertices[descIndex_points];
			if(p != nullptr) {
				convertDataArrayToFloat3(
					  points.data()
					, p
					, points.size()
					, geomCacheDescs[descIndex_points].format
				);
				printf("points[%zd]={", points.size());
				for(const auto& v : points) {
					printf("{%+5.3f,%+5.3f,%+5.3f}, ", v[0], v[1], v[2]);
				}
				printf("}\n");
			}
		}

		if(descIndex_uv1 >= 0 && geomCacheData.vertices != nullptr) {
			std::vector<float2> uv1s(geomCacheData.vertexCount);
			const auto* p = geomCacheData.vertices[descIndex_uv1];
			if(p != nullptr) {
				convertDataArrayToFloat2(
					  uv1s.data()
					, p
					, uv1s.size()
					, geomCacheDescs[descIndex_uv1].format
				);
				printf("uv1s[%zd]={", uv1s.size());
				for(const auto& v : uv1s) {
					printf("{%+5.3f,%+5.3f}, ", v[0], v[1]);
				}
				printf("}\n");
			}
		}

		printf("\n");
	}
}
*/

static void dump(const nvc::OutputGeomCache& outputGecomCache) {
	using namespace nvc;

	const auto nIndex = outputGecomCache.indices.size();
	printf("indices[%zd]={", nIndex);
	for(size_t iIndex = 0; iIndex < nIndex; ++iIndex) {
		printf("%d, ", outputGecomCache.indices[iIndex]);
	}
	printf("}\n");

	const auto nPoints = outputGecomCache.points.size();
	printf("points[%zd]={", nPoints);
	for(const auto& v : outputGecomCache.points) {
		printf("(%+5.3f,%+5.3f,%+5.3f), ", v[0], v[1], v[2]);
	}
	printf("}\n");

	const auto nNormals = outputGecomCache.normals.size();
	printf("normals[%zd]={", nNormals);
	for(const auto& v : outputGecomCache.normals) {
		printf("(%+5.3f,%+5.3f,%+5.3f), ", v[0], v[1], v[2]);
	}
	printf("}\n");

	const auto nTangents = outputGecomCache.tangents.size();
	printf("tangents[%zd]={", nTangents);
	for(const auto& v : outputGecomCache.tangents) {
		printf("(%+5.3f,%+5.3f,%+5.3f,%+5.3f), ", v[0], v[1], v[2], v[3]);
	}
	printf("}\n");

	const auto nUvs = outputGecomCache.uv0.size();
	printf("uv0[%zd]={", nUvs);
	for(const auto& v : outputGecomCache.uv0) {
		printf("(%+5.3f,%+5.3f), ", v[0], v[1]);
	}
	printf("}\n");

	const auto nColors = outputGecomCache.colors.size();
	printf("colors[%zd]={", nColors);
	for(const auto& v : outputGecomCache.colors) {
		printf("(%+5.3f,%+5.3f,%+5.3f,%+5.3f), ", v[0], v[1], v[2], v[3]);
	}
	printf("}\n");
}

static void test0() {
    using namespace nvc;
    using namespace nvcabc;

    const char* abcFilename = "../../../Data/Cloth-1frame.abc";
#if USE_QUANTISATION_COMPRESSOR
	const char* nvcFilename = "../../../Data/TestOutput/Cloth-1frame.quantisation.nvc";
#else
	const char* nvcFilename = "../../../Data/TestOutput/Cloth-1frame.nvc";
#endif
    assert(IsFileExist(abcFilename));
    RemoveFile(nvcFilename);

	// Import -> output .nvc
	{
	    ImportOptions opt;

	    const auto abcIgc = nvcabcAlembicToInputGeomCache(abcFilename, opt);
	    assert(abcIgc);

//		dump(*abcGeoms.geometry);

	    FileStream fs { nvcFilename, FileStream::OpenModes::Random_ReadWrite };
	    Compressor nc {};
	    nc.compress(*abcIgc, &fs);

        nvcIGCRelease(abcIgc);
    }

	// read .nvc
	{
		GeomCache geomCache;
		const auto r0 = geomCache.open(nvcFilename);
		assert(r0);
		geomCache.prefetch(0, 1);
		geomCache.setCurrentFrame(0.0f);

		OutputGeomCache outputGecomCache;
		const auto r1 = geomCache.assignCurrentDataToMesh(outputGecomCache);
		assert(r1);

//		dump(outputGecomCache);
	}
}

static void test1() {
    using namespace nvc;
    using namespace nvcabc;

    const char* abcFilename = "../../../Data/Cloth-300frames.abc";
#if USE_QUANTISATION_COMPRESSOR
	const char* nvcFilename = "../../../Data/TestOutput/Cloth-300frames.quantisation.nvc";
#else
	const char* nvcFilename = "../../../Data/TestOutput/Cloth-300frames.nvc";
#endif
    assert(IsFileExist(abcFilename));
    RemoveFile(nvcFilename);

	// Import -> output .nvc
	{
	    ImportOptions opt;

	    const auto abcIgc = nvcabcAlembicToInputGeomCache(abcFilename, opt);
	    assert(abcIgc);

//		dump(*abcGeoms.geometry);

	    FileStream fs { nvcFilename, FileStream::OpenModes::Random_ReadWrite };
	    Compressor nc {};
	    nc.compress(*abcIgc, &fs);

        nvcIGCRelease(abcIgc);
    }

	// read .nvc
	{
		GeomCache geomCache;
		const auto r0 = geomCache.open(nvcFilename);
		assert(r0);

		const auto nFrame = geomCache.getFrameCount();
		for(size_t iFrame = 9ull; iFrame < nFrame; ++iFrame) {
			printf("frame(%zd/%zd) @ %6.3fsec:\n", iFrame, nFrame, geomCache.getTimeByFrameIndex(iFrame));

			geomCache.prefetch(iFrame, 1);
			geomCache.setCurrentFrameIndex(iFrame);

			OutputGeomCache outputGecomCache;
			const auto r1 = geomCache.assignCurrentDataToMesh(outputGecomCache);
			assert(r1);

			dump(outputGecomCache);
		}
	}
}

// Random access by time(float).
static void test2() {
    using namespace nvc;
    using namespace nvcabc;

    const char* abcFilename = "../../../Data/Cloth-10frames.abc";
#if USE_QUANTISATION_COMPRESSOR
	const char* nvcFilename = "../../../Data/TestOutput/Cloth-10frames.quantisation.nvc";
#else
	const char* nvcFilename = "../../../Data/TestOutput/Cloth-10frames.nvc";
#endif
    assert(IsFileExist(abcFilename));
    RemoveFile(nvcFilename);

	// Import -> output .nvc
	{
	    ImportOptions opt;

	    const auto abcIgc = nvcabcAlembicToInputGeomCache(abcFilename, opt);
	    assert(abcIgc);

//		dump(*abcGeoms.geometry);

	    FileStream fs { nvcFilename, FileStream::OpenModes::Random_ReadWrite };
	    Compressor nc {};
	    nc.compress(*abcIgc, &fs);

        nvcIGCRelease(abcIgc);
    }

	// read .nvc
	{
		GeomCache geomCache;
		const auto r0 = geomCache.open(nvcFilename);
		assert(r0);

		const auto nFrame = geomCache.getFrameCount();
		Pcg pcg(123, 456);
		for(size_t iTest = 0, nTest = 4096; iTest < nTest; ++iTest) {
			const float time = (pcg.getUint32() >> 16) * 2.0f / 65536.0f - 1.0f;
			printf("iTest(%zd), time(%+6.3f (0x%08x))"
				   , iTest
				   , time
				   , * reinterpret_cast<const uint32_t*>(&time)
			);

			const size_t frameIndex = geomCache.getFrameIndexByTime(time);
			printf("  -> frameIndex(%zd)\n", frameIndex);

			geomCache.setCurrentFrame(time);
//			if(frameIndex != ~0u)
			{
//				geomCache.prefetch(frameIndex, 1);

				OutputGeomCache outputGecomCache;
				const auto r1 = geomCache.assignCurrentDataToMesh(outputGecomCache);
//				assert(r1);

			//	dump(outputGecomCache);
			}
		}
	}
}


// Constant data
static void test3() {
    using namespace nvc;
    using namespace nvcabc;

	const char* abcFilename = "../../../Data/Cloth-10frames.abc";
#if USE_QUANTISATION_COMPRESSOR
    const char* nvcFilename = "../../../Data/TestOutput/Cloth-10frames.quantisation.nvc";
#else
    const char* nvcFilename = "../../../Data/TestOutput/Cloth-10frames.nvc";
#endif
    assert(IsFileExist(abcFilename));
    RemoveFile(nvcFilename);

	// Import -> output .nvc
	{
	    ImportOptions opt;

	    const auto abcIgc = nvcabcAlembicToInputGeomCache(abcFilename, opt);
	    assert(abcIgc);

//		dump(*abcGeoms.geometry);

	    FileStream fs { nvcFilename, FileStream::OpenModes::Random_ReadWrite };
	    Compressor nc {};
	    nc.compress(*abcIgc, &fs);

        nvcIGCRelease(abcIgc);
	}

	// read .nvc
	{
		GeomCache geomCache;
		const auto r0 = geomCache.open(nvcFilename);
		assert(r0);

		const auto nString = geomCache.getConstantDataStringSize();
		for(size_t iString = 0; iString < nString; ++iString) {
			const char* str = geomCache.getConstantDataString(iString);
			printf("const[%zd]={%s}\n", iString, str);
		}
	}
}

void RunTest_AlembicToNvc()
{
//	test0();
	test1();
//	test2();
//	test3();
}

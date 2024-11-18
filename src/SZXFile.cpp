#include "SZXFile.h"

#include <assert.h>

bool rm::decompressData(byte const* compressedData, size_t compressedSize, byte* decompressedData, size_t decompressedSize) {
    z_stream zlibStream = {};
    zlibStream.next_in = (Bytef*)compressedData;
    zlibStream.avail_in = (uInt)compressedSize;
    zlibStream.next_out = decompressedData;
    zlibStream.avail_out = (uInt)decompressedSize;

    // Initialize zlib for decompression
    if (inflateInit(&zlibStream) != Z_OK) {
        return false;
    }

    // Decompress the data
    int ret = inflate(&zlibStream, Z_FINISH);

    if (ret != Z_STREAM_END) {
        inflateEnd(&zlibStream);
        return false;
    }

    assert(decompressedSize == zlibStream.total_out);

    // Cleanup zlib resources
    inflateEnd(&zlibStream);
    return true;
}

#include "GlBufferVector.h"
#include "glad/glad.h"

#include <iostream>
#include <format>

SGlBufferVector::SGlBufferVector(size_t size) : Size(size), Head(0)
{
    glCreateBuffers(1, &*Id);
    glNamedBufferStorage(*Id, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

SGlBufferVector::~SGlBufferVector()
{
    if (Id)
		glDeleteBuffers(1, &*Id);
}

SGlBufferVector::SGlBufferVector(SGlBufferVector&& Other)
{
    // Writing it like this to make it EXTRA explicit we're move-assigning this.
    operator=(std::move(Other));
}

SGlBufferVector& SGlBufferVector::operator=(SGlBufferVector&& Other)
{
    // just copy and memset for simplicity, dirty but will do for now
    memcpy(this, &Other, sizeof(SGlBufferVector));
    memset(&Other, 0, sizeof(SGlBufferVector));
    return *this;
}

SGlBufferRangeId SGlBufferVector::AppendRaw(size_t numBytes, const void* pData, uint32_t elemSize)
{
    assert(Id && "Attempting to use AppendRaw on invalid GlBufferVector");
    const bool bEmptyAppend = (numBytes == 0) || !pData;
    const bool bNotEnoughSpace = Head + numBytes > Size;
    if (bEmptyAppend || bNotEnoughSpace)
    {
        std::cout << std::format("AppendRaw failed - buffer={} - bEmptyAppend={} - NotEnoughSpace={}\n", 
            *Id, bEmptyAppend, bNotEnoughSpace);
        return SGlBufferRangeId {};
    }
    SGlBufferRangeId Out { Id, elemSize, Head, numBytes };
    glNamedBufferSubData(*Id, Head, numBytes, pData);
    Head += numBytes;
    return Out;
}

void SGlBufferVector::UpdateRaw(const SGlBufferRangeId& range, size_t numBytes, const void* pData, uint32_t elemSize)
{
    assert(Id && range && (*Id == *range) && "Attempting to use UpdateRaw on invalid GlBufferVector or with invalid BufferRangeId");
    assert((range.SizeBytes == numBytes) && (range.GetNumElems() == (numBytes / elemSize)) && "UpdateRaw invalid in data.");
    const bool bEmptyUpdate = (numBytes == 0) || !pData;
    if (!bEmptyUpdate)
    {
		glNamedBufferSubData(*Id, range.Head, numBytes, pData);
    }
}

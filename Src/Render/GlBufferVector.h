#pragma once

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <deque>
#include "glm/glm.hpp"
#include "GlIdTypes.h"
#include "Utils/GenericConcepts.h"
#include "Math/EngineMath.h"
#include "GlShader.h"

// TODO: Template this for type/alignment safety, and to enforce std140/430 on appends/inserts. 
// Should prob move it to its own header aswell.
struct SGlBufferVector
{
	SGlBufferId Id {};
	size_t Size = 0; // In bytes
	size_t Head = 0; // In bytes

	// Don't need these two now, but i'll leave this here as a reminder if i ever need it
	// GLbitfield Flags;
	// void* PMappedBuffer; 

	SGlBufferVector() = default;
	SGlBufferVector(size_t size);
	~SGlBufferVector();
	// Disable copy
	SGlBufferVector(const SGlBufferVector& Other) = delete;
	SGlBufferVector& operator=(const SGlBufferVector& Other) = delete;
	// Allow move
	SGlBufferVector(SGlBufferVector&& Other);
	SGlBufferVector& operator=(SGlBufferVector&& Other);

	// Resets head to zero, doesn't do anything else.
	void Reset() { Head = 0; };

	// Appends to the vector using glBufferSubData
	SGlBufferRangeId AppendRaw(size_t numBytes, const void* pData, uint32_t elemSize);
	template<typename T> 
	SGlBufferRangeId Append(const std::vector<T>& elems) { return AppendRaw(elems.size() * sizeof(T), elems.data(), sizeof(T)); }
	template<typename T> 
	SGlBufferRangeId Append(size_t numElems, const T* pElems) { return AppendRaw(numElems * sizeof(T), pElems, sizeof(T)); }

	// Updates vector using glBufferSubData
	void UpdateRaw(const SGlBufferRangeId& range, size_t numBytes, const void* pData, uint32_t elemSize);
	template<typename T> 
	void Update(const SGlBufferRangeId& range, const std::vector<T>& elems) { UpdateRaw(range, elems.size() * sizeof(T), elems.data(), sizeof(T)); }
	template<typename T> 
	void Update(const SGlBufferRangeId& range, size_t numElems, const T* pElems) { return UpdateRaw(range, numElems * sizeof(T), pElems, sizeof(T)); }
};

#pragma once

#ifndef __SenTinyObjectLoader__
#define __SenTinyObjectLoader__

#include <vector>
#include <unordered_map>

#define GLM_FORCE_SWIZZLE // Have to add this for new glm version without default structure initialization 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct VertexStruct {
	glm::vec3 position;
	glm::vec2 texCoord;

	bool operator==(const VertexStruct& other) const {
		return position == other.position && texCoord == other.texCoord;
	}
};

namespace stobjl
{
	void populateVertexIndexVector(const char* const tinyObjectDiskAddress,
		std::vector<VertexStruct>& vertexStructVectorToPopulate, std::vector<uint32_t>& indexVectorToPopulate);

} //namespace stobjl




#endif // __SenTinyObjectLoader__
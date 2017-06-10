#pragma once

#ifndef __SenVulkanMeshStruct__
#define __SenVulkanMeshStruct__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/gtx/hash.hpp>			// for tinyObjLoader

//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

#include <vector>
#include <unordered_map>


struct VertexStruct {
	//glm::vec3 position;
	//glm::vec3 normal;
	//glm::vec2 texCoords;
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
};










#endif // __SenVulkanMeshStruct__
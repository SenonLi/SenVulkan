
#include "SenTinyObjLoader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>			// for tinyObjLoader
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace std {
	template<> struct hash<VertexStruct> {
		size_t operator()(VertexStruct const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

namespace stobjl {

	void populateVertexIndexVector(const char* const tinyObjectDiskAddress,
		std::vector<VertexStruct>& vertexStructVectorToPopulate, std::vector<uint32_t>& indexVectorToPopulate) {

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapesVector;
		std::vector<tinyobj::material_t> materialsVector;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapesVector, &materialsVector, &err, tinyObjectDiskAddress)) {
			throw std::runtime_error(err);
		}

		std::unordered_map<VertexStruct, uint32_t> uniqueVertices{};

		// Iterate over all of the shapes to combine all of the faces in the file into a single model
		for (const auto& shape : shapesVector) {
			// shape == mesh == vertices + normals + texCoords + indices + materials
			for (const auto& indexStructVector : shape.mesh.indices) {
				VertexStruct vertexStruct{};

				vertexStruct.position = {
					attrib.vertices[3 * indexStructVector.vertex_index + 0],
					attrib.vertices[3 * indexStructVector.vertex_index + 1],
					attrib.vertices[3 * indexStructVector.vertex_index + 2]
				};
				vertexStruct.texCoord = {
					attrib.texcoords[2 * indexStructVector.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * indexStructVector.texcoord_index + 1]
				};

				if (uniqueVertices.count(vertexStruct) == 0) {
					uniqueVertices[vertexStruct] = static_cast<uint32_t>(vertexStructVectorToPopulate.size());
					vertexStructVectorToPopulate.push_back(vertexStruct);
				}
				indexVectorToPopulate.push_back(uniqueVertices[vertexStruct]);
			}
		}
	}// populateVertexIndexVector()


}// namespace stobjl
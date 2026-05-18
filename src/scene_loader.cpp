#include "scene_loader.h"
#include "triangle.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <filesystem>

bool SceneLoader::load_from_path(Scene& scene, const std::string& file_path) {
    std::filesystem::path path = file_path;
    fastgltf::Parser parser;

    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None) {
        std::cerr << "Failed to load file: " << file_path << "\n";
        return false;
    }

    constexpr auto options =
        fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::LoadExternalBuffers |
        fastgltf::Options::LoadExternalImages;

    auto assetResult = parser.loadGltf(data.get(), path.parent_path(), options);
    if (assetResult.error() != fastgltf::Error::None) {
        std::cerr << "Failed to parse glTF: " << static_cast<uint64_t>(assetResult.error()) << "\n";
        return false;
    }

    fastgltf::Asset asset = std::move(assetResult.get());

    // 1. Load materials first
    load_materials(scene, asset);

    // 2. Load nodes (geometry)
    load_nodes(scene, asset);

    std::cout << "Loaded " << scene.triangles.size() << " triangles\n";

    // 3. Build BVH
    scene.build_bvh();

    return true;
}

void SceneLoader::load_materials(Scene& scene, const fastgltf::Asset& asset) {
    // Index 0 is always the default material
    scene.materials.clear();
    scene.materials.push_back({.color = glm::vec3(1.0f)});

    for (const auto& gltfMat : asset.materials) {
        glm::vec3 color(1.0f);
        color.r = gltfMat.pbrData.baseColorFactor[0];
        color.g = gltfMat.pbrData.baseColorFactor[1];
        color.b = gltfMat.pbrData.baseColorFactor[2];
        
        scene.materials.push_back({.color = color});
    }
}

void SceneLoader::load_nodes(Scene& scene, const fastgltf::Asset& asset) {
    // We only support the first scene for now
    if (asset.scenes.empty()) return;

    fastgltf::iterateSceneNodes(asset, 0, fastgltf::math::fmat4x4(),
        [&](const fastgltf::Node& node, fastgltf::math::fmat4x4 matrix) {
            if (!node.meshIndex.has_value()) return;

            auto& mesh = asset.meshes[*node.meshIndex];
            glm::mat4 transform = glm::make_mat4(&matrix[0][0]);

            for (auto& primitive : mesh.primitives) {
                if (primitive.type != fastgltf::PrimitiveType::Triangles) continue;

                // Position Accessor
                auto positionIt = primitive.findAttribute("POSITION");
                if (positionIt == primitive.attributes.end()) continue;

                auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
                std::vector<glm::vec3> positions;
                positions.reserve(positionAccessor.count);

                fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, positionAccessor,
                    [&](glm::vec3 pos, size_t) {
                        glm::vec4 worldPos = transform * glm::vec4(pos, 1.0f);
                        positions.push_back(glm::vec3(worldPos));
                    });

                // Index Accessor
                std::vector<uint32_t> indices;
                if (primitive.indicesAccessor.has_value()) {
                    auto& indexAccessor = asset.accessors[*primitive.indicesAccessor];
                    indices.reserve(indexAccessor.count);
                    fastgltf::iterateAccessorWithIndex<uint32_t>(asset, indexAccessor,
                        [&](uint32_t idx, size_t) {
                            indices.push_back(idx);
                        });
                } else {
                    indices.resize(positions.size());
                    for (size_t i = 0; i < positions.size(); ++i) indices[i] = static_cast<uint32_t>(i);
                }

                // Material Index
                // glTF material index m maps to scene.materials[m + 1]
                int material_index = 0;
                if (primitive.materialIndex.has_value()) {
                    material_index = static_cast<int>(*primitive.materialIndex) + 1;
                }

                // Build triangles
                for (size_t i = 0; i + 2 < indices.size(); i += 3) {
                    glm::vec3 p0 = positions[indices[i + 0]];
                    glm::vec3 p1 = positions[indices[i + 1]];
                    glm::vec3 p2 = positions[indices[i + 2]];

                    glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

                    Triangle tri{};
                    tri.v0.pos = p0; tri.v1.pos = p1; tri.v2.pos = p2;
                    tri.v0.normal = normal; tri.v1.normal = normal; tri.v2.normal = normal;
                    tri.material_index = material_index;

                    scene.triangles.push_back(tri);
                }
            }
        });
}

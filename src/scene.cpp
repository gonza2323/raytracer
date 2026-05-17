#include "scene.h"
#include "hit_data.h"
#include "ray.h"
#include "lights.h"
#include "triangle.h"
#include "bvh/bvh_node.h"
#include "constants.h"
#include "interval.h"
#include <cmath>
#include <filesystem>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <iostream>

bool Scene::intersect(Ray ray, HitData& hit_data) {
    if (!bvh_root) {
        return false;
    }

    // Use Interval to track valid ray range
    Interval ray_t(0.001, infinity);
    return bvh_root->intersect(ray, hit_data, triangles, ray_t);
}

bool Scene::is_occluded(Ray ray, float max_t) {
    if (!bvh_root) {
        return false;
    }

    HitData hit_data;
    Interval ray_t(0.001, max_t);
    return bvh_root->intersect(ray, hit_data, triangles, ray_t);
}

void Scene::build_bvh() {
    if (triangles.empty()) {
        return;
    }

    bvh_root = BVHNode::build(triangles, 0, triangles.size());
}

void load_scene_from_path(Scene& scene, std::string& file_path) {
    std::filesystem::path path = file_path;

    fastgltf::Parser parser;

    auto data =
    fastgltf::GltfDataBuffer::FromPath(path);

    if (data.error() != fastgltf::Error::None) {
        std::cout << "Failed to load file\n";
        exit(1);
    }

    constexpr auto options =
        fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::LoadExternalBuffers;

    auto assetResult =
        parser.loadGltfBinary(
            data.get(),
            path.parent_path(),
            options
        );

    if (assetResult.error() != fastgltf::Error::None) {
        std::cout << "Failed to parse glTF\n";
        return;
    }

    fastgltf::Asset asset =
        std::move(assetResult.get());
    
    std::cout << "Scenes: " << asset.scenes.size() << "\n";
    std::cout << "Nodes: " << asset.nodes.size() << "\n";
    std::cout << "Meshes: " << asset.meshes.size() << "\n";
    std::cout << "Materials: " << asset.materials.size() << "\n";
    std::cout << "Textures: " << asset.textures.size() << "\n";
    std::cout << "Images: " << asset.images.size() << "\n";

    // Default material
    scene.materials.push_back({
        .color = glm::vec3(1.0f)
    });

    

    fastgltf::iterateSceneNodes(
        asset,
        0,
        fastgltf::math::fmat4x4(),
        [&](fastgltf::Node& node,
            fastgltf::math::fmat4x4 matrix)
        {
            if (!node.meshIndex.has_value()) {
                return;
            }

            auto& mesh =
                asset.meshes[*node.meshIndex];

            glm::mat4 transform =
                glm::make_mat4(&matrix[0][0]);

            for (auto& primitive : mesh.primitives) {

                if (primitive.type !=
                    fastgltf::PrimitiveType::Triangles) {
                    continue;
                }

                // ----------------------------
                // Positions
                // ----------------------------

                auto positionIt =
                    primitive.findAttribute("POSITION");

                if (positionIt ==
                    primitive.attributes.end()) {
                    continue;
                }

                auto& positionAccessor =
                    asset.accessors[
                        positionIt->accessorIndex
                    ];

                std::vector<glm::vec3> positions;

                fastgltf::iterateAccessorWithIndex<glm::vec3>(
                    asset,
                    positionAccessor,
                    [&](glm::vec3 pos, size_t)
                    {
                        glm::vec4 worldPos =
                            transform *
                            glm::vec4(pos, 1.0f);

                        positions.push_back(
                            glm::vec3(worldPos)
                        );
                    }
                );

                // ----------------------------
                // Indices
                // ----------------------------

                std::vector<uint32_t> indices;

                if (primitive.indicesAccessor.has_value()) {

                    auto& indexAccessor =
                        asset.accessors[
                            *primitive.indicesAccessor
                        ];

                    fastgltf::iterateAccessorWithIndex<uint32_t>(
                        asset,
                        indexAccessor,
                        [&](uint32_t idx, size_t)
                        {
                            indices.push_back(idx);
                        }
                    );
                }
                else {
                    // Non-indexed geometry
                    indices.resize(positions.size());

                    for (size_t i = 0;
                         i < positions.size();
                         ++i)
                    {
                        indices[i] =
                            static_cast<uint32_t>(i);
                    }
                }

                // ----------------------------
                // Material
                // ----------------------------

                uint32_t materialIndex = 0;

                if (primitive.materialIndex.has_value()) {

                    auto& gltfMaterial =
                        asset.materials[
                            *primitive.materialIndex
                        ];

                    glm::vec3 color(1.0f);

                    auto& pbr = gltfMaterial.pbrData;

                    color.r =
                        pbr.baseColorFactor[0];

                    color.g =
                        pbr.baseColorFactor[1];

                    color.b =
                        pbr.baseColorFactor[2];

                    materialIndex =
                        static_cast<uint32_t>(
                            scene.materials.size()
                        );

                    scene.materials.push_back({
                        .color = color
                    });
                }

                // ----------------------------
                // Build triangles
                // ----------------------------

                for (size_t i = 0;
                     i + 2 < indices.size();
                     i += 3)
                {
                    glm::vec3 p0 =
                        positions[indices[i + 0]];

                    glm::vec3 p1 =
                        positions[indices[i + 1]];

                    glm::vec3 p2 =
                        positions[indices[i + 2]];

                    glm::vec3 normal =
                        glm::normalize(
                            glm::cross(
                                p1 - p0,
                                p2 - p0
                            )
                        );

                    Triangle tri{};

                    tri.v0.pos = p0;
                    tri.v1.pos = p1;
                    tri.v2.pos = p2;

                    tri.v0.normal = normal;
                    tri.v1.normal = normal;
                    tri.v2.normal = normal;

                    tri.material_index =
                        materialIndex;

                    scene.triangles.push_back(tri);
                }
            }
        }
    );

    std::cout
        << "Loaded "
        << scene.triangles.size()
        << " triangles\n";

    // Build BVH for acceleration
    scene.build_bvh();
}
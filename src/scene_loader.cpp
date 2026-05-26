#include "scene_loader.h"
#include "triangle.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <filesystem>
#include <stb_image.h>

// Convert quaternion to Euler angles (pitch, yaw, roll in radians)
glm::vec3 quaternion_to_euler(const glm::quat& q)
{
    glm::vec3 euler = glm::eulerAngles(q);
    // Reorder from glm's (pitch, yaw, roll) to our convention if needed
    // glm returns (pitch, yaw, roll) which matches our rot order
    return euler;
}

void SceneLoader::load_camera(Scene& scene, const fastgltf::Asset& asset)
{
    // Default camera: looking down -Z from (0, 0, 10)
    glm::vec3 default_pos(0.0f, 0.0f, 10.0f);
    glm::vec3 default_rot(0.0f, 0.0f, 0.0f); // No rotation
    float default_focal_length = 30.0f * 0.001f; // 30 mm
    float default_sensor_x = 36.0f * 0.001f; // 36 mm (standard 35mm width)
    float default_sensor_y = 24.0f * 0.001f; // 24 mm (standard 35mm height)

    // If no cameras in the scene, use default
    if (asset.cameras.empty())
    {
        std::cout << "No camera in glTF file, using default camera\n";
        scene.camera = Camera(default_pos, default_rot, default_focal_length, default_sensor_x, default_sensor_y);
        return;
    }

    // Use the first camera
    const auto& gltf_camera = asset.cameras[0];
    glm::vec3 cam_pos = default_pos;
    glm::vec3 cam_rot = default_rot;
    float focal_length = default_focal_length;
    float sensor_x = default_sensor_x;
    float sensor_y = default_sensor_y;

    // Find the node that references this camera
    for (size_t node_idx = 0; node_idx < asset.nodes.size(); ++node_idx)
    {
        const auto& node = asset.nodes[node_idx];
        if (!node.cameraIndex.has_value() || *node.cameraIndex != 0)
            continue;

        // Found the node with the camera
        // Get the transform matrix (can be either TRS or matrix)
        glm::mat4 transform = glm::identity<glm::mat4>();

        if (std::holds_alternative<fastgltf::TRS>(node.transform))
        {
            const auto& trs = std::get<fastgltf::TRS>(node.transform);
            
            // Build the transformation matrix from TRS
            glm::vec3 translation(trs.translation[0], trs.translation[1], trs.translation[2]);
            glm::quat rotation(trs.rotation[3], trs.rotation[0], trs.rotation[1], trs.rotation[2]); // w, x, y, z
            glm::vec3 scale(trs.scale[0], trs.scale[1], trs.scale[2]);
            
            // Build the transformation: T * R * S
            transform = glm::translate(glm::identity<glm::mat4>(), translation);
            transform *= glm::mat4_cast(rotation);
            transform *= glm::scale(glm::identity<glm::mat4>(), scale);
            
            cam_rot = quaternion_to_euler(rotation);
        }
        else if (std::holds_alternative<fastgltf::math::mat<float, 4, 4>>(node.transform))
        {
            const auto& mat = std::get<fastgltf::math::mat<float, 4, 4>>(node.transform);
            transform = glm::make_mat4(&mat[0][0]);
            
            // Extract rotation as quaternion from the matrix, then convert to Euler angles
            glm::quat rotation_quat = glm::quat_cast(glm::mat3(transform));
            cam_rot = quaternion_to_euler(rotation_quat);
        }

        // Extract position (translation)
        cam_pos = glm::vec3(transform[3]);

        break;
    }

    // Convert camera parameters
    // glTF cameras store yfov (vertical field of view in radians)
    if (std::holds_alternative<fastgltf::Camera::Perspective>(gltf_camera.camera))
    {
        const auto& persp = std::get<fastgltf::Camera::Perspective>(gltf_camera.camera);

        // Standard 35mm sensor dimensions
        sensor_y = 24.0f * 0.001f; // 24 mm

        // Calculate focal length from vertical FOV
        // yfov = 2 * atan(sensor_height / (2 * focal_length))
        // focal_length = sensor_height / (2 * tan(yfov / 2))
        focal_length = sensor_y / (2.0f * std::tan(persp.yfov / 2.0f));

        // Calculate sensor width from aspect ratio
        if (persp.aspectRatio.has_value())
        {
            sensor_x = sensor_y * *persp.aspectRatio;
        }
        else
        {
            sensor_x = sensor_y * 1.5f; // Assume 3:2 ratio if not specified (35mm aspect)
        }

        std::cout << "Loaded camera from glTF\n";
        std::cout << "  Position: (" << cam_pos.x << ", " << cam_pos.y << ", " << cam_pos.z << ")\n";
        std::cout << "  Rotation (radians): (" << cam_rot.x << ", " << cam_rot.y << ", " << cam_rot.z << ")\n";
        std::cout << "  Focal length: " << focal_length * 1000.0f << " mm\n";
        std::cout << "  Sensor size: " << sensor_x * 1000.0f << "x" << sensor_y * 1000.0f << " mm\n";
    }
    else if (std::holds_alternative<fastgltf::Camera::Orthographic>(gltf_camera.camera))
    {
        // Orthographic cameras are not commonly used for ray tracing, use default
        std::cout << "Orthographic camera not supported, using default perspective\n";
    }

    scene.camera = Camera(cam_pos, cam_rot, focal_length, sensor_x, sensor_y);
}

bool SceneLoader::load_from_path(Scene& scene, const std::string& file_path)
{
    std::filesystem::path path = file_path;
    fastgltf::Parser parser(fastgltf::Extensions::KHR_lights_punctual);

    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None)
    {
        std::cerr << "Failed to load file: " << file_path << "\n";
        return false;
    }

    constexpr auto options =
        fastgltf::Options::DontRequireValidAssetMember |
        fastgltf::Options::LoadExternalBuffers |
        fastgltf::Options::LoadExternalImages;

    auto assetResult = parser.loadGltf(data.get(), path.parent_path(), options);
    if (assetResult.error() != fastgltf::Error::None)
    {
        std::cerr << "Failed to parse glTF: " << static_cast<uint64_t>(assetResult.error()) << "\n";
        return false;
    }

    fastgltf::Asset asset = std::move(assetResult.get());

    // 0. Load camera
    load_camera(scene, asset);

    // 1. Load textures
    load_textures(scene, asset, path.parent_path());

    // 2. Load materials
    load_materials(scene, asset);

    // 3. Load nodes (geometry)
    load_nodes(scene, asset);

    std::cout << "Loaded " << scene.triangles.size() << " triangles\n";

    // 4. Build BVH
    scene.build_bvh();

    return true;
}

void SceneLoader::load_textures(Scene& scene, const fastgltf::Asset& asset, const std::filesystem::path& base_path)
{
    scene.textures.reserve(asset.images.size());
    for (size_t i = 0; i < asset.images.size(); ++i)
    {
        const auto& gltfImage = asset.images[i];
        Image img;
        img.width = 0;
        img.height = 0;
        img.channels = 0;

        auto process_data = [&](const std::byte* bytes, size_t size)
        {
            unsigned char* data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(bytes),
                                                        static_cast<int>(size), &img.width, &img.height, &img.channels,
                                                        4);
            if (data)
            {
                img.channels = 4;
                img.data.assign(data, data + img.width * img.height * 4);
                stbi_image_free(data);
            }
        };

        std::visit(fastgltf::visitor{
                       [&](const fastgltf::sources::URI& uri)
                       {
                           if (uri.fileByteOffset != 0 || !uri.uri.isLocalPath()) return;
                           std::filesystem::path imagePath = base_path / uri.uri.path();
                           unsigned char* data = stbi_load(imagePath.string().c_str(), &img.width, &img.height,
                                                           &img.channels, 4);
                           if (data)
                           {
                               img.channels = 4;
                               img.data.assign(data, data + img.width * img.height * 4);
                               stbi_image_free(data);
                           }
                       },
                       [&](const fastgltf::sources::Vector& vector)
                       {
                           process_data(vector.bytes.data(), vector.bytes.size());
                       },
                       [&](const fastgltf::sources::ByteView& view)
                       {
                           process_data(view.bytes.data(), view.bytes.size());
                       },
                       [&](const fastgltf::sources::Array& array)
                       {
                           process_data(array.bytes.data(), array.bytes.size());
                       },
                       [&](const fastgltf::sources::BufferView& view)
                       {
                           auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                           auto& buffer = asset.buffers[bufferView.bufferIndex];
                           std::visit(fastgltf::visitor{
                                          [&](const fastgltf::sources::Vector& vector)
                                          {
                                              process_data(vector.bytes.data() + bufferView.byteOffset,
                                                           bufferView.byteLength);
                                          },
                                          [&](const fastgltf::sources::ByteView& view)
                                          {
                                              process_data(view.bytes.data() + bufferView.byteOffset,
                                                           bufferView.byteLength);
                                          },
                                          [&](const fastgltf::sources::Array& array)
                                          {
                                              process_data(array.bytes.data() + bufferView.byteOffset,
                                                           bufferView.byteLength);
                                          },
                                          [](auto&)
                                          {
                                          }
                                      }, buffer.data);
                       },
                       [](auto&)
                       {
                       }
                   }, gltfImage.data);

        scene.textures.push_back(std::move(img));
    }
}

void SceneLoader::load_materials(Scene& scene, const fastgltf::Asset& asset)
{
    scene.materials.clear();
    // Index 0 is always the default material
    scene.materials.push_back(Material{});

    for (const auto& gltfMat : asset.materials)
    {
        Material mat;
        mat.base_color_factor = glm::vec3(
            gltfMat.pbrData.baseColorFactor[0],
            gltfMat.pbrData.baseColorFactor[1],
            gltfMat.pbrData.baseColorFactor[2]
        );
        mat.roughness_factor = gltfMat.pbrData.roughnessFactor;
        mat.metallic_factor = gltfMat.pbrData.metallicFactor;

        if (gltfMat.pbrData.baseColorTexture.has_value())
        {
            auto& texInfo = gltfMat.pbrData.baseColorTexture.value();
            auto& texture = asset.textures[texInfo.textureIndex];
            if (texture.imageIndex.has_value())
            {
                mat.base_color_texture.texture_index = static_cast<int>(*texture.imageIndex);
                mat.base_color_texture.uv_index = static_cast<int>(texInfo.texCoordIndex);
            }
        }

        if (gltfMat.pbrData.metallicRoughnessTexture.has_value())
        {
            auto& texInfo = gltfMat.pbrData.metallicRoughnessTexture.value();
            auto& texture = asset.textures[texInfo.textureIndex];
            if (texture.imageIndex.has_value())
            {
                mat.metallic_roughness_texture.texture_index = static_cast<int>(*texture.imageIndex);
                mat.metallic_roughness_texture.uv_index = static_cast<int>(texInfo.texCoordIndex);
            }
        }

        if (gltfMat.normalTexture.has_value())
        {
            auto& texInfo = gltfMat.normalTexture.value();
            auto& texture = asset.textures[texInfo.textureIndex];
            if (texture.imageIndex.has_value())
            {
                mat.normal_texture.texture_index = static_cast<int>(*texture.imageIndex);
                mat.normal_texture.uv_index = static_cast<int>(texInfo.texCoordIndex);
            }
        }

        scene.materials.push_back(mat);
    }
}

void SceneLoader::load_nodes(Scene& scene, const fastgltf::Asset& asset)
{
    if (asset.scenes.empty()) return;

    fastgltf::iterateSceneNodes(asset, 0, fastgltf::math::fmat4x4(),
                                [&](const fastgltf::Node& node, fastgltf::math::fmat4x4 matrix)
                                {
                                    if (!node.meshIndex.has_value()) return;

                                    auto& mesh = asset.meshes[*node.meshIndex];
                                    glm::mat4 transform = glm::make_mat4(&matrix[0][0]);

                                    for (auto& primitive : mesh.primitives)
                                    {
                                        if (primitive.type != fastgltf::PrimitiveType::Triangles) continue;

                                        // Positions
                                        auto positionIt = primitive.findAttribute("POSITION");
                                        if (positionIt == primitive.attributes.end()) continue;
                                        auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
                                        std::vector<glm::vec3> positions;
                                        positions.reserve(positionAccessor.count);
                                        fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, positionAccessor,
                                            [&](glm::vec3 pos, size_t)
                                            {
                                                glm::vec4 worldPos = transform * glm::vec4(pos, 1.0f);
                                                positions.push_back(glm::vec3(worldPos));
                                            });

                                        // Normals
                                        auto normalIt = primitive.findAttribute("NORMAL");
                                        std::vector<glm::vec3> normals;
                                        if (normalIt != primitive.attributes.end())
                                        {
                                            auto& normalAccessor = asset.accessors[normalIt->accessorIndex];
                                            normals.reserve(normalAccessor.count);
                                            fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, normalAccessor,
                                                [&](glm::vec3 n, size_t)
                                                {
                                                    glm::vec3 worldNormal = glm::normalize(
                                                        glm::vec3(transform * glm::vec4(n, 0.0f)));
                                                    normals.push_back(worldNormal);
                                                });
                                        }
                                        else
                                        {
                                            normals.resize(positions.size(), glm::vec3(0.0f));
                                        }

                                        // UVs
                                        std::vector<glm::vec2> uvs[2];
                                        for (int i = 0; i < 2; ++i)
                                        {
                                            std::string attrName = "TEXCOORD_" + std::to_string(i);
                                            auto uvIt = primitive.findAttribute(attrName);
                                            if (uvIt != primitive.attributes.end())
                                            {
                                                auto& uvAccessor = asset.accessors[uvIt->accessorIndex];
                                                uvs[i].reserve(uvAccessor.count);
                                                fastgltf::iterateAccessorWithIndex<glm::vec2>(asset, uvAccessor,
                                                    [&](glm::vec2 uv, size_t)
                                                    {
                                                        uvs[i].push_back(uv);
                                                    });
                                            }
                                            else
                                            {
                                                uvs[i].resize(positions.size(), glm::vec2(0.0f));
                                            }
                                        }

                                        // Indices
                                        std::vector<uint32_t> indices;
                                        if (primitive.indicesAccessor.has_value())
                                        {
                                            auto& indexAccessor = asset.accessors[*primitive.indicesAccessor];
                                            indices.reserve(indexAccessor.count);
                                            fastgltf::iterateAccessorWithIndex<uint32_t>(asset, indexAccessor,
                                                [&](uint32_t idx, size_t)
                                                {
                                                    indices.push_back(idx);
                                                });
                                        }
                                        else
                                        {
                                            indices.resize(positions.size());
                                            for (size_t i = 0; i < positions.size(); ++i) indices[i] = static_cast<
                                                uint32_t>(i);
                                        }

                                        int material_index = 0;
                                        if (primitive.materialIndex.has_value())
                                        {
                                            material_index = static_cast<int>(*primitive.materialIndex) + 1;
                                        }

                                        for (size_t i = 0; i + 2 < indices.size(); i += 3)
                                        {
                                            uint32_t i0 = indices[i + 0];
                                            uint32_t i1 = indices[i + 1];
                                            uint32_t i2 = indices[i + 2];

                                            Triangle tri{};
                                            tri.v0.pos = positions[i0];
                                            tri.v1.pos = positions[i1];
                                            tri.v2.pos = positions[i2];
                                            tri.v0.normal = normals[i0];
                                            tri.v1.normal = normals[i1];
                                            tri.v2.normal = normals[i2];

                                            // If no normals were provided, compute geometric normal
                                            if (glm::length(tri.v0.normal) < 0.1f)
                                            {
                                                glm::vec3 n = glm::normalize(
                                                    glm::cross(tri.v1.pos - tri.v0.pos, tri.v2.pos - tri.v0.pos));
                                                tri.v0.normal = tri.v1.normal = tri.v2.normal = n;
                                            }

                                            for (int uv_set = 0; uv_set < 2; ++uv_set)
                                            {
                                                tri.v0.uvs[uv_set] = uvs[uv_set][i0];
                                                tri.v1.uvs[uv_set] = uvs[uv_set][i1];
                                                tri.v2.uvs[uv_set] = uvs[uv_set][i2];
                                            }

                                            tri.material_index = material_index;
                                            scene.triangles.push_back(tri);
                                        }
                                    }
                                });
}
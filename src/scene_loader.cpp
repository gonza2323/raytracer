#include "scene_loader.h"
#include "triangle.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <filesystem>
#include <stb_image.h>

bool SceneLoader::load_from_path(Scene& scene, const std::string& file_path)
{
    std::filesystem::path path = file_path;
    fastgltf::Parser parser;

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
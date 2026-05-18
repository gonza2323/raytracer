#pragma once

#include "scene.h"
#include <string>
#include <fastgltf/core.hpp>

class SceneLoader {
public:
    static bool load_from_path(Scene& scene, const std::string& file_path);

private:
    static void load_materials(Scene& scene, const fastgltf::Asset& asset);
    static void load_nodes(Scene& scene, const fastgltf::Asset& asset);
};

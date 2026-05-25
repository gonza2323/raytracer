#pragma once

#include "bounding_box.h"
#include "../ray.h"
#include <vector>

struct Triangle;
struct HitData;

class BVHNode
{
public:
    BoundingBox bbox;

    BVHNode* left = nullptr;
    BVHNode* right = nullptr;

    int first_triangle = 0;
    int triangle_count = 0;

    BVHNode()
    {
    }

    ~BVHNode();

    bool is_leaf() const
    {
        return triangle_count > 0;
    }

    bool intersect(const Ray& ray, HitData& hit_data, std::vector<Triangle>& triangles, Interval& ray_t) const;

    static BVHNode* build(std::vector<Triangle>& triangles, int start, int end);

private:
    static constexpr int MAX_TRIANGLES_PER_LEAF = 4;
};
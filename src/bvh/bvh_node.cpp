#include "bvh_node.h"
#include "../triangle.h"
#include "../hit_data.h"
#include <algorithm>
#include <cmath>

const float BBOX_PADDING = 1e-6f;

BVHNode::~BVHNode()
{
    delete left;
    delete right;
}

bool BVHNode::intersect(const Ray& ray, HitData& hit_data, std::vector<Triangle>& triangles, Interval& ray_t) const
{
    // Check if ray hits this node's bounding box
    if (!bbox.hit(ray, ray_t))
    {
        return false;
    }

    bool hit = false;

    if (is_leaf())
    {
        // Leaf node: check all triangles in this node
        for (int i = first_triangle; i < first_triangle + triangle_count; i++)
        {
            HitData temp_hit;
            if (triangles[i].intersect(ray, temp_hit))
            {
                if (temp_hit.t < ray_t.max)
                {
                    hit = true;
                    ray_t.max = temp_hit.t;
                    hit_data = temp_hit;
                }
            }
        }
    }
    else
    {
        // Interior node: recurse into children
        if (left)
        {
            if (left->intersect(ray, hit_data, triangles, ray_t))
            {
                hit = true;
            }
        }
        if (right)
        {
            if (right->intersect(ray, hit_data, triangles, ray_t))
            {
                hit = true;
            }
        }
    }

    return hit;
}

BVHNode* BVHNode::build(std::vector<Triangle>& triangles, int start, int end)
{
    BVHNode* node = new BVHNode();
    int triangle_count = end - start;

    // Compute bounding box for this range
    if (triangle_count > 0)
    {
        BoundingBox bbox = triangles[start].generate_bounding_box();
        for (int i = start + 1; i < end; i++)
        {
            bbox = BoundingBox(bbox, triangles[i].generate_bounding_box());
        }
        // Expand bounding box to avoid precision issues at edges
        node->bbox.x = bbox.x.expand(BBOX_PADDING);
        node->bbox.y = bbox.y.expand(BBOX_PADDING);
        node->bbox.z = bbox.z.expand(BBOX_PADDING);
    }

    // Base case: create leaf node
    if (triangle_count <= MAX_TRIANGLES_PER_LEAF)
    {
        node->first_triangle = start;
        node->triangle_count = triangle_count;
        return node;
    }

    // Find the axis with the largest spread
    glm::vec3 spread(
        node->bbox.x.max - node->bbox.x.min,
        node->bbox.y.max - node->bbox.y.min,
        node->bbox.z.max - node->bbox.z.min
    );

    int axis = 0;
    if (spread.y > spread.x) axis = 1;
    if (spread.z > spread[axis]) axis = 2;

    // Sort triangles along the chosen axis
    auto cmp = [&](const Triangle& a, const Triangle& b)
    {
        glm::vec3 center_a = (a.v0.pos + a.v1.pos + a.v2.pos) * (1.0f / 3.0f);
        glm::vec3 center_b = (b.v0.pos + b.v1.pos + b.v2.pos) * (1.0f / 3.0f);
        return center_a[axis] < center_b[axis];
    };

    std::sort(triangles.begin() + start, triangles.begin() + end, cmp);

    // Split at midpoint
    int mid = start + triangle_count / 2;

    // Recursively build subtrees
    node->left = build(triangles, start, mid);
    node->right = build(triangles, mid, end);

    return node;
}
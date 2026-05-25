#pragma once

#include "../interval.h"
#include "../ray.h"
#include <glm/vec3.hpp>

class BoundingBox
{
public:
    Interval x, y, z;

    BoundingBox()
    {
    } // The default AABB is empty, since Intervals are empty by default.

    BoundingBox(const Interval& x, const Interval& y, const Interval& z)
        : x(x), y(y), z(z)
    {
    }

    BoundingBox(const glm::vec3& a, const glm::vec3& b)
    {
        // Treat the two points a and b as extrema for the bounding box, so we don't require a
        // particular minimum/maximum coordinate order.

        x = (a[0] <= b[0]) ? Interval(a[0], b[0]) : Interval(b[0], a[0]);
        y = (a[1] <= b[1]) ? Interval(a[1], b[1]) : Interval(b[1], a[1]);
        z = (a[2] <= b[2]) ? Interval(a[2], b[2]) : Interval(b[2], a[2]);
    }

    BoundingBox(const BoundingBox& box0, const BoundingBox& box1)
    {
        x = Interval(box0.x, box1.x);
        y = Interval(box0.y, box1.y);
        z = Interval(box0.z, box1.z);
    }

    const Interval& axis_interval(int n) const
    {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    bool hit(const Ray& r, Interval ray_t) const
    {
        const glm::vec3& ray_orig = r.origin;
        const glm::vec3& ray_dir = r.dir;

        for (int axis = 0; axis < 3; axis++)
        {
            const Interval& ax = axis_interval(axis);
            const double adinv = 1.0 / ray_dir[axis];

            auto t0 = (ax.min - ray_orig[axis]) * adinv;
            auto t1 = (ax.max - ray_orig[axis]) * adinv;

            if (t0 < t1)
            {
                if (t0 > ray_t.min) ray_t.min = t0;
                if (t1 < ray_t.max) ray_t.max = t1;
            }
            else
            {
                if (t1 > ray_t.min) ray_t.min = t1;
                if (t0 < ray_t.max) ray_t.max = t0;
            }

            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;
    }
};
#ifndef VOXEL_MAP_H
#define VOXEL_MAP_H

#include "hittable.h"
#include "interval.h"
#include "material.h"
#include "moxel_common.h"
#include "ray.h"
#include "tree.h"
#include "vec3.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <utility>
#include <vector>

class voxel_map : public hittable {
public:
  std::vector<raw_node> node_pool;
  std::vector<uint8_t> leaf_data;
  raw_node root;
  int world_scale;
  shared_ptr<material> mat = make_shared<lambertian>(color(0.4, 0.7, 0.4));

  voxel_map(int depth) : world_scale(1) {
    for (int i = 0; i < depth; i++) {
      world_scale *= 4;
    }
  }

  void build() {
    root = generate_tree(node_pool, leaf_data, world_scale, vec3(0, 0, 0));
  }

  bool hit(const ray &r, interval ray_t, hit_record &rec) const override {
    return hit_node(root, r, ray_t, vec3(0, 0, 0), world_scale, rec);
  }

private:
  bool hit_node(const raw_node &node, const ray &r, interval ray_t, vec3 pos,
                int scale, hit_record &rec) const {
    vec3 inv_d = 1.0 / r.direction();
    interval t =
        intersectAABB(r.origin(), inv_d, pos, pos + vec3(scale, scale, scale));
    if (t.max <= t.min || t.max <= ray_t.min || t.min >= ray_t.max ||
        node.is_leaf == 1)
      return false;

    if (scale == 1 || node.is_leaf == 2) {
      double hit_t = t.min >= ray_t.min ? t.min : t.max;
      if (!ray_t.contains(hit_t))
        return false;

      rec.t = hit_t;
      rec.p = r.at(hit_t);

      vec3 outward_normal(0, 0, 0);
      double tmax_entry = -1e9;
      for (int i = 0; i < 3; i++) {
        double t0 = (pos[i] - r.origin()[i]) * inv_d[i];
        double t1 = (pos[i] + scale - r.origin()[i]) * inv_d[i];
        if (inv_d[i] < 0) {
          std::swap(t0, t1);
        }
        if (std::abs(hit_t - t0) < 1e-6) {
          outward_normal[i] = -1;
        } else if (std::abs(hit_t - t1) < 1e-6) {
          outward_normal[i] = 1;
        }
      }

      rec.set_face_normal(r, outward_normal);
      rec.mat = mat;
      return true;
    }

    int child_scale = scale / 4;

    double entry_t = std::max(t.min, ray_t.min) + 1e-6;
    vec3 entry = r.at(entry_t);
    vec3 cell = floor((entry - pos) / child_scale);
    cell = clamp(cell, vec3(0, 0, 0), vec3(3, 3, 3));

    vec3 step_dir(r.direction()[0] < 0 ? -1 : 1, r.direction()[1] < 0 ? -1 : 1,
                  r.direction()[2] < 0 ? -1 : 1);
    vec3 t_delta = vec3(child_scale) * abs(inv_d);
    vec3 t_max;
    for (int i = 0; i < 3; i++) {
      double boundary =
          pos[i] + (cell[i] + (r.direction()[i] < 0 ? 0 : 1)) * child_scale;
      t_max[i] = (boundary - r.origin()[i]) * inv_d[i];
      if (t_max[i] < t.min)
        t_max[i] += t_delta[i];
    }

    int axis = 0;

    for (int steps = 0; steps < 12; steps++) {
      int x = (int)cell[0], y = (int)cell[1], z = (int)cell[2];

      int bit = child_index(x, y, z);
      if (node.pop_mask & (1ULL << bit)) {
        vec3 child_pos = pos + cell * child_scale;
        raw_node child = {};
        if (child_scale != 1) {
          int rank = __builtin_popcountll(node.pop_mask & ((1ULL << bit) - 1));
          child = node_pool[node.child_ptr + rank];
        }
        if (hit_node(child, r, interval(ray_t.min, ray_t.max), child_pos,
                     child_scale, rec)) {
          return true;
        }
      }

      if (t_max[0] < t_max[1]) {
        if (t_max[0] < t_max[2]) {
          cell[0] += step_dir[0];
          if (cell[0] < 0 || cell[0] > 3)
            break;
          axis = 0;
          t_max[0] += t_delta[0];
        } else {
          cell[2] += step_dir[2];
          if (cell[2] < 0 || cell[2] > 3)
            break;
          axis = 2;
          t_max[2] += t_delta[2];
        }
      } else {
        if (t_max[1] < t_max[2]) {
          cell[1] += step_dir[1];
          if (cell[1] < 0 || cell[1] > 3)
            break;
          axis = 1;
          t_max[1] += t_delta[1];
        } else {
          cell[2] += step_dir[2];
          if (cell[2] < 0 || cell[2] > 3)
            break;
          axis = 2;
          t_max[2] += t_delta[2];
        }
      }
    }
    return false;
  }

  interval intersectAABB(vec3 pos, vec3 inv_dir, vec3 bb_min,
                         vec3 bb_max) const {
    vec3 t0 = (bb_min - pos) * inv_dir;
    vec3 t1 = (bb_max - pos) * inv_dir;

    vec3 temp = t0;
    t0 = min(temp, t1), t1 = max(temp, t1);

    interval t = interval(std::max(std::max(t0.x(), t0.y()), t0.z()),
                          std::min(std::min(t1.x(), t1.y()), t1.z()));

    return t;
  }

  vec3 step(const float edge, const vec3 n) const {
    vec3 result = vec3(1, 1, 1);
    for (int i = 0; i < 3; i++) {
      if (n[i] < edge) {
        result[i] = 0;
      }
    }

    return result;
  }
};

#endif

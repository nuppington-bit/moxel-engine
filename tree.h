#ifndef TREE_H
#define TREE_H

#include "camera.h"
#include "vec3.h"
#include <cassert>
#include <cstdint>
#include <vector>

struct [[gnu::packed]] raw_node {
  uint32_t is_leaf : 1 = 0;
  uint32_t child_ptr : 31 = 0;
  uint64_t pop_mask = 0;
};

int child_index(int x, int y, int z) { return z * 16 + y * 4 + x; }

raw_node generate_tree(std::vector<raw_node> &node_pool,
                       std::vector<uint8_t> &leaf_data, int32_t scale,
                       vec3 pos) {
  raw_node node;
  int child_scale = scale / 4;
  vec3 center = vec3(128, 128, 128);

  if (child_scale == 1) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
          vec3 child_pos = pos + vec3(k, j, i) * child_scale;
          uint8_t mat = (child_pos - center).length() < 96 ? 1 : 0;
          if (mat != 0) {
            node.pop_mask |= (1ULL << child_index(k, j, i));
            leaf_data.push_back(mat);
          }
        }
      }
    }
    return node;
  }
  raw_node children[64];
  bool has_child[64] = {};
  int num_children = 0;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 4; k++) {
        vec3 child_pos = pos + vec3(k, j, i) * child_scale;
        raw_node child =
            generate_tree(node_pool, leaf_data, child_scale, child_pos);
        if (child.pop_mask != 0) {
          int bit = child_index(k, j, i);
          node.pop_mask |= (1ULL << bit);
          children[bit] = child;
          has_child[bit] = true;
          num_children++;
        }
      }
    }
  }

  if (num_children > 0) {
    node.child_ptr = (uint32_t)node_pool.size();
    for (int bit = 0; bit < 64; bit++) {
      if (has_child[bit])
        node_pool.push_back(children[bit]);
    }
  }
  return node;
}

#endif

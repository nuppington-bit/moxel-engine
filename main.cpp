#include "moxel_common.h"

#include "camera.h"
#include "voxel_map.h"
#include <iostream>

int main() {
  voxel_map world(3);
  world.build();

  camera cam;

  cam.aspect_ratio = 16.0 / 9.0;
  cam.image_width = 800;
  cam.samples_per_pixel = 10;
  cam.max_depth = 5;

  cam.v_fov = 30;
  cam.lookfrom = point3(-20, 42, 0);
  cam.lookat = point3(32, 32, 32);
  cam.v_up = vec3(0, 1, 0);

  auto render_start = std::chrono::system_clock::now();
  cam.render(world);
  auto render_end = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(render_end -
                                                                  render_start);
  std::clog << "Render time: " << ms.count() << "ms" << "\n";
}

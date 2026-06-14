#include "moxel_common.h"

#include "camera.h"
#include "voxel_map.h"
#include <iostream>

int main() {
  // hittable_list world;
  // auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
  // world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

  // for (int a = -11; a < 11; a++) {
  //   for (int b = -11; b < 11; b++) {
  //     auto choose_mat = random_double();
  //     point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

  //     if ((center - point3(4, 0.2, 0)).length() > 0.9) {
  //       shared_ptr<material> sphere_material;

  //       if (choose_mat < 0.8) {
  //         // diffuse
  //         auto albedo = color::random() * color::random();
  //         sphere_material = make_shared<lambertian>(albedo);
  //         world.add(make_shared<sphere>(center, 0.2, sphere_material));
  //       } else if (choose_mat < 0.95) {
  //         // metal
  //         auto albedo = color::random(0.5, 1);
  //         auto fuzz = random_double(0, 0.5);
  //         sphere_material = make_shared<metal>(albedo, fuzz);
  //         world.add(make_shared<sphere>(center, 0.2, sphere_material));
  //       } else {
  //         // glass
  //         sphere_material = make_shared<dielectric>(1.5);
  //         world.add(make_shared<sphere>(center, 0.2, sphere_material));
  //       }
  //     }
  //   }
  // }

  // auto material1 = make_shared<dielectric>(1.5);
  // world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  // auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
  // world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  // auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  // world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
  voxel_map world(3);
  world.build();

  camera cam;

  cam.aspect_ratio = 16.0 / 9.0;
  cam.image_width = 800;
  cam.samples_per_pixel = 10;
  cam.max_depth = 2;

  cam.v_fov = 30;
  cam.lookfrom = point3(128, 128, 128);
  cam.lookat = point3(32, 32, 32);
  cam.v_up = vec3(0, 1, 0);

  // cam.defocus_angle = 0.6;
  // cam.focus_dist = 10.0;

  auto render_start = std::chrono::system_clock::now();
  cam.render(world);
  auto render_end = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(render_end -
                                                                  render_start);
  std::clog << "Render time: " << ms <<"\n";
}

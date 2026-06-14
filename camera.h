#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "moxel_common.h"
#include <chrono>
#include <ctime>

class camera {
public:
  double aspect_ratio =
      1.0;               // Image / camera aspect ratio (i.e. 16 / 9 for 16:9)
  int image_width = 100; // Rendered image width
  int samples_per_pixel = 10;        // Random samples per pixel
  int max_depth = 10;                // Max ray bounces
  double v_fov = 90;                 // Vertical FOV
  point3 lookfrom = point3(0, 0, 0); // Point camera is located at
  point3 lookat = point3(0, 0, -1);  // Point camera is looking at
  vec3 v_up = vec3(0, 1, 0);         // Up direction
  double defocus_angle = 0; // Variation of angle of rays through each pixel
  double focus_dist = 10;   // Distance from camera to plane of focus
  void render(const hittable &world) {
    initialize();

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int i = 0; i < image_height; i++) {
      std::clog << "\rScanlines remaining: " << (image_height - i) << ' '
                << std::flush;
      for (int j = 0; j < image_width; j++) {
        auto b4_time = std::chrono::system_clock::now();
        color pixel_color = color(0, 0, 0);

        auto ray_start = std::chrono::system_clock::now();
        for (int sample = 0; sample < samples_per_pixel; sample++) {
          ray r = get_ray(j, i);
          pixel_color += ray_color(r, max_depth, world);
        }
        auto ray_end = std::chrono::system_clock::now();

        write_color(std::cout, pixel_samples_scale * pixel_color);
        auto after_time = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            after_time - b4_time);
        if (ms.count() > 1) {
          // std::clog << "Pixel time: " << ms << ", ray time: "
          //           << std::chrono::duration_cast<std::chrono::milliseconds>(
          //                  ray_end - ray_start)
          //           << "\n";
        }
      }
    }
    std::clog << "\rDone.                 \n";
  }

private:
  int image_height;           // Rendered image height
  double pixel_samples_scale; // Color scale factor for sum of samples
  point3 center;              // Camera location
  point3 pixel00_loc;         // Location of pixel 0, 0
  vec3 pixel_delta_u,
      pixel_delta_v; // Offset of pixel to the right and below respectively
  vec3 u, v, w;      // Frame basis vectors
  vec3 defocus_disk_u,
      defocus_disk_v; // Defocus disk horizontal and vertical radius
  void initialize() {
    image_height = int(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    pixel_samples_scale = 1.0 / samples_per_pixel;

    center = lookfrom;

    auto theta = degrees_to_radians(v_fov);
    auto h = std::tan(theta / 2);
    auto viewport_height = 2 * h * focus_dist;
    auto viewport_width =
        viewport_height * (double(image_width) / image_height);

    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(v_up, w));
    v = cross(w, u);

    vec3 viewport_u = viewport_width * u;
    vec3 viewport_v = viewport_height * -v;

    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    auto viewport_upper_left =
        center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    auto defocus_radius =
        focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
  }

  ray get_ray(int x, int y) {
    auto ray_start = std::chrono::system_clock::now();
    auto offset = sample_square();
    auto pixel_sample = pixel00_loc + ((x + offset.x()) * pixel_delta_u) +
                        ((y + offset.y()) * pixel_delta_v);
    auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
    auto ray_direction = pixel_sample - ray_origin;

    auto ray_end = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ray_end -
                                                                    ray_start);
    if (ms.count() > 1) {
      std::clog << "Get_ray time: " << ms << "\n";
    }
    return ray(ray_origin, ray_direction);
  }

  vec3 sample_square() {
    return vec3(random_double() - 0.5, random_double() - 0.5, 0);
  }

  point3 defocus_disk_sample() const {
    auto p = random_in_unit_disk();
    return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
  }

  color ray_color(const ray &r, int depth, const hittable &world) const {
    if (depth <= 0)
      return color(0, 0, 0);

    hit_record rec;

    auto color_start = std::chrono::system_clock::now();
    if (world.hit(r, interval(0.001, infinity), rec)) {
      ray scattered;
      color attenuation;
      if (rec.mat->scatter(r, rec, attenuation, scattered))
        return attenuation * ray_color(scattered, depth - 1, world);
      return color(0, 0, 0);
    }
    auto color_end = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(color_end -
                                                                    color_start);
    if (ms.count() > 1) {
      std::clog << "Color time: " << ms << "\n";
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
  }
};

#endif

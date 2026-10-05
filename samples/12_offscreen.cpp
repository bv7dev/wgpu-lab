// 12: rendering into a texture and using it in a second pass
//     the scene is rendered with few pixels and shown magnified, press S to save it as an image

#include <lab>

#include <iostream>

int main() {
  lab::Gpu gpu;
  lab::Window window("offscreen - press S to save the small image", 960, 600);
  lab::Surface surface(gpu, window);

  // a texture can be rendered into just like a surface
  lab::Texture small_image(gpu, wgpu::TextureFormat::RGBA8Unorm, 96, 60);

  // the scene, a pipeline made for the texture
  lab::Buffer<float> time_buffer(gpu, {0.0f}, wgpu::BufferUsage::Uniform);
  lab::Shader scene_shader(gpu, "shaders/12_offscreen_scene.wgsl");
  lab::Pipeline scene_pipeline(gpu, scene_shader, {.target = small_image});
  lab::Draw draw_scene{.bind_groups = {scene_pipeline.bind_group(0, {{0, time_buffer}})}, .count = 3};

  // showing the texture in the window, a pipeline made for the surface
  // (a "nearest" sampler does not blend between pixels, so they stay visible as blocks)
  lab::Shader show_shader(gpu, "shaders/12_offscreen_show.wgsl");
  lab::Pipeline show_pipeline(gpu, show_shader, {.target = surface});
  wgpu::Sampler blocky = lab::sampler(gpu, {.filter = wgpu::FilterMode::Nearest});
  lab::Draw draw_image{.bind_groups = {show_pipeline.bind_group(0, {{0, small_image}, {1, blocky}})}, .count = 3};

  while (lab::tick()) {
    if (window.key_pressed(lab::KeyCode::escape)) {
      window.close();
    }
    if (window.key_pressed(lab::KeyCode::S)) {
      small_image.save_png("offscreen.png"); // what the previous frame rendered into it
      std::cout << "saved offscreen.png" << std::endl;
    }

    time_buffer.write(lab::elapsed_seconds());

    // one frame, two passes: the first one renders into the texture, the second one uses it
    lab::Frame frame(gpu);
    {
      lab::RenderPass scene_pass(frame, small_image, {.clear = wgpu::Color{0.10, 0.10, 0.20, 1.0}});
      scene_pass.draw(scene_pipeline, draw_scene);
    }
    lab::RenderPass window_pass(frame, surface);
    window_pass.draw(show_pipeline, draw_image);
  }
}

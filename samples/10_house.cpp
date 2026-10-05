// 10: a 3D scene: depth buffer, camera, lighting, a model file and an image file
//     drag with the left mouse button to look around, scroll to zoom

// glm then produces depth values from 0 to 1, as WebGPU expects them (OpenGL uses -1 to 1)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// tinygltf reads the model, the stb_image that comes with it reads the texture
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <lab>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <format>
#include <vector>

using enum wgpu::VertexFormat;

struct ModelVertex {
  glm::vec3 position;
  glm::vec3 normal;
};

struct GroundVertex {
  glm::vec3 position;
  glm::vec2 uv;
};

// has to match `struct Scene` in both shaders
struct Scene {
  glm::mat4 view_projection;
  glm::vec4 light_direction;
};

struct Model {
  std::vector<ModelVertex> vertices;
  std::vector<uint16_t> indices;
};

// Reads positions, normals and indices of the first mesh in a binary glTF file
Model load_model(const std::filesystem::path& path) {
  tinygltf::TinyGLTF loader;
  tinygltf::Model gltf;
  std::string error, warning;
  if (!loader.LoadBinaryFromFile(&gltf, &error, &warning, path.string())) {
    throw lab::Error(std::format("{}: {}", path.string(), error));
  }
  const tinygltf::Primitive& primitive = gltf.meshes.at(0).primitives.at(0);

  // an accessor describes a run of typed elements inside one of the binary buffers of the file
  auto bytes_of = [&gltf](const tinygltf::Accessor& accessor) {
    const tinygltf::BufferView& view = gltf.bufferViews.at(accessor.bufferView);
    return gltf.buffers.at(view.buffer).data.data() + view.byteOffset + accessor.byteOffset;
  };
  const tinygltf::Accessor& positions = gltf.accessors.at(primitive.attributes.at("POSITION"));
  const tinygltf::Accessor& normals = gltf.accessors.at(primitive.attributes.at("NORMAL"));
  const tinygltf::Accessor& indices = gltf.accessors.at(primitive.indices);
  if (indices.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
    throw lab::Error(std::format("{}: this sample only reads 16 bit indices", path.string()));
  }

  Model model;
  model.vertices.resize(positions.count);
  for (size_t i = 0; i < positions.count; ++i) {
    std::memcpy(&model.vertices[i].position, bytes_of(positions) + i * sizeof(glm::vec3), sizeof(glm::vec3));
    std::memcpy(&model.vertices[i].normal, bytes_of(normals) + i * sizeof(glm::vec3), sizeof(glm::vec3));
  }
  model.indices.resize(indices.count);
  std::memcpy(model.indices.data(), bytes_of(indices), indices.count * sizeof(uint16_t));
  return model;
}

// Reads an image file into a texture
lab::Texture load_texture(lab::Gpu& gpu, const std::filesystem::path& path) {
  int width, height, channels;
  stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, 4); // 4: always as RGBA
  if (!pixels) {
    throw lab::Error(std::format("{}: {}", path.string(), stbi_failure_reason()));
  }
  lab::Texture texture(gpu, wgpu::TextureFormat::RGBA8Unorm, width, height);
  texture.write(std::span<const uint32_t>{reinterpret_cast<const uint32_t*>(pixels), size_t(width) * height});
  stbi_image_free(pixels);
  return texture;
}

int main() {
  lab::Gpu gpu;
  lab::Window window("drag to look around, scroll to zoom", 900, 600);

  // with a depth buffer, near surfaces hide far ones, whatever the order they are drawn in
  lab::Surface surface(gpu, window, {.depth = wgpu::TextureFormat::Depth24Plus});

  // the house
  const Model model = load_model(lab::find_file("assets/house.glb"));
  lab::Buffer model_vertices(gpu, model.vertices);
  lab::Buffer model_indices(gpu, model.indices, wgpu::BufferUsage::Index);

  // the ground: two triangles, the texture is repeated 12 times across them
  const float size = 6.0f, repeat = 12.0f;
  std::vector<GroundVertex> ground = {
      {{-size, 0.0f, -size}, {0.0f, 0.0f}},     {{+size, 0.0f, -size}, {repeat, 0.0f}},
      {{+size, 0.0f, +size}, {repeat, repeat}}, {{-size, 0.0f, -size}, {0.0f, 0.0f}},
      {{+size, 0.0f, +size}, {repeat, repeat}}, {{-size, 0.0f, +size}, {0.0f, repeat}},
  };
  lab::Buffer ground_vertices(gpu, ground);
  lab::Texture ground_texture = load_texture(gpu, lab::find_file("assets/ground.png"));

  // camera and light are the same for both pipelines
  Scene scene{};
  lab::Buffer<Scene> scene_buffer(gpu, {scene}, wgpu::BufferUsage::Uniform);

  // the surface has a depth buffer, so pipelines made for it test against it
  lab::Shader model_shader(gpu, "shaders/10_house.wgsl");
  lab::Pipeline model_pipeline(
      gpu, model_shader, {.vertex_buffers = {lab::vertex<ModelVertex>({Float32x3, Float32x3})}, .target = surface});

  lab::Shader ground_shader(gpu, "shaders/10_house_ground.wgsl");
  lab::Pipeline ground_pipeline(
      gpu, ground_shader, {.vertex_buffers = {lab::vertex<GroundVertex>({Float32x3, Float32x2})}, .target = surface});

  lab::Draw draw_model{
      .vertex_buffers = {model_vertices},
      .index_buffer = model_indices,
      .bind_groups = {model_pipeline.bind_group(0, {{0, scene_buffer}})},
  };
  lab::Draw draw_ground{
      .vertex_buffers = {ground_vertices},
      .bind_groups = {ground_pipeline.bind_group(0, {{0, scene_buffer}, {1, ground_texture}, {2, lab::sampler(gpu)}})},
  };

  // the camera circles around a point above the ground
  float yaw = 0.6f, pitch = 0.35f, distance = 7.0f;

  while (lab::tick()) {
    if (window.key_pressed(lab::KeyCode::escape)) {
      window.close();
    }

    if (window.mouse_button(lab::MouseButton::left)) {
      yaw -= window.mouse_delta().x * 0.005f;
      pitch = std::clamp(pitch + window.mouse_delta().y * 0.005f, 0.05f, 1.5f);
    } else {
      yaw += 0.2f * lab::delta_seconds(); // left alone, the camera keeps circling slowly
    }
    distance = std::clamp(distance - window.scroll().y * 0.5f, 3.0f, 15.0f);

    const glm::vec3 target{0.0f, 1.0f, 0.0f};
    const glm::vec3 eye = target + distance * glm::vec3{std::cos(pitch) * std::sin(yaw), std::sin(pitch),
                                                        std::cos(pitch) * std::cos(yaw)};
    const glm::mat4 view = glm::lookAt(eye, target, glm::vec3{0.0f, 1.0f, 0.0f});
    const glm::mat4 projection = glm::perspective(glm::radians(45.0f), surface.aspect(), 0.1f, 100.0f);

    scene.view_projection = projection * view;
    scene.light_direction = glm::vec4{glm::normalize(glm::vec3{-0.5f, -1.0f, -0.3f}), 0.0f};
    scene_buffer.write(scene);

    lab::RenderPass pass(surface, {.clear = wgpu::Color{0.55, 0.70, 0.90, 1.0}}); // the sky
    pass.draw(ground_pipeline, draw_ground);
    pass.draw(model_pipeline, draw_model);
  }
}

#include <lab>

int main() {
  lab::Webgpu webgpu("My Instance");
  lab::Shader shader("My Shader", "shaders/vbuf2.wgsl");
  lab::Pipeline pipeline(shader, webgpu);

  lab::Window window("Vertex Buffer Demo", 640, 400);
  lab::Surface surface(window, webgpu);

  std::vector<float> vertex_data = {-0.5f,  -0.5f, 1.0f, +0.5f,  -0.5f, 0.6f, +0.0f,  0.5f, 0.2f,
                                    -0.55f, -0.5f, 1.0f, -0.05f, +0.5f, 0.6f, -0.55f, 0.5f, 0.2f};
  uint32_t vertex_count = static_cast<uint32_t>(vertex_data.size() / 3);

  lab::Buffer<float> vertex_buffer("My vertex buffer", vertex_data, webgpu);

  // WIP -------------

  wgpu::TextureDescriptor textureDesc;

  // [...] setup descriptor
  textureDesc.dimension = wgpu::TextureDimension::_2D;
  textureDesc.size = {256, 256, 1};
  //                             ^ ignored because it is a 2D texture
  textureDesc.mipLevelCount = 1;
  textureDesc.sampleCount = 1;
  textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
  textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
  textureDesc.viewFormatCount = 0;
  textureDesc.viewFormats = nullptr;

  wgpu::Texture texture = webgpu.device.createTexture(textureDesc);

  // Create image data
  std::vector<uint8_t> pixels(4 * textureDesc.size.width * textureDesc.size.height);
  for (uint32_t i = 0; i < textureDesc.size.width; ++i) {
    for (uint32_t j = 0; j < textureDesc.size.height; ++j) {
      uint8_t* p = &pixels[4 * (j * textureDesc.size.width + i)];
      p[0] = (uint8_t)i; // r
      p[1] = (uint8_t)j; // g
      p[2] = 128;        // b
      p[3] = 255;        // a
    }
  }
  // Arguments telling which part of the texture we upload to
  // (together with the last argument of writeTexture)
  wgpu::ImageCopyTexture destination;
  destination.texture = texture;
  destination.mipLevel = 0;
  destination.origin = {0, 0, 0}; // equivalent of the offset argument of Queue::writeBuffer
  destination.aspect = wgpu::TextureAspect::All; // only relevant for depth/Stencil textures

  // Arguments telling how the C++ side pixel memory is laid out
  wgpu::TextureDataLayout source;
  source.offset = 0;
  source.bytesPerRow = 4 * textureDesc.size.width;
  source.rowsPerImage = textureDesc.size.height;

  webgpu.queue.writeTexture(destination, pixels.data(), pixels.size(), source, textureDesc.size);

  // [...] do stuff with texture

  texture.destroy();
  texture.release();
  // -----------------

  pipeline.add_vertex_buffer(vertex_buffer.wgpu_buffer);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32x2, 0);
  pipeline.add_vertex_attribute(wgpu::VertexFormat::Float32, 1, 2 * sizeof(float));

  pipeline.finalize();

  while (lab::tick()) {
    pipeline.render_frame(surface, {vertex_count, 1});
    lab::sleep(50ms);
  }
}

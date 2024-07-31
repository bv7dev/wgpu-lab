#include <objects/lab_shader.h>
#include <objects/lab_webgpu.h>

#include <GLFW/glfw3.h>
#include <webgpu/webgpu.hpp>

// https://eliemichel.github.io/LearnWebGPU/basic-3d-rendering/input-geometry/a-first-vertex-attribute.html
// copied and modified to work with wgpu-lab
// from tutorial branch - step032-vanilla

namespace tutorial {

// We embbed the source of the shader module here
const char* shaderSource = R"(
@vertex
fn vs_main(@location(0) in_vertex_position: vec2f) -> @builtin(position) vec4f {
	return vec4f(in_vertex_position, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
	return vec4f(0.0, 0.4, 1.0, 1.0);
}
)";

WGPUTextureView GetNextSurfaceTextureView(WGPUSurface surface) {
  // Get the surface texture
  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    return nullptr;
  }

  // Create a view for this surface texture
  WGPUTextureViewDescriptor viewDescriptor;
  viewDescriptor.nextInChain = nullptr;
  viewDescriptor.label = "Surface texture view";
  viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
  viewDescriptor.dimension = WGPUTextureViewDimension_2D;
  viewDescriptor.baseMipLevel = 0;
  viewDescriptor.mipLevelCount = 1;
  viewDescriptor.baseArrayLayer = 0;
  viewDescriptor.arrayLayerCount = 1;
  viewDescriptor.aspect = WGPUTextureAspect_All;
  WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

  return targetView;
}

bool IsRunning(GLFWwindow* window) { return !glfwWindowShouldClose(window); }

void MainLoop(lab::Webgpu& webgpu, WGPUSurface surface, WGPURenderPipeline pipeline,
              WGPUBuffer vertexBuffer, uint32_t vertexCount) {
  // Get the next target texture view
  WGPUTextureView targetView = GetNextSurfaceTextureView(surface);
  if (!targetView) return;

  // Create a command encoder for the draw call
  WGPUCommandEncoderDescriptor encoderDesc = {};
  encoderDesc.nextInChain = nullptr;
  encoderDesc.label = "My command encoder";
  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(webgpu.device, &encoderDesc);

  // Create the render pass that clears the screen with our color
  WGPURenderPassDescriptor renderPassDesc = {};
  renderPassDesc.nextInChain = nullptr;

  // The attachment part of the render pass descriptor describes the target texture of the pass
  WGPURenderPassColorAttachment renderPassColorAttachment = {};
  renderPassColorAttachment.view = targetView;
  renderPassColorAttachment.resolveTarget = nullptr;
  renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
  renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
  renderPassColorAttachment.clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0};
#ifndef WEBGPU_BACKEND_WGPU
  renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

  renderPassDesc.colorAttachmentCount = 1;
  renderPassDesc.colorAttachments = &renderPassColorAttachment;
  renderPassDesc.depthStencilAttachment = nullptr;
  renderPassDesc.timestampWrites = nullptr;

  WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

  // Select which render pipeline to use
  wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);

  // Set vertex buffer while encoding the render pass
  wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0,
                                       wgpuBufferGetSize(vertexBuffer));

  // We use the `vertexCount` variable instead of hard-coding the vertex count
  wgpuRenderPassEncoderDraw(renderPass, vertexCount, 1, 0, 0);

  wgpuRenderPassEncoderEnd(renderPass);
  wgpuRenderPassEncoderRelease(renderPass);

  // Encode and submit the render pass
  WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
  cmdBufferDescriptor.nextInChain = nullptr;
  cmdBufferDescriptor.label = "Command buffer";
  WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
  wgpuCommandEncoderRelease(encoder);

  wgpuQueueSubmit(webgpu.queue, 1, &command);
  wgpuCommandBufferRelease(command);

  // At the end of the frame
  wgpuTextureViewRelease(targetView);
  wgpuSurfacePresent(surface);
  wgpuDeviceTick(webgpu.device);
}

auto InitializePipeline(lab::Webgpu& webgpu, lab::Shader& shader) {
  // Load the shader module
  WGPUShaderModuleDescriptor shaderDesc{};

  // We use the extension mechanism to specify the WGSL part of the shader module descriptor
  WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
  // Set the chained struct's header
  shaderCodeDesc.chain.next = nullptr;
  shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
  // Connect the chain
  shaderDesc.nextInChain = &shaderCodeDesc.chain;
  shaderCodeDesc.code = shader.source.c_str();

  WGPUShaderModule shaderModule =
      wgpuDeviceCreateShaderModule(*((WGPUDevice*)&webgpu.device), &shaderDesc);

  // Create the render pipeline
  WGPURenderPipelineDescriptor pipelineDesc{};
  pipelineDesc.nextInChain = nullptr;

  // Configure the vertex pipeline
  // We use one vertex buffer
  WGPUVertexBufferLayout vertexBufferLayout{};
  WGPUVertexAttribute positionAttrib;
  // == For each attribute, describe its layout, i.e., how to interpret the raw data ==
  // Corresponds to @location(...)
  positionAttrib.shaderLocation = 0;
  // Means vec2f in the shader
  positionAttrib.format = WGPUVertexFormat_Float32x2;
  // Index of the first element
  positionAttrib.offset = 0;

  vertexBufferLayout.attributeCount = 1;
  vertexBufferLayout.attributes = &positionAttrib;

  // == Common to attributes from the same buffer ==
  vertexBufferLayout.arrayStride = 2 * sizeof(float);
  vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;

  pipelineDesc.vertex.bufferCount = 1;
  pipelineDesc.vertex.buffers = &vertexBufferLayout;

  // NB: We define the 'shaderModule' in the second part of this chapter.
  // Here we tell that the programmable vertex shader stage is described
  // by the function called 'vs_main' in that module.
  pipelineDesc.vertex.module = shaderModule;
  pipelineDesc.vertex.entryPoint = "vs_main";
  pipelineDesc.vertex.constantCount = 0;
  pipelineDesc.vertex.constants = nullptr;

  // Each sequence of 3 vertices is considered as a triangle
  pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;

  // We'll see later how to specify the order in which vertices should be
  // connected. When not specified, vertices are considered sequentially.
  pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;

  // The face orientation is defined by assuming that when looking
  // from the front of the face, its corner vertices are enumerated
  // in the counter-clockwise (CCW) order.
  pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;

  // But the face orientation does not matter much because we do not
  // cull (i.e. "hide") the faces pointing away from us (which is often
  // used for optimization).
  pipelineDesc.primitive.cullMode = WGPUCullMode_None;

  // We tell that the programmable fragment shader stage is described
  // by the function called 'fs_main' in the shader module.
  WGPUFragmentState fragmentState{};
  fragmentState.module = shaderModule;
  fragmentState.entryPoint = "fs_main";
  fragmentState.constantCount = 0;
  fragmentState.constants = nullptr;

  WGPUBlendState blendState{};
  blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
  blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
  blendState.color.operation = WGPUBlendOperation_Add;
  blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
  blendState.alpha.dstFactor = WGPUBlendFactor_One;
  blendState.alpha.operation = WGPUBlendOperation_Add;

  WGPUColorTargetState colorTarget{};
  colorTarget.format = webgpu.capabilities.formats[0];
  colorTarget.blend = &blendState;
  colorTarget.writeMask =
      WGPUColorWriteMask_All; // We could write to only some of the color channels.

  // We have only one target because our render pass has only one output color
  // attachment.
  fragmentState.targetCount = 1;
  fragmentState.targets = &colorTarget;
  pipelineDesc.fragment = &fragmentState;

  // We do not use stencil/depth testing for now
  pipelineDesc.depthStencil = nullptr;

  // Samples per pixel
  pipelineDesc.multisample.count = 1;

  // Default value for the mask, meaning "all bits on"
  pipelineDesc.multisample.mask = ~0u;

  // Default value as well (irrelevant for count = 1 anyways)
  pipelineDesc.multisample.alphaToCoverageEnabled = false;

  pipelineDesc.layout = nullptr;

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(webgpu.device, &pipelineDesc);

  // We no longer need to access the shader module
  wgpuShaderModuleRelease(shaderModule);

  return pipeline;
}

// If you do not use webgpu.hpp, I suggest you create a function to init the
// WGPULimits structure:
void setDefault(WGPULimits& limits) {
  limits.maxTextureDimension1D = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxTextureDimension2D = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxTextureDimension3D = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxTextureArrayLayers = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxBindGroups = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxBindGroupsPlusVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxBindingsPerBindGroup = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxDynamicUniformBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxDynamicStorageBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxSampledTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxSamplersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxStorageBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxStorageTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxUniformBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxUniformBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
  limits.maxStorageBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
  limits.minUniformBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
  limits.minStorageBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxBufferSize = WGPU_LIMIT_U64_UNDEFINED;
  limits.maxVertexAttributes = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxVertexBufferArrayStride = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxInterStageShaderComponents = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxComputeInvocationsPerWorkgroup = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED;
  limits.maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED;
}

WGPURequiredLimits GetRequiredLimits(WGPUAdapter adapter) {
  // Get adapter supported limits, in case we need them
  WGPUSupportedLimits supportedLimits;
  supportedLimits.nextInChain = nullptr;
  wgpuAdapterGetLimits(adapter, &supportedLimits);

  WGPURequiredLimits requiredLimits{};
  setDefault(requiredLimits.limits);

  // We use at most 1 vertex attribute for now
  requiredLimits.limits.maxVertexAttributes = 1;
  // We should also tell that we use 1 vertex buffers
  requiredLimits.limits.maxVertexBuffers = 1;
  // Maximum size of a buffer is 6 vertices of 2 float each
  requiredLimits.limits.maxBufferSize = 6 * 2 * sizeof(float);
  // Maximum stride between 2 consecutive vertices in the vertex buffer
  requiredLimits.limits.maxVertexBufferArrayStride = 2 * sizeof(float);

  // These two limits are different because they are "minimum" limits,
  // they are the only ones we are may forward from the adapter's supported
  // limits.
  requiredLimits.limits.minUniformBufferOffsetAlignment =
      supportedLimits.limits.minUniformBufferOffsetAlignment;
  requiredLimits.limits.minStorageBufferOffsetAlignment =
      supportedLimits.limits.minStorageBufferOffsetAlignment;

  return requiredLimits;
}

struct InitializeBuffersReturn {
  WGPUBuffer vertexBuffer;
  uint32_t vertexCount;
};

InitializeBuffersReturn InitializeBuffers(lab::Webgpu& webgpu) {
  // Vertex buffer data
  // There are 2 floats per vertex, one for x and one for y.
  std::vector<float> vertexData = {// Define a first triangle:
                                   -0.5, -0.5, +0.5, -0.5, +0.0, +0.5,

                                   // Add a second triangle:
                                   -0.55f, -0.5, -0.05f, +0.5, -0.55f, +0.5};
  uint32_t vertexCount = static_cast<uint32_t>(vertexData.size() / 2);

  // Create vertex buffer
  WGPUBufferDescriptor bufferDesc{};
  bufferDesc.nextInChain = nullptr;
  bufferDesc.size = vertexData.size() * sizeof(float);
  bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex; // Vertex usage here!
  bufferDesc.mappedAtCreation = false;
  WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(webgpu.device, &bufferDesc);

  // Upload geometry data to the buffer
  wgpuQueueWriteBuffer(webgpu.queue, vertexBuffer, 0, vertexData.data(), bufferDesc.size);

  return {vertexBuffer, vertexCount};
}

} // namespace tutorial

#ifndef WEBGPU_UTILS_H
#define WEBGPU_UTILS_H

#include <webgpu/webgpu.h>

/**
 * Utility function to get a WebGPU adapter, so that
 *     WGPUAdapter adapter = requestAdapter(options);
 * is roughly equivalent to
 *     const adapter = await navigator.gpu.requestAdapter(options);
 */
WGPUAdapter requestAdapterSync(WGPUInstance instance,
                               WGPURequestAdapterOptions const* options);

/**
 * Utility function to get a WebGPU device, so that
 *     WGPUAdapter device = requestDevice(adapter, options);
 * is roughly equivalent to
 *     const device = await adapter.requestDevice(descriptor);
 * It is very similar to requestAdapter
 */
WGPUDevice requestDeviceSync(WGPUAdapter adapter,
                             WGPUDeviceDescriptor const* descriptor);

/**
 * An example of how we can inspect the capabilities of the hardware through
 * the adapter object.
 */
void inspectAdapter(WGPUAdapter adapter);

/**
 * Display information about a device
 */
void inspectDevice(WGPUDevice device);

#endif // WEBGPU_UTILS_H

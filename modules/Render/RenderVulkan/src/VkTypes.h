/*
 * Copyright 2018-2019 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Core/Types.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/RenderVulkan/RendererIDs.h>

#include <MSL/Client/ModuleC.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>

#define DS_NOT_SUBMITTED (uint64_t)-1
#define DS_DELAY_FRAMES 3
#define DS_EXPECTED_FRAME_FLUSHES 2
#define DS_MAX_SUBMITS (DS_DELAY_FRAMES*DS_EXPECTED_FRAME_FLUSHES)
#define DS_PENDING_RESOURCES_ARRAY 2
#define DS_DELETE_RESOURCES_ARRAY 2
// 10 seconds in nanoseconds
#define DS_DEFAULT_WAIT_TIMEOUT 10000000000
#define DS_MAX_DYNAMIC_STATES VK_DYNAMIC_STATE_STENCIL_REFERENCE + 1
#define DS_COMMAND_BUFFER_CHUNK_SIZE 20
#define DS_RECENTLY_ADDED_SIZE 10
#define DS_TEMP_BUFFER_CAPACITY 524288
#define DS_MAX_TEMP_BUFFER_ALLOC 262144

typedef struct dsVkInstance
{
	dsDynamicLib library;

	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	PFN_vkCreateInstance vkCreateInstance;
	PFN_vkDestroyInstance vkDestroyInstance;
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
	PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR;
	PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
	PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
	PFN_vkCreateDevice vkCreateDevice;
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
	PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;

	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;

	PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;

	VkDebugUtilsMessengerEXT debugCallback;
	VkDebugReportCallbackEXT oldDebugCallback;

	VkAllocationCallbacks allocCallbacks;
	const VkAllocationCallbacks* allocCallbacksPtr;
	VkInstance instance;
} dsVkInstance;

typedef struct dsVkDevice
{
	dsVkInstance instance;

	PFN_vkDestroyDevice vkDestroyDevice;
	PFN_vkGetDeviceQueue vkGetDeviceQueue;

	PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
	PFN_vkQueuePresentKHR vkQueuePresentKHR;

	PFN_vkCreateCommandPool vkCreateCommandPool;
	PFN_vkResetCommandPool vkResetCommandPool;
	PFN_vkDestroyCommandPool vkDestroyCommandPool;
	PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
	PFN_vkResetCommandBuffer vkResetCommandBuffer;
	PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
	PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
	PFN_vkEndCommandBuffer vkEndCommandBuffer;
	PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
	PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
	PFN_vkQueueSubmit vkQueueSubmit;
	PFN_vkQueueWaitIdle vkQueueWaitIdle;

	PFN_vkCreateFence vkCreateFence;
	PFN_vkDestroyFence vkDestroyFence;
	PFN_vkResetFences vkResetFences;

	PFN_vkCreateSemaphore vkCreateSemaphore;
	PFN_vkDestroySemaphore vkDestroySemaphore;

	PFN_vkWaitForFences vkWaitForFences;
	PFN_vkAllocateMemory vkAllocateMemory;
	PFN_vkFreeMemory vkFreeMemory;
	PFN_vkMapMemory vkMapMemory;
	PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
	PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
	PFN_vkUnmapMemory vkUnmapMemory;

	PFN_vkCreateBuffer vkCreateBuffer;
	PFN_vkDestroyBuffer vkDestroyBuffer;
	PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
	PFN_vkBindBufferMemory vkBindBufferMemory;
	PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
	PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
	PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
	PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
	PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
	PFN_vkCreateBufferView vkCreateBufferView;
	PFN_vkDestroyBufferView vkDestroyBufferView;

	PFN_vkCreateImage vkCreateImage;
	PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
	PFN_vkDestroyImage vkDestroyImage;
	PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
	PFN_vkBindImageMemory vkBindImageMemory;
	PFN_vkCmdCopyImage vkCmdCopyImage;
	PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
	PFN_vkCmdBlitImage vkCmdBlitImage;
	PFN_vkCmdClearColorImage vkCmdClearColorImage;
	PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
	PFN_vkCmdResolveImage vkCmdResolveImage;
	PFN_vkCreateImageView vkCreateImageView;
	PFN_vkDestroyImageView vkDestroyImageView;

	PFN_vkCreateFramebuffer vkCreateFramebuffer;
	PFN_vkDestroyFramebuffer vkDestroyFramebuffer;

	PFN_vkCreateRenderPass vkCreateRenderPass;
	PFN_vkDestroyRenderPass vkDestroyRenderPass;
	PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
	PFN_vkCmdNextSubpass vkCmdNextSubpass;
	PFN_vkCmdEndRenderPass vkCmdEndRenderPass;

	PFN_vkCreateQueryPool vkCreateQueryPool;
	PFN_vkDestroyQueryPool vkDestroyQueryPool;
	PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
	PFN_vkCmdBeginQuery vkCmdBeginQuery;
	PFN_vkCmdEndQuery vkCmdEndQuery;
	PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
	PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
	PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;

	PFN_vkCreateShaderModule vkCreateShaderModule;
	PFN_vkDestroyShaderModule vkDestroyShaderModule;

	PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
	PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;

	PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
	PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
	PFN_vkResetDescriptorPool vkResetDescriptorPool;
	PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
	PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
	PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
	PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;

	PFN_vkCreateSampler vkCreateSampler;
	PFN_vkDestroySampler vkDestroySampler;

	PFN_vkCreatePipelineCache vkCreatePipelineCache;
	PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
	PFN_vkGetPipelineCacheData vkGetPipelineCacheData;

	PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
	PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;

	PFN_vkCreateComputePipelines vkCreateComputePipelines;
	PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
	PFN_vkCmdBindPipeline vkCmdBindPipeline;
	PFN_vkDestroyPipeline vkDestroyPipeline;

	PFN_vkCmdPushConstants vkCmdPushConstants;
	PFN_vkCmdSetViewport vkCmdSetViewport;
	PFN_vkCmdSetScissor vkCmdSetScissor;
	PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
	PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
	PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
	PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
	PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
	PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
	PFN_vkCmdSetStencilReference vkCmdSetStencilReference;

	PFN_vkCmdDraw vkCmdDraw;
	PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
	PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
	PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
	PFN_vkCmdDispatch vkCmdDispatch;
	PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;

	PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBeginEXT;
	PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEndEXT;

	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue queue;
	uint32_t queueFamilyIndex;

	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceProperties properties;
	bool hasPVRTC;
	bool hasLazyAllocation;

	VkPhysicalDeviceMemoryProperties memoryProperties;
} dsVkDevice;

typedef struct dsVkFormatInfo
{
	VkFormat vkFormat;
	VkFormatProperties properties;
} dsVkFormatInfo;

typedef struct dsVkResource
{
	dsSpinlock lock;
	uint64_t lastUsedSubmit;
	uint32_t commandBufferCount;
} dsVkResource;

typedef struct dsVkDirtyRange
{
	size_t start;
	size_t size;
} dsVkDirtyRange;

typedef struct dsVkBufferView
{
	VkBufferView bufferView;
	dsGfxFormat format;
	size_t offset;
	size_t count;
} dsVkBufferView;

typedef struct dsVkGfxBufferData
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	dsAllocator* scratchAllocator;
	dsLifetime* lifetime;

	dsVkResource resource;

	VkDeviceMemory deviceMemory;
	VkBuffer deviceBuffer;

	VkDeviceMemory hostMemory;
	VkBuffer hostBuffer;
	uint64_t uploadedSubmit;
	void* submitQueue;

	dsGfxBufferUsage usage;
	dsGfxMemory memoryHints;
	size_t size;

	dsVkDirtyRange* dirtyRanges;
	uint32_t dirtyRangeCount;
	uint32_t maxDirtyRanges;

	dsSpinlock bufferViewLock;
	dsVkBufferView* bufferViews;
	uint32_t bufferViewCount;
	uint32_t maxBufferViews;

	size_t mappedStart;
	size_t mappedSize;
	bool mappedWrite;

	bool keepHost;
	bool hostMemoryCoherent;
	bool used;
	bool needsInitialCopy;
} dsVkGfxBufferData;

typedef struct dsVkGfxBuffer
{
	dsGfxBuffer buffer;
	dsSpinlock lock;
	dsVkGfxBufferData* bufferData;
} dsVkGfxBuffer;

typedef struct dsVkDrawGeometry
{
	dsDrawGeometry drawGeometry;
	uint32_t vertexHash;
} dsVkDrawGeometry;

typedef struct dsVkTempBuffer
{
	dsVkResource resource;
	dsAllocator* allocator;
	dsVkDevice* device;
	VkBuffer buffer;
	VkDeviceMemory memory;
	bool coherent;
	uint8_t* contents;
	size_t size;
	size_t capacity;
} dsVkTempBuffer;

typedef struct dsVkTexture
{
	dsTexture texture;
	dsVkResource resource;
	dsLifetime* lifetime;

	VkDeviceMemory deviceMemory;
	VkImage deviceImage;
	VkImageView deviceImageView;

	VkDeviceMemory hostMemory;
	VkDeviceSize hostMemorySize;
	bool hostMemoryCoherent;
	VkBuffer hostBuffer;
	uint64_t uploadedSubmit;
	void* submitQueue;

	VkDeviceMemory surfaceMemory;
	VkImage surfaceImage;
	VkImageView surfaceImageView;
	uint64_t lastDrawSubmit;

	VkImageAspectFlags aspectMask;
	bool needsInitialCopy;
	bool hasSplitLayouts;
} dsVkTexture;

typedef struct dsVkRenderbuffer
{
	dsRenderbuffer renderbuffer;
	dsVkResource resource;

	VkDeviceMemory memory;
	VkImage image;
	VkImageView imageView;
} dsVkRenderbuffer;

typedef struct dsVkRenderSurfaceData dsVkRenderSurfaceData;

typedef struct dsVkRealFramebuffer
{
	dsAllocator* allocator;
	dsVkDevice* device;
	dsVkResource resource;

	VkFramebuffer* framebuffers;
	dsLifetime* renderPassData;
	const dsVkRenderSurfaceData* surfaceData;

	VkImageView* imageViews;
	bool* imageViewTemp;
	uint32_t imageCount;
	uint32_t framebufferCount;
} dsVkRealFramebuffer;

typedef struct dsVkFramebuffer
{
	dsFramebuffer framebuffer;
	dsAllocator* scratchAllocator;
	dsLifetime* lifetime;
	dsSpinlock lock;

	dsLifetime* renderSurface;

	dsVkRealFramebuffer** realFramebuffers;
	uint32_t framebufferCount;
	uint32_t maxFramebuffers;
} dsVkFramebuffer;

typedef struct dsVkGfxFence
{
	dsGfxFence fence;
	dsVkResource resource;
} dsVkGfxFence;

typedef struct dsVkGfxQueryPool
{
	dsGfxQueryPool queries;
	dsVkResource resource;
	VkQueryPool vkQueries;
} dsVkGfxQueryPool;

typedef struct dsVkSamplerList
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	dsVkResource resource;
	VkSampler* samplers;
	uint32_t samplerCount;
	float defaultAnisotropy;
} dsVkSamplerList;

typedef struct dsVkBindingCounts
{
	uint32_t textures;
	uint32_t buffers;
	uint32_t texelBuffers;
	uint32_t total;
} dsVkBindingCounts;

typedef struct dsVkTexelBufferBinding
{
	dsVkGfxBufferData* buffer;
	dsGfxFormat format;
	size_t offset;
	size_t count;
} dsVkTexelBufferBinding;

typedef struct dsVkGfxBufferBinding
{
	dsVkGfxBufferData* buffer;
	size_t offset;
	size_t size;
} dsVkGfxBufferBinding;

typedef struct dsVkMaterialDescriptor
{
	dsListNode node;
	dsRenderer* renderer;
	dsAllocator* allocator;
	dsVkResource resource;
	const dsMaterialDesc* materialDesc;
	const dsVkSamplerList* samplers; // Only used for comparison

	dsVkBindingCounts counts;
	dsMaterialBinding binding;

	dsTexture** textures;
	dsVkGfxBufferBinding* buffers;
	dsVkTexelBufferBinding* texelBuffers;

	VkDescriptorPool pool;
	VkDescriptorSet set;
} dsVkMaterialDescriptor;

typedef struct dsVkMaterialDescriptorRef
{
	dsVkMaterialDescriptor* descriptor;
	dsLifetime* shader;
} dsVkMaterialDescriptorRef;

typedef struct dsVkMaterialDescBindings
{
	uint32_t setIndex;
	VkDescriptorSetLayoutBinding* bindings;
	dsVkBindingCounts bindingCounts;
	VkDescriptorSetLayout descriptorSets;
	dsList descriptorFreeList;
	dsSpinlock lock;
} dsVkMaterialDescBindings;

typedef struct dsVkMaterialDesc
{
	dsMaterialDesc materialDesc;
	dsLifetime* lifetime;
	uint32_t* elementMappings;

	// Indices match dsMaterialBinding enum.
	dsVkMaterialDescBindings bindings[3];
} dsVkMaterialDesc;

typedef struct dsVkBindingMemory
{
	dsVkBindingCounts counts;

	dsTexture** textures;
	dsVkGfxBufferBinding* buffers;
	dsVkTexelBufferBinding* texelBuffers;

	VkWriteDescriptorSet* bindings;
	VkDescriptorImageInfo* imageInfos;
	VkDescriptorBufferInfo* bufferInfos;
	VkBufferView* bufferViews;
} dsVkBindingMemory;

struct dsDeviceMaterial
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	dsAllocator* scratchAllocator;
	dsMaterial* material;
	dsLifetime* lifetime;

	dsVkMaterialDescriptorRef* descriptors;
	uint32_t descriptorCount;
	uint32_t maxDescriptors;

	dsVkBindingMemory bindingMemory;

	dsSpinlock lock;
};

typedef struct dsVkComputePipeline
{
	dsAllocator* allocator;
	dsVkResource resource;
	dsVkDevice* device;
	VkPipeline pipeline;
} dsVkComputePipeline;

typedef struct dsVkPipeline
{
	dsAllocator* allocator;
	dsVkResource resource;
	dsVkDevice* device;

	VkPipeline pipeline;

	uint32_t hash;
	uint32_t samples;
	float defaultAnisotropy;
	uint32_t subpass;
	dsPrimitiveType primitiveType;
	dsVertexFormat formats[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
	dsLifetime* renderPass;
} dsVkPipeline;

typedef struct dsVkSamplerMapping
{
	uint32_t uniformIndex;
	uint32_t samplerIndex;
} dsVkSamplerMapping;

typedef struct dsVkPushConstantMapping
{
	uint32_t materialElement;
	uint32_t offset;
	uint32_t count;
	uint32_t stride;
} dsVkPushConstantMapping;

typedef struct dsVkShader
{
	dsShader shader;
	dsAllocator* scratchAllocator;
	dsLifetime* lifetime;
	mslPipeline pipeline;

	VkShaderStageFlags stages;
	VkShaderStageFlags pushConstantStages;
	bool computeUsesPushConstants;
	mslSizedData spirv[mslStage_Count];
	VkShaderModule shaders[mslStage_Count];
	VkPipelineLayout layout;
	VkPipelineLayout computeLayout;

	VkPipelineTessellationStateCreateInfo tessellationInfo;
	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkSampleMask sampleMask;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	VkPipelineColorBlendStateCreateInfo blendInfo;
	VkPipelineColorBlendAttachmentState attachments[DS_MAX_ATTACHMENTS];
	VkPipelineDynamicStateCreateInfo dynamicInfo;
	VkDynamicState dynamicStates[DS_MAX_DYNAMIC_STATES];

	bool dynamicLineWidth;
	bool dynamicDepthBias;
	bool dynamicBlendConstants;
	bool dynamicDepthBounds;
	bool dynamicStencilCompareMask;
	bool dynamicStencilWriteMask;
	bool dynamicStencilReference;

	float depthBiasConstantFactor;
	float depthBiasClamp;
	float depthBiasSlopeFactor;

	dsVkComputePipeline* computePipeline;

	dsVkPushConstantMapping* pushConstants;
	uint32_t pushConstantCount;
	uint32_t pushConstantSize;

	dsLifetime** usedMaterials;
	uint32_t usedMaterialCount;
	uint32_t maxUsedMaterials;

	dsLifetime** usedRenderPasses;
	uint32_t usedRenderPassCount;
	uint32_t maxUsedRenderPasses;

	dsVkPipeline** pipelines;
	uint32_t pipelineCount;
	uint32_t maxPipelines;

	dsVkSamplerList* samplers;
	dsVkSamplerMapping* samplerMapping;
	uint32_t samplerCount;
	bool samplersHaveDefaultAnisotropy;

	dsSpinlock materialLock;
	dsSpinlock pipelineLock;
	dsSpinlock samplerLock;
} dsVkShader;

typedef struct dsVkRenderPassData
{
	dsAllocator* allocator;
	dsVkResource resource;
	dsVkDevice* device;
	dsLifetime* lifetime;
	const dsRenderPass* renderPass;

	uint32_t* resolveIndices;
	bool* resolveAttachment;
	uint32_t attachmentCount;
	uint32_t fullAttachmentCount;

	VkRenderPass vkRenderPass;

	dsLifetime** usedShaders;
	uint32_t usedShaderCount;
	uint32_t maxUsedShaders;

	dsLifetime** usedFramebuffers;
	uint32_t usedFramebufferCount;
	uint32_t maxUsedFramebuffers;

	dsSpinlock shaderLock;
	dsSpinlock framebufferLock;
} dsVkRenderPassData;

typedef struct dsVkRenderPass
{
	dsRenderPass renderPass;
	dsAllocator* scratchAllocator;

	VkSubpassDependency* vkDependencies;

	uint64_t lastCheckedFrame;
	uint32_t defaultSamples;
	bool usesDefaultSamples;

	dsVkRenderPassData* renderPassData;
	dsSpinlock lock;
} dsVkRenderPass;

typedef struct dsVkSurfaceImageData
{
	VkSemaphore semaphore;
	uint64_t lastUsedSubmit;
} dsVkSurfaceImageData;

struct dsVkRenderSurfaceData
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsVkResource resource;

	VkSwapchainKHR swapchain;
	VkImage* images;
	VkImageView* leftImageViews;
	VkImageView* rightImageViews;
	dsVkSurfaceImageData* imageData;
	uint32_t imageCount;

	uint32_t width;
	uint32_t height;
	dsRenderSurfaceRotation rotation;

	bool vsync;

	uint32_t imageIndex;
	uint32_t imageDataIndex;

	VkDeviceMemory resolveMemory;
	VkImage resolveImage;
	VkImageView resolveImageView;

	VkDeviceMemory depthMemory;
	VkImage depthImage;
	VkImageView depthImageView;
};

typedef struct dsVkRenderSurface
{
	dsRenderSurface renderSurface;
	dsAllocator* scratchAllocator;
	dsLifetime* lifetime;

	VkSurfaceKHR surface;
	dsVkRenderSurfaceData* surfaceData;
	bool clientRotations;
	bool surfaceError;
	uint64_t updatedFrame;
	dsSpinlock lock;
} dsVkRenderSurface;

typedef struct dsVkCommandBuffer dsVkCommandBuffer;

typedef struct dsVkCommandPoolData
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsVkResource resource;

	dsVkCommandBuffer* vkCommandBuffers;
	dsCommandBuffer** commandBuffers;
	uint32_t count;
} dsVkCommandPoolData;

typedef struct dsVkCommandBufferPool
{
	dsCommandBufferPool commandBufferPool;

	dsVkCommandPoolData* commandPools[DS_DELAY_FRAMES];
	uint32_t curCommandPool;
} dsVkCommandBufferPool;

typedef struct dsVkResourceList
{
	dsAllocator* allocator;

	dsVkGfxBufferData** buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;

	dsTexture** textures;
	uint32_t textureCount;
	uint32_t maxTextures;

	dsVkTempBuffer** tempBuffers;
	uint32_t tempBufferCount;
	uint32_t maxTempBuffers;

	dsRenderbuffer** renderbuffers;
	uint32_t renderbufferCount;
	uint32_t maxRenderbuffers;

	dsVkRealFramebuffer** framebuffers;
	uint32_t framebufferCount;
	uint32_t maxFramebuffers;

	dsGfxFence** fences;
	uint32_t fenceCount;
	uint32_t maxFences;

	dsGfxQueryPool** queries;
	uint32_t queryCount;
	uint32_t maxQueries;

	dsVkMaterialDescriptor** descriptors;
	uint32_t descriptorCount;
	uint32_t maxDescriptors;

	dsVkSamplerList** samplers;
	uint32_t samplerCount;
	uint32_t maxSamplers;

	dsVkComputePipeline** computePipelines;
	uint32_t computePipelineCount;
	uint32_t maxComputePipelines;

	dsVkPipeline** pipelines;
	uint32_t pipelineCount;
	uint32_t maxPipelines;

	dsVkRenderSurfaceData** renderSurfaces;
	uint32_t renderSurfaceCount;
	uint32_t maxRenderSurfaces;

	dsVkCommandPoolData** commandPools;
	uint32_t commandPoolCount;
	uint32_t maxCommandPools;

	dsVkRenderPassData** renderPasses;
	uint32_t renderPassCount;
	uint32_t maxRenderPasses;
} dsVkResourceList;

typedef struct dsVkProcessResourceList
{
	dsAllocator* allocator;

	// No strict lifetime guarantees with respect to processing.
	dsLifetime** buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;

	// No strict lifetime guarantees with respect to processing.
	dsLifetime** textures;
	uint32_t textureCount;
	uint32_t maxTextures;

	dsRenderbuffer** renderbuffers;
	uint32_t renderbufferCount;
	uint32_t maxRenderbuffers;

	dsVkRenderSurfaceData** renderSurfaces;
	uint32_t renderSurfaceCount;
	uint32_t maxRenderSurfaces;
} dsVkProcessResourceList;

typedef struct dsVkBarrierList
{
	dsAllocator* allocator;
	dsVkDevice* device;

	VkBufferMemoryBarrier* bufferBarriers;
	uint32_t bufferBarrierCount;
	uint32_t maxBufferBarriers;

	VkImageMemoryBarrier* imageBarriers;
	uint32_t imageBarrierCount;
	uint32_t maxImageBarriers;
} dsVkBarrierList;

typedef struct dsVkBufferCopyInfo
{
	VkBuffer srcBuffer;
	VkBuffer dstBuffer;
	uint32_t firstRange;
	uint32_t rangeCount;
} dsVkBufferCopyInfo;

typedef struct dsVkImageCopyInfo
{
	VkBuffer srcBuffer;
	VkImage dstImage;
	VkImageLayout dstLayout;
	uint32_t firstRange;
	uint32_t rangeCount;
} dsVkImageCopyInfo;

typedef struct dsVkSharedBindingInfo
{
	VkDescriptorType type;
	uint32_t resourceIndex;
} dsVkSharedBindingInfo;

typedef struct dsVkSharedDescriptorSets
{
	dsRenderer* renderer;
	dsAllocator* allocator;

	dsLifetime* lastMaterialDesc;
	dsVkMaterialDescriptor* lastDescriptor;

	dsVkBindingMemory bindingMemory;
	dsMaterialBinding binding;
	uint32_t maxTextures;
	uint32_t maxBuffers;
	uint32_t maxTexelBuffers;
	uint32_t maxBindings;
	uint32_t maxImageInfos;
	uint32_t maxBufferInfos;
	uint32_t maxBufferViews;

	uint32_t* offsets;
	uint32_t offsetCount;
	uint32_t maxOffsets;
} dsVkSharedDescriptorSets;

typedef struct dsVkCommandBufferChunk
{
	VkCommandBuffer commandBuffers[DS_COMMAND_BUFFER_CHUNK_SIZE];
	uint32_t nextBuffer;
} dsVkCommandBufferChunk;

typedef struct dsVkCommandBufferData
{
	dsAllocator* allocator;
	dsVkDevice* device;

	VkCommandPool commandPool;
	dsVkCommandBufferChunk** chunks;
	uint32_t chunkCount;
	uint32_t maxChunks;
	uint32_t activeChunk;
	bool renderPass;
} dsVkCommandBufferData;

typedef struct dsVkSubpassBufferRange
{
	uint32_t start;
	uint32_t count;
} dsVkSubpassBufferRange;

typedef struct dsVkSubpassBuffers
{
	dsAllocator* allocator;
	VkCommandBuffer* commandBuffers;
	uint32_t commandBufferCount;
	uint32_t maxCommandBuffers;

	dsVkSubpassBufferRange* subpasses;
	uint32_t subpassCount;
	uint32_t maxSubpasses;
} dsVkSubpassBuffers;

struct dsVkCommandBuffer
{
	dsCommandBuffer commandBuffer;
	dsVkResource* resource;

	VkCommandPool commandPool;
	dsVkCommandBufferData commandBufferData;
	dsVkCommandBufferData subpassBufferData;

	VkCommandBuffer activeCommandBuffer;
	VkCommandBuffer activeSubpassBuffer;
	VkRenderPass activeRenderPass;
	VkFramebuffer activeFramebuffer;
	VkRect2D renderArea;
	dsVector2f depthRange;
	VkPipeline activePipeline;
	VkPipeline activeComputePipeline;
	VkDescriptorSet activeDescriptorSets[2][3];
	const dsDrawGeometry* activeVertexGeometry;
	const dsIndexBuffer* activeIndexBuffer;

	VkClearValue* clearValues;
	uint32_t clearValueCount;
	uint32_t maxClearValues;

	dsVkBarrierList barriers;
	dsVkSharedDescriptorSets globalDescriptorSets;
	dsVkSharedDescriptorSets instanceDescriptorSets;

	VkCommandBuffer* submitBuffers;
	uint32_t submitBufferCount;
	uint32_t maxSubmitBuffers;

	dsVkResource** usedResources;
	uint32_t usedResourceCount;
	uint32_t maxUsedResources;

	dsVkTempBuffer* curTempBuffer;
	dsVkTempBuffer** tempBuffers;
	uint32_t tempBufferCount;
	uint32_t maxTempBuffers;

	dsOffscreen** readbackOffscreens;
	uint32_t readbackOffscreenCount;
	uint32_t maxReadbackOffscreens;

	dsVkRenderSurfaceData** renderSurfaces;
	uint32_t renderSurfaceCount;
	uint32_t maxRenderSurfaces;

	VkImageMemoryBarrier* imageBarriers;
	uint32_t imageBarrierCount;
	uint32_t maxImageBarriers;

	VkBufferMemoryBarrier* bufferBarriers;
	uint32_t bufferBarrierCount;
	uint32_t maxBufferBarriers;

	VkBufferMemoryBarrier* copyBufferBarriers;
	uint32_t copyBufferBarrierCount;
	uint32_t maxCopyBufferBarriers;

	VkImageMemoryBarrier* copyImageBarriers;
	uint32_t copyImageBarrierCount;
	uint32_t maxCopyImageBarriers;

	dsVkSubpassBuffers subpassBuffers;

	VkBufferImageCopy* imageCopies;
	uint8_t* pushConstantBytes;
	uint32_t maxImageCopies;
	uint32_t maxPushConstantBytes;

	bool fenceSet;
	bool fenceReadback;
};

typedef struct dsVkCommandBufferWrapper
{
	dsCommandBuffer commandBuffer;
	dsCommandBuffer* realCommandBuffer;
} dsVkCommandBufferWrapper;

typedef void* (*dsVkGetDisplayFunction)(void);
typedef void (*dsVkReleaseDisplayFunction)(void* display);
typedef VkSurfaceKHR (*dsVkCreateSurfaceFunction)(dsVkInstance* instance, void* display,
	void* window);

typedef struct dsVkPlatform
{
	dsVkGetDisplayFunction getDisplayFunc;
	dsVkReleaseDisplayFunction releaseDisplayFunc;
	dsVkCreateSurfaceFunction createSurfaceFunc;

	dsVkDevice* device;
	void* display;
	bool createdDisplay;
} dsVkPlatform;

typedef struct dsVkSubmitInfo
{
	uint64_t submitIndex;
	dsVkCommandBuffer commandBuffer;
	VkCommandBuffer resourceCommands;
	VkFence fence;
	VkSemaphore semaphore;
} dsVkSubmitInfo;

typedef struct dsVkRenderer
{
	dsRenderer renderer;
	dsVkDevice device;
	dsVkPlatform platform;

	bool colorSurfaceAlpha;

	dsSpinlock resourceLock;
	dsSpinlock deleteLock;
	dsMutex* submitLock;
	dsConditionVariable* waitCondition;

	uint64_t submitCount;
	DS_ALIGN(8) uint64_t finishedSubmitCount;
	dsVkSubmitInfo submits[DS_MAX_SUBMITS];
	uint32_t curSubmit;
	uint32_t waitCount;

	dsVkCommandBufferWrapper mainCommandBuffer;

	VkSampler defaultSampler;

	dsVkBarrierList preResourceBarriers;
	dsVkBarrierList postResourceBarriers;
	dsVkProcessResourceList pendingResources[DS_PENDING_RESOURCES_ARRAY];
	dsVkResourceList deleteResources[DS_DELETE_RESOURCES_ARRAY];
	uint32_t curPendingResources;
	uint32_t curDeleteResources;

	VkBufferCopy* bufferCopies;
	uint32_t bufferCopiesCount;
	uint32_t maxBufferCopies;

	dsVkBufferCopyInfo* bufferCopyInfos;
	uint32_t bufferCopyInfoCount;
	uint32_t maxBufferCopyInfos;

	VkBufferImageCopy* imageCopies;
	uint32_t imageCopyCount;
	uint32_t maxImageCopies;

	dsVkImageCopyInfo* imageCopyInfos;
	uint32_t imageCopyInfoCount;
	uint32_t maxImageCopyInfos;
} dsVkRenderer;

typedef struct dsVkResourceManager
{
	dsResourceManager resourceManager;
	dsVkDevice* device;

	dsVkFormatInfo standardFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	dsVkFormatInfo specialFormats[dsGfxFormat_SpecialCount];
	dsVkFormatInfo compressedFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	uint32_t maxPushConstantSize;

	const char* shaderCacheDir;
	VkPipelineCache pipelineCache;
} dsVkResourceManager;

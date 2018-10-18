#include "common.h"

#include "kernel/vc4_packet.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateImageView
 */
VkResult vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
	assert(device);
	assert(pCreateInfo);
	assert(pView);

	assert(pAllocator == 0); //TODO

	_imageView* view = malloc(sizeof(_imageView));

	if(!view)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	view->image = pCreateInfo->image;
	view->viewType = pCreateInfo->viewType;
	view->interpretedFormat = pCreateInfo->format;
	view->swizzle = pCreateInfo->components;
	view->subresourceRange = pCreateInfo->subresourceRange;

	//TODO errors/validation

	*pView = view;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateBuffer
 */
VkResult vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
	assert(device);
	assert(pCreateInfo);
	assert(pBuffer);

	assert(pAllocator == 0); //TODO

	_buffer* buf = malloc(sizeof(_buffer));
	if(!buf)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	buf->size = pCreateInfo->size;
	buf->usage = pCreateInfo->usage;
	buf->boundMem = 0;
	buf->alignment = ARM_PAGE_SIZE; //TODO
	buf->alignedSize = getBOAlignedSize(buf->size);

	*pBuffer = buf;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetBufferMemoryRequirements
 */
void vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
	assert(device);
	assert(buffer);
	assert(pMemoryRequirements);

	pMemoryRequirements->alignment = ((_buffer*)buffer)->alignment;
	pMemoryRequirements->size = ((_buffer*)buffer)->alignedSize;
	pMemoryRequirements->memoryTypeBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; //TODO
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBindBufferMemory
 */
VkResult vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
	assert(device);
	assert(buffer);
	assert(memory);

	_buffer* buf = buffer;
	_deviceMemory* mem = memory;

	assert(!buf->boundMem);
	assert(memoryOffset < mem->size);
	assert(memoryOffset % buf->alignment == 0);
	assert(buf->alignedSize <= mem->size - memoryOffset);

	buf->boundMem = mem;
	buf->boundOffset = memoryOffset;

	return VK_SUCCESS;
}

void vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(buffer);

	assert(pAllocator == 0); //TODO

	_buffer* buf = buffer;
	free(buf);
}

void vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
	//TODO
}

void vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
	assert(device);
	assert(imageView);

	assert(pAllocator == 0); //TODO

	_imageView* view = imageView;
	free(view);
}


/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateBufferView
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateBufferView(
	VkDevice                                    device,
	const VkBufferViewCreateInfo*               pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkBufferView*                               pView)
{
	assert(device);
	assert(pCreateInfo);
	assert(pView);

	assert(pAllocator == 0); //TODO

	_bufferView* bv = malloc(sizeof(_bufferView));

	bv->buffer = pCreateInfo->buffer;
	bv->format = pCreateInfo->format;
	bv->offset = pCreateInfo->offset;
	bv->range = pCreateInfo->range;

	*pView = bv;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyBufferView
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyBufferView(
	VkDevice                                    device,
	VkBufferView                                bufferView,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);
	assert(bufferView);

	assert(pAllocator == 0); //TODO

	_bufferView* bv = bufferView;
	free(bv);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateImage
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateImage(
	VkDevice                                    device,
	const VkImageCreateInfo*                    pCreateInfo,
	const VkAllocationCallbacks*                pAllocator,
	VkImage*                                    pImage)
{
	assert(device);
	assert(pCreateInfo);
	assert(pImage);

	assert(pAllocator == 0);

	_image* i = malloc(sizeof(_image));
	if(!i)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	//TODO flags?
	//TODO? pCreateInfo->imageType
	i->format = pCreateInfo->format;
	i->width = pCreateInfo->extent.width;
	i->height = pCreateInfo->extent.height;
	i->depth = pCreateInfo->extent.depth;
	i->miplevels = pCreateInfo->mipLevels;
	i->layers = pCreateInfo->arrayLayers;
	i->samples = pCreateInfo->samples; //TODO?
	i->tiling = pCreateInfo->tiling; //TODO?
	i->usageBits = pCreateInfo->usage;
	i->concurrentAccess = pCreateInfo->sharingMode; //TODO?
	i->numQueueFamiliesWithAccess = pCreateInfo->queueFamilyIndexCount;
	if(i->numQueueFamiliesWithAccess > 0)
	{
		i->queueFamiliesWithAccess = malloc(sizeof(uint32_t) * i->numQueueFamiliesWithAccess);
		if(!i->queueFamiliesWithAccess)
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		memcpy(i->queueFamiliesWithAccess, pCreateInfo->pQueueFamilyIndices, sizeof(uint32_t) * i->numQueueFamiliesWithAccess);
	}
	i->layout = pCreateInfo->initialLayout;

	//TODO what else memory allocation, buffer object creation etc?

	*pImage = i;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyImage
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyImage(
	VkDevice                                    device,
	VkImage                                     image,
	const VkAllocationCallbacks*                pAllocator)
{
	assert(device);
	assert(image);

	assert(pAllocator == 0); //TODO

	_image* i = image;

	if(i->numQueueFamiliesWithAccess > 0);
		free(i->queueFamiliesWithAccess);

	free(i);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetImageMemoryRequirements
 */
VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements(
	VkDevice                                    device,
	VkImage                                     image,
	VkMemoryRequirements*                       pMemoryRequirements)
{
	assert(device);
	assert(image);
	assert(pMemoryRequirements);

	_image* i = image;

	//TODO??
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkBindImageMemory
 */
VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory(
	VkDevice                                    device,
	VkImage                                     image,
	VkDeviceMemory                              memory,
	VkDeviceSize                                memoryOffset)
{
	assert(device);
	assert(image);
	assert(memory);

	//TODO

	return VK_SUCCESS;
}

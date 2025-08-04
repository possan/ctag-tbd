/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2024 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSPAllocator.hpp"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <cstdint>
#include <cassert>

using namespace CTAG::SP;

// create all definitions
void *ctagSPAllocator::internalBuffer = nullptr;
void *ctagSPAllocator::buffer[40] = {
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    }    ;
void *ctagSPAllocator::blockmems[40] = {
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    nullptr,    nullptr,    nullptr,    nullptr,
    }    ;
// void *ctagSPAllocator::buffer2 = nullptr;
std::size_t ctagSPAllocator::totalSize = 0;
std::size_t ctagSPAllocator::buffersizes[40] = {
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
};
std::size_t ctagSPAllocator::blockmemsizes[40] = {
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
};
std::size_t ctagSPAllocator::offsets[40] = {
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
};
int ctagSPAllocator::channel = 0;
// std::size_t ctagSPAllocator::size2 = 0;
// ctagSPAllocator::AllocationType ctagSPAllocator::allocationType = ctagSPAllocator::AllocationType::CH0;

void ctagSPAllocator::AllocateInternalBuffer(std::size_t const &size) {
    ESP_LOGI("ctagSPAllocator", "AllocateInternalBuffer: allocating %d bytes", size);
    internalBuffer = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    ESP_LOGI("ctagSPAllocator", "AllocateInternalBuffer: Buffer at %lx", (unsigned long)internalBuffer);
    if (nullptr == internalBuffer){
        ESP_LOGE("ctagSPAllocator", "AllocateInternalBuffer: could not allocate memory of size %d", size);
        assert(nullptr != internalBuffer);
    }
    totalSize = size;
}

void ctagSPAllocator::ReleaseInternalBuffer() {
    ESP_LOGI("ctagSPAllocator", "ReleaseInternalBuffer: releasing memory");
    heap_caps_free(internalBuffer);
    internalBuffer = nullptr;
    // buffer1 = nullptr;
    // buffer2 = nullptr;
    totalSize = 0;
    // size1 = 0;
    // size2 = 0;
    for(int k=0;k<40; k++){
        offsets[k] = 0;
        buffer[k] = nullptr;
        buffersizes[k] = 0;
        blockmems[k] = nullptr;
        blockmemsizes[k] = 0;
    }
}

void *ctagSPAllocator::Allocate(std::size_t const &size) {
    void *ptr = nullptr;

    ptr = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (ptr == nullptr) {
        ESP_LOGE("ctagSPAllocator", "Allocate: could not allocate memory of size %d", size);
        return nullptr; // early return if allocation fails
    }

    offsets[channel] += size;
    buffer[channel] = ptr;
    buffersizes[channel] = size;

    // if  size )
    // if(allocationType == AllocationType::CH0){
    //     if(size1 >= size){
    //         ptr = buffer1;
    //         buffer1 = static_cast<uint8_t *>(buffer1) + size;
    //         size1 -= size;
    //     }else{
    //         ESP_LOGE("ctagSPAllocator", "Allocate: not enough memory for CH0 request %d bytes, %d bytes free", size, size1);
    //         assert(false);
    //     }
    // }else if(allocationType == AllocationType::CH1){
    //     if(size2 >= size){
    //         ptr = buffer2;
    //         buffer2 = static_cast<uint8_t *>(buffer2) + size;
    //         size2 -= size;
    //     }else{
    //         ESP_LOGE("ctagSPAllocator", "Allocate: not enough memory for CH1 request %d bytes, %d bytes free", size, size1);
    //         assert(false);
    //     }
    // }else if(allocationType == AllocationType::STEREO){
    //     if(size1 >= size){
    //         ptr = buffer1;
    //         buffer1 = static_cast<uint8_t *>(buffer1) + size;
    //         size1 -= size;
    //     }else{
    //         ESP_LOGE("ctagSPAllocator", "Allocate: not enough memory for STEREO request %d bytes, %d bytes free", size, size1);
    //         assert(false);
    //     }
    // }
    // switch(allocationType){
    //     case AllocationType::CH0:
    //         ESP_LOGI("ctagSPAllocator", "Allocate: allocating CH0 %d bytes object size, ch0 %d bytes blockMem", size, size1);
    //         break;
    //     case AllocationType::CH1:
    //         ESP_LOGI("ctagSPAllocator", "Allocate: allocating CH1 %d bytes object size, ch1 %d bytes blockMem", size, size2);
    //         break;
    //     case AllocationType::STEREO:

    int rem = GetRemainingBufferSize();

    ESP_LOGI("ctagSPAllocator", "Allocate: allocating %d bytes object size at: %lx (%d bytes remaining)", size, (unsigned long)ptr, rem);
    //         break;
    //     default:
    //         ESP_LOGE("ctagSPAllocator", "Allocate: unknown allocation type");
    //         assert(false);
    // }
    return ptr;
}

std::size_t ctagSPAllocator::GetRemainingBufferSize() {
    // if(allocationType == AllocationType::CH0 || allocationType == AllocationType::STEREO){
    //     ESP_LOGD("ctagSPAllocator", "GetRemainingBuffer: CH0 or STEREO %d bytes free", size1);
    //     return size1;
    // }else if(allocationType == AllocationType::CH1){
    //     ESP_LOGD("ctagSPAllocator", "GetRemainingBuffer: CH1 %d bytes free", size2);
    //     return size2;
    // }else
    //     ESP_LOGE("ctagSPAllocator", "GetRemainingBufferSize: unknown allocation type");

    int tot = 0;
    for(int k=0; k<40; k++){
        tot += buffersizes[k];
        tot += blockmemsizes[k];
    }
    int rem = totalSize - tot;
    ESP_LOGI("ctagSPAllocator", "GetRemainingBufferSize: returning %d bytes", rem);
    return rem;

    // return 32768;
}

void *ctagSPAllocator::GetRemainingBuffer() {
    void *ptr = nullptr;
    // if(allocationType == AllocationType::CH0 || allocationType == AllocationType::STEREO){
    //     ptr = buffer1;
    // }
    // else if(allocationType == AllocationType::CH1){
    //     ptr = buffer2;
    // }
    // else
    //     ESP_LOGE("ctagSPAllocator", "GetRemainingBuffer: unknown allocation type");
    return ptr;
}


// void ctagSPAllocator::PrepareAllocation(AllocationType const &type) {
//     allocationType = type;
//     if(allocationType == AllocationType::CH0){
//         ESP_LOGI("ctagSPAllocator", "SetAllocationType: Single Channel CH0");
//         size1 = totalSize / 2;
//         buffer1 = internalBuffer;
//     }else if(allocationType == AllocationType::CH1){
//         ESP_LOGI("ctagSPAllocator", "SetAllocationType: Single Channel CH1");
//         size2 = totalSize / 2;
//         buffer2 = static_cast<uint8_t *>(internalBuffer) + size2;
//     }else if(allocationType == AllocationType::STEREO){
//         ESP_LOGI("ctagSPAllocator", "SetAllocationType: Stereo");
//         size1 = totalSize;
//         buffer1 = internalBuffer;
//         size2 = 0;
//         buffer2 = nullptr;
//     }else{
//         ESP_LOGE("ctagSPAllocator", "SetAllocationType: unknown allocation type");
//         assert(false);
//     }
// }

void ctagSPAllocator::PrepareAllocation2(int ch) {
    channel = ch;

    // if (blockmems[channel] != nullptr) {
    //     ESP_LOGI("ctagSPAllocator", "PrepareAllocation2: channel %d already has block memory allocated", channel);
    // }

    // if (buffer[channel] != nullptr) {
    //     ESP_LOGI("ctagSPAllocator", "PrepareAllocation2: channel %d already has memory allocated", channel);
    // }

    // size[0] = totalSize / 40;
    // buffer[0] = internalBuffer + ;
    // if (ch == 0) {
    //     ESP_LOGI("ctagSPAllocator", "PrepareAllocation2: Single Channel CH0");
    //     size[0] = totalSize / 2;
    //     buffer[0] = internalBuffer;
    // } else if (ch == 1) {
    //     ESP_LOGI("ctagSPAllocator", "PrepareAllocation2: Single Channel CH1");
    //     size[1] = totalSize / 2;
    //     buffer[1] = static_cast<uint8_t *>(internalBuffer) + size[0];
    // } else {
    //     ESP_LOGE("ctagSPAllocator", "PrepareAllocation2: unknown channel %d", ch);
    //     assert(false);
    // }
}

void *ctagSPAllocator::AllocateBlockMem(std::size_t const &size) {
    void *ptr = nullptr;

    ptr = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (ptr == nullptr) {
        ESP_LOGE("ctagSPAllocator", "AllocateBlockMme: could not allocate memory of size %d", size);
        return nullptr; // early return if allocation fails
    }

    offsets[channel] += size;
    blockmemsizes[channel] = size;
    blockmems[channel] = ptr;
    int rem = GetRemainingBufferSize();

    ESP_LOGI("ctagSPAllocator", "AllocateBlockMem: allocated %d bytes at %lx (%d bytes remaining)", size, (unsigned long)ptr, rem);

    return ptr;
}

size_t ctagSPAllocator::GetRemainingBlockMem() {
    size_t rem = GetRemainingBufferSize();

    return rem;
}

void ctagSPAllocator::ReleaseBlockMem(void *ptr) {
    size_t foundsize = 0;
    for(int k=0; k<40; k++){
        if (blockmems[k] == ptr) {
            foundsize = blockmemsizes[k];
            blockmems[k] = nullptr;
            blockmemsizes[k] = 0;
        }
    }

    int rem = GetRemainingBufferSize();

    ESP_LOGI("ctagSPAllocator", "ReleaseBlockMem: releasing memory at %lx (%d bytes?, %d bytes remaining)", (unsigned long)ptr, foundsize, rem);
    free(ptr);
}

void ctagSPAllocator::Release(void *ptr) {
    size_t foundsize = 0;
    for(int k=0; k<40; k++){
        if (buffer[k] == ptr) {
            foundsize = buffersizes[k];
            buffer[k] = nullptr;
            buffersizes[k] = 0;
        }
    }

    int rem = GetRemainingBufferSize();

    ESP_LOGI("ctagSPAllocator", "Release: releasing memory at %lx (%d bytes?, %d bytes remaining)", (unsigned long)ptr, foundsize, rem);
    free(ptr);
}

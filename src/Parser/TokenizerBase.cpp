// Filename:  TokenizerBase.cpp
// Content:   TokenizerBase class implementation
// Provided AS IS under MIT License; see LICENSE file in root folder.

#include "TokenizerBase.h"

using namespace SGParser;

// ***** Tokenizer buffer implementation

// Set input stream and setup the initial buffer
bool TokenizerBase::SetInputStream(InputStream* pinputStream) {
    if (!pinputStream) {
        // Removing an input stream, release everything
        FreeAllBuffers();
        pInputStream = nullptr;
    } else {
        pInputStream = pinputStream;
        if (pHeadBuffer) {
            // Make sure we only have one buffer at pHead
            AdjustHead();
            // Free any hanging off tails (flush)
            while (pHeadBuffer->pNext)
                delete std::exchange(pHeadBuffer->pNext, pHeadBuffer->pNext->pNext);

            // Get a block from the stream
            const auto bufferSize = pinputStream->Read(
                                        reinterpret_cast<uint8_t*>(pHeadBuffer->Buffer),
                                        pHeadBuffer->BufferSize);
            if (bufferSize <= 0) {
                FreeAllBuffers();
                pInputStream = nullptr;
                return false;
            }
            // Set the buffer tail to point to the edge of the buffer
            pHeadBuffer->pBufferTail = &pHeadBuffer->Buffer[bufferSize];
        } else {
            pHeadBuffer = LoadNewBuffer(false);
            if (!pHeadBuffer) {
                FreeAllBuffers();
                pInputStream = nullptr;
                return false;
            }
        }

        // Setup the Head and Tail
        pHead       = &pHeadBuffer->Buffer[0u];
        pTail       = &pHeadBuffer->Buffer[0u];
        pTailBuffer = pHeadBuffer;
        pFreeBuffer = nullptr;
    }

    return true;
}


// Free all the buffers and reset the tokenizer data
void TokenizerBase::FreeAllBuffers() noexcept {
    while (pHeadBuffer)
        delete std::exchange(pHeadBuffer, pHeadBuffer->pNext);

    delete std::exchange(pFreeBuffer, nullptr);

    // Reset all pointers
    pHead       = nullptr;
    pTail       = nullptr;
    pTailBuffer = nullptr;
}


// Loads in a new buffer and returns it
TokenizerBuffer* TokenizerBase::LoadNewBuffer(bool freeOnEmpty) {
    // Use a free temporary buffer if we have one
    // If not, create a new tokenizer buffer
    // Basic exception safety is provided (if new TokenizerBuffer fails)
    const auto newBuffer = pFreeBuffer ? std::exchange(pFreeBuffer, nullptr)
                                       : new TokenizerBuffer;

    // Get a block from the stream
    auto bufferSize = pInputStream->Read(reinterpret_cast<uint8_t*>(newBuffer->Buffer),
                                         newBuffer->BufferSize);

    // if nothing was read or read error happened
    if (bufferSize <= 0) {
        // if freeOnEmpty than fail
        if (freeOnEmpty) {
            pFreeBuffer = newBuffer;
            return nullptr;
        }
        // Otherwise, behave like we read 0 bytes 
        bufferSize = 0;
    }

    // Set the buffer tail to point to the edge of the buffer
    newBuffer->pBufferTail = &newBuffer->Buffer[bufferSize];
    newBuffer->pNext       = nullptr;
    return newBuffer;
}


// Reload the buffer loads in a new buffer and adjusts the tail
bool TokenizerBase::ReloadBuffer() {
    SG_ASSERT(pTailBuffer);

    // If there is not already an allocated buffer than create one
    if (!pTailBuffer->pNext) {
        // Get the new buffer
        const auto newBuffer = LoadNewBuffer();

        if (!newBuffer) {
            // Back the pTail up so next time we will also get EOF
            if (pTail > pTailBuffer->pBufferTail)
                --pTail;
            return false;
        }
        // Add the new buffer to the tail
        pTailBuffer->pNext = newBuffer;
    }

    // Point the tail to the new buffer
    pTailBuffer = pTailBuffer->pNext;
    pTail       = &pTailBuffer->Buffer[0u];
    return true;
}


// Moves the head to the tail position and frees all the discarded blocks
// If there is no free buffer available and a block is free than the system
// caches it for next time (could be a cache list, etc)
void TokenizerBase::AdjustHead() noexcept {
    SG_ASSERT(pHeadBuffer && pTailBuffer);

    // Move the Head to the Tail
    pHead = pTail;

    // Need to destroy a buffer
    if (pHeadBuffer != pTailBuffer) {
        // Save one of the buffers
        if (!pFreeBuffer) {
            pFreeBuffer = pHeadBuffer;
            pHeadBuffer = pHeadBuffer->pNext;
        }

        // Now go through and destroy all unused buffers
        while (pHeadBuffer != pTailBuffer)
            delete std::exchange(pHeadBuffer, pHeadBuffer->pNext);
    }
}

/*
* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Code Aurora Forum, Inc. nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WebTech_BackingStore_h
#define WebTech_BackingStore_h

#include "RefCount.h"

namespace WebTech {

class IBackingStore : public virtual IRefCount {
public:
    enum Param {
        ALLOW_INPLACE_SCROLL,
        ALLOW_TEXTURE_COORDINATE,
        PRIORITY,
        QUALITY,
        ALLOW_PARTIAL_RENDER,
        PARAM_EXTENSIONS_START = 0x10000,
    };

    enum UpdateMode {
        UPDATE_ALL, // the existing AND the exposed region should be updated. the user can use this to perform a partial update on the existing region
        UPDATE_EXPOSED_ONLY, // only the exposed region should be updated
        UPDATE_MODE_MAX,
        UPDATE_MODE_EXTENSIONS_START = 0x10000,
    };

    enum RegionAvailability {
        NOT_AVAILABLE,
        FULLY_AVAILABLE,
        PARTIALLY_AVAILABLE,
    };

    enum UpdateQuality {
        LOW_QUALITY,
        HIGH_QUALITY,
    };

    // a simple structure representing an update region
    struct UpdateRegion {
        int x1;
        int y1;
        int x2;
        int y2;
    };

    // A buffer to hold a part of the backing store.  The backing store may contain many buffers.
    // A IBuffer object is supplied by the user to supply a buffer (through IUpdater::createBuffer()).  The implementation can maintain
    // any type of buffer (e.g. a piece of memory, a GL texture, etc)
    // The user of IBackingStore must implement this
    class IBuffer {
    public:
        virtual ~IBuffer() {}
        virtual void release() = 0;
    };

    // IUpdater is an interface to allow IBackingStore to ask the user to create/render on a buffer.
    // An IBackingStore object is not responsible for the actual rendering.  The user will perform the
    // rendering by implementing this interface.
    // The user of IBackingStore must implement this
    class IUpdater {
    public:
        virtual ~IUpdater() {}
        virtual IBuffer* createBuffer(int w, int h) = 0; // the user should supply a buffer (an IBuffer implementation)
        virtual void inPlaceScroll(IBuffer*, int x, int y, int w, int h, int dx, int dy) = 0; // the user should move a rectangle (x,y,w,h) by offset (dx,dy) on the supplied IBuffer
        // the user should render the content "region" (in scaled document space) onto location (bufferX, bufferY) on the supplied IBuffer.
        // the flag "existingRegion" indicates whether this region is an existing region or an exposed region.
        virtual void renderToBackingStoreRegion(IBuffer*, int bufferX, int bufferY, UpdateRegion&, UpdateQuality quality, bool existingRegion) = 0;
    };

    // When drawing onto the screen, the backing store supplies the user with a list of buffers and regions to copy
    // to the screen.
    // IDrawRegionIterator allows the user to iterate through a list of valid regions (and their associate buffers)
    // that it can use to draw to the screen.
    class IDrawRegionIterator {
    public:
        virtual ~IDrawRegionIterator() {}
        virtual void release() = 0;
        virtual IBackingStore::IBuffer* buffer() = 0; // the buffer that contains the valid region
        virtual int outX() = 0; // x location of the screen to draw to
        virtual int outY() = 0; // y location of the screen to draw to
        virtual int inX() = 0; // x location of the buffer to draw from
        virtual int inY() = 0; // y location of the buffer to draw from
        virtual int width() = 0; // width of region to copy to screen
        virtual int height() = 0; // height of region to copy to screen
        virtual bool next() = 0; // iterate to next region.  return false if there are no more regions.
    };

    virtual ~IBackingStore() {};

    // set static parameters for the backing store
    virtual void setParam(IBackingStore::Param, int value) = 0;

    virtual void cleanup() = 0; // the backing store should be freed.
    virtual bool checkError() = 0; // returns true if any error occured and the backing store should not be used.
    virtual bool hasContent() = 0; // returns true if the backing store contains some valid content.
    virtual void invalidate() = 0; // invalidate the content of the backing store
    virtual void finish() = 0; // stop all ongoing updates to the backing store

    // Perform an update of the backing store
    // returns true if the requested region is available, and beginDrawRegion() can be used to draw the region.
    virtual bool update(UpdateRegion*, // the region to update (in scaled document coordinate).  Must be smaller than or equal to the size of the viewport.  If null, the backing store will find something to update.
                        UpdateMode, // see IBackingStore::UpdateMode
                        int viewportX, int viewportY, // location of the top left corner of the viewport.
                        int viewportWidth, int viewportHeight, // size of the viewport
                        int contentWidth, int contentHeight, // size of the document (in scaled document coordinate)
                        bool contentChanged // content changed since last update?  This is a hint only.
                        ) = 0;

    // returns the available draw region available for drawing
    virtual RegionAvailability canDrawRegion(IBackingStore::UpdateRegion&, IBackingStore::UpdateRegion&) = 0;
    // returns a list (iterator) of regions to draw to the screen.
    // (viewportX, viewportY) is the location of the top left corner of the viewport in scaled document coordinate.
    // For example, when scrolling down a document, the viewport moves in the positive y direction and the viewportY will be increasing.
    virtual IDrawRegionIterator* beginDrawRegion(UpdateRegion&, int viewportX, int viewportY) = 0;

};


// create an IBackingStore object
extern "C" IBackingStore* createBackingStore(IBackingStore::IUpdater*);

}; // WebTech

#endif // WebTech_BackingStore_h


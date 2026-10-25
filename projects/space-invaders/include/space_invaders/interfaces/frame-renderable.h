#pragma once

namespace CE::Rendering { class FrameWriter; }

struct FrameRenderable {
    virtual ~FrameRenderable() = default;
    virtual void append_passes(CE::Rendering::FrameWriter& frame) const = 0;
};

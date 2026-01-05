#include "dakt/gui/draw/DrawBatcher.hpp"
#include <algorithm>

namespace dakt::gui {

// ============================================================================
// DrawBatcher Implementation
// ============================================================================

DrawBatcher::DrawBatcher() { reset(); }

DrawBatcher::~DrawBatcher() = default;

void DrawBatcher::reset() {
    batchedCommands_.clear();
    clipRectStack_.clear();
    currentClipRect_ = Rect(0, 0, 100000, 100000); // Large default
    currentState_ = RenderState{};
    stats_ = BatchStats{};
}

void DrawBatcher::batchCommands(const DrawList& drawList) {
    const auto& commands = drawList.getCommands();

    stats_.originalCommandCount = static_cast<uint32_t>(commands.size());
    batchedCommands_.clear();
    batchedCommands_.reserve(commands.size());

    RenderState state{};
    uint64_t lastTextureID = 0;
    Rect lastClipRect;

    for (const auto& cmd : commands) {
        switch (cmd.type) {
        case DrawCommandType::SetClipRect:
            if (cmd.clipRect != lastClipRect) {
                lastClipRect = cmd.clipRect;
                state.clipRect = cmd.clipRect;
                stats_.clipRectChanges++;
            }
            break;

        case DrawCommandType::SetTexture:
            if (cmd.textureID != lastTextureID) {
                lastTextureID = cmd.textureID;
                state.textureID = cmd.textureID;
                state.isTextured = (cmd.textureID != 0);
                stats_.textureChanges++;
            }
            break;

        case DrawCommandType::DrawTriangles: {
            BatchedDrawCommand batch;
            batch.state = state;
            batch.vertexOffset = cmd.vertexOffset;
            batch.vertexCount = cmd.vertexCount;
            batch.indexOffset = cmd.indexOffset;
            batch.indexCount = cmd.indexCount;

            // Try to merge with previous command
            if (mergeCommands_ && !batchedCommands_.empty() && canMerge(batchedCommands_.back(), batch)) {
                mergeCommand(batchedCommands_.back(), batch);
            } else {
                batchedCommands_.push_back(batch);
            }
            break;
        }

        default:
            break;
        }
    }

    // Sort by texture if enabled
    if (sortByTexture_) {
        sortCommands();
    }

    stats_.batchedCommandCount = static_cast<uint32_t>(batchedCommands_.size());
    stats_.drawCalls = stats_.batchedCommandCount;
}

bool DrawBatcher::canMerge(const BatchedDrawCommand& a, const BatchedDrawCommand& b) const {
    // Must have same render state
    if (a.state != b.state) {
        return false;
    }

    // Must be contiguous in the buffer
    if (a.vertexOffset + a.vertexCount != b.vertexOffset) {
        return false;
    }

    if (a.indexOffset + a.indexCount != b.indexOffset) {
        return false;
    }

    return true;
}

void DrawBatcher::mergeCommand(BatchedDrawCommand& target, const BatchedDrawCommand& source) {
    target.vertexCount += source.vertexCount;
    target.indexCount += source.indexCount;
}

void DrawBatcher::sortCommands() {
    // Sort by texture to minimize state changes
    // Note: This may affect visual order for overlapping elements!
    std::stable_sort(batchedCommands_.begin(), batchedCommands_.end(), [](const BatchedDrawCommand& a, const BatchedDrawCommand& b) {
        // Sort by texture first, then by clip rect
        if (a.state.textureID != b.state.textureID) {
            return a.state.textureID < b.state.textureID;
        }
        // Keep original order for same texture
        return false;
    });

    // Re-merge after sorting
    if (mergeCommands_ && batchedCommands_.size() > 1) {
        std::vector<BatchedDrawCommand> merged;
        merged.reserve(batchedCommands_.size());
        merged.push_back(batchedCommands_[0]);

        for (size_t i = 1; i < batchedCommands_.size(); ++i) {
            if (canMerge(merged.back(), batchedCommands_[i])) {
                mergeCommand(merged.back(), batchedCommands_[i]);
            } else {
                merged.push_back(batchedCommands_[i]);
            }
        }

        batchedCommands_ = std::move(merged);
    }
}

void DrawBatcher::processClipRect(const Rect& clipRect) {
    currentClipRect_ = clipRect;
    currentState_.clipRect = clipRect;
}

} // namespace dakt::gui

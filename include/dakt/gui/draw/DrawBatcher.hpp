#ifndef DAKTLIB_GUI_DRAW_BATCHER_HPP
#define DAKTLIB_GUI_DRAW_BATCHER_HPP

#include "DrawList.hpp"
#include <vector>

namespace dakt::gui {

/**
 * @brief Render state for batching decisions
 */
struct RenderState {
    uint64_t textureID = 0;
    Rect clipRect;
    bool isTextured = false;
    bool isSDF = false; // For text rendering

    bool operator==(const RenderState& other) const { return textureID == other.textureID && clipRect == other.clipRect && isTextured == other.isTextured && isSDF == other.isSDF; }

    bool operator!=(const RenderState& other) const { return !(*this == other); }
};

/**
 * @brief Batched draw command with render state
 */
struct BatchedDrawCommand {
    RenderState state;
    uint32_t vertexOffset = 0;
    uint32_t vertexCount = 0;
    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;
};

/**
 * @brief Draw command batcher for optimal GPU submission
 *
 * Optimizes draw commands by:
 * - Merging adjacent commands with same render state
 * - Sorting by texture to minimize state changes
 * - Managing clip rect stack
 * - Batching instanced draws when possible
 */
class DAKTLIB_GUI_API DrawBatcher {
  public:
    DrawBatcher();
    ~DrawBatcher();

    /**
     * Reset batcher state for new frame
     */
    void reset();

    /**
     * Process and optimize draw list commands
     * @param drawList Source draw list
     */
    void batchCommands(const DrawList& drawList);

    /**
     * Get batched commands after processing
     */
    const std::vector<BatchedDrawCommand>& getBatchedCommands() const { return batchedCommands_; }

    /**
     * Get statistics about batching
     */
    struct BatchStats {
        uint32_t originalCommandCount = 0;
        uint32_t batchedCommandCount = 0;
        uint32_t textureChanges = 0;
        uint32_t clipRectChanges = 0;
        uint32_t drawCalls = 0;
    };

    const BatchStats& getStats() const { return stats_; }

    /**
     * Enable/disable sorting by texture (may affect draw order)
     */
    void setSortByTexture(bool enabled) { sortByTexture_ = enabled; }

    /**
     * Enable/disable command merging
     */
    void setMergeCommands(bool enabled) { mergeCommands_ = enabled; }

  private:
    // Merge compatible commands
    bool canMerge(const BatchedDrawCommand& a, const BatchedDrawCommand& b) const;
    void mergeCommand(BatchedDrawCommand& target, const BatchedDrawCommand& source);

    // Sort commands by render state
    void sortCommands();

    // Process clip rect stack
    void processClipRect(const Rect& clipRect);

    std::vector<BatchedDrawCommand> batchedCommands_;
    std::vector<Rect> clipRectStack_;
    Rect currentClipRect_;
    RenderState currentState_;

    BatchStats stats_;

    bool sortByTexture_ = false;
    bool mergeCommands_ = true;
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_DRAW_BATCHER_HPP

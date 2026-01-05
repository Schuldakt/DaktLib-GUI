#ifndef DAKT_GUI_TEXT_CURSOR_HPP
#define DAKT_GUI_TEXT_CURSOR_HPP

#include "../core/Types.hpp"
#include "TextShaper.hpp"
#include <string>
#include <vector>

namespace dakt::gui {

// Forward declarations
class DrawList;
class Font;

/**
 * @brief Text cursor position within a text run
 */
struct CursorPosition {
    size_t charIndex = 0;    // Character index in text
    size_t clusterIndex = 0; // Cluster index in shaped run
    float x = 0;             // X position in pixels
    float y = 0;             // Y position (top of line)
    bool afterChar = false;  // Cursor is after the character (for RTL)
};

/**
 * @brief Text selection range
 */
struct TextSelection {
    size_t start = 0; // Start character index
    size_t end = 0;   // End character index (exclusive)
    float startX = 0; // Start X position
    float endX = 0;   // End X position

    bool isEmpty() const { return start == end; }
    size_t length() const { return (end > start) ? end - start : 0; }
    void normalize() {
        if (start > end) {
            std::swap(start, end);
            std::swap(startX, endX);
        }
    }
};

/**
 * @brief Text cursor and selection manager
 *
 * Handles cursor positioning, blinking animation,
 * text selection, and click-to-position logic.
 */
class TextCursor {
  public:
    TextCursor();
    ~TextCursor();

    /**
     * Initialize cursor for a text run
     * @param run Shaped text run
     * @param text Original text string
     * @param fontSize Font size in pixels
     * @param ascender Font ascender in pixels
     */
    void init(const ShapedRun& run, const std::string& text, float fontSize, float ascender);

    /**
     * Set cursor position by character index
     */
    void setPosition(size_t charIndex);

    /**
     * Get current cursor position
     */
    const CursorPosition& getPosition() const { return cursor_; }

    /**
     * Move cursor left/right by characters
     */
    void moveLeft(bool extendSelection = false);
    void moveRight(bool extendSelection = false);
    void moveToStart(bool extendSelection = false);
    void moveToEnd(bool extendSelection = false);

    /**
     * Move cursor to position from mouse click
     * @param x X coordinate relative to text origin
     * @return Character index at click position
     */
    size_t hitTest(float x);

    /**
     * Set selection range
     */
    void setSelection(size_t start, size_t end);
    void clearSelection();
    void selectAll();

    /**
     * Get current selection
     */
    const TextSelection& getSelection() const { return selection_; }
    bool hasSelection() const { return !selection_.isEmpty(); }

    /**
     * Get selected text
     */
    std::string getSelectedText() const;

    /**
     * Update cursor animation (call each frame)
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime);

    /**
     * Check if cursor should be visible (blink state)
     */
    bool isCursorVisible() const { return cursorVisible_; }

    /**
     * Cursor blink settings
     */
    void setBlinkRate(float rate) { blinkRate_ = rate; }
    float getBlinkRate() const { return blinkRate_; }
    void setBlinkEnabled(bool enabled) { blinkEnabled_ = enabled; }

    /**
     * Draw cursor
     * @param drawList DrawList to render to
     * @param originX X origin of text
     * @param originY Y origin of text (baseline)
     * @param color Cursor color
     */
    void drawCursor(DrawList& drawList, float originX, float originY, Color color);

    /**
     * Draw selection highlight
     * @param drawList DrawList to render to
     * @param originX X origin of text
     * @param originY Y origin of text
     * @param color Selection highlight color
     */
    void drawSelection(DrawList& drawList, float originX, float originY, Color color);

  private:
    // Calculate X position for character index
    float getXForCharIndex(size_t charIndex) const;

    // Find character index from X position
    size_t getCharIndexFromX(float x) const;

    // Update selection from cursor movement
    void updateSelection(size_t newIndex, bool extend);

    // Shaped run data
    ShapedRun shapedRun_;
    std::string text_;
    float fontSize_ = 16.0f;
    float ascender_ = 12.0f;
    float lineHeight_ = 16.0f;

    // Cursor state
    CursorPosition cursor_;
    TextSelection selection_;

    // Blink animation
    float blinkTimer_ = 0.0f;
    float blinkRate_ = 0.5f; // Seconds per blink cycle
    bool blinkEnabled_ = true;
    bool cursorVisible_ = true;

    // Cached positions for each character
    std::vector<float> charPositions_;
};

} // namespace dakt::gui

#endif

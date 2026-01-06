#ifndef DAKTLIB_GUI_TEXT_INPUT_HPP
#define DAKTLIB_GUI_TEXT_INPUT_HPP

#include "WidgetBase.hpp"
#include <vector>

namespace dakt::gui {

/**
 * @brief Undo/redo action for text editing
 */
struct TextEditAction {
    enum class Type { Insert, Delete, Replace };

    Type type;
    size_t position;
    std::string text;         // Text that was inserted or deleted
    std::string replacedText; // For Replace: the text that was replaced
    size_t cursorBefore;      // Cursor position before action
    size_t cursorAfter;       // Cursor position after action
};

/**
 * @brief Text input/edit widget with full editing support
 *
 * Features:
 * - Text selection (mouse drag, Shift+arrow keys, double-click word select)
 * - Copy/Cut/Paste (Ctrl+C/X/V)
 * - Undo/Redo (Ctrl+Z/Y)
 * - Word navigation (Ctrl+Left/Right)
 * - Home/End navigation
 * - Select all (Ctrl+A)
 */
class DAKTLIB_GUI_API TextInput : public Widget {
  public:
    TextInput();

    const std::string& getText() const { return text_; }
    void setText(const std::string& text);

    const std::string& getPlaceholder() const { return placeholder_; }
    void setPlaceholder(const std::string& placeholder) {
        placeholder_ = placeholder;
        markDirty();
    }

    bool isPassword() const { return password_; }
    void setPassword(bool password) {
        password_ = password;
        markDirty();
    }

    bool isMultiline() const { return multiline_; }
    void setMultiline(bool multiline) {
        multiline_ = multiline;
        markDirty();
    }

    bool isReadOnly() const { return readOnly_; }
    void setReadOnly(bool readOnly) { readOnly_ = readOnly; }

    size_t getMaxLength() const { return maxLength_; }
    void setMaxLength(size_t length) { maxLength_ = length; }

    // Cursor/selection
    size_t getCursorPosition() const { return cursorPos_; }
    void setCursorPosition(size_t pos);

    void selectAll();
    void clearSelection();
    bool hasSelection() const { return selectionStart_ != selectionEnd_; }
    std::string getSelectedText() const;
    void setSelection(size_t start, size_t end);

    // Clipboard operations
    void copy();
    void cut();
    void paste();

    // Undo/redo
    bool canUndo() const { return undoIndex_ > 0; }
    bool canRedo() const { return undoIndex_ < undoStack_.size(); }
    void undo();
    void redo();
    void clearUndoHistory();

    size_t getUndoLimit() const { return undoLimit_; }
    void setUndoLimit(size_t limit) { undoLimit_ = limit; }

    // Colors
    Color getBackgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const Color& color) {
        backgroundColor_ = color;
        markDirty();
    }

    Color getBorderColor() const { return borderColor_; }
    void setBorderColor(const Color& color) {
        borderColor_ = color;
        markDirty();
    }

    Color getTextColor() const { return textColor_; }
    void setTextColor(const Color& color) {
        textColor_ = color;
        markDirty();
    }

    Color getSelectionColor() const { return selectionColor_; }
    void setSelectionColor(const Color& color) {
        selectionColor_ = color;
        markDirty();
    }

    Vec2 measureContent() override;
    bool handleInput(const WidgetEvent& event) override;
    void drawBackground(DrawList& drawList) override;
    void drawContent(DrawList& drawList) override;

  private:
    // Text manipulation
    void insertText(const std::string& str);
    void deleteSelection();
    void deleteCharacter(bool forward);
    void deleteWord(bool forward);

    // Cursor/selection
    void moveCursor(int delta, bool select);
    void moveCursorWord(bool forward, bool select);
    void moveCursorToLineEnd(bool forward, bool select);
    size_t findWordBoundary(size_t pos, bool forward) const;
    size_t getCharIndexAtPosition(float x) const;
    float getPositionOfChar(size_t index) const;

    // Undo/redo helpers
    void recordAction(TextEditAction::Type type, size_t pos, const std::string& text, const std::string& replaced = "");
    void applyAction(const TextEditAction& action, bool undo);

    std::string text_;
    std::string placeholder_;
    size_t cursorPos_ = 0;
    size_t selectionStart_ = 0;
    size_t selectionEnd_ = 0;
    size_t selectionAnchor_ = 0; // Fixed end of selection when extending
    size_t maxLength_ = 1024;
    bool password_ = false;
    bool multiline_ = false;
    bool readOnly_ = false;

    // Visual state
    float cursorBlinkTime_ = 0.0f;
    bool cursorVisible_ = true;
    float scrollOffset_ = 0.0f; // Horizontal scroll for long text

    // Interaction state
    bool selecting_ = false; // Mouse drag selection in progress
    bool doubleClickSelect_ = false;

    // Undo/redo
    std::vector<TextEditAction> undoStack_;
    size_t undoIndex_ = 0;
    size_t undoLimit_ = 100;
    bool recordingAction_ = true; // Set false during undo/redo

    // Colors
    Color backgroundColor_{40, 40, 50, 255};
    Color borderColor_{80, 80, 100, 255};
    Color textColor_{220, 220, 220, 255};
    Color selectionColor_{51, 153, 255, 100};
    Color placeholderColor_{128, 128, 128, 255};
};

} // namespace dakt::gui

#endif // DAKTLIB_GUI_TEXT_INPUT_HPP

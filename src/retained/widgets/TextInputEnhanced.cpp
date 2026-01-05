#include "dakt/gui/draw/DrawList.hpp"
#include "dakt/gui/retained/widgets/TextInput.hpp"
#include <algorithm>
#include <cctype>

namespace dakt::gui {

// Implementation of enhanced TextInput methods
// The basic methods are in Widget.cpp - these are the new enhanced features

void TextInput::setSelection(size_t start, size_t end) {
    selectionStart_ = std::min(start, text_.length());
    selectionEnd_ = std::min(end, text_.length());
    selectionAnchor_ = selectionStart_;
    cursorPos_ = selectionEnd_;
    markDirty();
}

void TextInput::copy() {
    if (!hasSelection())
        return;

    // Store in internal clipboard (platform clipboard integration would go here)
    // For now we just have the selected text available via getSelectedText()
    // Platform-specific clipboard integration should be added in the future
}

void TextInput::cut() {
    if (!hasSelection() || readOnly_)
        return;

    std::string selected = getSelectedText();
    copy();

    // Record for undo
    size_t start = std::min(selectionStart_, selectionEnd_);
    recordAction(TextEditAction::Type::Delete, start, selected);

    deleteSelection();
}

void TextInput::paste() {
    if (readOnly_)
        return;

    // Platform-specific clipboard integration would go here
    // For now this is a placeholder that would be called with clipboard text
}

void TextInput::undo() {
    if (!canUndo())
        return;

    recordingAction_ = false;
    undoIndex_--;
    applyAction(undoStack_[undoIndex_], true);
    recordingAction_ = true;
    markDirty();
}

void TextInput::redo() {
    if (!canRedo())
        return;

    recordingAction_ = false;
    applyAction(undoStack_[undoIndex_], false);
    undoIndex_++;
    recordingAction_ = true;
    markDirty();
}

void TextInput::clearUndoHistory() {
    undoStack_.clear();
    undoIndex_ = 0;
}

void TextInput::deleteCharacter(bool forward) {
    if (hasSelection()) {
        std::string deleted = getSelectedText();
        size_t pos = std::min(selectionStart_, selectionEnd_);
        recordAction(TextEditAction::Type::Delete, pos, deleted);
        deleteSelection();
        return;
    }

    if (forward) {
        // Delete character after cursor
        if (cursorPos_ < text_.length()) {
            std::string deleted = text_.substr(cursorPos_, 1);
            recordAction(TextEditAction::Type::Delete, cursorPos_, deleted);
            text_.erase(cursorPos_, 1);
            markDirty();
        }
    } else {
        // Delete character before cursor (backspace)
        if (cursorPos_ > 0) {
            std::string deleted = text_.substr(cursorPos_ - 1, 1);
            recordAction(TextEditAction::Type::Delete, cursorPos_ - 1, deleted);
            text_.erase(cursorPos_ - 1, 1);
            cursorPos_--;
            markDirty();
        }
    }
}

void TextInput::deleteWord(bool forward) {
    if (hasSelection()) {
        deleteCharacter(forward); // Just delete selection
        return;
    }

    size_t boundary = findWordBoundary(cursorPos_, forward);

    if (forward) {
        if (boundary > cursorPos_) {
            std::string deleted = text_.substr(cursorPos_, boundary - cursorPos_);
            recordAction(TextEditAction::Type::Delete, cursorPos_, deleted);
            text_.erase(cursorPos_, boundary - cursorPos_);
            markDirty();
        }
    } else {
        if (boundary < cursorPos_) {
            std::string deleted = text_.substr(boundary, cursorPos_ - boundary);
            recordAction(TextEditAction::Type::Delete, boundary, deleted);
            text_.erase(boundary, cursorPos_ - boundary);
            cursorPos_ = boundary;
            markDirty();
        }
    }
}

void TextInput::moveCursorWord(bool forward, bool select) {
    size_t newPos = findWordBoundary(cursorPos_, forward);

    if (select) {
        if (!hasSelection()) {
            selectionAnchor_ = cursorPos_;
            selectionStart_ = cursorPos_;
        }
        selectionEnd_ = newPos;
        // Normalize selection
        if (selectionStart_ > selectionEnd_) {
            selectionStart_ = selectionEnd_;
            selectionEnd_ = selectionAnchor_;
        } else {
            selectionStart_ = selectionAnchor_;
        }
    } else {
        clearSelection();
    }

    cursorPos_ = newPos;
    markDirty();
}

void TextInput::moveCursorToLineEnd(bool forward, bool select) {
    size_t newPos = forward ? text_.length() : 0;

    // For multiline, find actual line end
    if (multiline_) {
        if (forward) {
            size_t pos = text_.find('\n', cursorPos_);
            newPos = (pos != std::string::npos) ? pos : text_.length();
        } else {
            size_t pos = text_.rfind('\n', cursorPos_ > 0 ? cursorPos_ - 1 : 0);
            newPos = (pos != std::string::npos) ? pos + 1 : 0;
        }
    }

    if (select) {
        if (!hasSelection()) {
            selectionAnchor_ = cursorPos_;
            selectionStart_ = cursorPos_;
        }
        selectionEnd_ = newPos;
        if (selectionStart_ > selectionEnd_) {
            selectionStart_ = selectionEnd_;
            selectionEnd_ = selectionAnchor_;
        } else {
            selectionStart_ = selectionAnchor_;
        }
    } else {
        clearSelection();
    }

    cursorPos_ = newPos;
    markDirty();
}

size_t TextInput::findWordBoundary(size_t pos, bool forward) const {
    if (text_.empty())
        return 0;

    if (forward) {
        // Skip current word
        while (pos < text_.length() && !std::isspace(static_cast<unsigned char>(text_[pos]))) {
            pos++;
        }
        // Skip whitespace
        while (pos < text_.length() && std::isspace(static_cast<unsigned char>(text_[pos]))) {
            pos++;
        }
        return pos;
    } else {
        if (pos == 0)
            return 0;
        pos--;
        // Skip whitespace
        while (pos > 0 && std::isspace(static_cast<unsigned char>(text_[pos]))) {
            pos--;
        }
        // Skip word
        while (pos > 0 && !std::isspace(static_cast<unsigned char>(text_[pos - 1]))) {
            pos--;
        }
        return pos;
    }
}

size_t TextInput::getCharIndexAtPosition(float x) const {
    float textX = bounds_.x + padding_.left - scrollOffset_;
    float relativeX = x - textX;

    if (relativeX <= 0)
        return 0;

    // Simple fixed-width calculation (8 pixels per character)
    size_t index = static_cast<size_t>(relativeX / 8.0f + 0.5f);
    return std::min(index, text_.length());
}

float TextInput::getPositionOfChar(size_t index) const {
    float textX = bounds_.x + padding_.left - scrollOffset_;
    return textX + static_cast<float>(index) * 8.0f;
}

void TextInput::recordAction(TextEditAction::Type type, size_t pos, const std::string& text, const std::string& replaced) {
    if (!recordingAction_)
        return;

    // Remove any redo actions
    if (undoIndex_ < undoStack_.size()) {
        undoStack_.resize(undoIndex_);
    }

    TextEditAction action;
    action.type = type;
    action.position = pos;
    action.text = text;
    action.replacedText = replaced;
    action.cursorBefore = cursorPos_;
    action.cursorAfter = (type == TextEditAction::Type::Insert) ? pos + text.length() : pos;

    // Try to merge with previous action for consecutive typing
    if (!undoStack_.empty() && type == TextEditAction::Type::Insert) {
        auto& last = undoStack_.back();
        if (last.type == TextEditAction::Type::Insert && last.position + last.text.length() == pos && text.length() == 1 && !std::isspace(static_cast<unsigned char>(text[0]))) {
            // Merge with previous insert
            last.text += text;
            last.cursorAfter = action.cursorAfter;
            return;
        }
    }

    undoStack_.push_back(action);
    undoIndex_ = undoStack_.size();

    // Enforce undo limit
    while (undoStack_.size() > undoLimit_) {
        undoStack_.erase(undoStack_.begin());
        undoIndex_--;
    }
}

void TextInput::applyAction(const TextEditAction& action, bool undo) {
    if (undo) {
        switch (action.type) {
        case TextEditAction::Type::Insert:
            // Undo insert = delete the text
            text_.erase(action.position, action.text.length());
            cursorPos_ = action.cursorBefore;
            break;

        case TextEditAction::Type::Delete:
            // Undo delete = insert the text back
            text_.insert(action.position, action.text);
            cursorPos_ = action.cursorBefore;
            break;

        case TextEditAction::Type::Replace:
            // Undo replace = put back original text
            text_.erase(action.position, action.text.length());
            text_.insert(action.position, action.replacedText);
            cursorPos_ = action.cursorBefore;
            break;
        }
    } else {
        // Redo
        switch (action.type) {
        case TextEditAction::Type::Insert:
            text_.insert(action.position, action.text);
            cursorPos_ = action.cursorAfter;
            break;

        case TextEditAction::Type::Delete:
            text_.erase(action.position, action.text.length());
            cursorPos_ = action.cursorAfter;
            break;

        case TextEditAction::Type::Replace:
            text_.erase(action.position, action.replacedText.length());
            text_.insert(action.position, action.text);
            cursorPos_ = action.cursorAfter;
            break;
        }
    }

    clearSelection();
}

} // namespace dakt::gui

#include "dakt/gui/text/TextCursor.hpp"
#include "dakt/gui/draw/DrawList.hpp"
#include <algorithm>
#include <cmath>

namespace dakt::gui {

TextCursor::TextCursor() = default;
TextCursor::~TextCursor() = default;

void TextCursor::init(const ShapedRun& run, const std::string& text, float fontSize, float ascender) {
    shapedRun_ = run;
    text_ = text;
    fontSize_ = fontSize;
    ascender_ = ascender;
    lineHeight_ = fontSize * 1.2f; // Default line height

    // Build character position cache
    charPositions_.clear();
    charPositions_.reserve(run.glyphs.size() + 1);

    float x = 0;
    charPositions_.push_back(x); // Position before first char

    for (const auto& glyph : run.glyphs) {
        x += glyph.xAdvance * fontSize / 1000.0f; // Convert from font units
        charPositions_.push_back(x);
    }

    // Reset cursor and selection
    cursor_ = CursorPosition{};
    selection_ = TextSelection{};
    blinkTimer_ = 0;
    cursorVisible_ = true;
}

void TextCursor::setPosition(size_t charIndex) {
    charIndex = std::min(charIndex, charPositions_.size() > 0 ? charPositions_.size() - 1 : 0);

    cursor_.charIndex = charIndex;
    cursor_.x = getXForCharIndex(charIndex);
    cursor_.y = 0;

    // Reset blink
    blinkTimer_ = 0;
    cursorVisible_ = true;
}

void TextCursor::moveLeft(bool extendSelection) {
    size_t newPos = cursor_.charIndex > 0 ? cursor_.charIndex - 1 : 0;

    if (extendSelection) {
        updateSelection(newPos, true);
    } else {
        if (hasSelection()) {
            // Move to start of selection
            selection_.normalize();
            newPos = selection_.start;
            clearSelection();
        }
    }

    setPosition(newPos);
}

void TextCursor::moveRight(bool extendSelection) {
    size_t maxPos = charPositions_.size() > 0 ? charPositions_.size() - 1 : 0;
    size_t newPos = cursor_.charIndex < maxPos ? cursor_.charIndex + 1 : maxPos;

    if (extendSelection) {
        updateSelection(newPos, true);
    } else {
        if (hasSelection()) {
            // Move to end of selection
            selection_.normalize();
            newPos = selection_.end;
            clearSelection();
        }
    }

    setPosition(newPos);
}

void TextCursor::moveToStart(bool extendSelection) {
    if (extendSelection) {
        updateSelection(0, true);
    } else {
        clearSelection();
    }
    setPosition(0);
}

void TextCursor::moveToEnd(bool extendSelection) {
    size_t endPos = charPositions_.size() > 0 ? charPositions_.size() - 1 : 0;

    if (extendSelection) {
        updateSelection(endPos, true);
    } else {
        clearSelection();
    }
    setPosition(endPos);
}

size_t TextCursor::hitTest(float x) { return getCharIndexFromX(x); }

void TextCursor::setSelection(size_t start, size_t end) {
    size_t maxPos = charPositions_.size() > 0 ? charPositions_.size() - 1 : 0;

    selection_.start = std::min(start, maxPos);
    selection_.end = std::min(end, maxPos);
    selection_.startX = getXForCharIndex(selection_.start);
    selection_.endX = getXForCharIndex(selection_.end);
}

void TextCursor::clearSelection() { selection_ = TextSelection{}; }

void TextCursor::selectAll() {
    if (charPositions_.empty())
        return;

    setSelection(0, charPositions_.size() - 1);
}

std::string TextCursor::getSelectedText() const {
    if (selection_.isEmpty() || text_.empty())
        return "";

    TextSelection sel = selection_;
    sel.normalize();

    // Convert character indices to byte indices (UTF-8 aware)
    size_t byteStart = 0;
    size_t byteEnd = 0;
    size_t charIdx = 0;

    for (size_t i = 0; i < text_.length();) {
        if (charIdx == sel.start) {
            byteStart = i;
        }
        if (charIdx == sel.end) {
            byteEnd = i;
            break;
        }

        // UTF-8 byte length
        unsigned char c = text_[i];
        if ((c & 0x80) == 0) {
            i += 1;
        } else if ((c & 0xE0) == 0xC0) {
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            i += 4;
        } else {
            i += 1;
        }

        ++charIdx;
    }

    if (byteEnd == 0 && charIdx >= sel.end) {
        byteEnd = text_.length();
    }

    return text_.substr(byteStart, byteEnd - byteStart);
}

void TextCursor::update(float deltaTime) {
    if (!blinkEnabled_) {
        cursorVisible_ = true;
        return;
    }

    blinkTimer_ += deltaTime;

    if (blinkTimer_ >= blinkRate_) {
        blinkTimer_ -= blinkRate_;
        cursorVisible_ = !cursorVisible_;
    }
}

void TextCursor::drawCursor(DrawList& drawList, float originX, float originY, Color color) {
    if (!cursorVisible_ && blinkEnabled_)
        return;

    float x = originX + cursor_.x;
    float y = originY - ascender_; // Top of line
    float cursorWidth = 2.0f;
    float cursorHeight = lineHeight_;

    Rect cursorRect(x, y, cursorWidth, cursorHeight);
    drawList.drawRectFilled(cursorRect, color);
}

void TextCursor::drawSelection(DrawList& drawList, float originX, float originY, Color color) {
    if (selection_.isEmpty())
        return;

    TextSelection sel = selection_;
    sel.normalize();

    float x1 = originX + sel.startX;
    float x2 = originX + sel.endX;
    float y = originY - ascender_;
    float height = lineHeight_;

    Rect selRect(x1, y, x2 - x1, height);
    drawList.drawRectFilled(selRect, color);
}

float TextCursor::getXForCharIndex(size_t charIndex) const {
    if (charPositions_.empty())
        return 0;
    if (charIndex >= charPositions_.size())
        return charPositions_.back();
    return charPositions_[charIndex];
}

size_t TextCursor::getCharIndexFromX(float x) const {
    if (charPositions_.empty())
        return 0;

    // Find closest character position
    size_t closest = 0;
    float closestDist = std::abs(charPositions_[0] - x);

    for (size_t i = 1; i < charPositions_.size(); ++i) {
        float dist = std::abs(charPositions_[i] - x);
        if (dist < closestDist) {
            closestDist = dist;
            closest = i;
        }
    }

    return closest;
}

void TextCursor::updateSelection(size_t newIndex, bool extend) {
    if (!extend) {
        clearSelection();
        return;
    }

    // If no selection, start from current cursor
    if (selection_.isEmpty()) {
        selection_.start = cursor_.charIndex;
        selection_.startX = cursor_.x;
    }

    // Extend to new position
    selection_.end = newIndex;
    selection_.endX = getXForCharIndex(newIndex);
}

} // namespace dakt::gui

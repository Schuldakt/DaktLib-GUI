#include "dakt/gui/immediate/Widgets/InputText.hpp"
#include "dakt/gui/core/Types.hpp"
#include "dakt/gui/immediate/internal//ImmediateState.hpp"
#include "dakt/gui/immediate/internal/ImmediateItem.hpp"

namespace dakt::gui {


bool inputText(const char* label, char* buf, size_t bufSize, InputTextFlags flags) {
    // TODO: implement text editing
    // For now, render label + box, handle click focus, and return false.
    (void)label; (void)buf; (void)bufSize; (void)flags;
    return false;
}

bool inputTextMultiline(const char* label, char* buf, size_t bufSize, Vec2 size, InputTextFlags flags) {
    (void)label; (void)buf; (void)bufSize; (void)size; (void)flags;
    return false;
}

bool inputInt(const char* label, int* value, int step, int stepFast) {
    (void)stepFast;
    // Minimal: treat as inputText + parse; stub:
    (void)label; (void)value; (void)step;
    return false;
}

bool inputFloat(const char* label, float* value, float step, float stepFast, const char* format) {
    (void)format; (void)stepFast;
    (void)label; (void)value; (void)step;
    return false;
}

} // namespace dakt::gui
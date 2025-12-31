// ============================================================================
// DaktLib GUI Module - Group & Layout Containers
// ============================================================================

#pragma once

#include <dakt/core/Types.hpp>
#include <dakt/gui/core/Types.hpp>

namespace dakt::gui
{

using namespace dakt::core;

// ============================================================================
// Group Functions
// ============================================================================

// Group widgets together (for layout purposes)
void beginGroup();
void endGroup();

// Dummy space (for alignment)
void dummy(Vec2 size);

// Separator line
void separator();

// Spacing (small empty space)
void spacing();

// Same line (continue on same line as previous widget)
void sameLine(f32 offsetFromStartX = 0.0f, f32 spacing = -1.0f);

// New line (force a new line)
void newLine();

// Indentation
void indent(f32 indentWidth = 0.0f);
void unindent(f32 indentWidth = 0.0f);

}  // namespace dakt::gui

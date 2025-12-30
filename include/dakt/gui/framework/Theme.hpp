// ============================================================================
// DaktLib GUI Module - Theme
// ============================================================================
// Color schemes and styling for the GUI.
// ============================================================================

#pragma once

#include <dakt/gui/core/Types.hpp>

#include <memory>

namespace dakt::gui
{

// ============================================================================
// Style Variables
// ============================================================================

enum class StyleVar : i32
{
    Alpha,
    WindowPadding,
    WindowRounding,
    WindowBorderSize,
    WindowMinSize,
    ChildRounding,
    ChildBorderSize,
    FramePadding,
    FrameRounding,
    FrameBorderSize,
    ItemSpacing,
    ItemInnerSpacing,
    IndentSpacing,
    ScrollbarSize,
    ScrollbarRounding,
    GrabMinSize,
    GrabRounding,
    TabRounding,
    ButtonTextAlign,
    SelectableTextAlign,
    Count
};

// ============================================================================
// Style Colors
// ============================================================================

enum class StyleColor : i32
{
    Text,
    TextDisabled,
    WindowBg,
    ChildBg,
    PopupBg,
    Border,
    BorderShadow,
    FrameBg,
    FrameBgHovered,
    FrameBgActive,
    TitleBg,
    TitleBgActive,
    TitleBgCollapsed,
    MenuBarBg,
    ScrollbarBg,
    ScrollbarGrab,
    ScrollbarGrabHovered,
    ScrollbarGrabActive,
    CheckMark,
    SliderGrab,
    SliderGrabActive,
    Button,
    ButtonHovered,
    ButtonActive,
    Header,
    HeaderHovered,
    HeaderActive,
    Separator,
    SeparatorHovered,
    SeparatorActive,
    ResizeGrip,
    ResizeGripHovered,
    ResizeGripActive,
    Tab,
    TabHovered,
    TabActive,
    TabUnfocused,
    TabUnfocusedActive,
    PlotLines,
    PlotLinesHovered,
    PlotHistogram,
    PlotHistogramHovered,
    TableHeaderBg,
    TableBorderStrong,
    TableBorderLight,
    TableRowBg,
    TableRowBgAlt,
    TextSelectedBg,
    DragDropTarget,
    NavHighlight,
    NavWindowingHighlight,
    NavWindowingDimBg,
    ModalWindowDimBg,
    Count
};

// ============================================================================
// Theme
// ============================================================================

class Theme
{
public:
    Theme();
    ~Theme() = default;

    // Colors
    Color text;
    Color textDisabled;
    Color windowBg;
    Color childBg;
    Color popupBg;
    Color border;
    Color borderShadow;
    Color frameBg;
    Color frameBgHovered;
    Color frameBgActive;
    Color titleBg;
    Color titleBgActive;
    Color titleBgCollapsed;
    Color menuBarBg;
    Color scrollbarBg;
    Color scrollbarGrab;
    Color scrollbarGrabHovered;
    Color scrollbarGrabActive;
    Color checkMark;
    Color sliderGrab;
    Color sliderGrabActive;
    Color button;
    Color buttonHovered;
    Color buttonActive;
    Color header;
    Color headerHovered;
    Color headerActive;
    Color separator;
    Color separatorHovered;
    Color separatorActive;
    Color resizeGrip;
    Color resizeGripHovered;
    Color resizeGripActive;
    Color tab;
    Color tabHovered;
    Color tabActive;
    Color tabUnfocused;
    Color tabUnfocusedActive;
    Color plotLines;
    Color plotLinesHovered;
    Color plotHistogram;
    Color plotHistogramHovered;
    Color tableHeaderBg;
    Color tableBorderStrong;
    Color tableBorderLight;
    Color tableRowBg;
    Color tableRowBgAlt;
    Color textSelectedBg;
    Color dragDropTarget;
    Color navHighlight;
    Color navWindowingHighlight;
    Color navWindowingDimBg;
    Color modalWindowDimBg;

    // Sizes and spacing
    f32 alpha = 1.0f;
    Vec2 windowPadding = {8, 8};
    f32 windowRounding = 4.0f;
    f32 windowBorderSize = 1.0f;
    Vec2 windowMinSize = {32, 32};
    f32 childRounding = 0.0f;
    f32 childBorderSize = 1.0f;
    Vec2 framePadding = {4, 3};
    f32 frameRounding = 2.0f;
    f32 frameBorderSize = 0.0f;
    Vec2 itemSpacing = {8, 4};
    Vec2 itemInnerSpacing = {4, 4};
    f32 indentSpacing = 21.0f;
    f32 scrollbarSize = 14.0f;
    f32 scrollbarRounding = 9.0f;
    f32 grabMinSize = 10.0f;
    f32 grabRounding = 2.0f;
    f32 tabRounding = 4.0f;
    Vec2 buttonTextAlign = {0.5f, 0.5f};
    Vec2 selectableTextAlign = {0.0f, 0.0f};

    // Color access by enum
    [[nodiscard]] Color getColor(StyleColor color) const;
    void setColor(StyleColor color, Color value);

    // Variable access by enum
    [[nodiscard]] f32 getVarFloat(StyleVar var) const;
    [[nodiscard]] Vec2 getVarVec2(StyleVar var) const;
    void setVar(StyleVar var, f32 value);
    void setVar(StyleVar var, Vec2 value);

    // Preset themes
    static std::unique_ptr<Theme> dark();
    static std::unique_ptr<Theme> light();
    static std::unique_ptr<Theme> classic();
    static std::unique_ptr<Theme> starCitizen();  // Custom SC-themed
};

}  // namespace dakt::gui

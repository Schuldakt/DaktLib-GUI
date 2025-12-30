// ============================================================================
// DaktLib GUI Module - Theme Implementation
// ============================================================================

#include <dakt/gui/framework/Theme.hpp>

namespace dakt::gui
{

Theme::Theme()
{
    // Set default dark theme colors
    text = Color{255, 255, 255, 255};
    textDisabled = Color{128, 128, 128, 255};
    windowBg = Color{30, 30, 30, 240};
    childBg = Color{0, 0, 0, 0};
    popupBg = Color{40, 40, 40, 240};
    border = Color{80, 80, 80, 128};
    borderShadow = Color{0, 0, 0, 0};
    frameBg = Color{50, 50, 50, 200};
    frameBgHovered = Color{70, 70, 70, 200};
    frameBgActive = Color{90, 90, 90, 200};
    titleBg = Color{25, 25, 25, 255};
    titleBgActive = Color{40, 70, 120, 255};
    titleBgCollapsed = Color{25, 25, 25, 130};
    menuBarBg = Color{35, 35, 35, 255};
    scrollbarBg = Color{25, 25, 25, 200};
    scrollbarGrab = Color{80, 80, 80, 255};
    scrollbarGrabHovered = Color{100, 100, 100, 255};
    scrollbarGrabActive = Color{120, 120, 120, 255};
    checkMark = Color{100, 160, 220, 255};
    sliderGrab = Color{100, 160, 220, 255};
    sliderGrabActive = Color{120, 180, 240, 255};
    button = Color{60, 100, 160, 200};
    buttonHovered = Color{80, 120, 180, 255};
    buttonActive = Color{100, 140, 200, 255};
    header = Color{60, 100, 160, 200};
    headerHovered = Color{80, 120, 180, 255};
    headerActive = Color{100, 140, 200, 255};
    separator = Color{80, 80, 80, 255};
    separatorHovered = Color{120, 120, 120, 255};
    separatorActive = Color{160, 160, 160, 255};
    resizeGrip = Color{60, 60, 60, 64};
    resizeGripHovered = Color{100, 160, 220, 170};
    resizeGripActive = Color{100, 160, 220, 240};
    tab = Color{45, 45, 45, 220};
    tabHovered = Color{80, 120, 180, 255};
    tabActive = Color{60, 100, 160, 255};
    tabUnfocused = Color{35, 35, 35, 220};
    tabUnfocusedActive = Color{50, 70, 100, 255};
    plotLines = Color{156, 156, 156, 255};
    plotLinesHovered = Color{255, 110, 89, 255};
    plotHistogram = Color{230, 179, 0, 255};
    plotHistogramHovered = Color{255, 153, 0, 255};
    tableHeaderBg = Color{48, 48, 51, 255};
    tableBorderStrong = Color{79, 79, 89, 255};
    tableBorderLight = Color{59, 59, 64, 255};
    tableRowBg = Color{0, 0, 0, 0};
    tableRowBgAlt = Color{255, 255, 255, 15};
    textSelectedBg = Color{60, 100, 160, 180};
    dragDropTarget = Color{255, 255, 0, 230};
    navHighlight = Color{100, 160, 220, 255};
    navWindowingHighlight = Color{255, 255, 255, 178};
    navWindowingDimBg = Color{204, 204, 204, 51};
    modalWindowDimBg = Color{204, 204, 204, 89};
}

Color Theme::getColor(StyleColor color) const
{
    switch (color)
    {
        case StyleColor::Text:
            return text;
        case StyleColor::TextDisabled:
            return textDisabled;
        case StyleColor::WindowBg:
            return windowBg;
        case StyleColor::ChildBg:
            return childBg;
        case StyleColor::PopupBg:
            return popupBg;
        case StyleColor::Border:
            return border;
        case StyleColor::BorderShadow:
            return borderShadow;
        case StyleColor::FrameBg:
            return frameBg;
        case StyleColor::FrameBgHovered:
            return frameBgHovered;
        case StyleColor::FrameBgActive:
            return frameBgActive;
        case StyleColor::TitleBg:
            return titleBg;
        case StyleColor::TitleBgActive:
            return titleBgActive;
        case StyleColor::TitleBgCollapsed:
            return titleBgCollapsed;
        case StyleColor::MenuBarBg:
            return menuBarBg;
        case StyleColor::ScrollbarBg:
            return scrollbarBg;
        case StyleColor::ScrollbarGrab:
            return scrollbarGrab;
        case StyleColor::ScrollbarGrabHovered:
            return scrollbarGrabHovered;
        case StyleColor::ScrollbarGrabActive:
            return scrollbarGrabActive;
        case StyleColor::CheckMark:
            return checkMark;
        case StyleColor::SliderGrab:
            return sliderGrab;
        case StyleColor::SliderGrabActive:
            return sliderGrabActive;
        case StyleColor::Button:
            return button;
        case StyleColor::ButtonHovered:
            return buttonHovered;
        case StyleColor::ButtonActive:
            return buttonActive;
        case StyleColor::Header:
            return header;
        case StyleColor::HeaderHovered:
            return headerHovered;
        case StyleColor::HeaderActive:
            return headerActive;
        case StyleColor::Separator:
            return separator;
        case StyleColor::SeparatorHovered:
            return separatorHovered;
        case StyleColor::SeparatorActive:
            return separatorActive;
        case StyleColor::ResizeGrip:
            return resizeGrip;
        case StyleColor::ResizeGripHovered:
            return resizeGripHovered;
        case StyleColor::ResizeGripActive:
            return resizeGripActive;
        case StyleColor::Tab:
            return tab;
        case StyleColor::TabHovered:
            return tabHovered;
        case StyleColor::TabActive:
            return tabActive;
        case StyleColor::TabUnfocused:
            return tabUnfocused;
        case StyleColor::TabUnfocusedActive:
            return tabUnfocusedActive;
        case StyleColor::PlotLines:
            return plotLines;
        case StyleColor::PlotLinesHovered:
            return plotLinesHovered;
        case StyleColor::PlotHistogram:
            return plotHistogram;
        case StyleColor::PlotHistogramHovered:
            return plotHistogramHovered;
        case StyleColor::TableHeaderBg:
            return tableHeaderBg;
        case StyleColor::TableBorderStrong:
            return tableBorderStrong;
        case StyleColor::TableBorderLight:
            return tableBorderLight;
        case StyleColor::TableRowBg:
            return tableRowBg;
        case StyleColor::TableRowBgAlt:
            return tableRowBgAlt;
        case StyleColor::TextSelectedBg:
            return textSelectedBg;
        case StyleColor::DragDropTarget:
            return dragDropTarget;
        case StyleColor::NavHighlight:
            return navHighlight;
        case StyleColor::NavWindowingHighlight:
            return navWindowingHighlight;
        case StyleColor::NavWindowingDimBg:
            return navWindowingDimBg;
        case StyleColor::ModalWindowDimBg:
            return modalWindowDimBg;
        default:
            return Color::white();
    }
}

void Theme::setColor(StyleColor color, Color value)
{
    switch (color)
    {
        case StyleColor::Text:
            text = value;
            break;
        case StyleColor::TextDisabled:
            textDisabled = value;
            break;
        case StyleColor::WindowBg:
            windowBg = value;
            break;
        case StyleColor::ChildBg:
            childBg = value;
            break;
        case StyleColor::PopupBg:
            popupBg = value;
            break;
        case StyleColor::Border:
            border = value;
            break;
        case StyleColor::BorderShadow:
            borderShadow = value;
            break;
        case StyleColor::FrameBg:
            frameBg = value;
            break;
        case StyleColor::FrameBgHovered:
            frameBgHovered = value;
            break;
        case StyleColor::FrameBgActive:
            frameBgActive = value;
            break;
        case StyleColor::TitleBg:
            titleBg = value;
            break;
        case StyleColor::TitleBgActive:
            titleBgActive = value;
            break;
        case StyleColor::TitleBgCollapsed:
            titleBgCollapsed = value;
            break;
        case StyleColor::MenuBarBg:
            menuBarBg = value;
            break;
        case StyleColor::ScrollbarBg:
            scrollbarBg = value;
            break;
        case StyleColor::ScrollbarGrab:
            scrollbarGrab = value;
            break;
        case StyleColor::ScrollbarGrabHovered:
            scrollbarGrabHovered = value;
            break;
        case StyleColor::ScrollbarGrabActive:
            scrollbarGrabActive = value;
            break;
        case StyleColor::CheckMark:
            checkMark = value;
            break;
        case StyleColor::SliderGrab:
            sliderGrab = value;
            break;
        case StyleColor::SliderGrabActive:
            sliderGrabActive = value;
            break;
        case StyleColor::Button:
            button = value;
            break;
        case StyleColor::ButtonHovered:
            buttonHovered = value;
            break;
        case StyleColor::ButtonActive:
            buttonActive = value;
            break;
        case StyleColor::Header:
            header = value;
            break;
        case StyleColor::HeaderHovered:
            headerHovered = value;
            break;
        case StyleColor::HeaderActive:
            headerActive = value;
            break;
        case StyleColor::Separator:
            separator = value;
            break;
        case StyleColor::SeparatorHovered:
            separatorHovered = value;
            break;
        case StyleColor::SeparatorActive:
            separatorActive = value;
            break;
        case StyleColor::ResizeGrip:
            resizeGrip = value;
            break;
        case StyleColor::ResizeGripHovered:
            resizeGripHovered = value;
            break;
        case StyleColor::ResizeGripActive:
            resizeGripActive = value;
            break;
        case StyleColor::Tab:
            tab = value;
            break;
        case StyleColor::TabHovered:
            tabHovered = value;
            break;
        case StyleColor::TabActive:
            tabActive = value;
            break;
        case StyleColor::TabUnfocused:
            tabUnfocused = value;
            break;
        case StyleColor::TabUnfocusedActive:
            tabUnfocusedActive = value;
            break;
        case StyleColor::PlotLines:
            plotLines = value;
            break;
        case StyleColor::PlotLinesHovered:
            plotLinesHovered = value;
            break;
        case StyleColor::PlotHistogram:
            plotHistogram = value;
            break;
        case StyleColor::PlotHistogramHovered:
            plotHistogramHovered = value;
            break;
        case StyleColor::TableHeaderBg:
            tableHeaderBg = value;
            break;
        case StyleColor::TableBorderStrong:
            tableBorderStrong = value;
            break;
        case StyleColor::TableBorderLight:
            tableBorderLight = value;
            break;
        case StyleColor::TableRowBg:
            tableRowBg = value;
            break;
        case StyleColor::TableRowBgAlt:
            tableRowBgAlt = value;
            break;
        case StyleColor::TextSelectedBg:
            textSelectedBg = value;
            break;
        case StyleColor::DragDropTarget:
            dragDropTarget = value;
            break;
        case StyleColor::NavHighlight:
            navHighlight = value;
            break;
        case StyleColor::NavWindowingHighlight:
            navWindowingHighlight = value;
            break;
        case StyleColor::NavWindowingDimBg:
            navWindowingDimBg = value;
            break;
        case StyleColor::ModalWindowDimBg:
            modalWindowDimBg = value;
            break;
        default:
            break;
    }
}

f32 Theme::getVarFloat(StyleVar var) const
{
    switch (var)
    {
        case StyleVar::Alpha:
            return alpha;
        case StyleVar::WindowRounding:
            return windowRounding;
        case StyleVar::WindowBorderSize:
            return windowBorderSize;
        case StyleVar::ChildRounding:
            return childRounding;
        case StyleVar::ChildBorderSize:
            return childBorderSize;
        case StyleVar::FrameRounding:
            return frameRounding;
        case StyleVar::FrameBorderSize:
            return frameBorderSize;
        case StyleVar::IndentSpacing:
            return indentSpacing;
        case StyleVar::ScrollbarSize:
            return scrollbarSize;
        case StyleVar::ScrollbarRounding:
            return scrollbarRounding;
        case StyleVar::GrabMinSize:
            return grabMinSize;
        case StyleVar::GrabRounding:
            return grabRounding;
        case StyleVar::TabRounding:
            return tabRounding;
        default:
            return 0.0f;
    }
}

Vec2 Theme::getVarVec2(StyleVar var) const
{
    switch (var)
    {
        case StyleVar::WindowPadding:
            return windowPadding;
        case StyleVar::WindowMinSize:
            return windowMinSize;
        case StyleVar::FramePadding:
            return framePadding;
        case StyleVar::ItemSpacing:
            return itemSpacing;
        case StyleVar::ItemInnerSpacing:
            return itemInnerSpacing;
        case StyleVar::ButtonTextAlign:
            return buttonTextAlign;
        case StyleVar::SelectableTextAlign:
            return selectableTextAlign;
        default:
            return {};
    }
}

void Theme::setVar(StyleVar var, f32 value)
{
    switch (var)
    {
        case StyleVar::Alpha:
            alpha = value;
            break;
        case StyleVar::WindowRounding:
            windowRounding = value;
            break;
        case StyleVar::WindowBorderSize:
            windowBorderSize = value;
            break;
        case StyleVar::ChildRounding:
            childRounding = value;
            break;
        case StyleVar::ChildBorderSize:
            childBorderSize = value;
            break;
        case StyleVar::FrameRounding:
            frameRounding = value;
            break;
        case StyleVar::FrameBorderSize:
            frameBorderSize = value;
            break;
        case StyleVar::IndentSpacing:
            indentSpacing = value;
            break;
        case StyleVar::ScrollbarSize:
            scrollbarSize = value;
            break;
        case StyleVar::ScrollbarRounding:
            scrollbarRounding = value;
            break;
        case StyleVar::GrabMinSize:
            grabMinSize = value;
            break;
        case StyleVar::GrabRounding:
            grabRounding = value;
            break;
        case StyleVar::TabRounding:
            tabRounding = value;
            break;
        default:
            break;
    }
}

void Theme::setVar(StyleVar var, Vec2 value)
{
    switch (var)
    {
        case StyleVar::WindowPadding:
            windowPadding = value;
            break;
        case StyleVar::WindowMinSize:
            windowMinSize = value;
            break;
        case StyleVar::FramePadding:
            framePadding = value;
            break;
        case StyleVar::ItemSpacing:
            itemSpacing = value;
            break;
        case StyleVar::ItemInnerSpacing:
            itemInnerSpacing = value;
            break;
        case StyleVar::ButtonTextAlign:
            buttonTextAlign = value;
            break;
        case StyleVar::SelectableTextAlign:
            selectableTextAlign = value;
            break;
        default:
            break;
    }
}

std::unique_ptr<Theme> Theme::dark()
{
    return std::make_unique<Theme>();  // Default is dark theme
}

std::unique_ptr<Theme> Theme::light()
{
    auto theme = std::make_unique<Theme>();

    theme->text = Color{0, 0, 0, 255};
    theme->textDisabled = Color{100, 100, 100, 255};
    theme->windowBg = Color{240, 240, 240, 255};
    theme->childBg = Color{0, 0, 0, 0};
    theme->popupBg = Color{255, 255, 255, 250};
    theme->border = Color{180, 180, 180, 255};
    theme->frameBg = Color{255, 255, 255, 255};
    theme->frameBgHovered = Color{230, 230, 230, 255};
    theme->frameBgActive = Color{200, 200, 200, 255};
    theme->titleBg = Color{220, 220, 220, 255};
    theme->titleBgActive = Color{70, 130, 200, 255};
    theme->titleBgCollapsed = Color{220, 220, 220, 130};
    theme->menuBarBg = Color{230, 230, 230, 255};
    theme->scrollbarBg = Color{230, 230, 230, 200};
    theme->scrollbarGrab = Color{180, 180, 180, 255};
    theme->scrollbarGrabHovered = Color{150, 150, 150, 255};
    theme->scrollbarGrabActive = Color{120, 120, 120, 255};
    theme->checkMark = Color{60, 120, 200, 255};
    theme->sliderGrab = Color{60, 120, 200, 255};
    theme->sliderGrabActive = Color{40, 100, 180, 255};
    theme->button = Color{60, 120, 200, 255};
    theme->buttonHovered = Color{80, 140, 220, 255};
    theme->buttonActive = Color{40, 100, 180, 255};
    theme->header = Color{60, 120, 200, 200};
    theme->headerHovered = Color{80, 140, 220, 255};
    theme->headerActive = Color{40, 100, 180, 255};
    theme->separator = Color{180, 180, 180, 255};

    return theme;
}

std::unique_ptr<Theme> Theme::classic()
{
    auto theme = std::make_unique<Theme>();

    // Classic gray theme
    theme->text = Color{0, 0, 0, 255};
    theme->windowBg = Color{212, 208, 200, 255};
    theme->border = Color{128, 128, 128, 255};
    theme->frameBg = Color{255, 255, 255, 255};
    theme->titleBg = Color{128, 128, 128, 255};
    theme->titleBgActive = Color{0, 0, 128, 255};
    theme->button = Color{212, 208, 200, 255};
    theme->buttonHovered = Color{230, 226, 220, 255};
    theme->buttonActive = Color{192, 188, 180, 255};
    theme->scrollbarBg = Color{212, 208, 200, 255};

    return theme;
}

std::unique_ptr<Theme> Theme::starCitizen()
{
    auto theme = std::make_unique<Theme>();

    // Star Citizen inspired theme - dark with cyan/blue accents
    Color bgDark{15, 20, 25, 250};
    Color bgMedium{25, 35, 45, 250};
    Color accent{0, 180, 220, 255};
    Color accentDim{0, 120, 160, 255};
    Color textMain{200, 210, 220, 255};

    theme->text = textMain;
    theme->textDisabled = Color{100, 110, 120, 255};
    theme->windowBg = bgDark;
    theme->popupBg = bgMedium;
    theme->border = Color{40, 60, 80, 200};
    theme->frameBg = bgMedium;
    theme->frameBgHovered = Color{35, 50, 65, 255};
    theme->frameBgActive = Color{45, 65, 85, 255};
    theme->titleBg = bgDark;
    theme->titleBgActive = Color{20, 50, 70, 255};
    theme->titleBgCollapsed = bgDark.withAlpha(static_cast<u8>(180));
    theme->menuBarBg = bgDark;
    theme->scrollbarBg = bgDark;
    theme->scrollbarGrab = accentDim;
    theme->scrollbarGrabHovered = accent;
    theme->scrollbarGrabActive = accent;
    theme->checkMark = accent;
    theme->sliderGrab = accent;
    theme->sliderGrabActive = accent;
    theme->button = accentDim;
    theme->buttonHovered = accent;
    theme->buttonActive = Color{0, 200, 240, 255};
    theme->header = accentDim.withAlpha(static_cast<u8>(180));
    theme->headerHovered = accent;
    theme->headerActive = accent;
    theme->separator = Color{40, 60, 80, 255};
    theme->tab = bgMedium;
    theme->tabHovered = accent;
    theme->tabActive = accentDim;
    theme->textSelectedBg = accentDim.withAlpha(static_cast<u8>(150));
    theme->navHighlight = accent;

    theme->windowRounding = 2.0f;
    theme->frameRounding = 1.0f;

    return theme;
}

}  // namespace dakt::gui

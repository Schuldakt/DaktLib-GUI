#include "dakt/gui/layout/Layout.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace dakt::gui {

// ============================================================================
// Flex Layout Algorithm Implementation
// ============================================================================

// Internal structure for flex item calculations
struct FlexItem {
    LayoutNode* node = nullptr;
    float baseSize = 0.0f; // flex-basis or measured size
    float hypotheticalSize = 0.0f;
    float finalSize = 0.0f;
    float crossSize = 0.0f;
    float grow = 0.0f;
    float shrink = 0.0f;
    bool frozen = false;
    float minSize = 0.0f;
    float maxSize = 0.0f;
};

// Internal structure for flex lines
struct FlexLine {
    std::vector<FlexItem*> items;
    float mainSize = 0.0f;
    float crossSize = 0.0f;
    float crossOffset = 0.0f;
};

class FlexLayoutEngine {
  public:
    static void computeFlexLayout(LayoutNode* container, float containerWidth, float containerHeight) {
        if (!container)
            return;

        const auto& flex = container->getFlexProperties();
        const auto& padding = flex.padding;

        // Available size after padding
        float availableMain = (flex.direction == FlexDirection::Row) ? containerWidth - padding.left - padding.right : containerHeight - padding.top - padding.bottom;
        float availableCross = (flex.direction == FlexDirection::Row) ? containerHeight - padding.top - padding.bottom : containerWidth - padding.left - padding.right;

        availableMain = std::max(0.0f, availableMain);
        availableCross = std::max(0.0f, availableCross);

        // Collect flex items
        std::vector<FlexItem> items;
        const auto& children = container->getChildren();

        for (const auto& child : children) {
            FlexItem item;
            item.node = child.get();
            const auto& childFlex = child->getFlexProperties();
            const auto& constraints = child->getConstraints();

            item.grow = childFlex.grow;
            item.shrink = childFlex.shrink;

            // Determine base size
            if (!std::isnan(childFlex.basis)) {
                item.baseSize = childFlex.basis;
            } else {
                // Use child's current size or a default
                item.baseSize = (flex.direction == FlexDirection::Row) ? child->getSize().x : child->getSize().y;
                if (item.baseSize <= 0.0f) {
                    item.baseSize = 100.0f; // Default size
                }
            }

            // Set min/max constraints
            if (flex.direction == FlexDirection::Row) {
                item.minSize = constraints.minWidth;
                item.maxSize = constraints.maxWidth;
            } else {
                item.minSize = constraints.minHeight;
                item.maxSize = constraints.maxHeight;
            }

            item.hypotheticalSize = std::clamp(item.baseSize, item.minSize, item.maxSize);
            items.push_back(item);
        }

        // Create flex lines (handle wrapping)
        std::vector<FlexLine> lines;
        createFlexLines(items, lines, flex.wrap, availableMain);

        // Resolve flexible lengths for each line
        for (auto& line : lines) {
            resolveFlexibleLengths(line, availableMain);
        }

        // Calculate cross sizes
        calculateCrossSizes(lines, flex, availableCross);

        // Align items and position them
        positionItems(container, lines, flex, padding, availableMain, availableCross);
    }

  private:
    static void createFlexLines(std::vector<FlexItem>& items, std::vector<FlexLine>& lines, FlexWrap wrap, float availableMain) {
        FlexLine currentLine;
        float lineMain = 0.0f;

        for (auto& item : items) {
            if (wrap != FlexWrap::NoWrap && !currentLine.items.empty() && lineMain + item.hypotheticalSize > availableMain) {
                // Start new line
                currentLine.mainSize = lineMain;
                lines.push_back(currentLine);
                currentLine = FlexLine();
                lineMain = 0.0f;
            }

            currentLine.items.push_back(&item);
            lineMain += item.hypotheticalSize;
        }

        // Add last line
        if (!currentLine.items.empty()) {
            currentLine.mainSize = lineMain;
            lines.push_back(currentLine);
        }
    }

    static void resolveFlexibleLengths(FlexLine& line, float availableMain) {
        float totalHypothetical = 0.0f;
        for (auto* item : line.items) {
            totalHypothetical += item->hypotheticalSize;
        }

        float freeSpace = availableMain - totalHypothetical;
        bool growing = freeSpace > 0.0f;

        // Calculate total grow/shrink factors
        float totalFactor = 0.0f;
        for (auto* item : line.items) {
            if (!item->frozen) {
                totalFactor += growing ? item->grow : item->shrink;
            }
        }

        // Distribute free space
        if (totalFactor > 0.0f) {
            for (auto* item : line.items) {
                if (!item->frozen) {
                    float factor = growing ? item->grow : item->shrink;
                    float share = freeSpace * (factor / totalFactor);
                    item->finalSize = item->hypotheticalSize + share;
                    item->finalSize = std::clamp(item->finalSize, item->minSize, item->maxSize);
                }
            }
        } else {
            // No flex: use hypothetical sizes
            for (auto* item : line.items) {
                item->finalSize = item->hypotheticalSize;
            }
        }
    }

    static void calculateCrossSizes(std::vector<FlexLine>& lines, const FlexProperties& flex, float availableCross) {
        float totalCross = 0.0f;

        for (auto& line : lines) {
            float maxCross = 0.0f;
            for (auto* item : line.items) {
                // Get item's cross size
                float itemCross = (flex.direction == FlexDirection::Row) ? item->node->getSize().y : item->node->getSize().x;

                if (itemCross <= 0.0f) {
                    itemCross = 30.0f; // Default cross size
                }

                item->crossSize = itemCross;
                maxCross = std::max(maxCross, itemCross);
            }
            line.crossSize = maxCross;
            totalCross += line.crossSize;
        }

        // Stretch items if alignItems is Stretch
        if (flex.alignItems == AlignItems::Stretch) {
            for (auto& line : lines) {
                for (auto* item : line.items) {
                    item->crossSize = line.crossSize;
                }
            }
        }
    }

    static void positionItems(LayoutNode* container, std::vector<FlexLine>& lines, const FlexProperties& flex, const EdgeInsets& padding, float availableMain, float availableCross) {
        bool isRow = (flex.direction == FlexDirection::Row);
        Rect containerRect = container->getRect();

        float mainOffset = isRow ? containerRect.x + padding.left : containerRect.y + padding.top;
        float crossOffset = isRow ? containerRect.y + padding.top : containerRect.x + padding.left;

        for (auto& line : lines) {
            // Calculate main-axis position based on justifyContent
            float lineMainStart = mainOffset;
            float extraSpace = availableMain - line.mainSize;
            float gap = 0.0f;
            float initialOffset = 0.0f;

            switch (flex.justifyContent) {
            case JustifyContent::FlexStart:
                break;
            case JustifyContent::FlexEnd:
                initialOffset = extraSpace;
                break;
            case JustifyContent::Center:
                initialOffset = extraSpace / 2.0f;
                break;
            case JustifyContent::SpaceBetween:
                if (line.items.size() > 1) {
                    gap = extraSpace / (line.items.size() - 1);
                }
                break;
            case JustifyContent::SpaceAround:
                if (!line.items.empty()) {
                    gap = extraSpace / line.items.size();
                    initialOffset = gap / 2.0f;
                }
                break;
            case JustifyContent::SpaceEvenly:
                if (!line.items.empty()) {
                    gap = extraSpace / (line.items.size() + 1);
                    initialOffset = gap;
                }
                break;
            }

            float currentMain = lineMainStart + initialOffset;

            for (auto* item : line.items) {
                // Calculate cross-axis position based on alignItems
                float itemCrossOffset = crossOffset;
                float crossSpace = line.crossSize - item->crossSize;

                switch (flex.alignItems) {
                case AlignItems::FlexStart:
                    break;
                case AlignItems::FlexEnd:
                    itemCrossOffset += crossSpace;
                    break;
                case AlignItems::Center:
                    itemCrossOffset += crossSpace / 2.0f;
                    break;
                case AlignItems::Stretch:
                case AlignItems::Baseline:
                    // Already handled in calculateCrossSizes
                    break;
                }

                // Set position and size
                if (isRow) {
                    item->node->setPosition(currentMain, itemCrossOffset);
                    item->node->setSize(item->finalSize, item->crossSize);
                } else {
                    item->node->setPosition(itemCrossOffset, currentMain);
                    item->node->setSize(item->crossSize, item->finalSize);
                }

                currentMain += item->finalSize + gap;

                // Recursively layout children
                item->node->computeLayout(item->node->getSize().x, item->node->getSize().y);
            }

            crossOffset += line.crossSize;
        }
    }
};

// Public function for flex layout
void FlexLayout_ComputeLayout(LayoutNode* container, float width, float height) { FlexLayoutEngine::computeFlexLayout(container, width, height); }

} // namespace dakt::gui

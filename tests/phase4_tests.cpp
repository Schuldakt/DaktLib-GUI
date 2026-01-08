/**
 * @file phase4_tests.cpp
 * @brief Phase 4 unit tests for DaktLib-GUI retained mode widgets
 *
 * Tests widget hierarchy, state management, layout, and event handling.
 */

#include "dakt/gui/retained/UIContainer.hpp"
#include "dakt/gui/retained/Widget.hpp"
#include "dakt/gui/retained/Widgets/Checkbox.hpp"
#include "dakt/gui/retained/Widgets/ColorPicker.hpp"
#include "dakt/gui/retained/Widgets/Menu.hpp"
#include "dakt/gui/retained/Widgets/Panel.hpp"
#include "dakt/gui/retained/Widgets/TabBar.hpp"
#include "dakt/gui/retained/Widgets/Table.hpp"
#include "dakt/gui/retained/Widgets/Button.hpp"
#include "dakt/gui/retained/Widgets/Label.hpp"
#include "dakt/gui/retained/Widgets/TextInput.hpp"
#include "dakt/gui/retained/Widgets/Slider.hpp"
#include "dakt/gui/retained/Widgets/TreeNode.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <memory>

using namespace dakt::gui;

// ============================================================================
// Test Helpers
// ============================================================================

#define TEST(name)                                                                                                                                                                                                                                       \
    static void test_##name();                                                                                                                                                                                                                           \
    struct TestRunner_##name {                                                                                                                                                                                                                           \
        TestRunner_##name() {                                                                                                                                                                                                                            \
            printf("Testing %s...\n", #name);                                                                                                                                                                                                            \
            test_##name();                                                                                                                                                                                                                               \
            printf("✓ %s passed\n", #name);                                                                                                                                                                                                              \
        }                                                                                                                                                                                                                                                \
    };                                                                                                                                                                                                                                                   \
    static void test_##name()

#define ASSERT(cond)                                                                                                                                                                                                                                     \
    do {                                                                                                                                                                                                                                                 \
        if (!(cond)) {                                                                                                                                                                                                                                   \
            printf("FAILED: %s at %s:%d\n", #cond, __FILE__, __LINE__);                                                                                                                                                                                  \
            assert(false);                                                                                                                                                                                                                               \
        }                                                                                                                                                                                                                                                \
    } while (0)

#define ASSERT_EQ(a, b)                                                                                                                                                                                                                                  \
    do {                                                                                                                                                                                                                                                 \
        if ((a) != (b)) {                                                                                                                                                                                                                                \
            printf("FAILED: %s == %s at %s:%d\n", #a, #b, __FILE__, __LINE__);                                                                                                                                                                           \
            assert(false);                                                                                                                                                                                                                               \
        }                                                                                                                                                                                                                                                \
    } while (0)

#define ASSERT_NEAR(a, b, eps)                                                                                                                                                                                                                           \
    do {                                                                                                                                                                                                                                                 \
        if (std::abs((a) - (b)) > (eps)) {                                                                                                                                                                                                               \
            printf("FAILED: %s ~= %s at %s:%d\n", #a, #b, __FILE__, __LINE__);                                                                                                                                                                           \
            assert(false);                                                                                                                                                                                                                               \
        }                                                                                                                                                                                                                                                \
    } while (0)

// ============================================================================
// Widget Base Class Tests
// ============================================================================

TEST(widget_construction) {
    Widget widget;
    ASSERT(widget.getId().empty());
    ASSERT(widget.getParent() == nullptr);
    ASSERT(widget.getChildren().empty());
    ASSERT(widget.isVisible());
    ASSERT(!widget.isDisabled());
}

TEST(widget_construction_with_id) {
    Widget widget("test_widget");
    ASSERT_EQ(widget.getId(), "test_widget");
}

TEST(widget_bounds) {
    Widget widget;
    widget.setBounds(Rect(10, 20, 100, 50));

    ASSERT_NEAR(widget.getPosition().x, 10.0f, 0.001f);
    ASSERT_NEAR(widget.getPosition().y, 20.0f, 0.001f);
    ASSERT_NEAR(widget.getSize().x, 100.0f, 0.001f);
    ASSERT_NEAR(widget.getSize().y, 50.0f, 0.001f);
}

TEST(widget_set_position) {
    Widget widget;
    widget.setPosition(Vec2(50, 60));

    ASSERT_NEAR(widget.getBounds().x, 50.0f, 0.001f);
    ASSERT_NEAR(widget.getBounds().y, 60.0f, 0.001f);
}

TEST(widget_set_size) {
    Widget widget;
    widget.setSize(Vec2(200, 150));

    ASSERT_NEAR(widget.getBounds().width, 200.0f, 0.001f);
    ASSERT_NEAR(widget.getBounds().height, 150.0f, 0.001f);
}

TEST(widget_visibility) {
    Widget widget;
    ASSERT(widget.isVisible());

    widget.setVisible(false);
    ASSERT(!widget.isVisible());

    widget.setVisible(true);
    ASSERT(widget.isVisible());
}

TEST(widget_enabled) {
    Widget widget;
    ASSERT(widget.isEnabled());
    ASSERT(!widget.isDisabled());

    widget.setEnabled(false);
    ASSERT(!widget.isEnabled());
    ASSERT(widget.isDisabled());

    widget.setEnabled(true);
    ASSERT(widget.isEnabled());
}

TEST(widget_state_flags) {
    Widget widget;
    ASSERT(!widget.isHovered());
    ASSERT(!widget.isActive());
    ASSERT(!widget.isFocused());

    widget.addFlag(RetainedWidgetFlags::Hovered);
    ASSERT(widget.isHovered());
    ASSERT(widget.hasFlag(RetainedWidgetFlags::Hovered));

    widget.addFlag(RetainedWidgetFlags::Active);
    ASSERT(widget.isHovered());
    ASSERT(widget.isActive());

    widget.removeFlag(RetainedWidgetFlags::Hovered);
    ASSERT(!widget.isHovered());
    ASSERT(widget.isActive());
}

TEST(widget_dirty_tracking) {
    Widget widget;
    ASSERT(widget.isDirty()); // New widgets start dirty

    widget.clearDirty();
    ASSERT(!widget.isDirty());

    widget.markDirty();
    ASSERT(widget.isDirty());
}

TEST(widget_needs_layout) {
    Widget widget;
    ASSERT(widget.needsLayout()); // New widgets need layout

    widget.clearNeedsLayout();
    ASSERT(!widget.needsLayout());

    widget.markNeedsLayout();
    ASSERT(widget.needsLayout());
    ASSERT(widget.isDirty()); // Marking needs layout also marks dirty
}

// ============================================================================
// Widget Hierarchy Tests
// ============================================================================

TEST(widget_add_child) {
    Widget parent;
    auto child = std::make_unique<Widget>("child1");
    Widget* childPtr = child.get();

    parent.addChild(std::move(child));

    ASSERT_EQ(parent.getChildren().size(), 1u);
    ASSERT_EQ(childPtr->getParent(), &parent);
}

TEST(widget_add_child_template) {
    Widget parent;
    Button& button = parent.addChild<Button>("Click Me");

    ASSERT_EQ(parent.getChildren().size(), 1u);
    ASSERT_EQ(button.getLabel(), "Click Me");
    ASSERT_EQ(button.getParent(), &parent);
}

TEST(widget_remove_child) {
    Widget parent;
    auto child = std::make_unique<Widget>("child1");
    Widget* childPtr = child.get();

    parent.addChild(std::move(child));
    ASSERT_EQ(parent.getChildren().size(), 1u);

    bool removed = parent.removeChild(childPtr);
    ASSERT(removed);
    ASSERT(parent.getChildren().empty());
}

TEST(widget_clear_children) {
    Widget parent;
    parent.addChild(std::make_unique<Widget>("child1"));
    parent.addChild(std::make_unique<Widget>("child2"));
    parent.addChild(std::make_unique<Widget>("child3"));

    ASSERT_EQ(parent.getChildren().size(), 3u);

    parent.clearChildren();
    ASSERT(parent.getChildren().empty());
}

TEST(widget_find_child) {
    Widget parent;
    parent.addChild(std::make_unique<Widget>("child1"));
    parent.addChild(std::make_unique<Widget>("child2"));

    Widget* found = parent.findChild("child2");
    ASSERT(found != nullptr);
    ASSERT_EQ(found->getId(), "child2");

    Widget* notFound = parent.findChild("nonexistent");
    ASSERT(notFound == nullptr);
}

TEST(widget_find_child_recursive) {
    Widget parent;
    auto child = std::make_unique<Widget>("child1");
    child->addChild(std::make_unique<Widget>("grandchild1"));
    parent.addChild(std::move(child));

    Widget* found = parent.findChild("grandchild1");
    ASSERT(found != nullptr);
    ASSERT_EQ(found->getId(), "grandchild1");
}

TEST(widget_dirty_propagation) {
    Widget parent;
    parent.clearDirty();

    auto child = std::make_unique<Widget>("child1");
    Widget* childPtr = child.get();
    parent.addChild(std::move(child));

    // Adding child should make parent dirty
    ASSERT(parent.isDirty());

    parent.clearDirty();
    childPtr->markDirty();

    // Child marking dirty should propagate to parent
    ASSERT(parent.isDirty());
}

// ============================================================================
// Label Widget Tests
// ============================================================================

TEST(label_construction) {
    Label label;
    ASSERT(label.getText().empty());
}

TEST(label_construction_with_text) {
    Label label("Hello World");
    ASSERT_EQ(label.getText(), "Hello World");
}

TEST(label_set_text) {
    Label label;
    label.setText("Test");
    ASSERT_EQ(label.getText(), "Test");
}

TEST(label_measure_content) {
    Label label("Test");
    Vec2 size = label.measureContent();

    ASSERT(size.x > 0);
    ASSERT(size.y > 0);
}

// ============================================================================
// Button Widget Tests
// ============================================================================

TEST(button_construction) {
    Button button;
    ASSERT(button.getLabel().empty());
}

TEST(button_construction_with_label) {
    Button button("Click Me");
    ASSERT_EQ(button.getLabel(), "Click Me");
}

TEST(button_set_label) {
    Button button;
    button.setLabel("New Label");
    ASSERT_EQ(button.getLabel(), "New Label");
}

TEST(button_measure_content) {
    Button button("Test Button");
    Vec2 size = button.measureContent();

    ASSERT(size.x > 0);
    ASSERT(size.y > 0);
}

TEST(button_click_callback) {
    Button button("Test");
    bool clicked = false;

    button.setOnClick([&clicked](const WidgetEvent& event) { clicked = true; });

    // Simulate press -> release sequence
    WidgetEvent pressEvent;
    pressEvent.type = WidgetEventType::Press;
    button.handleInput(pressEvent);

    ASSERT(button.isActive());

    WidgetEvent releaseEvent;
    releaseEvent.type = WidgetEventType::Release;
    button.handleInput(releaseEvent);

    ASSERT(clicked);
    ASSERT(!button.isActive());
}

// ============================================================================
// Checkbox Widget Tests
// ============================================================================

TEST(checkbox_construction) {
    Checkbox checkbox;
    ASSERT(!checkbox.isChecked());
}

TEST(checkbox_construction_with_label) {
    Checkbox checkbox("Option 1");
    ASSERT_EQ(checkbox.getLabel(), "Option 1");
    ASSERT(!checkbox.isChecked());
}

TEST(checkbox_set_checked) {
    Checkbox checkbox;
    ASSERT(!checkbox.isChecked());

    checkbox.setChecked(true);
    ASSERT(checkbox.isChecked());

    checkbox.setChecked(false);
    ASSERT(!checkbox.isChecked());
}

TEST(checkbox_toggle) {
    Checkbox checkbox("Test");
    bool changed = false;

    checkbox.setOnValueChanged([&changed](const WidgetEvent& event) { changed = true; });

    WidgetEvent clickEvent;
    clickEvent.type = WidgetEventType::Click;
    checkbox.handleInput(clickEvent);

    ASSERT(checkbox.isChecked());
    ASSERT(changed);
}

// ============================================================================
// Slider Widget Tests
// ============================================================================

TEST(slider_construction) {
    Slider slider;
    ASSERT_NEAR(slider.getValue(), 0.0f, 0.001f);
    ASSERT_NEAR(slider.getMin(), 0.0f, 0.001f);
    ASSERT_NEAR(slider.getMax(), 1.0f, 0.001f);
}

TEST(slider_construction_with_range) {
    Slider slider(10.0f, 100.0f, 50.0f);
    ASSERT_NEAR(slider.getValue(), 50.0f, 0.001f);
    ASSERT_NEAR(slider.getMin(), 10.0f, 0.001f);
    ASSERT_NEAR(slider.getMax(), 100.0f, 0.001f);
}

TEST(slider_set_value) {
    Slider slider(0.0f, 100.0f, 0.0f);
    slider.setValue(75.0f);
    ASSERT_NEAR(slider.getValue(), 75.0f, 0.001f);
}

TEST(slider_clamp_value) {
    Slider slider(0.0f, 100.0f, 50.0f);

    slider.setValue(150.0f);
    ASSERT_NEAR(slider.getValue(), 100.0f, 0.001f);

    slider.setValue(-50.0f);
    ASSERT_NEAR(slider.getValue(), 0.0f, 0.001f);
}

TEST(slider_step) {
    Slider slider(0.0f, 10.0f, 0.0f);
    slider.setStep(2.5f);

    slider.setValue(3.7f);
    ASSERT_NEAR(slider.getValue(), 2.5f, 0.001f); // 3.7 rounds to nearest step (2.5)

    slider.setValue(4.0f);
    ASSERT_NEAR(slider.getValue(), 5.0f, 0.001f); // 4.0 rounds to nearest step (5.0)
}

// ============================================================================
// TextInput Widget Tests
// ============================================================================

TEST(textinput_construction) {
    TextInput input;
    ASSERT(input.getText().empty());
    ASSERT_EQ(input.getCursorPosition(), 0u);
}

TEST(textinput_set_text) {
    TextInput input;
    input.setText("Hello");
    ASSERT_EQ(input.getText(), "Hello");
}

TEST(textinput_placeholder) {
    TextInput input;
    input.setPlaceholder("Enter text...");
    ASSERT_EQ(input.getPlaceholder(), "Enter text...");
}

TEST(textinput_password) {
    TextInput input;
    ASSERT(!input.isPassword());

    input.setPassword(true);
    ASSERT(input.isPassword());
}

TEST(textinput_select_all) {
    TextInput input;
    input.setText("Hello World");
    input.selectAll();

    ASSERT_EQ(input.getSelectedText(), "Hello World");
}

// ============================================================================
// Panel Widget Tests
// ============================================================================

TEST(panel_construction) {
    Panel panel;
    ASSERT(panel.getTitle().empty());
}

TEST(panel_construction_with_title) {
    Panel panel("Settings");
    ASSERT_EQ(panel.getTitle(), "Settings");
}

TEST(panel_show_header) {
    Panel panel("Test");
    ASSERT(panel.hasHeader());

    panel.setShowHeader(false);
    ASSERT(!panel.hasHeader());
}

// ============================================================================
// TreeNode Widget Tests
// ============================================================================

TEST(treenode_construction) {
    TreeNode node;
    ASSERT(node.getLabel().empty());
    ASSERT(!node.isExpanded());
    ASSERT(!node.isLeaf());
}

TEST(treenode_construction_with_label) {
    TreeNode node("Folder");
    ASSERT_EQ(node.getLabel(), "Folder");
}

TEST(treenode_expand) {
    TreeNode node("Test");
    ASSERT(!node.isExpanded());

    node.setExpanded(true);
    ASSERT(node.isExpanded());

    node.setExpanded(false);
    ASSERT(!node.isExpanded());
}

TEST(treenode_leaf) {
    TreeNode node("File.txt");
    node.setLeaf(true);

    ASSERT(node.isLeaf());
}

// ============================================================================
// TabBar Widget Tests
// ============================================================================

TEST(tabbar_construction) {
    TabBar tabBar;
    ASSERT_EQ(tabBar.getTabCount(), 0u);
}

TEST(tabbar_add_tab) {
    TabBar tabBar;
    tabBar.addTab("Tab 1");
    tabBar.addTab("Tab 2", "tab2");

    ASSERT_EQ(tabBar.getTabCount(), 2u);
    ASSERT_EQ(tabBar.getTab(0).label, "Tab 1");
    ASSERT_EQ(tabBar.getTab(1).id, "tab2");
}

TEST(tabbar_select_tab) {
    TabBar tabBar;
    tabBar.addTab("Tab 1");
    tabBar.addTab("Tab 2");
    tabBar.addTab("Tab 3");

    ASSERT_EQ(tabBar.getSelectedIndex(), 0u);

    tabBar.setSelectedIndex(2);
    ASSERT_EQ(tabBar.getSelectedIndex(), 2u);
}

TEST(tabbar_remove_tab) {
    TabBar tabBar;
    tabBar.addTab("Tab 1");
    tabBar.addTab("Tab 2");
    tabBar.addTab("Tab 3");

    tabBar.removeTab(1);
    ASSERT_EQ(tabBar.getTabCount(), 2u);
    ASSERT_EQ(tabBar.getTab(1).label, "Tab 3");
}

// ============================================================================
// ColorPicker Tests
// ============================================================================

TEST(colorpicker_construction) {
    ColorPicker picker;
    // Default color is white
    ASSERT(picker.getColor().r == 255);
    ASSERT(picker.getColor().g == 255);
    ASSERT(picker.getColor().b == 255);
    ASSERT(picker.getColor().a == 255);
}

TEST(colorpicker_set_color) {
    ColorPicker picker;
    picker.setColor(Color(100, 150, 200, 255));

    ASSERT_EQ(picker.getColor().r, 100);
    ASSERT_EQ(picker.getColor().g, 150);
    ASSERT_EQ(picker.getColor().b, 200);
}

TEST(colorpicker_hsv_conversion) {
    ColorPicker picker;

    // Set to pure red
    picker.setHSV(0.0f, 1.0f, 1.0f);
    ASSERT_EQ(picker.getColor().r, 255);
    ASSERT(picker.getColor().g < 5);
    ASSERT(picker.getColor().b < 5);

    // Set to pure green (120 degrees)
    picker.setHSV(120.0f, 1.0f, 1.0f);
    ASSERT(picker.getColor().r < 5);
    ASSERT_EQ(picker.getColor().g, 255);
    ASSERT(picker.getColor().b < 5);

    // Set to pure blue (240 degrees)
    picker.setHSV(240.0f, 1.0f, 1.0f);
    ASSERT(picker.getColor().r < 5);
    ASSERT(picker.getColor().g < 5);
    ASSERT_EQ(picker.getColor().b, 255);
}

// ============================================================================
// Menu Tests
// ============================================================================

TEST(menu_construction) {
    Menu menu;
    ASSERT(menu.getTitle().empty());
    ASSERT(!menu.isOpen());
}

TEST(menu_add_item) {
    Menu menu("File");
    bool itemClicked = false;

    menu.addItem("New", [&itemClicked](const WidgetEvent&) { itemClicked = true; }, "Ctrl+N");

    ASSERT_EQ(menu.getItems().size(), 1u);
}

TEST(menu_add_separator) {
    Menu menu("Edit");
    menu.addItem("Cut", nullptr, "Ctrl+X");
    menu.addSeparator();
    menu.addItem("Copy", nullptr, "Ctrl+C");

    ASSERT_EQ(menu.getItems().size(), 3u);
}

TEST(menu_submenu) {
    Menu menu("File");
    Menu& recent = menu.addSubmenu("Recent Files");
    recent.addItem("file1.txt", nullptr);
    recent.addItem("file2.txt", nullptr);

    ASSERT_EQ(menu.getItems().size(), 1u);
    ASSERT_EQ(recent.getItems().size(), 2u);
}

TEST(menubar_construction) {
    MenuBar menuBar;
    ASSERT_EQ(menuBar.getMenus().size(), 0u);
}

TEST(menubar_add_menu) {
    MenuBar menuBar;
    Menu& fileMenu = menuBar.addMenu("File");
    Menu& editMenu = menuBar.addMenu("Edit");

    fileMenu.addItem("New", nullptr);
    editMenu.addItem("Undo", nullptr);

    ASSERT_EQ(menuBar.getMenus().size(), 2u);
    ASSERT(menuBar.getMenu("File") != nullptr);
    ASSERT(menuBar.getMenu("Edit") != nullptr);
}

// ============================================================================
// Table Tests
// ============================================================================

TEST(table_construction) {
    Table table;
    ASSERT_EQ(table.getColumnCount(), 0);
    ASSERT_EQ(table.getRowCount(), 0);
}

TEST(table_add_column) {
    Table table;
    table.addColumn("Name", 100.0f);
    table.addColumn("Age", 50.0f);
    table.addColumn("Email", 200.0f);

    ASSERT_EQ(table.getColumnCount(), 3);
}

TEST(table_add_row) {
    Table table;
    table.addColumn("Name", 100.0f);
    table.addColumn("Age", 50.0f);

    table.addRow({"Alice", "30"});
    table.addRow({"Bob", "25"});
    table.addRow({"Charlie", "35"});

    ASSERT_EQ(table.getRowCount(), 3);
    // Use getCellValue for accessing cells
    ASSERT_EQ(std::get<std::string>(table.getCellValue(0, 0)), "Alice");
    ASSERT_EQ(std::get<std::string>(table.getCellValue(1, 1)), "25");
}

TEST(table_selection) {
    Table table;
    table.addColumn("Name", 100.0f);
    table.addRow({"Alice"});
    table.addRow({"Bob"});
    table.addRow({"Charlie"});

    table.selectRow(1);
    ASSERT(table.isRowSelected(1));

    table.selectRow(2);
    ASSERT(table.isRowSelected(2));

    table.deselectAll();
    ASSERT(!table.isRowSelected(1));
    ASSERT(!table.isRowSelected(2));
}

TEST(table_sorting) {
    Table table;
    TableColumn col1;
    col1.header = "Name";
    col1.width = 100.0f;
    col1.sortable = true;
    table.addColumn(col1);

    TableColumn col2;
    col2.header = "Age";
    col2.width = 50.0f;
    col2.sortable = true;
    table.addColumn(col2);

    table.addRow({"Charlie", "35"});
    table.addRow({"Alice", "30"});
    table.addRow({"Bob", "25"});

    // Sort by name ascending
    table.sortByColumn(0, SortDirection::Ascending);
    ASSERT_EQ(std::get<std::string>(table.getCellValue(0, 0)), "Alice");
    ASSERT_EQ(std::get<std::string>(table.getCellValue(1, 0)), "Bob");
    ASSERT_EQ(std::get<std::string>(table.getCellValue(2, 0)), "Charlie");

    // Sort by name descending
    table.sortByColumn(0, SortDirection::Descending);
    ASSERT_EQ(std::get<std::string>(table.getCellValue(0, 0)), "Charlie");
    ASSERT_EQ(std::get<std::string>(table.getCellValue(1, 0)), "Bob");
    ASSERT_EQ(std::get<std::string>(table.getCellValue(2, 0)), "Alice");
}

// ============================================================================
// UIContainer Tests
// ============================================================================

TEST(uicontainer_construction) {
    UIContainer container;
    ASSERT(container.getRoot() != nullptr);
}

TEST(uicontainer_construction_with_id) {
    UIContainer container("main_container");
    ASSERT_EQ(container.getId(), "main_container");
}

TEST(uicontainer_add_widget) {
    UIContainer container;
    Button& button = container.addWidget<Button>("Test Button");

    ASSERT_EQ(button.getLabel(), "Test Button");
    ASSERT(container.getRoot()->getChildren().size() == 1);
}

TEST(uicontainer_find_widget) {
    UIContainer container;
    auto button = std::make_unique<Button>("Test");
    button->setId("my_button");
    container.addWidget(std::move(button));

    Widget* found = container.findWidget("my_button");
    ASSERT(found != nullptr);
}

TEST(uicontainer_clear) {
    UIContainer container;
    container.addWidget<Button>("Button 1");
    container.addWidget<Button>("Button 2");

    ASSERT(container.getRoot()->getChildren().size() == 2);

    container.clear();
    ASSERT(container.getRoot()->getChildren().empty());
}

TEST(uicontainer_bounds) {
    UIContainer container;
    container.setBounds(Rect(0, 0, 800, 600));

    ASSERT_NEAR(container.getBounds().width, 800.0f, 0.001f);
    ASSERT_NEAR(container.getBounds().height, 600.0f, 0.001f);
}

TEST(uicontainer_layout_direction) {
    UIContainer container;
    ASSERT(container.getLayoutDirection() == LayoutDirection::Vertical);

    container.setLayoutDirection(LayoutDirection::Horizontal);
    ASSERT(container.getLayoutDirection() == LayoutDirection::Horizontal);
}

TEST(uicontainer_spacing) {
    UIContainer container;
    container.setSpacing(10.0f);
    ASSERT_NEAR(container.getSpacing(), 10.0f, 0.001f);
}

TEST(uicontainer_padding) {
    UIContainer container;
    container.setPadding(Insets(10, 20, 10, 20));

    ASSERT_NEAR(container.getPadding().top, 10.0f, 0.001f);
    ASSERT_NEAR(container.getPadding().left, 20.0f, 0.001f);
}

TEST(uicontainer_focus) {
    UIContainer container;
    Button& button = container.addWidget<Button>("Test");

    ASSERT(container.getFocusedWidget() == nullptr);

    container.setFocus(&button);
    ASSERT(container.getFocusedWidget() == &button);
    ASSERT(button.isFocused());

    container.clearFocus();
    ASSERT(container.getFocusedWidget() == nullptr);
    ASSERT(!button.isFocused());
}

// ============================================================================
// Main
// ============================================================================

int main() {
    printf("\n======== DaktLib-GUI Phase 4 Unit Tests ========\n\n");

    // Widget base class tests
    TestRunner_widget_construction runner_widget_construction;
    TestRunner_widget_construction_with_id runner_widget_construction_with_id;
    TestRunner_widget_bounds runner_widget_bounds;
    TestRunner_widget_set_position runner_widget_set_position;
    TestRunner_widget_set_size runner_widget_set_size;
    TestRunner_widget_visibility runner_widget_visibility;
    TestRunner_widget_enabled runner_widget_enabled;
    TestRunner_widget_state_flags runner_widget_state_flags;
    TestRunner_widget_dirty_tracking runner_widget_dirty_tracking;
    TestRunner_widget_needs_layout runner_widget_needs_layout;

    // Widget hierarchy tests
    TestRunner_widget_add_child runner_widget_add_child;
    TestRunner_widget_add_child_template runner_widget_add_child_template;
    TestRunner_widget_remove_child runner_widget_remove_child;
    TestRunner_widget_clear_children runner_widget_clear_children;
    TestRunner_widget_find_child runner_widget_find_child;
    TestRunner_widget_find_child_recursive runner_widget_find_child_recursive;
    TestRunner_widget_dirty_propagation runner_widget_dirty_propagation;

    // Label tests
    TestRunner_label_construction runner_label_construction;
    TestRunner_label_construction_with_text runner_label_construction_with_text;
    TestRunner_label_set_text runner_label_set_text;
    TestRunner_label_measure_content runner_label_measure_content;

    // Button tests
    TestRunner_button_construction runner_button_construction;
    TestRunner_button_construction_with_label runner_button_construction_with_label;
    TestRunner_button_set_label runner_button_set_label;
    TestRunner_button_measure_content runner_button_measure_content;
    TestRunner_button_click_callback runner_button_click_callback;

    // Checkbox tests
    TestRunner_checkbox_construction runner_checkbox_construction;
    TestRunner_checkbox_construction_with_label runner_checkbox_construction_with_label;
    TestRunner_checkbox_set_checked runner_checkbox_set_checked;
    TestRunner_checkbox_toggle runner_checkbox_toggle;

    // Slider tests
    TestRunner_slider_construction runner_slider_construction;
    TestRunner_slider_construction_with_range runner_slider_construction_with_range;
    TestRunner_slider_set_value runner_slider_set_value;
    TestRunner_slider_clamp_value runner_slider_clamp_value;
    TestRunner_slider_step runner_slider_step;

    // TextInput tests
    TestRunner_textinput_construction runner_textinput_construction;
    TestRunner_textinput_set_text runner_textinput_set_text;
    TestRunner_textinput_placeholder runner_textinput_placeholder;
    TestRunner_textinput_password runner_textinput_password;
    TestRunner_textinput_select_all runner_textinput_select_all;

    // Panel tests
    TestRunner_panel_construction runner_panel_construction;
    TestRunner_panel_construction_with_title runner_panel_construction_with_title;
    TestRunner_panel_show_header runner_panel_show_header;

    // TreeNode tests
    TestRunner_treenode_construction runner_treenode_construction;
    TestRunner_treenode_construction_with_label runner_treenode_construction_with_label;
    TestRunner_treenode_expand runner_treenode_expand;
    TestRunner_treenode_leaf runner_treenode_leaf;

    // TabBar tests
    TestRunner_tabbar_construction runner_tabbar_construction;
    TestRunner_tabbar_add_tab runner_tabbar_add_tab;
    TestRunner_tabbar_select_tab runner_tabbar_select_tab;
    TestRunner_tabbar_remove_tab runner_tabbar_remove_tab;

    // ColorPicker tests
    TestRunner_colorpicker_construction runner_colorpicker_construction;
    TestRunner_colorpicker_set_color runner_colorpicker_set_color;
    TestRunner_colorpicker_hsv_conversion runner_colorpicker_hsv_conversion;

    // Menu tests
    TestRunner_menu_construction runner_menu_construction;
    TestRunner_menu_add_item runner_menu_add_item;
    TestRunner_menu_add_separator runner_menu_add_separator;
    TestRunner_menu_submenu runner_menu_submenu;
    TestRunner_menubar_construction runner_menubar_construction;
    TestRunner_menubar_add_menu runner_menubar_add_menu;

    // Table tests
    TestRunner_table_construction runner_table_construction;
    TestRunner_table_add_column runner_table_add_column;
    TestRunner_table_add_row runner_table_add_row;
    TestRunner_table_selection runner_table_selection;
    TestRunner_table_sorting runner_table_sorting;

    // UIContainer tests
    TestRunner_uicontainer_construction runner_uicontainer_construction;
    TestRunner_uicontainer_construction_with_id runner_uicontainer_construction_with_id;
    TestRunner_uicontainer_add_widget runner_uicontainer_add_widget;
    TestRunner_uicontainer_find_widget runner_uicontainer_find_widget;
    TestRunner_uicontainer_clear runner_uicontainer_clear;
    TestRunner_uicontainer_bounds runner_uicontainer_bounds;
    TestRunner_uicontainer_layout_direction runner_uicontainer_layout_direction;
    TestRunner_uicontainer_spacing runner_uicontainer_spacing;
    TestRunner_uicontainer_padding runner_uicontainer_padding;
    TestRunner_uicontainer_focus runner_uicontainer_focus;

    printf("\n======== ✓ All Phase 4 tests passed! ========\n\n");
    return 0;
}

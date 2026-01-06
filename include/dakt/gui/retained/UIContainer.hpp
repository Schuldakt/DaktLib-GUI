#ifndef DAKTLIB_GUI_UICONTAINER_HPP
#define DAKTLIB_GUI_UICONTAINER_HPP

/**
 * @file UIContainer.hpp
 * @brief Backward compatibility header - includes all containers
 *
 * For new code, prefer including specific container headers:
 * - containers/ContainerBase.hpp for UIContainer base class
 * - containers/VBox.hpp, containers/HBox.hpp, etc. for specific containers
 * - Containers.hpp for all containers at once
 */

#include "Containers.hpp"

// Also include widgets for backward compatibility (UIContainer uses Widget)
#include "Widgets.hpp"

#endif // DAKTLIB_GUI_UICONTAINER_HPP

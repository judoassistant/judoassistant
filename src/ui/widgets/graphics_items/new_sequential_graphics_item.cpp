#include "ui/widgets/graphics_items/new_sequential_graphics_item.hpp"

#include <QBitmap>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>

#include "core/actions/tatami_actions.hpp"
#include "core/log.hpp"
#include "ui/misc/judoassistant_mime.hpp"
#include "ui/store_managers/store_manager.hpp"
#include "ui/stores/qtournament_store.hpp"
#include "ui/widgets/colors.hpp"
#include "ui/widgets/graphics_items/new_concurrent_graphics_item.hpp"
#include "ui/widgets/graphics_items/new_sequential_graphics_item.hpp"
#include "ui/widgets/new_tatamis_widget.hpp"


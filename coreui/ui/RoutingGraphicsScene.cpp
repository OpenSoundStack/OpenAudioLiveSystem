#include "RoutingGraphicsScene.h"

RoutingGraphicsScene::RoutingGraphicsScene() : QGraphicsScene() {
    addItem(new RackedIOItem(2, 4));
}

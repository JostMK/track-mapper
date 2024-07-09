//
// Created by Jost on 14/05/2024.
//

#ifndef IGRID_H
#define IGRID_H
#include "IGraph.h"

class IGrid {
public:
    virtual ~IGrid() = default;

    [[nodiscard]] virtual int GetClosestNode(Location location) const = 0;
};

#endif //IGRID_H

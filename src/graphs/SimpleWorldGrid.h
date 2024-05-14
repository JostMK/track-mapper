//
// Created by Jost on 14/05/2024.
//

#ifndef SIMPLEWORLDGRID_H
#define SIMPLEWORLDGRID_H
#include <memory>

#include "IGrid.h"


class SimpleWorldGrid final : public IGrid {
public:
    SimpleWorldGrid(const IGraph &graph, float resolution);

    [[nodiscard]] int GetClosestNode(Location location) const override;

    //TODO: move back to private after testing
    [[nodiscard]] int GetCellIndexForLocation(const Location& location) const;
    [[nodiscard]] std::vector<int> GetNodeIndicesInCell(int cellIndex) const;

private:
    const float m_Resolution;
    const int m_CellCountX;
    const int m_CellCountY;
    const std::unique_ptr<int[]> m_pNodeIndices;
    const std::unique_ptr<int[]> m_pCellLookupIndices;


};


#endif //SIMPLEWORLDGRID_H

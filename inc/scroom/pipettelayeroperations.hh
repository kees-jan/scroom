#pragma once
#include <vector>
#include <string>
#include <scroom/rectangle.hh>
#include <scroom/tile.hh>

class PipetteLayerOperations
{
public:
    typedef std::vector<std::pair<std::string, size_t>> PipetteColor;
    virtual ~PipetteLayerOperations() {}

    virtual PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr tile) = 0;
};

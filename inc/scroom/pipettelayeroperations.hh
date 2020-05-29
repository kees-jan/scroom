#pragma once
#include <vector>
#include <string>
#include <scroom/rectangle.hh>
#include <scroom/tile.hh>

class PipetteLayerOperations : public virtual Scroom::Utils::Base
{
public:
    typedef std::vector<std::pair<std::string, size_t>> PipetteColor;
    typedef boost::shared_ptr<PipetteLayerOperations> Ptr;
    
    virtual ~PipetteLayerOperations() {}

    virtual PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr tile) = 0;
};

#pragma once
#include <scroom/pipettelayeroperations.hh>
#include <scroom/rectangle.hh>
class PipetteViewInterface
{
public:
    virtual ~PipetteViewInterface()
    {}

    virtual PipetteLayerOperations::PipetteColor getAverages(Scroom::Utils::Rectangle<int> area) = 0;
};
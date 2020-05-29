#pragma once
#include <scroom/pipettelayeroperations.hh>
#include <scroom/rectangle.hh>
#include <scroom/utilities.hh>

class PipetteViewInterface : public virtual Scroom::Utils::Base
{
public:
    virtual ~PipetteViewInterface()
    {}
    typedef boost::shared_ptr<PipetteViewInterface> Ptr;

    virtual PipetteLayerOperations::PipetteColor getAverages(Scroom::Utils::Rectangle<int> area) = 0;
};
#include <scroom/pipettelayeroperations.hh>

class PipetteViewInterface
{
public:
    virtual ~PipetteViewInterface()
    {}

    virtual PipetteLayerOperations::PipetteColor getAverages() = 0;
};
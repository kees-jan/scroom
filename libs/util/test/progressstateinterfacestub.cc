#include "progressstateinterfacestub.hh"

#include <scroom/progressinterfacehelpers.hh>

ProgressStateInterfaceStub::Ptr ProgressStateInterfaceStub::create() { return Ptr(new ProgressStateInterfaceStub()); }

void ProgressStateInterfaceStub::setProgress(State s, double d)
{
  state    = s;
  progress = d;
}

#include "progressstateinterfacestub.hh"

ProgressStateInterfaceStub::ProgressStateInterfaceStub()
: state(IDLE), progress(0.0)
{}

ProgressStateInterfaceStub::Ptr ProgressStateInterfaceStub::create()
{
  return Ptr(new ProgressStateInterfaceStub());
}

void ProgressStateInterfaceStub::setProgress(State s, double d)
{
  state = s;
  progress = d;
}


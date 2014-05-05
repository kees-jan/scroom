#include "progressstateinterfacestub.hh"

ProgressStateInterfaceStub::ProgressStateInterfaceStub()
: state(IDLE), progress(0.0)
{}

ProgressStateInterfaceStub::Ptr ProgressStateInterfaceStub::create()
{
  return Ptr(new ProgressStateInterfaceStub());
}

void ProgressStateInterfaceStub::setState(State s)
{
  state = s;
}

void ProgressStateInterfaceStub::setProgress(int done, int total)
{
  setProgress(double(done)/total);
}

void ProgressStateInterfaceStub::setProgress(double d)
{
  progress = d;
}


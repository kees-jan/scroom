#ifndef SEQUENTIALLY_HH
#define SEQUENTIALLY_HH

#include <list>

#include <boost/thread/mutex.hpp>

#include <threadpool.hh>

class Sequentially
{
private:
  boost::mutex remainingJobsMutex;
  std::list<SeqJob*> remainingJobs;
  bool currentlyWorking;

  friend class SeqJob;
  
public:
  static Sequentially& getInstance();
  
public:
  Sequentially();
  ~Sequentially();

  void execute(SeqJob* job);

  // Helpers /////////////////////////////////////////////////////////////
private:
  void done();

public:
  void do_next();
};

#endif

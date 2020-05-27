#pragma once

#include <boost/shared_ptr.hpp>

#include <scroom/plugininformationinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/utilities.hh>
#include <scroom/viewinterface.hh>

class Listener : public SelectionListener, virtual public  Scroom::Utils::Base{
public:
	Listener();

public:
	typedef boost::shared_ptr<Listener> Ptr;

public:
	static Ptr create();

public:
	virtual ~Listener();

	virtual void onSelection(Selection* measurement);
};

class Pipette : public PluginInformationInterface, public ViewObserver, public PresentationObserver, virtual public  Scroom::Utils::Base{
public:
  typedef boost::shared_ptr<Pipette> Ptr;

private:
  Pipette();

public:
  static Ptr create();

public:
  virtual std::string getPluginName();
  virtual std::string getPluginVersion();
  virtual void registerCapabilities(ScroomPluginInterface::Ptr host);

  virtual Scroom::Bookkeeping::Token viewAdded(ViewInterface::Ptr v);

  virtual void presentationAdded(PresentationInterface::Ptr p);
  virtual void presentationDeleted();

  virtual ~Pipette();
};


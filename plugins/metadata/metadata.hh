// Metadata plugin class
// Created by andy on 13-06-21.
//

#include <scroom/imagemdinterface.hh>
#include <scroom/plugininformationinterface.hh>
#include <scroom/threadpool.hh>
#include <scroom/utilities.hh>
#include <scroom/viewinterface.hh>

void on_image_properties_activate(GtkMenuItem*, gpointer user_data);

class Metadata
  : public PluginInformationInterface
  , public ViewObserver
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<Metadata>;

private:
  Metadata() = default;

public:
  static Ptr create();

public:
  ////////////////////////////////////////////////////////////////////////
  // PluginInformationInterface

  std::string getPluginName() override;
  std::string getPluginVersion() override;
  void        registerCapabilities(ScroomPluginInterface::Ptr host) override;

  ////////////////////////////////////////////////////////////////////////
  // ViewObserver

  Scroom::Bookkeeping::Token viewAdded(ViewInterface::Ptr v) override;

  ////////////////////////////////////////////////////////////////////////
};

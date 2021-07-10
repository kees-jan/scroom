//
// Created by andy on 13-06-21.
//

#include <stdio.h>

#include <scroom/scroomplugin.hh>
#include <scroom/unused.hh>

#include "metadata.hh"

PluginInformationInterface::Ptr getPluginInformation() { return Metadata::create(); }

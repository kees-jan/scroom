//
// Created by andy on 13-06-21.
//

#pragma once

#include <scroom/interface.hh>
#include <scroom/rectangle.hh>

#include "utilities.hh"

const std::string METADATA_PROPERTY_NAME = "Metadata";

class MetadataViewInterface
  : public virtual Scroom::Utils::Base
  , private Interface
{
public:
  using Ptr = boost::shared_ptr<MetadataViewInterface>;

public:
  virtual void showMetadata() = 0;
};

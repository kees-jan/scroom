/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <sstream>
#include <fstream>

#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/config/warning_disable.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>

#include <scroom/roi.hh>

BOOST_FUSION_ADAPT_STRUCT(
                          Scroom::Roi::Detail::File,
                          (std::string, name)
                          )

BOOST_FUSION_ADAPT_STRUCT(
                          Scroom::Roi::Detail::Aggregate,
                          (std::string, name)
                          (std::vector<Scroom::Roi::Detail::Presentation>, children)
                          )

BOOST_FUSION_ADAPT_STRUCT(
                          Scroom::Roi::RoiBase,
                          (std::string, description)
                          (std::vector<Scroom::Roi::RoiItem>, children)
                          )

BOOST_FUSION_ADAPT_STRUCT(
                          Scroom::Roi::Rect,
                          (double, left)
                          (double, top)
                          (double, width)
                          (double, height)
                          (std::string, description)
                          (std::vector<Scroom::Roi::RoiItem>, children)
                          )

BOOST_FUSION_ADAPT_STRUCT(
                          Scroom::Roi::List,
                          (std::vector<Scroom::Roi::Detail::Presentation>, presentations)
                          (std::vector<Scroom::Roi::RoiItem>, regions)
                          )

namespace Scroom
{
  namespace Roi
  {
    namespace ascii = boost::spirit::ascii;
    namespace fusion = boost::fusion;
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    namespace spirit = boost::spirit;

    namespace Detail
    {
      namespace Roi = Scroom::Roi;

      struct escaped_char_ : qi::symbols<char, char>
      {
        escaped_char_()
        {
          add
            ("\\\\", '\\')
            ("\\\"", '\"');
        }
      } escaped_char;

      template <typename Iterator>
      struct My_skipper : qi::grammar<Iterator>
      {
        My_skipper()
          : My_skipper::base_type(start, "My_skipper")
        {
          endli %= qi::eol | qi::eoi;

          empty_line %= *ascii::blank >> endli;
          comment %= *ascii::blank >> '#' >> *(ascii::char_-qi::eol) >> endli;

          start %= empty_line | comment;
        }

        qi::rule<Iterator> start;
        qi::rule<Iterator> empty_line;
        qi::rule<Iterator> comment;
        qi::rule<Iterator> endli;
      };

      template <typename Iterator>
      struct My_parser : qi::grammar<Iterator, List(), My_skipper<Iterator> >
      {
        My_parser()
          : My_parser::base_type(start, "My_parser")
        {
          using ascii::char_;
          using ascii::string;
          using phoenix::construct;
          using phoenix::val;
          using phoenix::ref;
          using phoenix::at_c;
          using phoenix::push_back;
          using qi::double_;
          using qi::fail;
          using qi::int_;
          using qi::lexeme;
          using qi::no_skip;
          using qi::lit;
          using qi::on_error;
          using qi::eol;
          using qi::eoi;
          using qi::eps;

          // Preliminaries
          endli %= eol | eoi;
          quoted_string %= lexeme['"' > +(char_ - '"' - eol) > '"'];
          name %= quoted_string | lexeme[+(char_ - ascii::space)];

          start_sublist = no_skip[lit(qi::_r1) [ qi::_val = qi::_r1 ]
                                  >> +char_(' ') [ qi::_val += qi::_1 ]
                                  >> '*'];
          continue_sublist = no_skip[lit(qi::_r1) >> '*'];

          // List of presentations
          file %= qi::skip(ascii::blank)["File:" > name] > endli;

          aggregate = qi::skip(ascii::blank)["Aggregate:"
                                             > name [at_c<0>(qi::_val) = qi::_1]
                                             ] > endli
            >> presentations(qi::_r1) [at_c<1>(qi::_val) = qi::_1];

          presentation_start %= start_sublist(qi::_r1)
            >> (file | aggregate(at_c<0>(qi::_val)));

          presentation_continued %= continue_sublist(qi::_r1) >>
            (file | aggregate(qi::_r1));

          presentations =
            presentation_start(qi::_r1) [ qi::_a=at_c<0>(qi::_1), push_back(qi::_val, at_c<1>(qi::_1))]
            >> *presentation_continued(qi::_a)[ push_back(qi::_val, qi::_1)];

          // List of roi's
          description %= lit("Desc:") > name;
          rect = qi::skip(ascii::blank) ["Rect:" > double_ [at_c<0>(qi::_val) = qi::_1]
                                        > ',' > double_ [at_c<1>(qi::_val) = qi::_1]
                                        > ',' > double_ [at_c<2>(qi::_val) = qi::_1]
                                        > ',' > double_ [at_c<3>(qi::_val) = qi::_1]
                                         >> -(lit(',') >> description[at_c<4>(qi::_val) = qi::_1])
              ] > endli >> -rois(qi::_r1) [at_c<5>(qi::_val) = qi::_1];
          named_roi = qi::skip(ascii::blank) [ description[at_c<0>(qi::_val) = qi::_1]
              ] > endli >> rois(qi::_r1) [at_c<1>(qi::_val) = qi::_1];

          roi_start %= start_sublist(qi::_r1) >>
              ( rect(at_c<0>(qi::_val)) | named_roi(at_c<0>(qi::_val)));
          roi_continued %= continue_sublist(qi::_r1) >>
              ( rect(qi::_r1) | named_roi(qi::_r1));

          rois =
              roi_start(qi::_r1) [ qi::_a=at_c<0>(qi::_1), push_back(qi::_val, at_c<1>(qi::_1))]
              >> *roi_continued(qi::_a)[ push_back(qi::_val, qi::_1)];

          // Finally, the start token
          start %=
              lexeme[presentations(std::string(""))] ^
              lexeme[rois(std::string(""))];

          start.name("start");
          presentations.name("presentations");
          presentation_start.name("presentation_start");
          presentation_continued.name("presentation_continued");
          quoted_string.name("quoted_string");
          name.name("name");
          start_sublist.name("start_sublist");
          continue_sublist.name("continue_sublist");
          file.name("file");
          aggregate.name("aggregate");

          rois.name("rois");
          roi_start.name("roi_start");
          roi_continued.name("roi_continued");
          named_roi.name("named_roi");
          description.name("description");
          rect.name("rect");

          endli.name("end-of-line-or-input");

          on_error<fail>
            (
             start
             , std::cout
             << val("Error! Expecting ")
             << spirit::_4                               // what failed?
             << val(" here: \"")
             << construct<std::string>(spirit::_3, spirit::_2)   // iterators to error-pos, end
             << val("\"")
             << std::endl
             );
        }

        qi::rule<Iterator, std::string()> quoted_string;
        qi::rule<Iterator, std::string(), ascii::blank_type> name;

        qi::rule<Iterator, List(), My_skipper<Iterator> > start;

        qi::rule<Iterator, std::vector<Presentation>(std::string), qi::locals<std::string> > presentations;
        qi::rule<Iterator, fusion::vector<std::string,Presentation>(std::string)> presentation_start;
        qi::rule<Iterator, Presentation(std::string)> presentation_continued;
        qi::rule<Iterator, File()> file;
        qi::rule<Iterator, Aggregate(std::string)> aggregate;

        qi::rule<Iterator, std::vector<RoiItem>(std::string), qi::locals<std::string> > rois;
        qi::rule<Iterator, fusion::vector<std::string,RoiItem>(std::string)> roi_start;
        qi::rule<Iterator, RoiItem(std::string)> roi_continued;
        qi::rule<Iterator, RoiBase(std::string)> named_roi;
        qi::rule<Iterator, Rect(std::string)> rect;
        qi::rule<Iterator, std::string(), ascii::blank_type> description;

        qi::rule<Iterator, std::string(std::string)> start_sublist;
        qi::rule<Iterator, void(std::string)> continue_sublist;
        qi::rule<Iterator, qi::unused_type> endli;
      };

    }

    List::Ptr parse(std::string::const_iterator first, std::string::const_iterator last)
    {
      using ascii::char_;
      using qi::phrase_parse;
      using ascii::blank;
      using qi::eol;
      using qi::no_skip;

      Detail::My_parser<std::string::const_iterator> my_parser;
      Detail::My_skipper<std::string::const_iterator> my_skipper;

      List::Ptr result = List::create();

      // bool r = parse(first, last, my_parser, result);
      bool r = phrase_parse(first, last, my_parser, my_skipper, *result);

      if (first != last)
      {
        std::string remaining(first, last);
        std::string message = "Input could not be parsed entirely.\n";
        message += "Phrase_parse said: ";
        message += (r ? "true\n" : "false\n");
        message += "Remaining content:\n" + remaining;
        throw std::invalid_argument(message);
      }

      if (!r)
        throw std::invalid_argument("parse returned false");

      return result;
    }

    List::Ptr parse(std::stringstream const& s)
    {
      std::string input = s.str();
      return parse(input.begin(), input.end());
    }

    List::Ptr parse(std::string const& filename)
    {
      std::ifstream in(filename, std::ios::in | std::ios::binary);
      if (in)
      {
        std::string input;
        in.seekg(0, std::ios::end);
        input.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&input[0], input.size());
        in.close();

        if(!in)
          throw std::invalid_argument("Error reading file "+filename);

        return parse(input.begin(), input.end());
      }
      throw std::invalid_argument("Failed to open file "+filename);
    }
  }
}

/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ROI_PARSER_HH_
#define ROI_PARSER_HH_

#include <sstream>

#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <boost/config/warning_disable.hpp>
#include <boost/foreach.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>

#include <scroom/roi.hh>

namespace Scroom
{
  namespace Roi
  {
    namespace Detail
    {
      inline std::ostream& operator<<(std::ostream& os, File const& f)
      {
        return os << "[" << f.name << "]";
      }

      inline std::ostream& operator<<(std::ostream& os, Aggregate const& a)
      {
        os << "[" << a.name << "(";
        BOOST_FOREACH(Presentation const& p, a.children)
          os << p;
        os << ")]";

        return os;
      }
    }
  }
}

BOOST_FUSION_ADAPT_STRUCT(
                          Scroom::Roi::Detail::File,
                          (std::string, name)
                          )
     
BOOST_FUSION_ADAPT_STRUCT(
                          Scroom::Roi::Detail::Aggregate,
                          (std::string, name)
                          (std::vector<Scroom::Roi::Detail::Presentation>, children)
                          )

namespace Scroom
{
  namespace Roi
  {
    namespace Detail
    {
      namespace ascii = boost::spirit::ascii;
      namespace fusion = boost::fusion;
      namespace phoenix = boost::phoenix;
      namespace qi = boost::spirit::qi;
      namespace spirit = boost::spirit;
      
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
      struct My_parser : qi::grammar<Iterator, std::vector<Presentation>()>
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
          using qi::eps;

          quoted_string %= lexeme['"' > +(char_ - '"' - eol) > '"'];
          name %= quoted_string | lexeme[+(char_ - ascii::space)];
          // name %= lexeme[+(char_ - ascii::space)];

          start_sublist = no_skip[lit(qi::_r1) [ qi::_val = qi::_r1 ]
                                  >> +char_(' ') [ qi::_val += qi::_1 ]
                                  >> '*'];
          continue_sublist = no_skip[lit(qi::_r1) >> '*'];

          file %= qi::skip(ascii::blank)["File:" > name] > eol;

          aggregate = qi::skip(ascii::blank)["Aggregate:"
                                             > name [at_c<0>(qi::_val) = qi::_1]
                                             ] > eol
            >> presentations(qi::_r1) [at_c<1>(qi::_val) = qi::_1];

          presentation_start %= start_sublist(qi::_r1)
            >> (file | aggregate(at_c<0>(qi::_val)));

          presentation_continued %= continue_sublist(qi::_r1) >>
            (file | aggregate(qi::_r1));

          presentations =
            presentation_start(qi::_r1) [ qi::_a=at_c<0>(qi::_1), push_back(qi::_val, at_c<1>(qi::_1))]
            >> *presentation_continued(qi::_a)[ push_back(qi::_val, qi::_1)];

          start %= presentations(std::string(""));

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
        qi::rule<Iterator, std::vector<Presentation>()> start;
        qi::rule<Iterator, std::vector<Presentation>(std::string), qi::locals<std::string> > presentations;
        qi::rule<Iterator, fusion::vector<std::string,Presentation>(std::string)> presentation_start;
        qi::rule<Iterator, Presentation(std::string)> presentation_continued;
        qi::rule<Iterator, File()> file;
        qi::rule<Iterator, Aggregate(std::string)> aggregate;
        qi::rule<Iterator, std::string(std::string)> start_sublist;
        qi::rule<Iterator, void(std::string)> continue_sublist;
      };
      
      template<typename Iterator>
      std::vector<Presentation> parse(Iterator first, Iterator last)
      {
        using ascii::char_;
        using qi::phrase_parse;
        using ascii::blank;
        using qi::eol;
        using qi::no_skip;

        My_parser<Iterator> my_parser;
        std::vector<Presentation> result;

        bool r = parse(first, last, my_parser, result);

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
    }
  }
}

#endif /* ROI_PARSER_HH_ */

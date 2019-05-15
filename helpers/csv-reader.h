#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "helpers.h"

namespace helpers {

  /** Reads the CSV file line by line.

    This class provides a very basic, but reasonably robust CSV reader. It can even deal with situations such as unescaped columns spanning multiple lines (by calling append in row).

    TODO This needs much more work to be actually useful, but is a start...
    */
  template<char QUOTE = '"', char SEPARATOR = ','>
    class CSVReader {
      public:

        CSVReader() = default ;

      protected:
        /** This method gets called whenever a CSV row has been parsed with the columns stored in the row argument.

          This method must be override in subclasses since its implementation actually provides the implementation.
          */
        virtual void row(std::vector<std::string> & row)  = 0;

        virtual void error(std::ios_base::failure const & e) {
          std::cout << "line " << lineNum_  << ": " << e.what() << std::endl;
        }


        /** Parses the given file.

          If the file cannot be opened, throws the ios_base::failure exception, otherwise parses the file and when parsing of a line is finished, calls the row() method repeatedly until the end of file is reached.
          */
        void parse(std::string const & filename) {
          f_ = std::ifstream(filename, std::ios::in);
          lineNum_ = 1;
          //        f_.open(filename, std::ios::in);
          if (! f_.good())
            throw std::ios_base::failure(STR("Unable to openfile " << filename));
          while (! eof()) {
            try {
              append();
              if (!row_.empty()) {
                row(row_);
                row_.clear();
              }
            } catch(std::ios_base::failure const & e) {
              error(e);
            }
          }
          f_.close();
        }

        /** Returns true if the end of input file has been reached.
        */
        bool eof() const {
          return f_.eof();
        }

        /** Reads next line of the CSV appending it to the existing row vector.

          The parser is fairly simple for now, accepts non-quoted and quoted columns. Quoted columns may span multiple lines and their line endings can be escaped, in which case they will appear as single line.

          TODO make this function more robust.
          */
        void append() {
          if (std::getline(f_, rowbuf_)) ++lineNum_;
          if (rowbuf_.empty()) return;

          for (auto it = rowbuf_.begin(); it != rowbuf_.end(); ++it) {
            if (*it == SEPARATOR) {
              addColumn();
            } else {
              colbuf_.push_back(*it);
            }
          }

          addColumn();
          rowbuf_.clear();
        }

      private:
        /** Adds next column to the currently processed row.
          */
        void addColumn() {
            row_.push_back(colbuf_);
            colbuf_.clear();
        }

        std::string rowbuf_;
        std::string colbuf_;
        std::ifstream f_;
        std::vector<std::string> row_;
        size_t lineNum_;
    }; // CSVReader


} // namespace stuffz

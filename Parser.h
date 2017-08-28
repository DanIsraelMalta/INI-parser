/**
* Configuration namespace holds the configuration file parsing utilities.
*
* General remarks:
* > Access sections using round brackets ('(', ')').
* > Access key/value pairs using square brackets ('[', ']').
* > All values and keys are strings. Use the GetAs() function to cast to the required type.
*
* Dan I. Malta
**/
#pragma once
#ifndef _CONFIGURATION_PARSER_H_
#define _CONFIGURATION_PARSER_H_

#include <algorithm>
#include <type_traits>
#include <cstdint>
#include <cassert>
#include <map>
#include <list>
#include <vector>
#include <stdexcept>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

/*********************************/
/* Configuration file processing */
/*********************************/
namespace Configuration {

    /***********/
    /* Section */
    /***********/
    struct Section {
        // aliases
        using value_map_t   = std::map<std::string, std::string>;
        using section_map_t = std::map<std::string, Section>;
        using values_t      = std::list<value_map_t::const_iterator>;
        using sections_t    = std::list<section_map_t::const_iterator>;

        // properties
        value_map_t   mValues;            // map of key/value pairs
        section_map_t mSections;          // map of sections
        values_t      mOrderedValues;     // key/value pairs, as they were ordered in configuration file
        sections_t    mOrderedSections;   // sections, as they were ordered in configuration file
        Section*      mParent;            // section parent
        std::size_t   mDepth;             // number of nested sections.

        // constructor
                 Section()           : mParent(nullptr), mDepth(0) {}
        explicit Section(Section* p) : mParent(p),       mDepth(0) {}

        // given a key, return its value (within current section)
        const std::string& operator[] (const std::string& xi_key) { return mValues[xi_key]; }
        
        // return a given section by its name
        Section& operator()(const std::string& xi_section) { return mSections[xi_section]; }
    };

    /**********/
    /* Parser */
    /**********/
    class Parser {
        // properties
        private:
            Section       mRoot;            // section
            std::ifstream mConfigFile;      // configuration file
            std::istream* mConfigFileRef;   // reference to configuration file
            std::string   mConfigLine;      // current processed line
            std::size_t   mLineNumber;      // current processed line number in configuration file

        // internal methods
        private:
            /**
            * \brief export a given section
            * 
            * @param {ostream, in|out} exported stream
            * @param {section, in}     section to be exported
            * @param {section, in}     section to be exported
            * @param {string,  in}     section name
            **/
            void ExportSection(std::ostream& xio_stream, const Section& xi_section, const std::string& xi_section_name);

            /**
            * \brief parse a given section
            * 
            * @param {section, in} section to parse
            **/
            void ParseSection(Section& xi_section);

            /**
            * \brief parse a given section line
            *
            * @param {string, out} section name
            * @param {size_t, out} section depth
            **/
            void ParseLine(std::string& xo_name, std::size_t& xo_depth);

            /**
            * \brief return an error
            * 
            * @param {string, in} error label
            **/
            void Error(const char* xi_label);

        // constructor
        public:

            // construct by file name or string
            explicit Parser(const char* xi_file_name) : mConfigFile(xi_file_name), mConfigFileRef(&mConfigFile), mLineNumber(0) {
                if (!mConfigFile) {
                    throw std::runtime_error(std::string("failed to open file: ") + xi_file_name);
                }
                ParseSection(mRoot);
            }

            // construct by file handler
            explicit Parser(std::istream& xi_file) : mConfigFileRef(&xi_file), mLineNumber(0) { ParseSection(mRoot); }

            // class instance is a singleton
            Parser(const Parser &) = delete;
            Parser &operator= (const Parser &) = delete;

        // clear
        public:

            // destructor
            inline void Clear() {
                mConfigFile.close();
                mConfigFileRef = &mConfigFile;
                mConfigLine.clear();
                mLineNumber = 0;

                mRoot.mValues.clear();
                mRoot.mOrderedValues.clear();
                mRoot.mSections.clear();
                mRoot.mOrderedSections.clear();
            }

        // get/export
        public:

            /**
            * \brief get/set root section
            * 
            * @param {section, out} reference to root section
            **/
            inline       Section& Get()       { return mRoot; }
            inline const Section& Get() const { return mRoot; }

            /**
            * \brief export all sections
            * 
            * @param {ostream, in|out} output stream
            **/
            void ExportSection(std::ostream& xio_stream) { 
                ExportSection(xio_stream, Get(), ""); 
            }
    };

    /**
    * \brief return an error
    *
    * @param {string, in} error label
    **/
    inline void Parser::Error(const char* xi_label) {
        std::ostringstream os;
        os << xi_label << " on line #" << mLineNumber;
        throw std::runtime_error(os.str());
    }

    /**
    * \brief trim tab's, line feed and carriage return from a string
    * 
    * @param {string, in}  string
    * @param {string, out} trimmed string
    **/
    inline std::string TrimString(const std::string& xi_string) {
        char toTrim[] = " \t\r\n";
        int sp = 0,
            ep = xi_string.length() - 1;

        for (; sp <= ep; ++sp) {
            if (!std::strchr(toTrim, xi_string[sp])) {
                break;
            }
        }

        for (; ep >= 0; --ep) {
            if (!std::strchr(toTrim, xi_string[ep])) {
                break;
            }
        }

        return xi_string.substr(sp, ep - sp + 1);
    }

    /**
    * \brief return the value from a key/value pair string
    * 
    * @param {string, in}  key/value pair string
    * @param {string, out} value
    **/
    inline std::string ParseValue(const std::string& xi_string) {
        std::string coment = "#;",
                    v = xi_string;
        std::size_t ep = std::string::npos;

        for (std::size_t i = 0; i < coment.size(); ++i) {
            ep = xi_string.find(coment[i]);
            if (ep != std::string::npos) {
                break;
            }
        }

        if (ep != std::string::npos) {
            v = xi_string.substr(0, ep);
        }

        return v;
    }

    /**
    * \brief parse a given section line
    *
    * @param {string, out} section name
    * @param {size_t, out} section depth
    **/
    inline void Parser::ParseLine(std::string& xo_name, std::size_t& xo_depth) {
        xo_depth = 0;
        for (; xo_depth < mConfigLine.length(); ++xo_depth) {
            if (mConfigLine[xo_depth] != '[') {
                break;
            }
        }

        xo_name = mConfigLine.substr(xo_depth, mConfigLine.length() - 2 * xo_depth);
    }

    /**
    * \brief parse a given section
    *
    * @param {section, in} section to parse
    **/
    inline void Parser::ParseSection(Section& xi_section) {
        while (std::getline(*mConfigFileRef, mConfigLine)) {
            ++mLineNumber;

            // comment
            if (mConfigLine[0] == '#' || mConfigLine[0] == ';') {
                continue;
            }

            mConfigLine = TrimString(mConfigLine);

            // empty
            if (mConfigLine.empty()) {
                continue;
            }

            // section?
            if (mConfigLine[0] == '[') {
                std::size_t depth;
                std::string sectionName;

                ParseLine(sectionName, depth);
                Section* lp = nullptr;
                Section* mParent = &xi_section;

                if (depth > xi_section.mDepth + 1) {
                    Error("Section with wrong depth.");
                }

                if (xi_section.mDepth == depth - 1) {
                    lp = &xi_section.mSections[sectionName];
                }
                else {
                    lp = xi_section.mParent;
                    std::size_t n = xi_section.mDepth - depth;

                    for (std::size_t i = 0; i < n; ++i) {
                        lp = lp->mParent;
                    }
                    mParent = lp;
                    lp = &lp->mSections[sectionName];
                }
                if (lp->mDepth != 0) {
                    Error("Duplicate section name on the same level.");
                }
                if (!lp->mParent) {
                    lp->mDepth = depth;
                    lp->mParent = mParent;
                }
                mParent->mOrderedSections.push_back(mParent->mSections.find(sectionName));
                ParseSection(*lp);
            }
            else {
                std::size_t n = mConfigLine.find('=');

                if (n == std::string::npos) {
                    Error("no '=' found");
                }
                std::string v = ParseValue(TrimString(mConfigLine.substr(n + 1, mConfigLine.length() - n - 1)));
                std::pair<Section::value_map_t::const_iterator, bool> res = xi_section.mValues.insert(std::make_pair(TrimString(mConfigLine.substr(0, n)), v));
                if (!res.second) {
                    Error("duplicated key found");
                }
                xi_section.mOrderedValues.push_back(res.first);
            }
        }
    }

    /**
    * \brief export a given section
    *
    * @param {ostream, in|out} exported stream
    * @param {section, in}     section to be exported
    * @param {section, in}     section to be exported
    * @param {string,  in}     section name
    **/
    inline void Parser::ExportSection(std::ostream& xio_stream, const Section& xi_section, const std::string& xi_section_name) {
        if (!xi_section_name.empty()) {
            xio_stream << '\n';
        }

        for (std::size_t i = 0; i < xi_section.mDepth; ++i) {
            xio_stream << '[';
        }

        if (!xi_section_name.empty()) {
            xio_stream << xi_section_name;
        }

        for (std::size_t i = 0; i < xi_section.mDepth; ++i) {
            xio_stream << ']';
        }

        if (!xi_section_name.empty()) {
            xio_stream << std::endl;
        }

        for (Section::values_t::const_iterator it = xi_section.mOrderedValues.begin(); it != xi_section.mOrderedValues.end(); ++it) {
            xio_stream << (*it)->first << '=' << (*it)->second << std::endl;
        }

        for (Section::sections_t::const_iterator it = xi_section.mOrderedSections.begin(); it != xi_section.mOrderedSections.end(); ++it) {
            assert((*it)->second.mDepth == xi_section.mDepth + 1);
            ExportSection(xio_stream, (*it)->second, (*it)->first);
        }
    }

    /*********************************************************/
    /* Extracting & casting configuration file parser output */
    /*********************************************************/

    /**
    * \brief given a string and a set of delimiters,
    *        return a vector whose elements are of all the sub strings located between the given delimiters
    *
    * @param {string, in}  string
    * @param {char*,  in}  delimiters
    * @param {vector, out} vector with sub strings
    **/
    std::vector<std::string> Tokenize(const std::string& xi_string, const char *xi_delimiters) {
        char map[256] = {};

        while (*xi_delimiters++) {
            map[(unsigned)xi_delimiters[-1]] = '\1';
        }

        std::vector<std::string> xo_tokens(1);

        for (auto &ch : xi_string) {
            if (!map[(unsigned)ch]) {
                xo_tokens.back().push_back(ch);
            }
            else if (xo_tokens.back().size()) {
                xo_tokens.push_back(std::string());
            }
        }

        while (xo_tokens.size() && !xo_tokens.back().size()) {
            xo_tokens.pop_back();
        }

        return xo_tokens;
    }

    /**
    * \brief given an "array" value as a string and return it as tokenized vector
    * 
    * @param {string, in}  vector
    * @param {vector, out} clean vector
    **/
    inline void CleanArrayString(const std::string& xi_string, std::vector<std::string>& xo_vector) {
        // validity check
        assert(!xi_string.empty() || (xi_string[0] == '{') || (xi_string[xi_string.size()] == '}'));

        // remove unnecessary characters
        std::string str = xi_string.substr(1, xi_string.size() - 1);
        str.erase(remove_if(str.begin(), str.end(), isspace), str.end());

        // transform to vector
        xo_vector = Configuration::Tokenize(str, ",");
    }

    /**
    * \brief given a string value, cast and return it in the required type
    *
    * @param {string, in} string
    * @param {T,      out} T
    **/
    template<typename T> T GetAs(const std::string&) = delete;

    template<> int GetAs(const std::string& xi_string) {
        std::string str = xi_string;
        str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
        return std::stoi(str);
    }

    template<> long GetAs(const std::string& xi_string) {
        std::string str = xi_string;
        str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
        return std::stoul(str);
    }

    template<> float GetAs(const std::string& xi_string) {
        std::string str = xi_string;
        str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
        return std::stof(str);
    }

    template<> double GetAs(const std::string& xi_string) {
        std::string str = xi_string;
        str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
        return std::stod(str);
    }

    template<> bool GetAs(const std::string& xi_string) {
        std::string str = xi_string;
        str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
        return (str == "true") ? (true) : (false);
    }

    template<> std::vector<int> GetAs(const std::string& xi_string) {
        std::vector<std::string> vec;
        std::vector<int> xo_vector;
        CleanArrayString(xi_string, vec);
        xo_vector.resize(vec.size());

        for (std::size_t i = 0; i < vec.size(); ++i) {
            xo_vector[i] = std::stoi(vec[i]);
        }

        return xo_vector;
    }
   
    template<> std::vector<long> GetAs(const std::string& xi_string) {
        std::vector<std::string> vec;
        std::vector<long> xo_vector;
        CleanArrayString(xi_string, vec);
        xo_vector.resize(vec.size());

        xo_vector.resize(vec.size());
        for (std::size_t i = 0; i < vec.size(); ++i) {
            xo_vector[i] = std::stoul(vec[i]);
        }

        return xo_vector;
    }

    template<> std::vector<float> GetAs(const std::string& xi_string) {
        std::vector<std::string> vec;
        std::vector<float> xo_vector;
        CleanArrayString(xi_string, vec);
        xo_vector.resize(vec.size());

        xo_vector.resize(vec.size());
        for (std::size_t i = 0; i < vec.size(); ++i) {
            xo_vector[i] = std::stof(vec[i]);
        }

        return xo_vector;
    }

    template<> std::vector<double> GetAs(const std::string& xi_string) {
        std::vector<std::string> vec;
        std::vector<double> xo_vector;
        CleanArrayString(xi_string, vec);
        xo_vector.resize(vec.size());

        xo_vector.resize(vec.size());
        for (std::size_t i = 0; i < vec.size(); ++i) {
            xo_vector[i] = std::stod(vec[i]);
        }

        return xo_vector;
    }

    template<> std::vector<bool> GetAs(const std::string& xi_string) {
        std::vector<std::string> vec;
        std::vector<bool> xo_vector;
        CleanArrayString(xi_string, vec);
        xo_vector.resize(vec.size());

        xo_vector.resize(vec.size());
        for (std::size_t i = 0; i < vec.size(); ++i) {
            xo_vector[i] = (vec[i] == "true") ? (true) : (false);
        }

        return xo_vector;
    }
}

#endif // !_CONFIGURATION_PARSER_H_

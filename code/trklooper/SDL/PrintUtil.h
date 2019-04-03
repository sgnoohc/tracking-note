#ifndef PrintUtil_h
#define PrintUtil_h

#include <iostream>
#include <string>

namespace SDL
{

    class prefixbuf : public std::streambuf
    {
        private:
            std::string     prefix;
            std::streambuf* sbuf;
            bool            need_prefix;

            int sync();
            int overflow(int c);

        public:
            prefixbuf(std::string const& prefix, std::streambuf* sbuf);
    };

    class oprefixstream : private virtual prefixbuf, public std::ostream
    {
        public:
            oprefixstream(std::string const& prefix, std::ostream& out);
    };

    // The following modified ostream will prefix "SDL::  " for every line
    extern oprefixstream cout;

}

#endif

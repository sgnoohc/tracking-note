#include "PrintUtil.h"

namespace SDL
{
    // The following modified ostream will prefix "SDL::  " for every line
    oprefixstream cout("SDL::  ", std::cout);
}

int SDL::prefixbuf::sync()
{
    return this->sbuf->pubsync();
}

int SDL::prefixbuf::overflow(int c)
{
    if (c != std::char_traits<char>::eof())
    {
        if (this->need_prefix
                && !this->prefix.empty()
                && this->prefix.size() != (unsigned) this->sbuf->sputn(&this->prefix[0], this->prefix.size()))
            return std::char_traits<char>::eof();
        this->need_prefix = c == '\n';
    }
    return this->sbuf->sputc(c);
}

SDL::prefixbuf::prefixbuf(std::string const& prefix, std::streambuf* sbuf) : prefix(prefix) , sbuf(sbuf) , need_prefix(true)
{
}

SDL::oprefixstream::oprefixstream(std::string const& prefix, std::ostream& out):
    prefixbuf(prefix, out.rdbuf()),
    std::ios(static_cast<std::streambuf*>(this)),
    std::ostream(static_cast<std::streambuf*>(this))
{
}

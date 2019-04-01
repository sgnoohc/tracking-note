#ifndef Event_h
#define Event_h

#include <vector>

#include "Module.h"
#include "Hit.h"

namespace SDL
{
    class Event
    {
        private:

            std::vector<Module*> modules_;

            std::vector<Hit*> hits_;

        public:

            Event();

            ~Event();

    };
}

#endif

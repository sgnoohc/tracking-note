#ifndef Event_h
#define Event_h

#include <vector>
#include <map>

#include "Module.h"
#include "Hit.h"

namespace SDL
{
    class Event
    {
        private:

            // List of module pointers
            std::vector<Module*> modules_;

            // List of hit pointers
            std::vector<Hit*> hits_;

            // map of module pointers
            std::map<unsigned int, Module*> modulesMapByDetId_;

        public:

            Event();

            ~Event();

            void addModule(Module* module);

    };
}

#endif

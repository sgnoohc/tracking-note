#ifndef Event_h
#define Event_h

#include <vector>
#include <map>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>

#include "Module.h"
#include "Hit.h"

namespace SDL
{
    class Event
    {
        private:

            // map of modules
            std::map<unsigned int, Module> modulesMapByDetId_;

            // List of hits
            std::vector<Hit> hits_;

            // List of module pointers
            std::vector<Module*> modules_;

        public:

            Event();

            ~Event();

            bool hasModule(unsigned int detId);

            Module& getModule(unsigned int detId);

            const std::vector<Module*> getModulePtrs() const;

            void addHitToModule(Hit hit, unsigned int detId);

            friend std::ostream& operator<<(std::ostream& out, const Event& event);

    };
}

#endif

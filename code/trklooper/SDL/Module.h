#ifndef Module_h
#define Module_h

#include <vector>

#include "Hit.h"
#include "MiniDoublet.h"

namespace SDL
{

    class Module
    {

        private:

            // Layer
            // when order == 0, layer ranges from 1 to 6
            // when order == 1, layer ranges from 1 to 5
            unsigned short layer_;

            // Side
            // 1 = -z side of the endcap modules
            // 2 = +z side of the endcap modules
            // 3 = barrel modules
            unsigned short side_; // 1: 2: 3: barrel

            // Orientation of the module
            // 0: lying along beam (or tilted)
            // 1: lying along transverse (e.g. endcap)
            // For side == 3, order is always 0
            unsigned short order_;

            // Index of which rod in the barrel
            // Closest to x-axis is rod = 1, and it goes counter-clockwise
            // total number of rods for each layer: 18, 26, 36, 48, 60, and 78
            // if order==1, rod is always 0
            unsigned short rod_;

            // Index of which ring in the endcap
            // For the layer 1 and 2, there are 15 rings, first 10 are PS, the latter 5 are 2S
            // For the layer 3, 4, and 5, there are 12 rings, first 7 are PS, the latter 5 are 2S
            // if order==0, ring is always 0
            unsigned short ring_;

            // 4: endcap (i.e. side 1 or 2 and order 1)
            // 5: barrel (i.e. = order 0)
            unsigned short subdet_;

            // For order==1 the # of module depends on how far away from beam spot,
            // module 1 is closest along the x-axis
            // layer 1 or 2, ring 1: 20 modules
            // layer 1 or 2, ring 2: 24 modules
            // layer 1 or 2, ring 3: 24 modules
            // layer 1 or 2, ring 4: 28 modules
            // layer 1 or 2, ring 5: 32 modules
            // layer 1 or 2, ring 6: 32 modules
            // layer 1 or 2, ring 7: 36 modules
            // layer 1 or 2, ring 8: 40 modules
            // layer 1 or 2, ring 9: 40 modules
            // layer 1 or 2, ring 10: 44 modules
            // layer 1 or 2, ring 11: 52 modules
            // layer 1 or 2, ring 12: 60 modules
            // layer 1 or 2, ring 13: 64 modules
            // layer 1 or 2, ring 14: 72 modules
            // layer 1 or 2, ring 15: 76 modules
            // layer 3, 4, or 5, ring 1: 28 modules
            // layer 3, 4, or 5, ring 2: 28 modules
            // layer 3, 4, or 5, ring 3: 32 modules
            // layer 3, 4, or 5, ring 4: 36 modules
            // layer 3, 4, or 5, ring 5: 36 modules
            // layer 3, 4, or 5, ring 6: 40 modules
            // layer 3, 4, or 5, ring 7: 44 modules
            // layer 3, 4, or 5, ring 8: 52 modules
            // layer 3, 4, or 5, ring 9: 56 modules
            // layer 3, 4, or 5, ring 10: 64 modules
            // layer 3, 4, or 5, ring 11: 72 modules
            // layer 3, 4, or 5, ring 12: 76 modules
            //
            // For order==0, the # of module depends on how far away from beam spot,
            // for side==3: module 1 has lowest z (starting from the negative value)
            // layer 1, side 3: 7 modules
            // layer 2, side 3: 11 modules
            // layer 3, side 3: 15 modules
            // layer 4, 5, or 6, side 3: 24 modules
            // for side==1,2 (i.e. tilted): module 1 is along x-axis
            // layer 1, side 1, or 2: 18 modules
            // layer 2, side 1, or 2: 26 modules
            // layer 3, side 1, or 2: 36 modules
            unsigned short module_;

            // isLower is always the pixel if it's a PS module, if it's a 2S module it's whichever is the protruding side when 2S are staggered
            unsigned short isLower_;

            // The unique detector ID for the lower layer
            unsigned int detId_;

            // The modules are put in alternating order where the modules are inverted every other one
            bool isInverted_;

            // vector of hits
            std::vector<Hit*> hits_;

        public:

            // constructor/destructor
            Module();
            ~Module();

            // accessor functions
            const unsigned short& order() const;
            const unsigned short& rod() const;
            const unsigned short& ring() const;
            const unsigned short& subdet() const;
            const unsigned short& module() const;
            const unsigned short& isLower() const;
            const unsigned int& detId() const;
            const bool& isInverted() const;
            const std::vector<Hit*>& hits() const;
            void addHit(Hit* hit);

    };

}

#endif

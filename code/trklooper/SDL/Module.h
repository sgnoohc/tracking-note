#ifndef Module_h
#define Module_h

#include <vector>
#include <iostream>

#include "Hit.h"
#include "MiniDoublet.h"

namespace SDL
{

    class Module
    {

        private:

            // Decoding DetId
            //
            // detId comes in 29 bits. There are two formats depending on which sub detector it is.
            //
            // 29 bits total
            //
            // left to right index (useful python, i.e. string[idx:jdx])
            // 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
            //
            // right to left index (useful when C++ style, i.e. bit shifting)
            // 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
            //
            //  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x
            //
            //    -subdet-       -layer--          --------rod---------    -------module-------
            //             -side       --layer-       ----ring---          -------module-------
            //
            //

            //----------
            // * detId *
            //----------
            // The unique detector ID for the lower layer
            unsigned int detId_;

            //-----------
            // * subdet *
            //-----------
            // bits 27 to 25
            // subdet = (detId & (7 << 25)) >> 25;
            // subdet can take either 4 or 5
            // 4: endcap
            // 5: barrel
            unsigned short subdet_;

            //---------
            // * Side *
            //---------
            // bits 24 to 23
            // if (subdet_ == 4)
            // {
            //     side_ = (detId_ & (3 << 23)) >> 23;
            // }
            // else if (subdet_ == 5)
            // {
            //     side_ = 3;
            // }
            // 1 = -z side of the endcap modules
            // 2 = +z side of the endcap modules
            // 3 = barrel modules (determined via checking subdet)
            unsigned short side_;

            //----------
            // * Layer *
            //----------
            // either bits 22 to 20 or 20 to 18
            // if (subdet_ == 4)
            // {
            //     layer_ = (detId_ & (7 << 18)) >> 18;
            // }
            // else if (subdet_ == 5)
            // {
            //     layer_ = (detId_ & (7 << 20)) >> 20;
            // }
            // depending on whether it is subdet = 4 or 5, the position of layer information is different
            // layer = detId_bits[06:08] if subdet = 5
            // layer = detId_bits[08:10] if subdet = 4
            unsigned short layer_;

            //--------
            // * Rod *
            //--------
            // bits 16 to 10 only when subdet = 5
            // if (subdet_ == 5)
            // {
            //     rod_ = (detId_ & (127 << 10)) >> 10;
            // }
            // else if (subdet_ == 4)
            // {
            //     rod_ = 0;
            // }
            // Index of which rod in the barrel
            // Closest to the positive x-axis line is rod = 1, and it goes counter-clockwise in x-y plane projection
            // total number of rods for each layer: 18, 26, 36, 48, 60, and 78
            unsigned short rod_;

            //---------
            // * Ring *
            //---------
            // bits 15 to 12 only when subdet = 4
            // if (subdet_ == 5)
            // {
            //     ring_ = 0;
            // }
            // else if (subdet_ == 4)
            // {
            //     ring_ = (detId_ & (15 << 12)) >> 12;
            // }
            // Index of which ring in the endcap
            // For the layer 1 and 2, there are 15 rings, first 10 are PS, the latter 5 are 2S
            // For the layer 3, 4, and 5, there are 12 rings, first 7 are PS, the latter 5 are 2S
            unsigned short ring_;

            //-----------
            // * Module *
            //-----------
            // bits 8 to 2
            // module_ = (detId_ & (127 << 2)) >> 2;
            // For subdet==4 the # of module depends on how far away from beam spot,
            // module 1 is closest to the positive x-axis line and it goes counter-clockwise in x-y plane projection
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
            // For subdet==5, the # of module depends on how far away from beam spot,
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

            //------------
            // * isLower *
            //------------
            // bit 28
            // isLower_ = (detId_ & 1);
            // isLower is always the pixel if it's a PS module, if it's a 2S module it's whichever is the protruding side when 2S are staggered
            unsigned short isLower_;

            // The modules are put in alternating order where the modules are inverted every other one
            // TODO: Add description on even/odd module ID # and its relation to PS inversions
            bool isInverted_;

            // vector of hit pointers
            std::vector<Hit*> hits_;

            void setDerivedQuantities();

        public:

            // constructor/destructor
            Module();
            Module(unsigned int detId);
            Module(const Module&);
            ~Module();

            // accessor functions
            const unsigned int& detId() const;
            const unsigned short& subdet() const;
            const unsigned short& side() const;
            const unsigned short& layer() const;
            const unsigned short& rod() const;
            const unsigned short& ring() const;
            const unsigned short& module() const;
            const unsigned short& isLower() const;
            const bool& isInverted() const;
            const std::vector<Hit*>& hits() const;

            // modifying the class content
            void setDetId(unsigned int);
            void addHit(Hit* hit);

            // static functions to parse detId
            static unsigned short parseSubdet(unsigned int);
            static unsigned short parseSide(unsigned int);
            static unsigned short parseLayer(unsigned int);
            static unsigned short parseRod(unsigned int);
            static unsigned short parseRing(unsigned int);
            static unsigned short parseModule(unsigned int);
            static unsigned short parseIsLower(unsigned int);

            // printing
            friend std::ostream& operator<<(std::ostream& os, const Module& module);

    };

}

#endif

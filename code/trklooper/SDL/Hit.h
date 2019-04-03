#ifndef Hit_h
#define Hit_h

#include <math.h>
#include <iostream>
#include <cmath>

#include "Math.h"

namespace SDL
{

    class Hit
    {
        private:
            float x_;
            float y_;
            float z_;
            float r3_;
            float rt_;
            float phi_;

            void setDerivedQuantities();

        public:
            Hit();
            Hit(float x, float y, float z);
            Hit(const Hit&);
            ~Hit();
            void setX(float x);
            void setY(float y);
            void setZ(float z);
            const float& x() const;
            const float& y() const;
            const float& z() const;
            const float& rt() const;
            const float& r3() const;
            const float& phi() const;

            // variable computation between two hits
            float deltaPhi(Hit);
            float deltaPhiChange(Hit);

            // operators
            bool operator !=(const Hit&) const;
            bool operator ==(const Hit&) const;
            Hit  operator - (const Hit&) const;
            Hit& operator = (const Hit&);
            Hit& operator -=(const Hit&);
            Hit& operator +=(const Hit&);

            // pretty printing
            friend std::ostream& operator<<(std::ostream& out, const Hit& hit);
    };

}

#endif

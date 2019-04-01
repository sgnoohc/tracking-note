#ifndef Hit_h
#define Hit_h

#include <math.h>
#include <iostream>
#include <cmath>

namespace SDL
{

    class Hit
    {
        private:
            float x_;
            float y_;
            float z_;

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
            const float& X() const;
            const float& Y() const;
            const float& Z() const;

            // variable computation between two hits
            float deltaPhi(Hit* hit);
            float deltaPhiChange(Hit* hit);

            // operators
            bool operator !=(const Hit&) const;
            bool operator ==(const Hit&) const;
            Hit& operator = (const Hit&);
            Hit& operator -=(const Hit&);
            Hit& operator +=(const Hit&);

    };

}

#endif

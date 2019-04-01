#ifndef RecHit_h
#define RecHit_h

namespace SDL
{

    class RecHit
    {
        private:
            float x_;
            float y_;
            float z_;

        public:
            RecHit();
            RecHit(float x, float y, float z);
            ~RecHit();
            void setX(float x);
            void setY(float y);
            void setZ(float z);
            float x();
            float y();
            float z();
            float X();
            float Y();
            float Z();

    };

}

#endif

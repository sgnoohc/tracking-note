#ifndef SDLRecHit_h
#define SDLRecHit_h

class SDLRecHit
{
    private:
        float x;
        float y;
        float z;

    public:
        SDLRecHit();
        SDLRecHit(float x, float y, float z);
        ~SDLRecHit();
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

#endif

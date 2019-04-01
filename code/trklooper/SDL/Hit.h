#ifndef Hit_h
#define Hit_h

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
            ~Hit();
            void setX(float x);
            void setY(float y);
            void setZ(float z);
            const float& x();
            const float& y();
            const float& z();
            const float& X();
            const float& Y();
            const float& Z();

    };

}

#endif

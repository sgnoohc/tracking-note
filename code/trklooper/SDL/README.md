## SDL namespace

The namespace ```SDL``` contains:

    namespace SDL
    {
        class Event;
        class Hit;
        class Module;
        class MiniDoublet;

        namespace Math;
    }

### Event

Holds all the pointers to hits, modules, mini-doublets, and more in the instance.  

### Hit

Currently only contains 3-vector. May need to add ```detId```. I think it's okay to not have other id's. (e.g. order, layer etc.)  

### Module

A module is defined as one side of the double-modules. (i.e. 2S or PS)  
Each side of the double-modules has unique ```detId```.  

Each module has various qualifiers. The ```Module.h``` explains them.

### MiniDoublet

Holds two hits. Doesn't have much implemented.

### Math

It is a namespace that will hold various arithmetic operation functions.

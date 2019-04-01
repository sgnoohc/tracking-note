## SDL namespace

### Event

```Event``` will hold all the pointers to hits, modules, mini-doublets, and more in the instance.  

### Hit

Currently only contains 3-vector. May need to add ```detId```. I think it's okay to not have other id's. (e.g. order, layer etc.)  

### Module

A module is defined as one side of the double-modules. (i.e. 2S or PS)  
Each side of the double-modules has unique ```detId```.  

Each module has various qualifiers. The ```Module.h``` explains them.

### MiniDoublet

Holds two hits. Doesn't have much implemented.

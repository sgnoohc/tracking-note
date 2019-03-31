# SDL Tracking Code

Some notes https://github.com/slava77/sdlTracking/wiki  
This seems to contain steps to create tracking ntuples.  

## Tracking Ntuple Example

An example tracking ntuple exists here:  

    /hadoop/cms/store/user/slava77/CMSSW_10_4_0_patch1-tkNtuple/pass-e072c1a/27411.0_TenMuExtendedE_0_200/trackingNtuple.root

Copied the TTree to file.root (just pointers) and then created a looper based on ```rooutil/makeclass.sh```.  
It had an issue with ```ph2_bbxi``` branch. So I deleted duplicate lines by hand.  

Created ```code/trklooper```

    cd code/trklooper
    source ../../rooutil/thisrootuil.sh
    make -j

### Notes on the content of tracking ntuple

Prefixes

1. ```bsp_```: Beam spot
1. ```see_```: Seed
1. ```trk_```: Track
1. ```pix_```: Pixel hits? or including strips?
1. ```ph2_```: Phase 2 Hits?
1. ```vtx_```: Vertex
1. ```sim_```: Sim Track
1. ```simhit_```: Sim Hits
1. ```simvtx_```: Sim Vertex

Left over
1. ```simpv_idx``` : Sim primary vertices index?
1. ```inv_detId``` : inverse detector id?


### Questions about Tracking Ntuple

1. Did I guess the prefix meanings correctly?
2. ph2_bbxi branch seems to be declared twice (fix?)
3. What are the meanings of the left overs?

### Playing around with tracking ntuple

1. What does sim_pdgId = 22 mean..?
   2212, 211, 321, 22, 
1. What are the pca_pt == 0 non-mu events

### Petal TEC for Run-1

Staggered

    https://cds.cern.ch/record/1431470/files/TECpetal.jpg?subformat=icon-1440

### Navigating modules

Found out that one of the barrel module around z = 0, with layer = 1 is 438047761 and the pairing module is 438047762
Some modules are "inverted" where the strip is below, and pixel is upper, while some have pixel lower and strip is upper. (Because life is never easy...)

The ranges of variables printed:

    ph2_isBarrel
    [0]
    ph2_layer
    [1, 2, 3, 4, 5, 6]
    ph2_isLower
    [0, 1]
    ph2_isUpper
    [0, 1]
    ph2_isStack
    [0]
    ph2_order
    [0, 1]
    ph2_ring
    [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
    ph2_rod
    [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78]
    ph2_subdet
    [4, 5]
    ph2_side
    [1, 2, 3]
    ph2_module
    [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76]
    ph2_simType
    [0, 3]


 ph2_isBarrel    = (vector<short>*)0x67a08f0
 ph2_isLower     = (vector<unsigned short>*)0x6677740
 ph2_isUpper     = (vector<unsigned short>*)0x662be80
 ph2_isStack     = (vector<unsigned short>*)0x67c5000
 ph2_order       = (vector<unsigned short>*)0x66b7320
 ph2_ring        = (vector<unsigned short>*)0x68e9820
 ph2_rod         = (vector<unsigned short>*)0x4d287d0
 ph2_detId       = (vector<unsigned int>*)0x4ceda20
 ph2_subdet      = (vector<unsigned short>*)0x69601e0
 ph2_layer       = (vector<unsigned short>*)0x43d4aa0
 ph2_side        = (vector<unsigned short>*)0x6a963c0
 ph2_module      = (vector<unsigned short>*)0x4dd9cc0
 ph2_xySignificance = (vector<vector<float> >*)0x6981f70
 ph2_simType     = (vector<unsigned short>*)0x6862fb0
 ph2_x           = (vector<float>*)0x4a5e8f0
 ph2_y           = (vector<float>*)0x4a5f510
 ph2_z           = (vector<float>*)0x46e3e00
 ph2_xx          = (vector<float>*)0x67f3e30
 ph2_xy          = (vector<float>*)0x6956e60
 ph2_yy          = (vector<float>*)0x68aecf0
 ph2_yz          = (vector<float>*)0x3090e80
 ph2_zz          = (vector<float>*)0x6aa83c0
 ph2_zx          = (vector<float>*)0x68683d0
 ph2_radL        = (vector<float>*)0x666ed00
 ph2_bbxi        = (vector<float>*)0x2412a30
 ph2_bbxi        = (vector<float>*)0x687e680

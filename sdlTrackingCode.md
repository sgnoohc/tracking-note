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

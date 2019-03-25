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

### Questions about Tracking Ntuple

ph2_bbxi branch seems to be declared twice

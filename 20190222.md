
## Email from Slava

    Vyacheslav E. Krutelyov via fermicloud.onmicrosoft.com 
    3:16 PM (36 minutes ago)
    Reply
    to Balaji, Philip
    Hi Philip
    I'm adding Balaji in cc
    The latest version of the code is in
    https://github.com/slava77/cms-tkph2-ntuple/tree/superDoubletLinked-91X-noMock
    It needs input ntuples.
    The files are in
    /hadoop/cms/store/user/slava77/CMSSW_9_1_0_pre1-tkNtuple/2023PU200/
    20634.0_TTbar_*/trackingNtuple.root has the flat tracker layout
    21434.0_TTbar_*/trackingNtuple.root has the tilted tracker layout
    The plots in the slides were based on an older 62XSLHC release
    with the older flat tracker layout which was the default.
    I validated that results are about the same with the new release.
    So, perhaps we do not need to start from the earlier version for
    reproducibility purposes.
    The driver script is
    runSuperDoublets.sh
    it currently has the input ntuple set to the flat layout.
    The plots equivalent to what's in the slides should come out from
    ./runSuperDoublets.sh -n 30 -mm 3 -DB 16.0 -DE 24.0 -us 1
    The available options are:
    -n : the number of events to run
    -mm : "mock mode" configuration switch
            0-3 reduces the actual CMSSW layout to the simplified geometry layout
                    based on simhit position and momentum info
            0 : straight line trajectory from the anchor to the next layer
            1 : helix line trajectory from the anchor to the next layer
            3: as 1 but offset by rechit position to mimic realistic reco resolution
            11: use simhits from the actual geometry
            13: use actual rechits
            IIRC, mm=13 was not fully working yet
    -DB and -DE: distance between the anchor layer (odd) and the next layer
    -us : use of pixel tracker seed to link ... always 1
            0 : ignores input pixel seeds and doesn't make plots linking them with
    the OT segments
    The driver script calls
    ScanChainMockSuperDoublets function from the doubletAnalysis.C file.
    Almost everything is in one function.
    So, reading it may be a bit hard.
                    --slava

## Getting started

Working directory

    ~/public_html/analysis/tracking/cms-tkph2-ntuple

Setup

    source rooutil/root.sh

Modified the code a little bit
The version of the code used is https://github.com/slava77/cms-tkph2-ntuple/tree/184d2325147e6930030d3d1f780136bc2dd29ce6

    $ git diff runSuperDoublets.sh
    diff --git a/runSuperDoublets.sh b/runSuperDoublets.sh
    index 8f2fb54d..c1218313 100755
    --- a/runSuperDoublets.sh
    +++ b/runSuperDoublets.sh
    @@ -10,7 +10,14 @@ while [ X$# != X0 ] ; do
         esac
     done
    
    +# echo -e ".L doubletAnalysis.C++ \n\
    +# t9 = new TChain(\"trackingNtuple/tree\"); t9->Add(\"/Data/cms/tkph2/stublink/CMSSW_9_1_0_pre1-tkNtuple/2023PU140/20634.0_TTbar_14TeV+TTbar_14TeV_TuneCUETP8M1_2023D10PU_GenSimHLBeamSpotFull14+DigiFullTriggerPU_2023D10PU+RecoFullGlobalPU_2023D10PU+HARVESTFullGlobalPU_2023D10PU/trackingNtuple.root\"); ScanChainMockSuperDoublets(t9, ${n}, true, ${mm}, ${DB}, ${DE}, ${us}, false, true, true);\n.qqqq"\
    +#     | root -l -b >& a_ttbar_stats_91X_2023PU140_20634.0_mm${mm}_sd${DB}cm${DE}cm_us${us}.log
    +
    +# echo -e ".L doubletAnalysis.C++ \n\
    +# t9 = new TChain(\"trackingNtuple/tree\"); t9->Add(\"/hadoop/cms/store/user/slava77/CMSSW_9_1_0_pre1-tkNtuple/2023PU200/20634.0_TTbar_14TeV+TTbar_14TeV_TuneCUETP8M1_2023D10PU_GenSimHLBeamSpotFull14+DigiFullTriggerPU_2023D10PU+RecoFullGlobalPU_2023D10PU+HARVESTFullGlobalPU_2023D10PU/trackingNtuple.root\"); ScanChainMockSuperDoublets(t9, ${n}, true, ${mm}, ${DB}, ${DE}, ${us}, false, true, true);\n.qqqq"\
    +#     | root -l -b >& a_ttbar_stats_91X_2023PU200_20634.0_mm${mm}_sd${DB}cm${DE}cm_us${us}.log
    +
     echo -e ".L doubletAnalysis.C++ \n\
    -t9 = new TChain(\"trackingNtuple/tree\"); t9->Add(\"/Data/cms/tkph2/stublink/CMSSW_9_1_0_pre1-tkNtuple/2023PU140/20634.0_TTbar_14TeV+TTbar_14TeV_TuneCUETP8M1_2023D10PU_GenSimHLBeamSpotFull14+DigiFullTriggerPU_2023D10PU+RecoFullGlobalPU_2023D10PU+HARVESTFullGlobalPU_2023D10PU/trackingNtuple.root\"); ScanChainMockSuperDoublets(t9, ${n}, true, ${mm}, ${DB}, ${DE}, ${us}, false, true, true);\n.qqqq"\
    +t9 = new TChain(\"trackingNtuple/tree\"); t9->Add(\"/hadoop/cms/store/user/slava77/CMSSW_9_1_0_pre1-tkNtuple/2023PU140/20634.0_TTbar_14TeV+TTbar_14TeV_TuneCUETP8M1_2023D10PU_GenSimHLBeamSpotFull14+DigiFullTriggerPU_2023D10PU+RecoFullGlobalPU_2023D10PU+HARVESTFullGlobalPU_2023D10PU/trackingNtuple.root\"); ScanChainMockSuperDoublets(t9, ${n}, true, ${mm}, ${DB}, ${DE}, ${us}, false, true, true);\n.qqqq"\
         | root -l -b >& a_ttbar_stats_91X_2023PU140_20634.0_mm${mm}_sd${DB}cm${DE}cm_us${us}.log
    -

Then I ran with

    ./runSuperDoublets.sh -n 30 -mm 3 -DB 16.0 -DE 24.0 -us 1

I noticed that the PU200 version of the input would complain about missing branches. So I changed to PU140. Besides the slides were made with PU140 afaik.
Afterwards, code started running just fine.

The code ran for about 10 minutes.

    [02/22 03:51:08 PM] <1044> <jobs:0> [phchang@uaf-10] [~/public_html/analysis/tracking/cms-tkph2-ntuple]
    $ ./runSuperDoublets.sh -n 30 -mm 3 -DB 16.0 -DE 24.0 -us 1
    [02/22 04:00:41 PM] <1045> <jobs:0> [phchang@uaf-10] [~/public_html/analysis/tracking/cms-tkph2-ntuple]
    $

It seemed to have output a lot of .png files (see [here](results/20190222_firstrun))

Looking at the log files to understand the code. ([log file](results/20190222_firstrun/a_ttbar_stats_91X_2023PU140_20634.0_mm3_sd16.0cm24.0cm_us1.log))

    Running in mockMode 3
    Running with SD distance 16 24
    Running with useSeeds 1
    Endcap is enabled 
    Use dPhiPos in endcap
    Running with tiltedOT123 0

The mockMode 3, I think means the odd-layer (called anchor) has a hit from a simulated tracks and then the next hit in the even layer is smeared by reconstruction resolution.
The distance between the layer pairs are 16 cm and 24 cm in barrel and endcap respectively.
useSeeds I think is linking the segments to pixel seeds. But not entirely sure what that means...
Endcap is enabled in this case.
Does "Use dPhiPos in endcap" = the method Slava described in his [slide 10](https://indico.cern.ch/event/768528/contributions/3317920/attachments/1798894/2933495/190220_SegmentLinking.pdf)?

The constant expression in L1656-1663 seems to contain typos when compared to TDR value.
But who knows how it changed now...

## Jargons

    SD: Super Doublet? i.e. Segment?
    MD: Mini Doublet?
    PS: Pixel-Strip module (2.4 cm long, macro-pixel of 1.5mm length)
    2S: Strip-Strip module (5.0 cm long)
    SDL: super doublet linking
    NGLL: Not Ghost Lower Layer

## Outer tracker PS and 2S module details

These numbers are from TDR.
PS modules are 2.4cm x 100microns.
macro-pixels are either aligned in z (barrel) or r (in endcap).
macro-pixels are 1.5mm x 100microns

2S modules are
lower layer: 2x1016 strips ~5.0cm x 90 microns
top layer:   2x1016 strips ~5.0cm x 90 microns
    

## Code variable documentations

    constexpr float deltaZLum: luminous region size in cm? maybe?
    constexpr float pixelPSZpitch : 0.15 cm

## List of questions

What exactly is being done with ```useSeeds = 1```?
Does "Use dPhiPos in endcap" = the method Slava described in his slide?


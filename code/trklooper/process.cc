#include "trktree.h"
#include "rooutil.h"
#include "cxxopts.h"

class AnalysisConfig {

public:

    // TString that holds the input file list (comma separated)
    TString input_file_list_tstring;

    // TString that holds the name of the TTree to open for each input files
    TString input_tree_name;

    // Output TFile
    TFile* output_tfile;

    // Number of events to loop over
    int n_events;

    // Debug boolean
    bool debug;

    // TChain that holds the input TTree's
    TChain* events_tchain;

    // Custom Looper object to facilitate looping over many files
    RooUtil::Looper<trktree> looper;

    // Custom Cutflow framework
    RooUtil::Cutflow cutflow;

    // Custom Histograms object compatible with RooUtil::Cutflow framework
    RooUtil::Histograms histograms;

};

AnalysisConfig ana;

// ./process INPUTFILEPATH OUTPUTFILE [NEVENTS]
int main(int argc, char** argv)
{

//********************************************************************************
//
// 1. Parsing options
//
//********************************************************************************

    // cxxopts is just a tool to parse argc, and argv easily

    // Grand option setting
    cxxopts::Options options("\n  $ doAnalysis",  "\n         **********************\n         *                    *\n         *       Looper       *\n         *                    *\n         **********************\n");

    // Read the options
    options.add_options()
        ("i,input"       , "Comma separated input file list OR if just a directory is provided it will glob all in the directory BUT must end with '/' for the path", cxxopts::value<std::string>())
        ("t,tree"        , "Name of the tree in the root file to open and loop over"                                             , cxxopts::value<std::string>())
        ("o,output"      , "Output file name"                                                                                    , cxxopts::value<std::string>())
        ("n,nevents"     , "N events to loop over"                                                                               , cxxopts::value<int>()->default_value("-1"))
        ("d,debug"       , "Run debug job. i.e. overrides output option to 'debug.root' and 'recreate's the file.")
        ("h,help"        , "Print help")
        ;

    auto result = options.parse(argc, argv);

    // NOTE: When an option was provided (e.g. -i or --input), then the result.count("<option name>") is more than 0
    // Therefore, the option can be parsed easily by asking the condition if (result.count("<option name>");
    // That's how the several options are parsed below

    //_______________________________________________________________________________
    // --help
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(1);
    }

    //_______________________________________________________________________________
    // --input
    if (result.count("input"))
    {
        ana.input_file_list_tstring = result["input"].as<std::string>();
    }
    else
    {
        std::cout << options.help() << std::endl;
        std::cout << "ERROR: Input list is not provided! Check your arguments" << std::endl;
        exit(1);
    }

    //_______________________________________________________________________________
    // --tree
    if (result.count("tree"))
    {
        ana.input_tree_name = result["tree"].as<std::string>();
    }
    else
    {
        std::cout << options.help() << std::endl;
        std::cout << "ERROR: Input tree name is not provided! Check your arguments" << std::endl;
        exit(1);
    }

    //_______________________________________________________________________________
    // --debug
    if (result.count("debug"))
    {
        ana.output_tfile = new TFile("debug.root", "recreate");
    }
    else
    {
        //_______________________________________________________________________________
        // --output
        if (result.count("output"))
        {
            ana.output_tfile = new TFile(result["output"].as<std::string>().c_str(), "create");
            if (not ana.output_tfile->IsOpen())
            {
                std::cout << options.help() << std::endl;
                std::cout << "ERROR: output already exists! provide new output name or delete old file. OUTPUTFILE=" << result["output"].as<std::string>() << std::endl;
                exit(1);
            }
        }
        else
        {
            std::cout << options.help() << std::endl;
            std::cout << "ERROR: Output file name is not provided! Check your arguments" << std::endl;
            exit(1);
        }
    }

    //_______________________________________________________________________________
    // --nevents
    ana.n_events = result["nevents"].as<int>();

    //
    // Printing out the option settings overview
    //
    std::cout <<  "=========================================================" << std::endl;
    std::cout <<  " Setting of the analysis job based on provided arguments " << std::endl;
    std::cout <<  "---------------------------------------------------------" << std::endl;
    std::cout <<  " ana.input_file_list_tstring: " << ana.input_file_list_tstring <<  std::endl;
    std::cout <<  " ana.output_tfile: " << ana.output_tfile->GetName() <<  std::endl;
    std::cout <<  " ana.n_events: " << ana.n_events <<  std::endl;
    std::cout <<  "=========================================================" << std::endl;

//********************************************************************************
//
// 2. Opening input baby files
//
//********************************************************************************

    // Create the TChain that holds the TTree's of the baby ntuples
    ana.events_tchain = RooUtil::FileUtil::createTChain(ana.input_tree_name, ana.input_file_list_tstring);

    // Create a Looper object to loop over input files
    // the "www" object is defined in the wwwtree.h/cc
    // This is an instance which helps read variables in the WWW baby TTree
    // It is a giant wrapper that facilitates reading TBranch values.
    // e.g. if there is a TBranch named "lep_pt" which is a std::vector<float> then, one can access the branch via
    //
    //    std::vector<float> lep_pt = www.lep_pt();
    //
    // and no need for "SetBranchAddress" and declaring variable shenanigans necessary
    // This is a standard thing SNT does pretty much every looper we use
    ana.looper.init(ana.events_tchain, &trk, ana.n_events);

//********************************************************************************
//
// Interlude... notes on RooUtil framework
//
//********************************************************************************

    // ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
    // Quick tutorial on RooUtil::Cutflow object cut tree formation
    // ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
    //
    // The RooUtil::Cutflow object facilitates creating a tree structure of cuts
    //
    // To add cuts to each node of the tree with cuts defined, use "addCut" or "addCutToLastActiveCut"
    // The "addCut" or "addCutToLastActiveCut" accepts three argument, <name>, and two lambda's that define the cut selection, and the weight to apply to that cut stage
    //
    // e.g. To create following cut-tree structure one does
    //
    //             (Root) <--- Always exists as soon as RooUtil::Cutflow object is created. But this is basically hidden underneath and users do not have to care
    //                |
    //            CutWeight
    //            |       |
    //     CutPreSel1    CutPreSel2
    //       |                  |
    //     CutSel1           CutSel2
    //
    //
    //   code:
    //
    //      // Create the object (Root node is created as soon as the instance is created)
    //      RooUtil::Cutflow cutflow;
    //
    //      cutflow.addCut("CutWeight"                 , <lambda> , <lambda>); // CutWeight is added below "Root"-node
    //      cutflow.addCutToLastActiveCut("CutPresel1" , <lambda> , <lambda>); // The last "active" cut is "CutWeight" since I just added that. So "CutPresel1" is added below "CutWeight"
    //      cutflow.addCutToLastActiveCut("CutSel1"    , <lambda> , <lambda>); // The last "active" cut is "CutPresel1" since I just added that. So "CutSel1" is added below "CutPresel1"
    //
    //      cutflow.getCut("CutWeight"); // By "getting" the cut object, this makes the "CutWeight" the last "active" cut.
    //      cutflow.addCutToLastActiveCut("CutPresel2" , <lambda> , <lambda>); // The last "active" cut is "CutWeight" since I "getCut" on it. So "CutPresel2" is added below "CutWeight"
    //      cutflow.addCutToLastActiveCut("CutSel2"    , <lambda> , <lambda>); // The last "active" cut is "CutPresel2" since I just added that. So "CutSel2" is added below "CutPresel1"
    //
    // (Side note: "UNITY" lambda is defined in the framework to just return 1. This so that use don't have to type [&]() {return 1;} so many times.)
    //
    // Once the cutflow is formed, create cutflow histograms can be created by calling RooUtil::Cutflow::bookCutflows())
    // This function looks through the terminating nodes of the tree structured cut tree. and creates a histogram that will fill the yields
    // For the example above, there are two terminationg nodes, "CutSel1", and "CutSel2"
    // So in this case Root::Cutflow::bookCutflows() will create two histograms. (Actually four histograms.)
    //
    //  - TH1F* type object :  CutSel1_cutflow (4 bins, with first bin labeled "Root", second bin labeled "CutWeight", third bin labeled "CutPreSel1", fourth bin labeled "CutSel1")
    //  - TH1F* type object :  CutSel2_cutflow (...)
    //  - TH1F* type object :  CutSel1_rawcutflow (...)
    //  - TH1F* type object :  CutSel2_rawcutflow (...)
    //                                ^
    //                                |
    // NOTE: There is only one underscore "_" between <CutName>_cutflow or <CutName>_rawcutflow
    //
    // And later in the loop when RooUtil::Cutflow::fill() function is called, the tree structure will be traversed through and the appropriate yields will be filled into the histograms
    //
    // After running the loop check for the histograms in the output root file

    // ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
    // Quick tutorial on RooUtil::Histograms object
    // ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
    //
    // The RooUtil::Histograms object facilitates book keeping histogram definitions
    // And in conjunction with RooUtil::Cutflow object, one can book same histograms across different cut stages easily without copy pasting codes many times by hand.
    //
    // The histogram addition happens in two steps.
    // 1. Defining histograms
    // 2. Booking histograms to cuts
    //
    // Histograms are defined via following functions
    //
    //      RooUtil::Histograms::addHistogram       : Typical 1D histogram (TH1F*) "Fill()" called once per event
    //      RooUtil::Histograms::addVecHistogram    : Typical 1D histogram (TH1F*) "Fill()" called multiple times per event
    //      RooUtil::Histograms::add2DHistogram     : Typical 2D histogram (TH2F*) "Fill()" called once per event
    //      RooUtil::Histograms::add2DVecHistogram  : Typical 2D histogram (TH2F*) "Fill()" called multiple times per event
    // e.g.
    //
    //    RooUtil::Histograms histograms;
    //    histograms.addHistogram   ("MllSS"    , 180 , 0. , 300. , [&]() { return www.MllSS()  ; }); // The lambda returns float
    //    histograms.addVecHistogram("AllLepPt" , 180 , 0. , 300. , [&]() { return www.lep_pt() ; }); // The lambda returns vector<float>
    //
    // The addVecHistogram will have lambda to return vector<float> and it will loop over the values and call TH1F::Fill() for each item
    //
    // To book histograms to cuts one uses
    //
    //      RooUtil::Cutflow::bookHistogramsForCut()
    //      RooUtil::Cutflow::bookHistogramsForCutAndBelow()
    //      RooUtil::Cutflow::bookHistogramsForCutAndAbove()
    //      RooUtil::Cutflow::bookHistogramsForEndCuts()
    //
    // e.g. Given a tree like the following, we can book histograms to various cuts as we want
    //
    //              Root
    //                |
    //            CutWeight
    //            |       |
    //     CutPreSel1    CutPreSel2
    //       |                  |
    //     CutSel1           CutSel2
    //
    // For example,
    //
    //    1. book a set of histograms to one cut:
    //
    //       cutflow.bookHistogramsForCut(histograms, "CutPreSel2")
    //
    //    2. book a set of histograms to a cut and below
    //
    //       cutflow.bookHistogramsForCutAndBelow(histograms, "CutWeight") // will book a set of histograms to CutWeight, CutPreSel1, CutPreSel2, CutSel1, and CutSel2
    //
    //    3. book a set of histograms to a cut and above (... useless...?)
    //
    //       cutflow.bookHistogramsForCutAndAbove(histograms, "CutPreSel2") // will book a set of histograms to CutPreSel2, CutWeight (nothing happens to Root node)
    //
    //    4. book a set of histograms to a terminating nodes
    //
    //       cutflow.bookHistogramsForEndCuts(histograms) // will book a set of histograms to CutSel1 and CutSel2
    //
    // The naming convention of the booked histograms are as follows
    //
    //   cutflow.bookHistogramsForCut(histograms, "CutSel1");
    //
    //  - TH1F* type object : CutSel1__MllSS;
    //  - TH1F* type object : CutSel1__AllLepPt;
    //                               ^^
    //                               ||
    // NOTE: There are two underscores "__" between <CutName>__<HistogramName>
    //
    // And later in the loop when RooUtil::CutName::fill() function is called, the tree structure will be traversed through and the appropriate histograms will be filled with appropriate variables
    // After running the loop check for the histograms in the output root file

    // Set the cutflow object output file
    ana.cutflow.setTFile(ana.output_tfile);

    ana.cutflow.addCut("DiElChannel", [&]() { return 1/*set your cut here*/; }, [&]() { return 1/*set your weight here*/; } );
    ana.cutflow.addCut("DiMuChannel", [&]() { return 1/*set your cut here*/; }, [&]() { return 1/*set your weight here*/; } );

    ana.cutflow.getCut("DiElChannel");
    ana.cutflow.addCutToLastActiveCut("DiElChannelCutA", [&]() { return 1/*set your cut here*/; }, [&]() { return 1/*set your weight here*/; } );
    ana.cutflow.addCutToLastActiveCut("DiElChannelCutB", [&]() { return 1/*set your cut here*/; }, [&]() { return 1/*set your weight here*/; } );
    ana.cutflow.addCutToLastActiveCut("DiElChannelCutC", [&]() { return 1/*set your cut here*/; }, [&]() { return 1/*set your weight here*/; } );

    ana.cutflow.getCut("DiMuChannel");
    ana.cutflow.addCutToLastActiveCut("DiMuChannelCutA", [&]() { return 1/*set your cut here*/; }, [&]() { return 1/*set your weight here*/; } );
    ana.cutflow.addCutToLastActiveCut("DiMuChannelCutB", [&]() { return 1/*set your cut here*/; }, [&]() { return 1/*set your weight here*/; } );
    ana.cutflow.addCutToLastActiveCut("DiMuChannelCutC", [&]() { return 1/*set your cut here*/; }, [&]() { return 1/*set your weight here*/; } );

    // Print cut structure
    ana.cutflow.printCuts();

    // Histogram utility object that is used to define the histograms
    ana.histograms.addHistogram("Mll", 180, 0, 250, [&]() { return 1/* set your variable here*/; } );

    // // event id
    // // event, lumi, run

    // // Beam spot
    // float    bsp_x_;
    // float    bsp_y_;
    // float    bsp_z_;
    // float    bsp_sigmax_;
    // float    bsp_sigmay_;
    // float    bsp_sigmaz_;

    // // Seed
    // vector<float> *see_px_;
    // vector<float> *see_py_;
    // vector<float> *see_pz_;
    // vector<float> *see_pt_;
    // vector<float> *see_eta_;
    // vector<float> *see_phi_;
    // vector<float> *see_dxy_;
    // vector<float> *see_dz_;
    // vector<float> *see_ptErr_;
    // vector<float> *see_etaErr_;
    // vector<float> *see_phiErr_;
    // vector<float> *see_dxyErr_;
    // vector<float> *see_dzErr_;
    // vector<float> *see_chi2_;
    // vector<float> *see_statePt_;
    // vector<float> *see_stateTrajX_;
    // vector<float> *see_stateTrajY_;
    // vector<float> *see_stateTrajPx_;
    // vector<float> *see_stateTrajPy_;
    // vector<float> *see_stateTrajPz_;
    // vector<float> *see_stateTrajGlbX_;
    // vector<float> *see_stateTrajGlbY_;
    // vector<float> *see_stateTrajGlbZ_;
    // vector<float> *see_stateTrajGlbPx_;
    // vector<float> *see_stateTrajGlbPy_;
    // vector<float> *see_stateTrajGlbPz_;
    // vector<float> *see_stateCcov00_;
    // vector<float> *see_stateCcov01_;
    // vector<float> *see_stateCcov02_;
    // vector<float> *see_stateCcov03_;
    // vector<float> *see_stateCcov04_;
    // vector<float> *see_stateCcov05_;
    // vector<float> *see_stateCcov11_;
    // vector<float> *see_stateCcov12_;
    // vector<float> *see_stateCcov13_;
    // vector<float> *see_stateCcov14_;
    // vector<float> *see_stateCcov15_;
    // vector<float> *see_stateCcov22_;
    // vector<float> *see_stateCcov23_;
    // vector<float> *see_stateCcov24_;
    // vector<float> *see_stateCcov25_;
    // vector<float> *see_stateCcov33_;
    // vector<float> *see_stateCcov34_;
    // vector<float> *see_stateCcov35_;
    // vector<float> *see_stateCcov44_;
    // vector<float> *see_stateCcov45_;
    // vector<float> *see_stateCcov55_;
    // vector<int> *see_q_;
    // vector<unsigned int> *see_nValid_;
    // vector<unsigned int> *see_nPixel_;
    // vector<unsigned int> *see_nGlued_;
    // vector<unsigned int> *see_nStrip_;
    // vector<unsigned int> *see_nPhase2OT_;
    // vector<unsigned int> *see_nCluster_;
    // vector<unsigned int> *see_algo_;
    // vector<int> *see_trkIdx_;
    // vector<int> *see_bestSimTrkIdx_;
    // vector<int> *see_bestFromFirstHitSimTrkIdx_;
    // vector<float> *see_bestSimTrkShareFrac_;
    // vector<float> *see_bestFromFirstHitSimTrkShareFrac_;
    // vector<unsigned int> *see_offset_;
    // vector<vector<float> > *see_simTrkShareFrac_;
    // vector<vector<int> > *see_hitIdx_;
    // vector<vector<int> > *see_hitType_;
    // vector<vector<int> > *see_simTrkIdx_;

    // // Reco Tracks
    // vector<float> *trk_px_;
    // vector<float> *trk_py_;
    // vector<float> *trk_pz_;
    // vector<float> *trk_pt_;
    // vector<float> *trk_inner_px_;
    // vector<float> *trk_inner_py_;
    // vector<float> *trk_inner_pz_;
    // vector<float> *trk_inner_pt_;
    // vector<float> *trk_outer_px_;
    // vector<float> *trk_outer_py_;
    // vector<float> *trk_outer_pz_;
    // vector<float> *trk_outer_pt_;
    // vector<float> *trk_eta_;
    // vector<float> *trk_lambda_;
    // vector<float> *trk_cotTheta_;
    // vector<float> *trk_phi_;
    // vector<float> *trk_dxy_;
    // vector<float> *trk_dz_;
    // vector<float> *trk_dxyPV_;
    // vector<float> *trk_dzPV_;
    // vector<float> *trk_dxyClosestPV_;
    // vector<float> *trk_dzClosestPV_;
    // vector<float> *trk_ptErr_;
    // vector<float> *trk_etaErr_;
    // vector<float> *trk_lambdaErr_;
    // vector<float> *trk_phiErr_;
    // vector<float> *trk_dxyErr_;
    // vector<float> *trk_dzErr_;
    // vector<float> *trk_refpoint_x_;
    // vector<float> *trk_refpoint_y_;
    // vector<float> *trk_refpoint_z_;
    // vector<float> *trk_nChi2_;
    // vector<float> *trk_nChi2_1Dmod_;
    // vector<float> *trk_ndof_;
    // vector<float> *trk_mva_;
    // vector<int> *trk_q_;
    // vector<unsigned int> *trk_nValid_;
    // vector<unsigned int> *trk_nLost_;
    // vector<unsigned int> *trk_nInactive_;
    // vector<unsigned int> *trk_nPixel_;
    // vector<unsigned int> *trk_nStrip_;
    // vector<unsigned int> *trk_nOuterLost_;
    // vector<unsigned int> *trk_nInnerLost_;
    // vector<unsigned int> *trk_nOuterInactive_;
    // vector<unsigned int> *trk_nInnerInactive_;
    // vector<unsigned int> *trk_nPixelLay_;
    // vector<unsigned int> *trk_nStripLay_;
    // vector<unsigned int> *trk_n3DLay_;
    // vector<unsigned int> *trk_nLostLay_;
    // vector<unsigned int> *trk_nCluster_;
    // vector<unsigned int> *trk_algo_;
    // vector<unsigned int> *trk_originalAlgo_;
    // vector<int> *trk_seedIdx_;
    // vector<int> *trk_vtxIdx_;
    // vector<int> *trk_bestSimTrkIdx_;
    // vector<int> *trk_bestFromFirstHitSimTrkIdx_;
    // vector<float> *trk_bestSimTrkShareFrac_;
    // vector<float> *trk_bestSimTrkShareFracSimDenom_;
    // vector<float> *trk_bestSimTrkShareFracSimClusterDenom_;
    // vector<float> *trk_bestSimTrkNChi2_;
    // vector<float> *trk_bestFromFirstHitSimTrkShareFrac_;
    // vector<float> *trk_bestFromFirstHitSimTrkShareFracSimDenom_;
    // vector<float> *trk_bestFromFirstHitSimTrkShareFracSimClusterDenom_;
    // vector<float> *trk_bestFromFirstHitSimTrkNChi2_;
    // vector<vector<float> > *trk_simTrkNChi2_;
    // vector<vector<float> > *trk_simTrkShareFrac_;
    // vector<vector<int> > *trk_hitIdx_;
    // vector<vector<int> > *trk_hitType_;
    // vector<vector<int> > *trk_simTrkIdx_;

    // // Pixel hits?
    // vector<unsigned int> *pix_detId_;
    // vector<float> *pix_x_;
    // vector<float> *pix_y_;
    // vector<float> *pix_z_;
    // vector<float> *pix_xx_;
    // vector<float> *pix_xy_;
    // vector<float> *pix_yy_;
    // vector<float> *pix_yz_;
    // vector<float> *pix_zz_;
    // vector<float> *pix_zx_;
    // vector<float> *pix_radL_;
    // vector<float> *pix_bbxi_;
    // vector<vector<float> > *pix_chargeFraction_;
    // vector<vector<float> > *pix_xySignificance_;
    // vector<vector<int> > *pix_seeIdx_;
    // vector<vector<int> > *pix_simHitIdx_;
    // vector<vector<int> > *pix_trkIdx_;

    // // Phase 2 Hits?
    // vector<unsigned int> *ph2_detId_;
    // vector<float> *ph2_x_;
    // vector<float> *ph2_y_;
    // vector<float> *ph2_z_;
    // vector<float> *ph2_xx_;
    // vector<float> *ph2_xy_;
    // vector<float> *ph2_yy_;
    // vector<float> *ph2_yz_;
    // vector<float> *ph2_zz_;
    // vector<float> *ph2_zx_;
    // vector<float> *ph2_radL_;
    // vector<float> *ph2_bbxi_;
    // vector<vector<float> > *ph2_xySignificance_;
    // vector<vector<int> > *ph2_seeIdx_;
    // vector<vector<int> > *ph2_simHitIdx_;
    // vector<vector<int> > *ph2_trkIdx_;

    // // Reco Vertices
    // vector<float> *vtx_x_;
    // vector<float> *vtx_y_;
    // vector<float> *vtx_z_;
    // vector<float> *vtx_xErr_;
    // vector<float> *vtx_yErr_;
    // vector<float> *vtx_zErr_;
    // vector<float> *vtx_ndof_;
    // vector<float> *vtx_chi2_;
    // vector<vector<int> > *vtx_trkIdx_;

    // // Sim Tracks
    // vector<int> *sim_event_;
    // vector<int> *sim_bunchCrossing_;
    // vector<int> *sim_pdgId_;
    // vector<int> *sim_isFromBHadron_;
    // vector<float> *sim_px_;
    // vector<float> *sim_py_;
    // vector<float> *sim_pz_;
    // vector<float> *sim_pt_;
    // vector<float> *sim_eta_;
    // vector<float> *sim_phi_;
    // vector<float> *sim_pca_pt_;
    // vector<float> *sim_pca_eta_;
    // vector<float> *sim_pca_lambda_;
    // vector<float> *sim_pca_cotTheta_;
    // vector<float> *sim_pca_phi_;
    // vector<float> *sim_pca_dxy_;
    // vector<float> *sim_pca_dz_;
    // vector<int> *sim_q_;
    // vector<unsigned int> *sim_nValid_;
    // vector<unsigned int> *sim_nPixel_;
    // vector<unsigned int> *sim_nStrip_;
    // vector<unsigned int> *sim_nLay_;
    // vector<unsigned int> *sim_nPixelLay_;
    // vector<unsigned int> *sim_n3DLay_;
    // vector<unsigned int> *sim_nTrackerHits_;
    // vector<unsigned int> *sim_nRecoClusters_;
    // vector<int> *sim_parentVtxIdx_;
    // vector<vector<float> > *sim_trkShareFrac_;
    // vector<vector<int> > *sim_decayVtxIdx_;
    // vector<vector<int> > *sim_genPdgIds_;
    // vector<vector<int> > *sim_seedIdx_;
    // vector<vector<int> > *sim_simHitIdx_;
    // vector<vector<int> > *sim_trkIdx_;

    // // Sim Hits
    // vector<unsigned int> *simhit_detId_;
    // vector<float> *simhit_x_;
    // vector<float> *simhit_y_;
    // vector<float> *simhit_z_;
    // vector<float> *simhit_px_;
    // vector<float> *simhit_py_;
    // vector<float> *simhit_pz_;
    // vector<int> *simhit_particle_;
    // vector<float> *simhit_eloss_;
    // vector<float> *simhit_tof_;
    // vector<int> *simhit_simTrkIdx_;
    // vector<vector<int> > *simhit_hitIdx_;
    // vector<vector<int> > *simhit_hitType_;

    // // Sim Verticies
    // vector<int> *simvtx_event_;
    // vector<int> *simvtx_bunchCrossing_;
    // vector<unsigned int> *simvtx_processType_;
    // vector<float> *simvtx_x_;
    // vector<float> *simvtx_y_;
    // vector<float> *simvtx_z_;
    // vector<vector<int> > *simvtx_daughterSimIdx_;
    // vector<vector<int> > *simvtx_sourceSimIdx_;

    // // Sim primary vertices index..?
    // vector<int> *simpv_idx_;

    // // inputs
    // vector<unsigned int> *inv_detId_;


    // Book cutflows
    ana.cutflow.bookCutflows();

    // Book Histograms
    ana.cutflow.bookHistogramsForCutAndBelow(ana.histograms, "DiElChannel");
    ana.cutflow.bookHistogramsForCutAndBelow(ana.histograms, "DiMuChannel");
    // cutflow.bookHistograms(ana.histograms); // if just want to book everywhere

    // Looping input file
    while (ana.looper.nextEvent())
    {
        //Do what you need to do in for each event here
        //To save use the following function
        ana.cutflow.fill();
    }

    // Writing output file
    ana.cutflow.saveOutput();

    // The below can be sometimes crucial
    delete ana.output_tfile;
}

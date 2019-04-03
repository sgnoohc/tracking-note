#include "trktree.h"
#include "rooutil.h"
#include "cxxopts.h"

#include "SDL/Event.h" // SDL::Event
#include "SDL/PrintUtil.h" // SDL::out

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
    // ana.cutflow.addCut("CutWeight", UNITY, UNITY);

    // Print cut structure
    ana.cutflow.printCuts();

    // Histogram utility object that is used to define the histograms
    ana.histograms.addHistogram   ("sim_n"    , 30,     0   ,     30    , [&]() { return trk.sim_event().size()                                                             ; } );
    ana.histograms.addVecHistogram("sim_pdgId",  9,     0   ,      9    , [&]()
            {
                std::vector<float> rtn;

                for (auto& id : trk.sim_pdgId())
                {
                    if      (abs(id) ==   11) rtn.push_back(0); // electrons
                    else if (abs(id) ==   13) rtn.push_back(1); // muons
                    else if (abs(id) == 2212) rtn.push_back(2); // protons
                    else if (abs(id) ==  211) rtn.push_back(3); // pions
                    else if (abs(id) ==  321) rtn.push_back(4); // kaons
                    else if (abs(id) ==  310) rtn.push_back(5); // k-short
                    else if (abs(id) ==   22) rtn.push_back(6); // photons..?
                    else if (abs(id) ==  111) rtn.push_back(7); // pi zero..?
                    else                      rtn.push_back(8); // uncategorized
                }
                return rtn;
            } );
    ana.histograms.addHistogram("sim_n_muon", 20,     0   ,     20    , [&]()
            {
                float nmuon = 0;

                for (auto& id : trk.sim_pdgId())
                {
                    if (abs(id) == 13)
                        nmuon++;
                }
                return nmuon;
            } );
    ana.histograms.addVecHistogram("sim_mu_pt", 180,     0   ,     250    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pt().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_pt()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_mu_eta", 180,    -5   ,       5    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_eta().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_eta()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_mu_phi", 180,    -3.1416 ,    3.1416    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_phi().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_phi()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_pt", 180,     0   ,     250    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pt().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13)
                    {
                        rtn.push_back(trk.sim_pt()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_eta", 180,    -5   ,       5    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_eta().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13)
                    {
                        rtn.push_back(trk.sim_eta()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_phi", 180,    -3.1416 ,    3.1416    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_phi().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13)
                    {
                        rtn.push_back(trk.sim_phi()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_mu_pca_pt", 180,     0   ,     250    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_pt().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_pca_pt()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_mu_pca_eta", 180,    -5   ,       5    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_eta().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_pca_eta()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_mu_pca_phi", 180,    -3.1416 ,    3.1416    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_phi().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_pca_phi()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_mu_pca_lambda", 180,    -3.1416 ,    3.1416    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_lambda().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_pca_lambda()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_mu_pca_cotTheta", 180,    -3.1416 ,    3.1416    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_cotTheta().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_pca_cotTheta()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_mu_pca_dxy", 180,    -2.5    ,    2.5       , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_dxy().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_pca_dxy()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_mu_pca_dz", 180,    -5.0    ,    5.0       , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_dxy().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) == 13)
                    {
                        rtn.push_back(trk.sim_pca_dxy()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_pca_pt", 180,     0   ,     250    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_pt().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13 and trk.sim_pca_pt()[ip] > 0)
                    {
                        rtn.push_back(trk.sim_pca_pt()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_pca_eta", 180,    -5   ,       5    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_eta().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13 and trk.sim_pca_pt()[ip] > 0)
                    {
                        rtn.push_back(trk.sim_pca_eta()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_pca_phi", 180,    -3.1416 ,    3.1416    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_phi().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13 and trk.sim_pca_pt()[ip] > 0)
                    {
                        rtn.push_back(trk.sim_pca_phi()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_pca_lambda", 180,    -3.1416 ,    3.1416    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_lambda().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13 and trk.sim_pca_pt()[ip] > 0)
                    {
                        rtn.push_back(trk.sim_pca_lambda()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_pca_cotTheta", 180,    -3.1416 ,    3.1416    , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_cotTheta().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13 and trk.sim_pca_pt()[ip] > 0)
                    {
                        rtn.push_back(trk.sim_pca_cotTheta()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_pca_dxy", 180,    -2.5    ,    2.5       , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_dxy().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13 and trk.sim_pca_pt()[ip] > 0)
                    {
                        rtn.push_back(trk.sim_pca_dxy()[ip]);
                    }
                }
                return rtn;
            } );
    ana.histograms.addVecHistogram("sim_nonmu_pca_dz", 180,    -5.0    ,    5.0       , [&]()
            {
                std::vector<float> rtn;

                for (unsigned int ip = 0; ip < trk.sim_pca_dxy().size(); ++ip)
                {
                    int id = trk.sim_pdgId()[ip];
                    if (abs(id) != 13 and trk.sim_pca_pt()[ip] > 0)
                    {
                        rtn.push_back(trk.sim_pca_dxy()[ip]);
                    }
                }
                return rtn;
            } );

    ana.histograms.addHistogram("bsp_x", 180, -5.0 , 5.0 , [&]() { return trk.bsp_x(); } );
    ana.histograms.addHistogram("bsp_y", 180, -5.0 , 5.0 , [&]() { return trk.bsp_y(); } );
    ana.histograms.addHistogram("bsp_z", 180, -5.0 , 5.0 , [&]() { return trk.bsp_z(); } );
    ana.histograms.addHistogram("bsp_sigmax", 180, -5.0 , 5.0 , [&]() { return trk.bsp_sigmax(); } );
    ana.histograms.addHistogram("bsp_sigmay", 180, -5.0 , 5.0 , [&]() { return trk.bsp_sigmay(); } );
    ana.histograms.addHistogram("bsp_sigmaz", 180, -5.0 , 5.0 , [&]() { return trk.bsp_sigmaz(); } );

    ana.histograms.addVecHistogram("muon_simhit_x" , 1080 , -150 , 150 , [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_x().size(); ++ih)
                {
                    if (abs(trk.simhit_particle()[ih]) == 13)
                        rtn.push_back(trk.simhit_x()[ih]);
                }
                return rtn;
            });
    ana.histograms.addVecHistogram("muon_simhit_y" , 1080 , -150 , 150 , [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_y().size(); ++ih)
                {
                    if (abs(trk.simhit_particle()[ih]) == 13)
                        rtn.push_back(trk.simhit_y()[ih]);
                }
                return rtn;
            });
    ana.histograms.addVecHistogram("muon_simhit_r" , 1080 , -150 , 150 , [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_x().size(); ++ih)
                {
                    if (abs(trk.simhit_particle()[ih]) == 13)
                        rtn.push_back(sqrt(trk.simhit_x()[ih]*trk.simhit_x()[ih] + trk.simhit_y()[ih]*trk.simhit_y()[ih]));
                }
                return rtn;
            });
    ana.histograms.addVecHistogram("muon_simhit_z" , 1080 , -300 , 300 , [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_z().size(); ++ih)
                {
                    if (abs(trk.simhit_particle()[ih]) == 13)
                        rtn.push_back(trk.simhit_z()[ih]);
                }
                return rtn;
            });

    ana.histograms.add2DVecHistogram("muon_simhit_z", 1080, 0, 300, "muon_simhit_r", 1080, 0, 300,
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_z().size(); ++ih)
                {
                    rtn.push_back(fabs(trk.simhit_z()[ih]));
                }
                return rtn;
            },
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_x().size(); ++ih)
                {
                    rtn.push_back(sqrt(trk.simhit_x()[ih]*trk.simhit_x()[ih] + trk.simhit_y()[ih]*trk.simhit_y()[ih]));
                }
                return rtn;
            }
            );

    ana.histograms.add2DVecHistogram("muon_simhit_x", 1080, -120, 120, "muon_simhit_y", 1080, -120, 120,
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_x().size(); ++ih)
                {
                    if (fabs(trk.simhit_z()[ih] - 175) < 5)
                    if (abs(trk.simhit_particle()[ih]) == 13)
                        rtn.push_back(trk.simhit_x()[ih]);
                }
                return rtn;
            },
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_z().size(); ++ih)
                {
                    if (fabs(trk.simhit_z()[ih] - 175) < 5)
                    if (abs(trk.simhit_particle()[ih]) == 13)
                        rtn.push_back(trk.simhit_y()[ih]);
                }
                return rtn;
            }
            );

    ana.histograms.add2DVecHistogram("all_simhit_z", 1080, 0, 300, "all_simhit_r", 1080, 0, 300,
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_z().size(); ++ih)
                {
                    rtn.push_back(fabs(trk.simhit_z()[ih]));
                }
                return rtn;
            },
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_x().size(); ++ih)
                {
                    rtn.push_back(sqrt(trk.simhit_x()[ih]*trk.simhit_x()[ih] + trk.simhit_y()[ih]*trk.simhit_y()[ih]));
                }
                return rtn;
            }
            );

    ana.histograms.add2DVecHistogram("all_simhit_x", 1080, -120, 120, "all_simhit_y", 1080, -120, 120,
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_x().size(); ++ih)
                {
                    if (fabs(trk.simhit_z()[ih] - 175) < 5)
                        rtn.push_back(trk.simhit_x()[ih]);
                }
                return rtn;
            },
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.simhit_z().size(); ++ih)
                {
                    if (fabs(trk.simhit_z()[ih] - 175) < 5)
                        rtn.push_back(trk.simhit_y()[ih]);
                }
                return rtn;
            }
            );

    ana.histograms.addVecHistogram("simhit_particle"        , 5         , 0 ,       5       , [&]() { std::vector<float> rtn(trk.simhit_particle().begin()  , trk.simhit_particle().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_simTrkIdx"       , 80        , 0 ,       80      , [&]() { std::vector<float> rtn(trk.simhit_simTrkIdx().begin() , trk.simhit_simTrkIdx().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_process"         , 350       , 0 ,       350     , [&]() { std::vector<float> rtn(trk.simhit_process().begin()   , trk.simhit_process().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_detId"           , 5         , 0 ,       5       , [&]() { std::vector<float> rtn(trk.simhit_detId().begin()     , trk.simhit_detId().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_blade"           , 20        , 0 ,       20      , [&]() { std::vector<float> rtn(trk.simhit_blade().begin()     , trk.simhit_blade().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_isLower"         , 2         , 0 ,       2       , [&]() { std::vector<float> rtn(trk.simhit_isLower().begin()   , trk.simhit_isLower().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_isStack"         , 2         , 0 ,       2       , [&]() { std::vector<float> rtn(trk.simhit_isStack().begin()   , trk.simhit_isStack().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_isUpper"         , 2         , 0 ,       2       , [&]() { std::vector<float> rtn(trk.simhit_isUpper().begin()   , trk.simhit_isUpper().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_ladder"          , 40        , 0 ,       40      , [&]() { std::vector<float> rtn(trk.simhit_ladder().begin()    , trk.simhit_ladder().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_layer"           , 20        , 0 ,       20      , [&]() { std::vector<float> rtn(trk.simhit_layer().begin()     , trk.simhit_layer().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_module"          , 80        , 0 ,       80      , [&]() { std::vector<float> rtn(trk.simhit_module().begin()    , trk.simhit_module().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_order"           , 2         , 0 ,       2       , [&]() { std::vector<float> rtn(trk.simhit_order().begin()     , trk.simhit_order().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_panel"           , 5         , 0 ,       5       , [&]() { std::vector<float> rtn(trk.simhit_panel().begin()     , trk.simhit_panel().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_ring"            , 20        , 0 ,       20      , [&]() { std::vector<float> rtn(trk.simhit_ring().begin()      , trk.simhit_ring().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_rod"             , 80        , 0 ,       80      , [&]() { std::vector<float> rtn(trk.simhit_rod().begin()       , trk.simhit_rod().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_side"            , 5         , 0 ,       5       , [&]() { std::vector<float> rtn(trk.simhit_side().begin()      , trk.simhit_side().end()); return rtn; } );
    ana.histograms.addVecHistogram("simhit_subdet"          , 10        , 0 ,       10      , [&]() { std::vector<float> rtn(trk.simhit_subdet().begin()    , trk.simhit_subdet().end()); return rtn; } );

    ana.histograms.add2DVecHistogram("all_ph2_z", 1080, 0, 300, "all_ph2_r", 1080, 0, 300,
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.ph2_z().size(); ++ih)
                {
                    rtn.push_back(fabs(trk.ph2_z()[ih]));
                }
                return rtn;
            },
            [&]()
            {
                std::vector<float> rtn;
                for (unsigned int ih = 0; ih < trk.ph2_x().size(); ++ih)
                {
                    rtn.push_back(sqrt(trk.ph2_x()[ih]*trk.ph2_x()[ih] + trk.ph2_y()[ih]*trk.ph2_y()[ih]));
                }
                return rtn;
            }
            );


// vector<float>         &simhit_eloss     ();
// vector<float>         &simhit_px        ();
// vector<float>         &simhit_py        ();
// vector<float>         &simhit_pz        ();
// vector<float>         &simhit_tof       ();
// vector<float>         &simhit_x         ();
// vector<float>         &simhit_y         ();
// vector<float>         &simhit_z         ();
// vector<int>           &simhit_particle  ();
// vector<int>           &simhit_simTrkIdx ();
// vector<short>         &simhit_process   ();
// vector<unsignedint>   &simhit_detId     ();
// vector<unsignedshort> &simhit_blade     ();
// vector<unsignedshort> &simhit_isLower   ();
// vector<unsignedshort> &simhit_isStack   ();
// vector<unsignedshort> &simhit_isUpper   ();
// vector<unsignedshort> &simhit_ladder    ();
// vector<unsignedshort> &simhit_layer     ();
// vector<unsignedshort> &simhit_module    ();
// vector<unsignedshort> &simhit_order     ();
// vector<unsignedshort> &simhit_panel     ();
// vector<unsignedshort> &simhit_ring      ();
// vector<unsignedshort> &simhit_rod       ();
// vector<unsignedshort> &simhit_side      ();
// vector<unsignedshort> &simhit_subdet    ();

    // ana.histograms.addVecHistogram("see_px", 180, -5, 5, [&]() { return trk.see_px(); } );
    // ana.histograms.addVecHistogram("see_py", 180, -5, 5, [&]() { return trk.see_py(); } );
    // ana.histograms.addVecHistogram("see_pz", 180, -5, 5, [&]() { return trk.see_pz(); } );
    // ana.histograms.addVecHistogram("see_pt", 180, -5, 5, [&]() { return trk.see_pt(); } );
    // ana.histograms.addVecHistogram("see_eta", 180, -5, 5, [&]() { return trk.see_eta(); } );
    // ana.histograms.addVecHistogram("see_phi", 180, -5, 5, [&]() { return trk.see_phi(); } );
    // ana.histograms.addVecHistogram("see_dxy", 180, -5, 5, [&]() { return trk.see_dxy(); } );
    // ana.histograms.addVecHistogram("see_dz", 180, -5, 5, [&]() { return trk.see_dz(); } );
    // ana.histograms.addVecHistogram("see_ptErr", 180, -5, 5, [&]() { return trk.see_ptErr(); } );
    // ana.histograms.addVecHistogram("see_etaErr", 180, -5, 5, [&]() { return trk.see_etaErr(); } );
    // ana.histograms.addVecHistogram("see_phiErr", 180, -5, 5, [&]() { return trk.see_phiErr(); } );
    // ana.histograms.addVecHistogram("see_dxyErr", 180, -5, 5, [&]() { return trk.see_dxyErr(); } );
    // ana.histograms.addVecHistogram("see_dzErr", 180, -5, 5, [&]() { return trk.see_dzErr(); } );
    // ana.histograms.addVecHistogram("see_chi2", 180, -5, 5, [&]() { return trk.see_chi2(); } );
    // ana.histograms.addVecHistogram("see_statePt", 180, -5, 5, [&]() { return trk.see_statePt(); } );
    // ana.histograms.addVecHistogram("see_stateTrajX", 180, -5, 5, [&]() { return trk.see_stateTrajX(); } );
    // ana.histograms.addVecHistogram("see_stateTrajY", 180, -5, 5, [&]() { return trk.see_stateTrajY(); } );
    // ana.histograms.addVecHistogram("see_stateTrajPx", 180, -5, 5, [&]() { return trk.see_stateTrajPx(); } );
    // ana.histograms.addVecHistogram("see_stateTrajPy", 180, -5, 5, [&]() { return trk.see_stateTrajPy(); } );
    // ana.histograms.addVecHistogram("see_stateTrajPz", 180, -5, 5, [&]() { return trk.see_stateTrajPz(); } );
    // ana.histograms.addVecHistogram("see_stateTrajGlbX", 180, -5, 5, [&]() { return trk.see_stateTrajGlbX(); } );
    // ana.histograms.addVecHistogram("see_stateTrajGlbY", 180, -5, 5, [&]() { return trk.see_stateTrajGlbY(); } );
    // ana.histograms.addVecHistogram("see_stateTrajGlbZ", 180, -5, 5, [&]() { return trk.see_stateTrajGlbZ(); } );
    // ana.histograms.addVecHistogram("see_stateTrajGlbPx", 180, -5, 5, [&]() { return trk.see_stateTrajGlbPx(); } );
    // ana.histograms.addVecHistogram("see_stateTrajGlbPy", 180, -5, 5, [&]() { return trk.see_stateTrajGlbPy(); } );
    // ana.histograms.addVecHistogram("see_stateTrajGlbPz", 180, -5, 5, [&]() { return trk.see_stateTrajGlbPz(); } );
    // ana.histograms.addVecHistogram("see_stateCcov00", 180, -5, 5, [&]() { return trk.see_stateCcov00(); } );
    // ana.histograms.addVecHistogram("see_stateCcov01", 180, -5, 5, [&]() { return trk.see_stateCcov01(); } );
    // ana.histograms.addVecHistogram("see_stateCcov02", 180, -5, 5, [&]() { return trk.see_stateCcov02(); } );
    // ana.histograms.addVecHistogram("see_stateCcov03", 180, -5, 5, [&]() { return trk.see_stateCcov03(); } );
    // ana.histograms.addVecHistogram("see_stateCcov04", 180, -5, 5, [&]() { return trk.see_stateCcov04(); } );
    // ana.histograms.addVecHistogram("see_stateCcov05", 180, -5, 5, [&]() { return trk.see_stateCcov05(); } );
    // ana.histograms.addVecHistogram("see_stateCcov11", 180, -5, 5, [&]() { return trk.see_stateCcov11(); } );
    // ana.histograms.addVecHistogram("see_stateCcov12", 180, -5, 5, [&]() { return trk.see_stateCcov12(); } );
    // ana.histograms.addVecHistogram("see_stateCcov13", 180, -5, 5, [&]() { return trk.see_stateCcov13(); } );
    // ana.histograms.addVecHistogram("see_stateCcov14", 180, -5, 5, [&]() { return trk.see_stateCcov14(); } );
    // ana.histograms.addVecHistogram("see_stateCcov15", 180, -5, 5, [&]() { return trk.see_stateCcov15(); } );
    // ana.histograms.addVecHistogram("see_stateCcov22", 180, -5, 5, [&]() { return trk.see_stateCcov22(); } );
    // ana.histograms.addVecHistogram("see_stateCcov23", 180, -5, 5, [&]() { return trk.see_stateCcov23(); } );
    // ana.histograms.addVecHistogram("see_stateCcov24", 180, -5, 5, [&]() { return trk.see_stateCcov24(); } );
    // ana.histograms.addVecHistogram("see_stateCcov25", 180, -5, 5, [&]() { return trk.see_stateCcov25(); } );
    // ana.histograms.addVecHistogram("see_stateCcov33", 180, -5, 5, [&]() { return trk.see_stateCcov33(); } );
    // ana.histograms.addVecHistogram("see_stateCcov34", 180, -5, 5, [&]() { return trk.see_stateCcov34(); } );
    // ana.histograms.addVecHistogram("see_stateCcov35", 180, -5, 5, [&]() { return trk.see_stateCcov35(); } );
    // ana.histograms.addVecHistogram("see_stateCcov44", 180, -5, 5, [&]() { return trk.see_stateCcov44(); } );
    // ana.histograms.addVecHistogram("see_stateCcov45", 180, -5, 5, [&]() { return trk.see_stateCcov45(); } );
    // ana.histograms.addVecHistogram("see_stateCcov55", 180, -5, 5, [&]() { return trk.see_stateCcov55(); } );

    // ana.histograms.addVecHistogram("trk_px", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_px().begin(), trk.trk_px().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_py", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_py().begin(), trk.trk_py().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_pz", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_pz().begin(), trk.trk_pz().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_pt", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_pt().begin(), trk.trk_pt().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_inner_px", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_inner_px().begin(), trk.trk_inner_px().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_inner_py", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_inner_py().begin(), trk.trk_inner_py().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_inner_pz", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_inner_pz().begin(), trk.trk_inner_pz().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_inner_pt", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_inner_pt().begin(), trk.trk_inner_pt().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_outer_px", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_outer_px().begin(), trk.trk_outer_px().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_outer_py", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_outer_py().begin(), trk.trk_outer_py().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_outer_pz", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_outer_pz().begin(), trk.trk_outer_pz().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_outer_pt", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_outer_pt().begin(), trk.trk_outer_pt().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_eta", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_eta().begin(), trk.trk_eta().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_lambda", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_lambda().begin(), trk.trk_lambda().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_cotTheta", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_cotTheta().begin(), trk.trk_cotTheta().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_phi", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_phi().begin(), trk.trk_phi().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_dxy", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_dxy().begin(), trk.trk_dxy().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_dz", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_dz().begin(), trk.trk_dz().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_dxyPV", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_dxyPV().begin(), trk.trk_dxyPV().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_dzPV", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_dzPV().begin(), trk.trk_dzPV().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_dxyClosestPV", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_dxyClosestPV().begin(), trk.trk_dxyClosestPV().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_dzClosestPV", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_dzClosestPV().begin(), trk.trk_dzClosestPV().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_ptErr", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_ptErr().begin(), trk.trk_ptErr().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_etaErr", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_etaErr().begin(), trk.trk_etaErr().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_lambdaErr", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_lambdaErr().begin(), trk.trk_lambdaErr().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_phiErr", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_phiErr().begin(), trk.trk_phiErr().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_dxyErr", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_dxyErr().begin(), trk.trk_dxyErr().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_dzErr", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_dzErr().begin(), trk.trk_dzErr().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_refpoint_x", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_refpoint_x().begin(), trk.trk_refpoint_x().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_refpoint_y", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_refpoint_y().begin(), trk.trk_refpoint_y().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_refpoint_z", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_refpoint_z().begin(), trk.trk_refpoint_z().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nChi2", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nChi2().begin(), trk.trk_nChi2().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nChi2_1Dmod", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nChi2_1Dmod().begin(), trk.trk_nChi2_1Dmod().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_ndof", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_ndof().begin(), trk.trk_ndof().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_mva", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_mva().begin(), trk.trk_mva().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_q", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_q().begin(), trk.trk_q().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nValid", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nValid().begin(), trk.trk_nValid().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nLost", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nLost().begin(), trk.trk_nLost().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nInactive", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nInactive().begin(), trk.trk_nInactive().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nPixel", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nPixel().begin(), trk.trk_nPixel().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nStrip", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nStrip().begin(), trk.trk_nStrip().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nOuterLost", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nOuterLost().begin(), trk.trk_nOuterLost().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nInnerLost", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nInnerLost().begin(), trk.trk_nInnerLost().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nOuterInactive", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nOuterInactive().begin(), trk.trk_nOuterInactive().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nInnerInactive", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nInnerInactive().begin(), trk.trk_nInnerInactive().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nPixelLay", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nPixelLay().begin(), trk.trk_nPixelLay().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nStripLay", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nStripLay().begin(), trk.trk_nStripLay().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_n3DLay", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_n3DLay().begin(), trk.trk_n3DLay().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nLostLay", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nLostLay().begin(), trk.trk_nLostLay().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_nCluster", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_nCluster().begin(), trk.trk_nCluster().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_algo", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_algo().begin(), trk.trk_algo().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_originalAlgo", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_originalAlgo().begin(), trk.trk_originalAlgo().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_seedIdx", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_seedIdx().begin(), trk.trk_seedIdx().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_vtxIdx", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_vtxIdx().begin(), trk.trk_vtxIdx().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestSimTrkIdx", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestSimTrkIdx().begin(), trk.trk_bestSimTrkIdx().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestFromFirstHitSimTrkIdx", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestFromFirstHitSimTrkIdx().begin(), trk.trk_bestFromFirstHitSimTrkIdx().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestSimTrkShareFrac", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestSimTrkShareFrac().begin(), trk.trk_bestSimTrkShareFrac().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestSimTrkShareFracSimDenom", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestSimTrkShareFracSimDenom().begin(), trk.trk_bestSimTrkShareFracSimDenom().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestSimTrkShareFracSimClusterDenom", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestSimTrkShareFracSimClusterDenom().begin(), trk.trk_bestSimTrkShareFracSimClusterDenom().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestSimTrkNChi2", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestSimTrkNChi2().begin(), trk.trk_bestSimTrkNChi2().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestFromFirstHitSimTrkShareFrac", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestFromFirstHitSimTrkShareFrac().begin(), trk.trk_bestFromFirstHitSimTrkShareFrac().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestFromFirstHitSimTrkShareFracSimDenom", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestFromFirstHitSimTrkShareFracSimDenom().begin(), trk.trk_bestFromFirstHitSimTrkShareFracSimDenom().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestFromFirstHitSimTrkShareFracSimClusterDenom", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestFromFirstHitSimTrkShareFracSimClusterDenom().begin(), trk.trk_bestFromFirstHitSimTrkShareFracSimClusterDenom().end()); return rtn; } );
    // ana.histograms.addVecHistogram("trk_bestFromFirstHitSimTrkNChi2", 180, -5, 5, [&]() { std::vector<float> rtn(trk.trk_bestFromFirstHitSimTrkNChi2().begin(), trk.trk_bestFromFirstHitSimTrkNChi2().end()); return rtn; } );

    // ana.histograms.addVecHistogram("pix_detId", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_detId().begin(), trk.pix_detId().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_x", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_x().begin(), trk.pix_x().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_y", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_y().begin(), trk.pix_y().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_z", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_z().begin(), trk.pix_z().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_xx", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_xx().begin(), trk.pix_xx().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_xy", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_xy().begin(), trk.pix_xy().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_yy", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_yy().begin(), trk.pix_yy().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_yz", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_yz().begin(), trk.pix_yz().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_zz", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_zz().begin(), trk.pix_zz().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_zx", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_zx().begin(), trk.pix_zx().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_radL", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_radL().begin(), trk.pix_radL().end()); return rtn; } );
    // ana.histograms.addVecHistogram("pix_bbxi", 180, -5, 5, [&]() { std::vector<float> rtn(trk.pix_bbxi().begin(), trk.pix_bbxi().end()); return rtn; } );

    // ana.histograms.addVecHistogram("ph2_detId", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_detId().begin(), trk.ph2_detId().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_x", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_x().begin(), trk.ph2_x().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_y", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_y().begin(), trk.ph2_y().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_z", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_z().begin(), trk.ph2_z().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_xx", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_xx().begin(), trk.ph2_xx().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_xy", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_xy().begin(), trk.ph2_xy().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_yy", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_yy().begin(), trk.ph2_yy().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_yz", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_yz().begin(), trk.ph2_yz().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_zz", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_zz().begin(), trk.ph2_zz().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_zx", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_zx().begin(), trk.ph2_zx().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_radL", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_radL().begin(), trk.ph2_radL().end()); return rtn; } );
    // ana.histograms.addVecHistogram("ph2_bbxi", 180, -5, 5, [&]() { std::vector<float> rtn(trk.ph2_bbxi().begin(), trk.ph2_bbxi().end()); return rtn; } );

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


    // vector<int> *sim_event_;
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
    // ana.cutflow.bookCutflows();

    // Book Histograms
    ana.cutflow.bookHistograms(ana.histograms); // if just want to book everywhere

    // Set extendable axes
    // ana.cutflow.setHistsAxesExtendable(); // Make histogram axes extendable

    // Looping input file
    while (ana.looper.nextEvent())
    {
        //Do what you need to do in for each event here
        //To save use the following function
        ana.cutflow.fill();

        // <--------------------------
        // <--------------------------
        // <--------------------------
        //
        // ***************************
        // Testing SDL Implementations
        // ***************************
        //

        // Main instance that will hold modules, hits, minidoublets, etc. (i.e. main data structure)
        SDL::Event event;

        // Adding hits to modules
        for (unsigned int ihit = 0; ihit < trk.ph2_x().size(); ++ihit)
        {
            // Takes two arguments, SDL::Hit, and detId
            // SDL::Event internally will structure whether we already have the module instance or we need to create a new one.
            event.addHitToModule(
                    // a hit
                    SDL::Hit(trk.ph2_x()[ihit], trk.ph2_y()[ihit], trk.ph2_z()[ihit]),
                    // add to module with "detId"
                    trk.ph2_detId()[ihit]
                    );
        }

        // Print content in the event
        // (SDL::cout is a modified version of std::cout where each line is prefixed by SDL::)
        SDL::cout << event;

        // <--------------------------
        // <--------------------------
        // <--------------------------
    }

    // Writing output file
    ana.cutflow.saveOutput();

    // The below can be sometimes crucial
    delete ana.output_tfile;
}

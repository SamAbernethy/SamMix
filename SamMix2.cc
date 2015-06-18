#include "SamMix2.h"
#include <iostream>
#include <string>
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"

ofstream fout("data.txt"); // output for asymmetry data

Int_t mix2()
{
    Int_t theta_bin; // which theta bin
    Int_t n_but_files; // number of files for Butanol
    Int_t n_carb_files; // number of files for Carbon
    Int_t carbonstart; // 3407 normally
    Int_t butanolstart; // 3680 normally

    ppi0 PolPar; // initialize PolPar, of class ppi0
    std::cout << "Initializing Carbon data... " << endl;
    std::cout << "Number of Files for Carbon data: ";
    std::cin >> n_carb_files;
    if ((!n_carb_files) || (n_carb_files == 0)) {
        std::cout << "Number of files incorrectly specified. Try again." << endl;
        return 1;
    } else {
        std::cout << "Number of files successfully specified." << endl;
    }

    std::cout << "Which theta bin to use: ";
    std::cin >> theta_bin;
    if ((!theta_bin) || (theta_bin == 0) || (theta_bin == 1)) {
        std::cout << "Theta bin incorrectly specified. Try again." << endl;
        return 1;
    } else {
        std::cout << "Theta bin successfully specified." << endl;
        PolPar.SetTheta_bin(theta_bin); // simply sets theta_bin in PolPar equal to this specified theta_bin
    }

    // INITIALIZING CARBON
    std::cout << "Carbon run number to start with: ";
    std::cin >> carbonstart;
    if (!carbonstart) {
        std::cout << "Carbon run number not specified. Try again." << endl;
        return 1;
    } else {
        std::cout << "Carbon run number successfully specified, as " << carbonstart << endl;
        PolPar.SetCarbonStart(carbonstart);
        PolPar.InitialCarbon();
    }

    // CARBON LOOP
    for (Int_t Carb_index = 1; Carb_index <= n_carb_files; Carb_index++) {
        PolPar.CarbonLoop(Carb_index);
    }
    std::cout << "Total Number of Carbon Background Entries Found: " << PolPar.GetCarbEntries() << endl;

    // BUTANOL
    std::cout << "************************************************ " << endl;
    std::cout << "Number of files for Butanol data: ";
    std::cin >> n_but_files;
    if ((!n_but_files) || (n_but_files == 0)) {
        std::cout << "Number of files for Butanol data incorrectly specified. Try again." << endl;
        return 1;
    } else {
        std::cout << "Number of files for Butanol data successfully specified." << endl;
        std::cout << "Butanol run number to start with: ";
        std::cin >> butanolstart;
        if (!butanolstart) {
            std::cout << "Butanol run number not specified. Try again." << endl;
            return 1;
        } else {
            std::cout << " " << endl;
            PolPar.SetButanolStart(butanolstart);
            std::cout << "Starting Butanol file input loop... " << endl;
            for (Int_t i = 0; i < n_but_files; i++) {
                PolPar.Asymmetry(i);
            }
            fout.close();
            PolPar.Graph();
            std::cout << "Success! You win." << endl;
        }
    }
}

ppi0 :: ppi0() {} // what does this really do?
ppi0 :: ~ppi0() {}

// *******************************************************************************************************
// *******************************************************************************************************

void ppi0 :: InitialCarbon()
{
    TString carbnumber = Form("%d", carbonstart);
    TString FirstSource = "/local/raid0/work/aberneth/a2GoAT/postreconApril/";
    FirstCarbonLocation = FirstSource + "pi0-samApril_CBTaggTAPS_" + carbnumber + ".root";
    ifstream Afile(FirstCarbonLocation);
    if (!Afile) {
        std::cout << "Initial Carbon file not found." << endl;
        return;
    }
    TFile FirstFile (FirstCarbonLocation); // call on the first root file as FirstFile
    std::cout << "First file found!" << endl;

    FirstFile.GetObject("Theta_1", C3500_1); // get Theta_1 from TH1 C3500_1
    FirstFile.GetObject("Theta_0", C3500_0); // get Theta_0 from TH1 C3500_0
    C3500_1->SetDirectory(0); // detaches this so that you can open and close root files without destroying it
    C3500_0->SetDirectory(0);

    // MM_pi0_n_2g_h1 and MM_pi0_n_2g_h0 need to exist in FirstFile
/*  FirstFile.GetObject("MM_pi0_n_2g_h1", C_MissMass_1);
    FirstFile.GetObject("MM_pi0_n_2g_h0", C_MissMass_0);
    C_MissMass_1->SetDirectory(0);
    C_MissMass_0->SetDirectory(0); */
}

// ******************************************************************************************************
// ******************************************************************************************************

// LOOPING CARBON
void ppi0 :: CarbonLoop(Int_t j)
{
    // Carbon data from acqu and Pi0 must exist in the following locations:
    TString AcqCarb_source = "/local/raid0/work/aberneth/a2GoAT/Apr2014/";
    TString Pi0Carb_source = "/local/raid0/work/aberneth/a2GoAT/postreconApril/";

    Int_t n_carb_run = carbonstart + j;
    TString carb_ext = Form("%d", n_carb_run);
    if ((n_carb_run == 3492) || (n_carb_run == 3494)) { return; }

    // acqu data must be called "Acqu_CBTaggTAPS_", while Pi0 data must be "pi0-samApril_CBTaggTAPS_"
    acqu_Carbon = AcqCarb_source + "Acqu_CBTaggTAPS_" + carb_ext + ".root";
    Pi0_Carbon = Pi0Carb_source + "pi0-samApril_CBTaggTAPS_" + carb_ext + ".root";

    std::cout << "Checking for Carbon file " << n_carb_run << "..." << endl;
    ifstream Bfile(acqu_Carbon);
    if (!Bfile) {
        std::cout << "Acqu Carbon files not found for file " << n_carb_run << endl;
        return;
    }
    ifstream Cfile(Pi0_Carbon);
    if (!Cfile) {
        std::cout << "Pi0 Carbon files not found for file " << n_carb_run << endl;
        return;
    }
    std::cout << "Found!" << endl;

    TFile Pi0Carb (Pi0_Carbon);
    TFile AcqCarb (acqu_Carbon);

    // Theta_1 and Theta_0 must exist in Pi0Carb
    Pi0Carb.GetObject("Theta_1", Carb_1); // get Theta_1 from Pi0Carb
    Pi0Carb.GetObject("Theta_0", Carb_0);
    Carb_1->SetDirectory(0); // detach histogram
    Carb_0->SetDirectory(0);

    C3500_1->Add(Carb_1,1); // add Carb_1 to the Carbon stack
    C3500_0->Add(Carb_0,1);

    AcqCarb.GetObject("trigger", AcqTree); // trigger (OR A BETTER TREE) must be in AcqCarb
    std::cout << "For run number " << n_carb_run << ", the number of Acqu Entries is: " << AcqTree -> GetEntries() << endl;
    std::cout << "Carbon bin content for helicity 1: " << C3500_1 -> GetBinContent(theta_bin) << endl;
    std::cout << "Carbon bin content for helicity 0: " << C3500_0 -> GetBinContent(theta_bin) << endl;
    CarbEvnt += AcqTree -> GetEntries();
}

// ************************************************************************************************************
// ************************************************************************************************************

// ASYMMETRY WITH BUTANOL DATA
void ppi0 :: Asymmetry(Int_t index)
{
    Int_t n_but_run = butanolstart + index;
    TString but_ext = Form("%d", n_but_run);
    if (n_but_run == 3699) { return; }

    // Butanol data must exist for acqu and Pi0 as specified:
    TString AcqBut_source = "/local/raid0/work/aberneth/a2GoAT/May2014/";
    TString Pi0But_source = "/local/raid0/work/aberneth/a2GoAT/postreconMay/";
    TString histogram_source = "/local/raid0/work/aberneth/a2GoAT/histograms/";

    // Pi0 data must have "pi0-samMay_CBTaggTAPS", while acqu data must have "Acqu_CBTaggTAPS_"
    Pi0_Butanol = Pi0But_source + "pi0-samMay_CBTaggTAPS_" + but_ext + ".root";
    acqu_Butanol = AcqBut_source + "Acqu_CBTaggTAPS_" + but_ext + ".root";

    std::cout << "Checking for Butanol file " << n_but_run << "..." << endl;
    ifstream Dfile(Pi0_Butanol);
    if (!Dfile) {
        std::cout << "Pi0 Butanol files not found for file " << n_but_run << endl;
        return;
    }
    ifstream Efile(acqu_Butanol);
    if (!Efile) {
        std::cout << "Acqu Butanol files not found for file " << n_but_run << endl;
        return;
    }
    std::cout << "Found!" << endl;

    TFile Pi0But(Pi0_Butanol);
    TFile AcqBut(acqu_Butanol);

    AcqBut.GetObject("trigger", Acqu_but); // trigger (OR A BETTER TREE) must be in AcqBut
    ButaEvnt = Acqu_but -> GetEntries();

    if(ButaEvnt < 4.0e+6) {
        std::cout << "Butanol event count is too low for file " << n_but_run << endl;
        return;
    }

    Pi0But.GetObject("Theta_1", BThet_1); // get Theta_1 from Pi0But
    Pi0But.GetObject("Theta_0", BThet_0);
    // Pi0But.GetObject("MM_pi0_n_2g_h1", B_MissMass_1);
    // Pi0But.GetObject("MM_pi0_n_2g_h0", B_MissMass_0);
    TFile hist(histogram_source + "histo" + but_ext + ".root", "RECREATE");
    std::cout << "Original butanol bin content for helicity 1: " << BThet_1 -> GetBinContent(theta_bin) << endl;
    std::cout << "Original butanol bin content for helicity 0: " << BThet_0 -> GetBinContent(theta_bin) << endl;
    std::cout << "Scale was: " << Scale() << endl;
    BThet_1 -> Add(C3500_1, (-0.001)*Scale());
    BThet_0 -> Add(C3500_0, (-0.001)*Scale());
    BThet_1 -> Write();
    BThet_0 -> Write();
    std::cout << "yield_1: " << BThet_1 -> GetBinContent(theta_bin) << endl;
    std::cout << "yield_0: " << BThet_0 -> GetBinContent(theta_bin) << endl;

    // MM_pi0_n_2g_h1 must exist in Pi0But
/*  B_MissMass_1 -> Add(C_MissMass_1, (-1)*Scale());
    B_MissMass_0 -> Add(C_MissMass_0, (-1)*Scale());
    B_MissMass_1 -> Write();
    B_MissMass_0 -> Write(); */

    yield_0 = BThet_0 -> GetBinContent(theta_bin);
    yield_1 = BThet_1 -> GetBinContent(theta_bin);
    yield_0_e = BThet_0 -> GetBinError(theta_bin);
    yield_1_e = BThet_1 -> GetBinError(theta_bin);

    /* if (yield_0 < 0) {
        std::cout << "Yield for helicity 0 was negative for file " << n_but_run << endl;
        // yield_0 = yield_0*(-1);
    }
    if (yield_1 < 0) {
         std::cout << "Yield for helicity 1 was negative for file " << n_but_run << endl;
        // yield_1 = yield_1*(-1);
    } */

    asym = (yield_1 - yield_0) / (yield_0 + yield_1);
    err = (2./(pow(yield_0 + yield_1, 2.)))*sqrt(pow(yield_0, 2.)*pow(yield_1_e, 2.) + pow(yield_1, 2.)*pow(yield_0_e, 2.));

   /* Optional output for each run:
    std::cout << "The run number was: " << n_but_run << endl;
    std::cout << "The number of Carbon entries was: " << CarbEvnt << endl;
    std::cout << "The number of Butanol entries was: " << ButaEvnt << endl;
    std::cout << "Therefore, the scale used was: " << Scale() << endl; */

    std::cout << "The data written is: " << n_but_run << " " << asym << " " << err << endl;
    std::cout << "********************************************" << endl;
    fout << n_but_run << " " << asym << " " << err << endl;
    hist.Close();
}

// *************************************************************************************
// *************************************************************************************

// GRAPHING
void ppi0 :: Graph()
{
    TCanvas *c1 = new TCanvas();
    c1->SetGrid();
    TGraphErrors data("data.txt", "%lg %lg %lg"); // graph the run number, asymmetry, and error
    data.SetTitle("Frozen Spin Target Polarization/Asymmetry ; Run Number; #Sigma P_{#gamma}");
    data.SetMarkerStyle(kCircle);
    data.SetFillColor(0);
    data.SetLineColor(56);
    data.DrawClone();
    c1->Print("MyGraph.png", "png");
    TFile f("data.root", "recreate");
    data.Write();
}

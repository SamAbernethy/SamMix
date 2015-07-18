#include "SamMix2.h"
#include <iostream>
#include <fstream>
#include <string>
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <math.h>

// polarization flips at 3994/4000
ofstream fout("data.txt"); // output for asymmetry data
ofstream fout2("PostRebinnedData.txt"); // output for rebinned data
ofstream fout3("YieldData.txt"); // output for yield data

Int_t mix2()
{
    Int_t theta_bin; // which theta bin
    Int_t n_but_files; // number of files for Butanol
    Int_t n_carb_files; // number of files for Carbon
    Int_t carbonstart; // 3407 normally
    Int_t butanolstart; // 3680 normally
    Int_t rebinnumber; // number of bins clumped together
    Double_t CarbonScalingFactor;

    ppi0 PolPar; // initialize PolPar, of class ppi0
    std::cout << "Which theta bin to use: ";
    std::cin >> theta_bin;
    if ((!theta_bin) || (theta_bin == 0) || (theta_bin == 1)) {
        std::cout << "Theta bin incorrectly specified. Try again." << endl;
        return 1;
    } else {
        std::cout << "Theta bin successfully specified." << endl;
        PolPar.SetTheta_bin(theta_bin); // simply sets theta_bin in PolPar equal to this specified theta_bin
    }

    std::cout << "Initializing Carbon data... " << endl;
    std::cout << "Number of Files for Carbon data: ";
    std::cin >> n_carb_files;
    if ((!n_carb_files) || (n_carb_files == 0)) {
        std::cout << "Number of files incorrectly specified. Try again." << endl;
        return 1;
    } else {
        std::cout << "Number of files successfully specified." << endl;
    }

    // INITIALIZING CARBON
    std::cout << "Carbon run number to start with: ";
    std::cin >> carbonstart;
    if (!carbonstart) {
        std::cout << "Carbon run number not specified. Try again." << endl;
        return 1;
    } else {
        std::cout << "Carbon run number successfully specified." << endl;
        PolPar.SetCarbonStart(carbonstart);
        PolPar.InitialCarbon();
    }

    // CARBON LOOP
    for (Int_t j = 1; j <= n_carb_files; j++) {
        PolPar.CarbonLoop(j);
    }
    std::cout << "Total Number of Carbon Background Entries Found: " << PolPar.GetCarbEntries() << endl;

    // BUTANOL
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
            PolPar.SetButanolStart(butanolstart);
            std::cout << "Set Carbon Background Scaling Factor: " << endl;
            std::cout << "(Note that -1 is full subtraction, 0 is none.) " << endl;
            std::cin >> CarbonScalingFactor;
            PolPar.SetCarbonScale(CarbonScalingFactor);
            std::cout << "Starting Butanol file input loop... " << endl;
            for (Int_t i = 0; i < n_but_files; i++) {
                PolPar.Asymmetry(i);
            }
            fout.close();
            fout3.close();
            PolPar.GraphIndividual();
            std::cout << "How many runs should be binned together?" << endl;
            std::cin >> rebinnumber;
            PolPar.SetRebinning(rebinnumber);
            PolPar.RebinData();
            fout2.close();
            PolPar.GraphRebinned();
            std::cout << "Success! You win." << endl;
        }
    }
}

ppi0 :: ppi0() {}
ppi0 :: ~ppi0() {}

// ************************************************************************************************
// ************************************************************************************************

void ppi0 :: InitialCarbon()
{
    TString carbnumber = Form("%d", carbonstart);
    TString FirstSource = "/local/raid0/work/aberneth/a2GoAT/CarbonPi0-sam/";
    FirstCarbonLocation = FirstSource + "pi0-samApril_CBTaggTAPS_" + carbnumber + ".root";
    ifstream Afile(FirstCarbonLocation);
    if (!Afile) {
        std::cout << "Initial Carbon file not found." << endl;
        return;
    }
    TFile FirstFile (FirstCarbonLocation); // call on the first root file as FirstFile
    std::cout << "First file found!" << endl;

    FirstFile.GetObject("Theta_1", CThet_1); // get Theta_1 from TH1 CThet_1
    FirstFile.GetObject("Theta_0", CThet_0); // get Theta_0 from TH1 CThet_0
    CThet_1 -> SetDirectory(0); // detaches this so that you can open and close root files without destroying it
    CThet_0 -> SetDirectory(0);

    FirstFile.GetObject("MM_pi0_n_2g_h1", C_MissMass_1);
    FirstFile.GetObject("MM_pi0_n_2g_h0", C_MissMass_0);
    C_MissMass_1 -> SetDirectory(0);
    C_MissMass_0 -> SetDirectory(0);

    TString FirstAcquLocation = "/local/raid0/work/aberneth/a2GoAT/Apr2014/Acqu_CBTaggTAPS" + carbnumber + ".root";
    TFile FirstAcqu(FirstAcquLocation);
    FirstAcqu.GetObject("tagger", First_carb);
    CarbEvnt = FirstAcqu -> GetEntries();
}

// ******************************************************************************************************
// ******************************************************************************************************

// LOOPING CARBON
void ppi0 :: CarbonLoop(Int_t j)
{
    // Carbon data from acqu and Pi0 must exist in the following locations:
    TString AcqCarb_source = "/local/raid0/work/aberneth/a2GoAT/Apr2014/";
    TString Pi0Carb_source = "/local/raid0/work/aberneth/a2GoAT/CarbonPi0-sam/";

    Int_t n_carb_run = carbonstart + j;
    TString carb_ext = Form("%d", n_carb_run);

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

    AcqCarb.GetObject("tagger", Acqu_carb);
    if (Acqu_carb -> GetEntries() > 3.0e+6) {
        Pi0Carb.GetObject("Theta_1", Carb_1); // get Theta_1 from Pi0Carb, etc
        Pi0Carb.GetObject("Theta_0", Carb_0);
        Pi0Carb.GetObject("MM_pi0_n_2g_h1", MM_1);
        Pi0Carb.GetObject("MM_pi0_n_2g_h0", MM_0);

        Carb_1 -> SetDirectory(0); // detach histograms
        Carb_0 -> SetDirectory(0);
        MM_1 -> SetDirectory(0);
        MM_0 -> SetDirectory(0);

        C_MissMass_1 -> Add(MM_1, 1);
        C_MissMass_0 -> Add(MM_0, 1);
        CThet_1 -> Add(Carb_1, 1); // add Carb_1 to the Carbon stack
        CThet_0 -> Add(Carb_0, 1);

        std::cout << "For run number " << n_carb_run << ", the number of Acqu Entries is: " << Acqu_carb -> GetEntries() << endl;
        std::cout << "Carbon bin content for helicity 1: " << CThet_1 -> GetBinContent(theta_bin) << endl;
        std::cout << "Carbon bin content for helicity 0: " << CThet_0 -> GetBinContent(theta_bin) << endl;
        CarbEvnt += Acqu_carb -> GetEntries();
    }
}

// *******************************************************************************************************
// *******************************************************************************************************

// ASYMMETRY WITH BUTANOL DATA
void ppi0 :: Asymmetry(Int_t index)
{
    Int_t n_but_run = butanolstart + index;
    TString but_ext = Form("%d", n_but_run);

    // Butanol data must exist for acqu and Pi0 as specified:
    TString AcqBut_source = "/local/raid0/work/aberneth/a2GoAT/May2014/";
    TString Pi0But_source = "/local/raid0/work/aberneth/a2GoAT/ButanolPi0-sam/";
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

    AcqBut.GetObject("tagger", Acqu_but);
    ButaEvnt = Acqu_but -> GetEntries();
    if (ButaEvnt > 4.0e+6) {
        Pi0But.GetObject("Theta_1", BThet_1); // get Theta_1 from Pi0But
        Pi0But.GetObject("Theta_0", BThet_0);

        Pi0But.GetObject("MM_pi0_n_2g_h1", B_MissMass_1);
        Pi0But.GetObject("MM_pi0_n_2g_h0", B_MissMass_0);
        TFile hist(histogram_source + "histo" + but_ext + ".root", "RECREATE");
        std::cout << "Original butanol bin content for helicity 1: " << BThet_1 -> GetBinContent(theta_bin) << endl;
        std::cout << "Original butanol bin content for helicity 0: " << BThet_0 -> GetBinContent(theta_bin) << endl;
        std::cout << "Scale was: " << Scale() << endl;
        BThet_1 -> Add(CThet_1, (CarbonScalingFactor)*Scale());
        BThet_0 -> Add(CThet_0, (CarbonScalingFactor)*Scale());
        BThet_1 -> Write();
        BThet_0 -> Write();
        std::cout << "yield_1: " << BThet_1 -> GetBinContent(theta_bin) << endl;
        std::cout << "yield_0: " << BThet_0 -> GetBinContent(theta_bin) << endl;

        B_MissMass_1 -> Add(C_MissMass_1, (CarbonScalingFactor)*Scale());
        B_MissMass_0 -> Add(C_MissMass_0, (CarbonScalingFactor)*Scale());
        B_MissMass_1 -> Write();
        B_MissMass_0 -> Write();

        yield_0 = BThet_0 -> GetBinContent(theta_bin);
        yield_1 = BThet_1 -> GetBinContent(theta_bin);
        yield_0_e = BThet_0 -> GetBinError(theta_bin);
        yield_1_e = BThet_1 -> GetBinError(theta_bin);

        if ((yield_0 < 100) || (yield_1 < 100)) {
            return;
        }

        asym = (yield_1 - yield_0) / (yield_0 + yield_1);
        err = (2./(pow(yield_0 + yield_1, 2.)))*sqrt(pow(yield_0, 2.)*pow(yield_1_e, 2.) + pow(yield_1, 2.)*pow(yield_0_e, 2.));

        std::cout << "The number of Carbon entries was: " << CarbEvnt << endl;
        std::cout << "The number of Butanol entries was: " << ButaEvnt << endl;
        std::cout << "Therefore, the scale used was: " << Scale() << endl;
        std::cout << "The data written is: " << n_but_run << " " << asym << " " << err << endl;
        std::cout << "********************************************" << endl;
        fout << n_but_run << " " << asym << " " << err << endl; // for graphing individual runs
        fout3 << n_but_run << " " << yield_1 << " " << yield_0 << " " << yield_1_e << " " << yield_0_e << endl; // for rebinning purposes
        hist.Close();
    }
}

// *************************************************************************************
// *************************************************************************************

// GRAPHING
void ppi0 :: GraphIndividual()
{
    TCanvas *c1 = new TCanvas();
    c1 -> cd();
    c1 -> SetGrid();
    TGraphErrors data("data.txt", "%lg %lg %lg"); // graph the run number, asymmetry, and error
    data.SetTitle("Individual Run Frozen Spin Target Polarization/Asymmetry ; Run Number; #Sigma_{2z} P_{T} P_{#gamma}");
    data.SetMarkerStyle(kCircle);
    data.SetFillColor(0);
    data.SetLineColor(56);
    data.DrawClone("AP*");
    c1 -> Print("MyGraph.png", "png");
    TFile f("data.root", "RECREATE");
    f.Write();
}

// ********************************************************************
// ********************************************************************

// REBINNING
void ppi0 :: RebinData() // very long variable names, but this can be changed later
{
    ifstream input;
    input.open("YieldData.txt");
    if (!input) {
        std::cout << "No input found!" << endl;
        return;
    }
    else {
        Int_t num = 1;
        Double_t runnumber[500], helicity1[500], helicity0[500], helicity1error[500], helicity0error[500];
        while (!input.eof()) {
            input >> runnumber[num] >> helicity1[num] >> helicity0[num] >> helicity1error[num] >> helicity0error[num];
            num++;
        }
    }
    std::cout << "Number of data points is: " << num << endl;
    std::cout << "Number of runs to bin together is: " << rebinnumber << endl;
    Int_t newdatapoint = ceil( (double) num/rebinnumber);
    const int newdatapoints = newdatapoint;
    std::cout << "Therefore, number of rebinned points is: " << (newdatapoints - 1) << endl;
    Double_t averagerunnumber[newdatapoints] = {0};
    Double_t asymmetry[newdatapoints] = {0};
    Double_t propagatederror[newdatapoints] = {0};
    Double_t sumofruns[newdatapoints] = {0};
    Double_t sumofyield1[newdatapoints] = {0};
    Double_t sumofyield0[newdatapoints] = {0};
    Double_t sumofyield1errorsquares[newdatapoints] = {0};
    Double_t sumofyield0errorsquares[newdatapoints] = {0};
    for ( Int_t k = 1; k < newdatapoints; k++ ) {
        for ( Int_t u = 1; u <= rebinnumber; u++ ) {
            sumofruns[k] += runnumber[u + (k-1)*rebinnumber];
            sumofyield1[k] += helicity1[u + (k-1)*rebinnumber];
            sumofyield0[k] += helicity0[u + (k-1)*rebinnumber];
            sumofyield1errorsquares[k] += helicity1error[u + (k-1)*rebinnumber] * helicity1error[u + (k-1)*rebinnumber];
            sumofyield0errorsquares[k] += helicity0error[u + (k-1)*rebinnumber] * helicity0error[u + (k-1)*rebinnumber];
        }
        averagerunnumber[k] = sumofruns[k] / rebinnumber;
        asymmetry[k] = (sumofyield1[k] - sumofyield0[k]) / (sumofyield0[k] + sumofyield1[k]);
        propagatederror[k] = (2./(pow(sumofyield0[k] + sumofyield1[k], 2.)))*sqrt(pow(sumofyield0[k], 2.)*pow(sumofyield1errorsquares[k], 1.) + pow(sumofyield1[k], 2.)*pow(sumofyield0errorsquares[k], 1.)) ;
        fout2 << averagerunnumber[k] << " " << asymmetry[k] << " " << propagatederror[k] << endl;
    }
}

void ppi0 :: GraphRebinned()
{
    TCanvas *c2 = new TCanvas;
    c2 -> cd();
    c2 -> SetGrid();
    TGraphErrors rebinned("PostRebinnedData.txt", "%lg %lg %lg");
    rebinned.SetTitle("Rebinned Frozen Spin Target Polarization/Asymmetry ; Run Number; #Sigma_{2z} P_{T} P_{#gamma}");
    rebinned.SetMarkerStyle(kCircle);
    rebinned.SetFillColor(0);
    rebinned.SetLineColor(56);
    rebinned.DrawClone("AP*");
    c2 -> Print("MyRebinnedGraph.png", "png");
    TFile f2("rebinneddata.root", "RECREATE");
    f2.Write();
}


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
ofstream fout4("ScaledData.txt");
ofstream fout5("ScaledData2.txt");
ofstream fout6("Helicity1+0.txt");

Int_t SamMix()
{
    Int_t theta_bin; // which theta bin
    Int_t n_but_files; // number of files for Butanol
    Int_t n_carb_files = 250; // number of files for Carbon, chosen as 250 since they're on my computer now
    Int_t carbonstart = 3407; // 3407 normally
    Int_t butanolstart; // 3680 normally
    Int_t rebinnumber; // number of bins clumped together
    Double_t CarbonScalingFactor; // -1 for full subtraction, 0 for none

    ppi0 PolPar; // initialize PolPar, of class ppi0
    std::cout << "Which theta bin to use: ";
    std::cin >> theta_bin;
    if ((!theta_bin) || (theta_bin == 0) || (theta_bin > 9)) {
        std::cout << "Theta bin incorrectly specified. Try again." << endl;
        return 1;
    } else {
        std::cout << "Theta bin successfully specified." << endl;
        PolPar.SetTheta_bin(theta_bin); // simply sets theta_bin in PolPar equal to this specified theta_bin
    }

    // INITIALIZING CARBON
    std::cout << "Initializing Carbon data... " << endl;
    PolPar.SetCarbonStart(carbonstart);
    PolPar.InitialCarbon();

    // CARBON LOOP
    for (Int_t j = 1; j <= n_carb_files; j++) {
        PolPar.CarbonLoop(j);
    }
    std::cout << "Total Number of Carbon Background Entries Found: " << PolPar.GetCarbEntries() << endl;
    std::cout << "Carbon bin content for helicity 1: " << PolPar.CThet_1 -> GetBinContent(theta_bin) << endl;
    std::cout << "Carbon bin content for helicity 0: " << PolPar.CThet_0 -> GetBinContent(theta_bin) << endl;

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
            PolPar.TheoreticalAsymmetry();
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
            // PolPar.HelicitiesVsSize();
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
    TString FirstSource = "/home/sam/work/histograms/CarbonPi0-sam/";
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

    CarbEvnt = 7.81607e+8;
}

// ******************************************************************************************************
// ******************************************************************************************************

// LOOPING CARBON
void ppi0 :: CarbonLoop(Int_t j)
{
    // Carbon data from acqu and Pi0 must exist in the following locations:
    TString Pi0Carb_source = "/home/sam/work/histograms/CarbonPi0-sam/";

    Int_t n_carb_run = carbonstart + j;
    TString carb_ext = Form("%d", n_carb_run);

    Pi0_Carbon = Pi0Carb_source + "pi0-samApril_CBTaggTAPS_" + carb_ext + ".root";

    ifstream Cfile(Pi0_Carbon);
    if (!Cfile) { return; }

    TFile Pi0Carb (Pi0_Carbon);
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
}

// *******************************************************************************************************
// *******************************************************************************************************

// ASYMMETRY WITH BUTANOL DATA
void ppi0 :: Asymmetry(Int_t index)
{
    Int_t n_but_run = butanolstart + index;
    TString but_ext = Form("%d", n_but_run);

    // Butanol data must exist for acqu and Pi0 as specified:
    TString Pi0But_source = "/home/sam/work/histograms/ButanolPi0-sam/";
    TString histogram_source = "/home/sam/work/histograms/subhistos/";

    // Pi0 data must have "pi0-samMay_CBTaggTAPS", while acqu data must have "Acqu_CBTaggTAPS_"
    Pi0_Butanol = Pi0But_source + "pi0-samMay_CBTaggTAPS_" + but_ext + ".root";

    ifstream Dfile(Pi0_Butanol);
    if (!Dfile) { return; }

    TFile Pi0But(Pi0_Butanol);

    ifstream butasize;
    butasize.open("SizeOfAcquBut.txt");
    if (!butasize) { return; }
    const Int_t m = 500;
    Double_t ButanolRuns[m] = {0};
    Double_t ButanolSizes[m] = {0};
    Int_t num = 1;
    while (!butasize.eof()) {
        butasize >> ButanolRuns[num] >> ButanolSizes[num];
        num++;
    }

    for (Int_t j = 1; j < 335; j++) {
        if (n_but_run == ButanolRuns[j]) {
            ButaEvnt = ButanolSizes[j];
            // std::cout << "Found the size." << endl;
        }
    }

    if (ButaEvnt > 4.0e+6) {
        Pi0But.GetObject("Theta_1", BThet_1); // get Theta_1 from Pi0But
        Pi0But.GetObject("Theta_0", BThet_0);

        Pi0But.GetObject("MM_pi0_n_2g_h1", B_MissMass_1);
        Pi0But.GetObject("MM_pi0_n_2g_h0", B_MissMass_0);
        TFile hist(histogram_source + "histo" + but_ext + ".root", "RECREATE");

        yield_0_e = BThet_0 -> GetBinError(theta_bin);
        yield_1_e = BThet_1 -> GetBinError(theta_bin);

        SecondScalingFactor = ((BThet_0 -> GetBinContent(theta_bin)) + (BThet_1 -> GetBinContent(theta_bin)) ) / (16000); // 16000 is an average chosen from graph
        if (SecondScalingFactor < 0.7) { return; } // cuts the weird ones with large size but small yields

        GraphARun();
        if (KeepOrRemove == 0) { return; }

        yield_0 = (BThet_0 -> GetBinContent(theta_bin)) + (SecondScalingFactor)*(CarbonScalingFactor)*Scale()*(CThet_0 -> GetBinContent(theta_bin));
        yield_1 = (BThet_1 -> GetBinContent(theta_bin)) + (SecondScalingFactor)*(CarbonScalingFactor)*Scale()*(CThet_1 -> GetBinContent(theta_bin));

        std::cout << n_but_run << "- Original butanol bin content (1/0): " << BThet_1 -> GetBinContent(theta_bin) << "/" << BThet_0 -> GetBinContent(theta_bin) << endl;
        std::cout << n_but_run << "- Post CB Subtraction bin content (1/0): " << yield_1 << "/" << yield_0 << endl;

        B_MissMass_1 -> Add(C_MissMass_1, (SecondScalingFactor)*(CarbonScalingFactor)*Scale());
        B_MissMass_0 -> Add(C_MissMass_0, (SecondScalingFactor)*(CarbonScalingFactor)*Scale());
        B_MissMass_1 -> Write();
        B_MissMass_0 -> Write();

        if ((yield_0 < 1000) || (yield_1 < 1000)) { return; }

        Double_t Pg = 0.692;
        Double_t Pg_error = 0.025;
        asym = (1 / Pg)*((yield_1 - yield_0) / (yield_0 + yield_1));
        err = sqrt(((asym/Pg)*(asym/Pg)*Pg_error*Pg_error) + ((2*yield_1)/(Pg*(yield_1+yield_0)*(yield_1+yield_0))) * ((2*yield_1)/(Pg*(yield_1+yield_0)*(yield_1+yield_0)))*yield_0_e*yield_0_e + ((2*yield_0)/(Pg*(yield_1+yield_0)*(yield_1+yield_0)))*((2*yield_0)/(Pg*(yield_1+yield_0)*(yield_1+yield_0)))*yield_1_e*yield_1_e);

        fout << n_but_run << " " << asym << " " << err << endl; // for graphing individual runs
        fout3 << n_but_run << " " << yield_1 << " " << yield_0 << " " << yield_1_e << " " << yield_0_e << endl; // for rebinning purposes
        fout6 << ((BThet_1 -> GetBinContent(theta_bin)) + (BThet_0 -> GetBinContent(theta_bin))) << endl;
        hist.Close();
    }
}

// *************************************************************************************
// *************************************************************************************

// GRAPHING
void ppi0 :: GraphIndividual()
{
    TCanvas *c1 = new TCanvas("c1", "Individual Canvas", 600, 400);
    c1 -> cd();
    c1 -> SetGrid();
    TGraphErrors data("data.txt", "%lg %lg %lg"); // graph the run number, asymmetry, and error
    data.SetTitle("Individual Run Frozen Spin Target Polarization/Asymmetry ; Run Number; #Sigma_{2z} P_{T}");
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
    std::cout << "P{T} (less accurate) or Sigma P_{T} (more accurate)? 1 for just PT" << endl;
    std::cin >> PTOrSigmaPT;
    Double_t ratio = 0;
    if (PTOrSigmaPT == 1) { ratio = 0.58; }
    if (PTOrSigmaPT == 0) { ratio = 1; }
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

    Double_t Pg = 0.692;
    Double_t Pg_error = 0.025;
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
        asymmetry[k] = (1 / (ratio*Pg))*(sumofyield1[k] - sumofyield0[k]) / (sumofyield0[k] + sumofyield1[k]);
        propagatederror[k] = sqrt(((asymmetry[k]/Pg)*(asymmetry[k]/Pg)*Pg_error*Pg_error) + ((2*sumofyield1[k])/(Pg*(sumofyield1[k]+sumofyield0[k])*(sumofyield1[k]+sumofyield0[k]))) * ((2*sumofyield1[k])/(Pg*(sumofyield1[k]+sumofyield0[k])*(sumofyield1[k]+sumofyield0[k])))*sumofyield0errorsquares[k] + ((2*sumofyield0[k])/(Pg*(sumofyield1[k]+sumofyield0[k])*(sumofyield1[k]+sumofyield0[k])))*((2*sumofyield0[k])/(Pg*(sumofyield1[k]+sumofyield0[k])*(sumofyield1[k]+sumofyield0[k])))*sumofyield1errorsquares[k]);
        fout2 << averagerunnumber[k] << " " << asymmetry[k] << " " << propagatederror[k] << endl;
    }
}

void ppi0 :: GraphRebinned()
{
    TCanvas *c2 = new TCanvas("c2", "Rebinned Canvas", 600, 400);
    c2 -> cd();
    c2 -> SetGrid();
    TGraphErrors rebinned("PostRebinnedData.txt", "%lg %lg %lg");
    // TAKING OUT SIGMA 2Z FROM LINE BELOW
    rebinned.SetTitle("Rebinned Frozen Spin Target Polarization/Asymmetry ; Run Number; P_{T}");
    rebinned.SetMarkerStyle(kCircle);
    rebinned.SetFillColor(0);
    rebinned.SetLineColor(56);
    rebinned.DrawClone("AP*");
    c2 -> Print("MyRebinnedGraph.png", "png");
    TFile f2("rebinneddata.root", "RECREATE");
    f2.Write();
}

void ppi0 :: TheoreticalAsymmetry()
{
    ifstream input;
    input.open("Pi0P_DMT_E_300.txt");
    if (!input) { return; }
    Int_t num = 1;
    Double_t angle[40], sigma[40], sigmascaled[40], sigmascaled2[40];
    while (!input.eof()) {
        input >> angle[num] >> sigma[num];
        num++;
    }

    Double_t scale;
    std::cout << "What Minimum Scaling Factor?" << endl;
    std::cin >> scale;

    for (Int_t q = 1; q < 38; q++) {
        sigmascaled[q] = sigma[q] * scale;
        fout4 << angle[q] << " " << sigmascaled[q] << endl;
    }

    Double_t scale2;
    std::cout << "What Maximum Scaling Factor?" << endl;
    std::cin >> scale2;

    for (Int_t r = 1; r < 38; r++) {
        sigmascaled2[r] = sigma[r] * scale2;
        fout5 << angle[r] << " " << sigmascaled2[r] << endl;
    }
    std::cout << "Positive (1) or Negative (-1) Polarization?" << endl;
    std::cin >> PositiveOrNegative;
}

void ppi0 :: GraphARun()
{
    const Int_t n = 10;
    Double_t runyield_1[n] = {0};
    Double_t runyield_0[n] = {0};
    Double_t runyield_1error[n] = {0};
    Double_t runyield_0error[n] = {0};
    Double_t runasymmetry[n] = {0};
    Double_t runerror[n] = {0};
    Double_t thetarange[n] = {0};
    Double_t theta_error[n] = {0};

    for (Int_t bin = 1; bin < n; bin++) {
        runyield_1[bin] = BThet_1 -> GetBinContent(bin) + (SecondScalingFactor)*(CarbonScalingFactor)*Scale()*(CThet_1 -> GetBinContent(bin));
        runyield_0[bin] = BThet_0 -> GetBinContent(bin) + (SecondScalingFactor)*(CarbonScalingFactor)*Scale()*(CThet_0 -> GetBinContent(bin));
        runyield_1error[bin] = BThet_1 -> GetBinError(bin);
        runyield_0error[bin] = BThet_0 -> GetBinError(bin);
        runasymmetry[bin] = PositiveOrNegative*(runyield_0[bin] - runyield_1[bin]) / (runyield_1[bin] + runyield_0[bin]);
        runerror[bin] =  (2./(pow(runyield_0[bin] + runyield_1[bin], 2.))) * sqrt(pow(runyield_0[bin], 2.)*pow(runyield_1error[bin], 2.) + pow(runyield_1[bin], 2.)*pow(runyield_0error[bin], 2.));
        thetarange[bin] = 20*bin - 10;
        theta_error[bin] = 10;
    }

    TCanvas *c3 = new TCanvas();
    c3 -> cd();
    c3 -> SetGrid();
    TGraph *data = new TGraph("ScaledData.txt","%lg %lg");
    TGraph *data2 = new TGraph("ScaledData2.txt","%lg %lg");
    TGraphErrors *data3 = new TGraphErrors(n, thetarange, runasymmetry, theta_error, runerror);
    data -> Fit("pol9");
    data -> SetLineColor(4);
    data -> GetFunction("pol9") -> SetLineColor(4);
    data2 -> Fit("pol9");
    data2 -> SetLineColor(3);
    data2 -> GetFunction("pol9") -> SetLineColor(3);
    TMultiGraph *mg = new TMultiGraph();
    mg -> Add(data);
    mg -> Add(data2);
    mg -> Add(data3);
    mg -> SetTitle("Cross Section Asymmetry ; #theta [degrees]; #Sigma_{2z} P_{T} P_{#gamma}");
    mg -> Draw("AP");
    leg = new TLegend(0.3, 0.7, 0.7, 0.9);
    leg -> AddEntry(data, "50% Polarization", "lep");
    leg -> AddEntry(data2, "65% Polarization", "lep");
    leg -> AddEntry(data3, "Experimental Data", "lep");
    leg -> Draw();
    c3 -> Print("TestRun.png", "png");

    std::cout << "Acceptable? 1 for yes, 0 for no." << endl;
    std::cin >> KeepOrRemove;
    // delete c3;
}

void ppi0 :: HelicitiesVsSize()
{
    ifstream sum;
    sum.open("Helicity1+0.txt");
    if (!sum) {
        std::cout << "Not found" << endl;
        return;
    }
    Int_t num = 1;
    const Int_t m = 500;
    Double_t SumOfBoth[m] = {0};
    while (!sum.eof()) {
        sum >> SumOfBoth[num];
        num++;
    }
    const Int_t mmm = num;

    ifstream butasize;
    butasize.open("SizeOfAcquBut.txt");
    if (!butasize) {
        std::cout << "Not found" << endl;
        return;
    }
    Double_t ButanolRuns[m] = {0};
    Double_t ButanolSizes[m] = {0};
    Int_t num2 = 1;
    while (!butasize.eof()) {
        butasize >> ButanolRuns[num2] >> ButanolSizes[num2];
        num2++;
    }
    const Int_t mm = num2;

    TCanvas *c4 = new TCanvas();
    c4 -> cd();
    c4 -> SetGrid();
    TGraph *gr = new TGraph(mmm, ButanolSizes, SumOfBoth);
    gr -> SetTitle("Size vs Yields ; ButanolSize ; Sum of Yields ");
    gr -> DrawClone("AP*");
    c4 -> Update();
/*
    TCanvas *c5 = new TCanvas();
    c5 -> cd();
    c5 -> SetGrid();
    TGraph *gr2 = new TGraph(mmm, ButanolRuns, ButanolSizes);
    gr2 -> SetTitle("Run Number vs Size ; Run Number ; ButanolSize ");
    gr2 -> DrawClone("AP*");
    c5 -> Update();

    TCanvas *c6 = new TCanvas();
    c6 -> cd();
    c6 -> SetGrid();
    TGraph *gr3 = new TGraph(mm, ButanolRuns, SumOfBoth);
    gr3 -> SetTitle("Run Number vs Yields ; Run Number ; Sum of Yields ");
    gr3 -> DrawClone("AP*");
    c6 -> Update(); */
}

#include <iostream>
#include <fstream>
#include <string>
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <math.h>

ofstream fout("ScaledData.txt"); // output of scaled txt file
ofstream fout2("ScaledData2.txt");
ofstream fout3("ExperimentalData.txt");

void graph()
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
        fout << angle[q] << " " << sigmascaled[q] << endl;
    }

    Double_t scale2;
    std::cout << "What Maximum Scaling Factor?" << endl;
    std::cin >> scale2;

    for (Int_t r = 1; r < 38; r++) {
        sigmascaled2[r] = sigma[r] * scale2;
        fout2 << angle[r] << " " << sigmascaled2[r] << endl;
    }

    // ****
    TString Location = "/local/raid0/work/aberneth/a2GoAT/ButanolPi0-sam/pi0-samMay_CBTaggTAPS_3407.root";
    TFile File(Location);
    Double_t ButaEvnt = Acqu_but -> GetEntries();
    TH1* BThet_1;
    TH1* BThet_0;

    if (ButaEvnt > 4.0e+6) {
        File.GetObject("Theta_1", BThet_1);
        File.GetObject("Theta_0", BThet_0);

        for (Int_t theta_bin = 1; theta_bin < 10; theta_bin++) {




        }
    }




    // ****


	TCanvas *c1 = new TCanvas();
    c1 -> cd();
	c1 -> SetGrid();
    TGraphErrors *data = new TGraphErrors("ScaledData.txt","%lg %lg %lg");
    TGraphErrors *data2 = new TGraphErrors("ScaledData2.txt","%lg %lg %lg");
    // another TGraphErrors data3 can be brought in here with the actual experimental data
    data -> Fit("pol9");
    data -> GetFunction("pol9") -> SetLineColor(4);
    data2 -> Fit("pol9");
    data2 -> GetFunction("pol9") -> SetLineColor(3);
    TMultiGraph *mg = new TMultiGraph();
    mg -> Add(data);
    mg -> Add(data2);
    // mg -> Add(data3);
    mg -> SetTitle("Cross Section Asymmetry ; #theta [degrees]; #Sigma_{2z} P_{T} P_{#gamma}");
    mg -> Draw("AP");
    c1 -> Update();
    c1 -> Print("new.png", "png");

    std::cout << "Since Photon Polarization is 0.692, this gives: " << endl;
    std::cout << "Minimum Target Polarization: " << scale / 0.692 << endl;
    std::cout << "Maximum Target Polarization: " << scale2 / 0.692 << endl;

}
	

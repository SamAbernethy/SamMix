#ifndef __SAMMIX2_h__
#define __SAMMIX2_h__

#include "TFile.h" // this will need to be run somewhere with TFile.h

class ppi0
{
private:

    TH1* C3500_0; // within Car_3500
    TH1* C3500_1; // within Car_3500
    TH1* Carb_0; // within Pi0Carb, which is found in Pi0_Carbon
    TH1* Carb_1; // within Pi0Carb, which is found in Pi0_Carbon
    TH1* BThet_0; // within Pi0But, which is found in Pi0_Butanol
    TH1* BThet_1; // within Pi0But, which is found in Pi0_Butanol
    TH1* C_MissMass_1; // within Car_3500, the first root file
    TH1* C_MissMass_0; // within Car_3500, the first root file
    TH1* B_MissMass_1; // within Pi0But, which is found in Pi0_Butanol
    TH1* B_MissMass_0; // within Pi0But, which is found in Pi0_Butanol

    TTree* AcqTree; // within acqu_Carbon, used to find CarbEvnt
    TTree* Acqu_but; // within acqu_Butanol, used to find ButEvnt

    Int_t theta_bin; // theta bin chosen
    Int_t n_but_files; // number of butanol files
    Int_t n_carb_files; // number of carbon files
    Int_t carbonstart; // run number to start at for carbon
    Int_t butanolstart; // run number to start at for butanol
    Int_t rebinnumber; // number of runs to bin together

    Double_t ButaEvnt; // number of butanol events
    Double_t CarbEvnt; // number of carbon events
    Double_t CarbEvnt1;
    Double_t CarbEvnt0;
    Double_t ButaEvnt1;
    Double_t ButaEvnt0;

    TString FirstCarbonLocation; // location of the first root file used for carbon
    TString Pi0_Carbon; // formerly Pi0file, where post-GoAT carbon data is found
    TString acqu_Carbon; // formerly acqu_run, where pre-GoAT carbon data is found
    TString Pi0_Butanol; // formerly but_run, where post-GoAT butanol data is found
    TString acqu_Butanol; // formerly acq_run, where pre-GoAT butanol data is found

    Double_t yield_0; // yield within a given theta bin for helicity 0
    Double_t yield_1; // yield within a given theta bin for helicity 1
    Double_t yield_0_e; // error on yield_0
    Double_t yield_1_e; // error on yield_1
    Double_t asym; // asymmetry from yields
    Double_t err; // error on asymmetry

public:
    ppi0(); // class constructor
    ~ppi0(); // class destructor
    void SetCarbonStart(Int_t i) { carbonstart = i; } // change this when I know more about pointers/private/public/etc
    void SetButanolStart(Int_t i) { butanolstart = i; }
    void SetTheta_bin(Int_t i) { theta_bin = i; } // simple void to assign theta_bin to theta_bin, archaic from Dylan
    void CarbonLoop(Int_t j); // void to loop through carbon data
    void Asymmetry(Int_t index); // void to calculate asymmetry from butanol data
    void GraphIndividual(); // graph it and write into root file
    void InitialCarbon(); // void to initialize one root file as stack base
    Double_t GetCarbEntries() { return CarbEvnt; } // self-explanatory
    Double_t Scale() { return ButaEvnt/CarbEvnt; } // Why this scaling from Dylan?
    void RebinData();
    void GraphRebinned();
    void SetRebinning(Int_t i) { rebinnumber = i; }
};

#endif // SAMMIX2_H

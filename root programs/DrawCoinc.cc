#include <Riostream.h>
#include <stdlib.h>
#include <TROOT.h>
#include <TSystem.h>
#include "TGeoManager.h"
#include "TNtuple.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TView.h"
#include "TF1.h"
#include "TGeometry.h"
#include "TClonesArray.h"
#include "TPolyLine3D.h"
#include "TPolyMarker3D.h"
#include "TGeoPhysicalNode.h"
#include "TParticle.h"
#include "TRandom2.h"
#include "TView3D.h"
#include "TLine.h"
#include "Math/ProbFuncMathCore.h"

#ifndef LABO
#define LABO
struct acqParams_t {
	UInt_t		acqTimestamp;
	UShort_t	dppVersion;
	UShort_t	nsPerSample;
	UShort_t	nsPerTimetag;
	UShort_t	numChannels;
	UShort_t	numSamples[32];
};

struct psdParams_t {
	UInt_t		channel;
	UInt_t		threshold;
	UInt_t		pretrigger;
	UInt_t		pregate;
	UInt_t		shortgate;
	UInt_t		longgate;
	UInt_t		numsamples;
};

struct acqEventPSD_t {
	ULong64_t	timetag;
	UInt_t		baseline;
	UShort_t	qshort;
	UShort_t	qlong;
	UShort_t	pur;
	UShort_t	samples[4096];
};
#endif

void DrawCoinc(const char *filename, int chA, int chB ) {
	
  TString st="ntupla-";
  st += filename;

  TFile *outfile = new TFile(st.Data(),"recreate");
  
  acqEventPSD_t inc_data1, inc_data2;
   
  TNtuple *nt = new TNtuple("nt","coincidences","eA:eB:qlongA:qlongB:timediff:timeA");

  TH1F *htime = new TH1F("htime","time difference in ns - 1 bin = 1 ADC clock",100,-200,200);

  TH1F *hqlongA = new TH1F("hqlongA","energy of detector A",1000,0,50000);
  TH1F *hqlongB = new TH1F("hqlongB","energy of detector B",1000,0,50000);

  TFile *incfile = new TFile(filename);
  TTree *inctree= (TTree*)incfile->Get("acq_tree_0");
  
  // aligned channels
  TBranch *incbranch_1, *incbranch_2;
  incbranch_1 = inctree->GetBranch(Form("acq_ch%i",chA));
  incbranch_1->SetAddress(&inc_data1);
  incbranch_2 = inctree->GetBranch(Form("acq_ch%i",chB));
  incbranch_2->SetAddress(&inc_data2);
  
  psdParams_t params;
  TBranch *config = inctree->GetBranch("psd_params");
  config->SetAddress(&params);
  config->GetEntry(0);
  
  long int eA = 0, eB = 0;
  int skipA = 0, skipB = 0, nGood = 0;
  long int timediff = 0;
  long int time0A = 0;
  
  int tot_events = incbranch_1->GetEntries();
  int tot_eventsB = incbranch_2->GetEntries();


  while (eA<(incbranch_1->GetEntries()) && eB<(incbranch_2->GetEntries())) {
    if (eA % (tot_events/100) == 0) {
      printf("Load %.0f percent in eA \r", (float)((eA*100.0)/tot_events));
      fflush(stdout);
    }
    

    incbranch_1->GetEntry(eA);
    incbranch_2->GetEntry(eB);
    
    // tempo 0 dell'acquisizione per il detector A
    if (!time0A) time0A = inc_data1.timetag;

    // differenza tempi nella scheda in adc clock
    timediff = inc_data1.timetag - inc_data2.timetag;
    
    if (TMath::Abs(timediff)>100) { // coincidenza a 100x4 = 400 ns
      // alignment
      if (timediff>0) { eB++; skipB++; } else { eA++; skipA++; }
    } 
    else {
      // c'e' la coincidenza...
      hqlongA->Fill(inc_data1.qlong);
      hqlongB->Fill(inc_data2.qlong);

      float tempo_ns_A = (inc_data1.timetag - time0A) * 4.0e9; // tempo detector A in s
      float timediff_ns = timediff*4.0;

      htime->Fill(timediff_ns);

      // migliorare la misura del tempo di riferimento (e quindi la risol. temporale):
      //           1) definendo una soglia variabile f(E)
      //           2) interpolando il punto di superamento con una retta
      //
      // ... TODO in caso di relazione...
          
      nt->Fill(eA, eB, inc_data1.qlong, inc_data2.qlong, timediff_ns, tempo_ns_A);
      
      eA++; eB++; nGood++;
      
    }
  }

  // alignment report
  cout << "Channels " << chA << " and " << chB << " aligned with " << nGood << " coincident events" << endl;
  cout << "Unmatched entries: " << skipA*100/(float)incbranch_1->GetEntries() << "% (chA), ";
  cout << skipB*100/(float)incbranch_2->GetEntries() << "% (chB)" << endl;
  
  incfile->Close();
  
  // salva gli oggetti di analisi
  outfile->cd();
  outfile->Write();
  outfile->Close(); // chiusura per completare la scrittura in modo corretto
 
  // riapre il file di analisi
  new TFile(st.Data());
   
}

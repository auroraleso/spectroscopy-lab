#include <Riostream.h>
#include <stdlib.h>
#include <TROOT.h>
#include <TSystem.h>
#include "TNtuple.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"

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

int AnaBraggDgtz(const char *filename, int ch,  int nsig=0) {

  if (ch<0 || ch>7) return -1;// ch is counts, to calibrate 
  
  acqParams_t params;
  acqEventPSD_t inc_data1;

  TFile *infile = new TFile(filename);
  TTree *tree   = (TTree*)infile->Get("acq_tree_0");// maybe change string

 // setup info
  TBranch *br_inc_setup = tree->GetBranch("acq_params");
  br_inc_setup->SetAddress(&params.acqTimestamp);
  br_inc_setup->GetEntry(0);
  
  printf("acqTimestamp = %d \n",params.acqTimestamp);
  printf("nsPerSample = %d \n",params.nsPerSample);
  printf("nsPerTimetag = %d \n",params.nsPerTimetag);
  printf("numChannels = %d \n",params.numChannels);
  printf("numSamples[%d] = %d \n",ch,params.numSamples[ch]);
  
  char st[100];
  sprintf(st,"acq_ch%d",ch);
  TBranch *branch1 = tree->GetBranch(st);
  if (!branch1) {printf("no branch!\n");return -1;}

  branch1->SetAddress(&inc_data1.timetag); 
  printf("Entries in data branch: %lld\n", branch1->GetEntries());

  int nev = branch1->GetEntries();

  // ANALIZZA EVENTO x EVENTO

  // altri parametri iniziali
  int blfrom = 0, blto = 600; // regione per il calcolo della baseline


  float vmax; // massimo relativo alla bl
  float bl;

  char outfilename[200];
  strcpy(outfilename,"anabragg_");
  const char *cc=strrchr(filename,'/');
  if (cc) {cc++; strcat(outfilename,cc);}
  else strcat(outfilename,filename);

  TFile *fout=new TFile(outfilename,"RECREATE"); // output file

  TNtuple *nt=new TNtuple("nt","","ev:vmax:baseline");

  int maxev=nev;
  if (nsig && nsig<nev) maxev=nsig;
  
  // LOOP SUGLI EVENTI
  for (int i=0; i<maxev; i++) {

    // recupera l'evento
    branch1->GetEntry(i);

    // inizializza a zero
    bl=0; 
    vmax=0;				     

    // calcolo baseline
    for (int j=blfrom; j<blto; j++) 
      bl += inc_data1.samples[j]; bl /= (blto-blfrom);    
    
    // calcolo vmax
    for (int j=600; j<3000; j++) {
      if ( (inc_data1.samples[j] - bl) > vmax ) vmax = (inc_data1.samples[j] - bl);
    }

    nt->Fill(i,vmax,bl);
  }
  std::cout << maxev << " events analyzed..." << std::endl;

  fout->Write();
  fout->Close();

  infile->Close();

  new TFile(outfilename); // riapre il file dei risultati

  return 0;
}

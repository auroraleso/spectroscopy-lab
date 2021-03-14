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

void plotScope(int ch, acqEventPSD_t &inc_data, acqParams_t &params, int entry){
  TCanvas *cps=NULL;
  char tit[50];
  sprintf(tit,"canvas_scope_ch%d",ch);
  if (gROOT->FindObject(tit)) cps=(TCanvas*)gROOT->FindObject(tit);
  else  cps = new TCanvas(tit,tit,0,0,640,480);
  
  TH1F *scope = NULL;
  sprintf(tit,"hscope_ch%d",ch);
  if (gROOT->FindObject(tit)) gROOT->FindObject(tit)->Delete();
  
  char tit2[200];
  sprintf(tit2,"entry %d - channel %d ",entry,ch);
  int nsamp = params.numSamples[ch];
  if (nsamp>4000) nsamp=4000;
  if (nsamp<=0) nsamp=1;
  scope = new TH1F(tit,tit2,nsamp,0,nsamp);
  scope->Reset();
  scope->SetStats(0);

  for(int j=0; j<nsamp; j++) 
    {scope->SetBinContent(j+1,inc_data.samples[j]);}	

  cps->cd();
  scope->Draw();
  cps->Update();
  gSystem->ProcessEvents();
}

void PlotSignalsDgtz(const char *filename, int ch, int plfrom=0, int plto=10) {
  
  if (ch<0 || ch>7) return;
  
  acqParams_t params;
  psdParams_t psdparams;
  acqEventPSD_t inc_data1;

  TFile *infile = new TFile(filename);
  TTree *tree   = (TTree*)infile->Get("acq_tree_0");
  if (!tree) {printf("no tree!\n");return;}

  // setup info
  TBranch *br_inc_setup = tree->GetBranch("acq_params");
  if (!br_inc_setup) {printf("no acq params!\n");return;}
  br_inc_setup->SetAddress(&params);
  br_inc_setup->GetEntry(0);  

  // channel data
  char st[100];
  sprintf(st,"acq_ch%d",ch);
  TBranch *branch1 = tree->GetBranch(st);
  if (!branch1) {printf("no branch!\n");return;}
  
  // channel info
  TBranch *br_inc_psd = tree->GetBranch("psd_params");
  int psdinfo = -1;
  if (br_inc_psd) {
    if ((int)br_inc_psd->GetEntries()<=ch) psdinfo=0;
    else psdinfo=ch;
    br_inc_psd->SetAddress(&psdparams);
    br_inc_psd->GetEntry(psdinfo);
  }
  
  // bufgix Bragg
  if (params.numSamples[ch]==0 && psdparams.threshold==9400) params.numSamples[ch]=4000;

  printf("acqTimestamp = %d \n",params.acqTimestamp);
  printf("nsPerSample = %d \n",params.nsPerSample);
  printf("nsPerTimetag = %d \n",params.nsPerTimetag);
  printf("numChannels = %d \n",params.numChannels);
  printf("   numSamples[%d] = %d \n",ch,params.numSamples[ch]);
  if (psdinfo>=0) {
    printf("   pretrigger[%d] = %d \n",ch, psdparams.pretrigger);
    printf("   threshold[%d] = %d \n",ch, psdparams.threshold);
  }

  branch1->SetAddress(&inc_data1); 
  if (branch1->GetEntries()<1) {printf("no entries in this channel!\n");return;}
  printf("Entries in branch1: %lld\n", branch1->GetEntries());
 
  for (int i=plfrom; i<plto; i++) {

    branch1->GetEntry(i);

    plotScope(ch, inc_data1, params ,i);

    gSystem->Sleep(500);

  }

}

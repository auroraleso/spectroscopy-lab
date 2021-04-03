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

struct bragg_signal {
  short int s[128];
};

int AnaBraggDani(const char *filename, int intto=128, float blfix=13, int nsig=0) {

  bragg_signal signal;

  TFile *fin=new TFile(filename);
  if (!fin->IsOpen()) {
    std::cout << "file not found! " << std::endl;
    return -1;
  }

  TTree *tree = (TTree*)fin->Get("bragg");
  if (!tree) {
    std::cout << "Bragg tree not found! " << std::endl;
    return -2;
  }

  TBranch *br = tree->GetBranch("signals");
  if (!br) {
    std::cout << "Signal branch not found! " << std::endl;
    return -3;
  }

  br->SetAddress(&signal);
  int nev = br->GetEntries();
  std::cout << "Number of events in file : " << nev << std::endl;

  // ANALIZZA EVENTO x EVENTO

  // altri parametri iniziali DA VERIFICARE ED EVENTUALMENTE MODIFICARE
  float thr_frac = 0.4; // soglia rispetto al vmax per il calcolo della larghezza
  int intfrom = 0;// regione di integrazione da 0 a intto
  if (intto>128) intto=128;
  int blfrom = 80, blto = 128; // regione per il calcolo della baseline


  float bl; // baseline evento x evento
  float integral; // integrale di carica
  float vmax; // massimo relativo alla bl
  float width; // larghezza temporale dei segnali
  float integral2; //integrale Lunardon
  float t; //tmax
  float width2; // width per la vdrift
  

  char outfilename[200];
  strcpy(outfilename,"anabraggDani_");
  const char *cc=strrchr(filename,'/');
  if (cc) {cc++; strcat(outfilename,cc);}
  else strcat(outfilename,filename);

  TFile *fout=new TFile(outfilename,"RECREATE"); // output file

  TNtuple *nt=new TNtuple("nt","","ev:vmax:integral:width:baseline:tmax:width2");

  int maxev=nev;
  if (nsig && nsig<nev) maxev=nsig;
  
  // LOOP SUGLI EVENTI
  for (int i=0; i<maxev; i++) {

    // recupera l'evento
    br->GetEntry(i);

    // inizializza a zero
    bl=0; 
    integral=0;
    vmax=0;				     
    width=0;
	integral2=0;
	t=0;
	width2=0;
	
	//calcolo della baseline
	for (int r=blfrom; r<blto; r++) 
		bl += signal.s[r]; bl /= (blto-blfrom);
    
    // calcolo integrali e vmax
    for (int j=intfrom; j<intto; j++) {
      integral += (signal.s[j] - bl);
      if ( (signal.s[j] - bl) > vmax ) vmax = (signal.s[j] - bl);
    }



	// calcolo della larghezza di segnale
	float perc = 0.3;
	float limit = vmax*perc;
	float V1a,V1b,V2a,V2b,t1a,t1b,t2a,t2b;
	for (int p=intfrom; p<intto;p++) {
        if(signal.s[p]>=(limit+bl)) {
           V1a=signal.s[p-1];
		   V1b=signal.s[p];
		   t1a = p-1;
		   t1b = p;
		   break;
        }
    }
	for (int p=t1b; p<intto;p++) {
        if(signal.s[p]<=(limit+bl)) {
           V2a=signal.s[p-1];
		   V2b=signal.s[p];
		   t2a=p-1;
		   t2b=p;
		   break;
        }
    }
	float a1,b1,a2,b2;
	a1=(t1a*V1b-t1b*V1a)/(t1a-t1b);
	b1=(V1a-V1b)/(t1a-t1b);
	a2=(t2a*V2b-t2b*V2a)/(t2a-t2b);
	b2=(V2a-V2b)/(t2a-t2b);	
	
	float w2,w1;
	w1=(limit-a1)/b1;
	w2=(limit-a2)/b2;
	width=w2-w1;
	
	    // calcolo integrali Lunardon
    for (int j=intfrom; j<intto; j++) {
      integral2 += (signal.s[j] - blfix);
    }
	
	// calcolo dei tmax
	for (int j=intfrom; j<intto; j++) {
      if ( (signal.s[j] - bl) == vmax ) t = j;
    }
	
	if ((integral2 >= 3200) && (integral2 <= 3700) && (width >= 52) && (width <= 60) && (vmax >= 84) && (vmax <= 100)){
		width2 = width;
	}
	else {
		width2 = NAN;
	}
	
    nt->Fill(i,vmax,integral2,width,bl,t,width2);
  }
  std::cout << maxev << " events analyzed..." << std::endl;

  fout->Write();
  fout->Close();

  fin->Close();

  new TFile(outfilename); // riapre il file dei risultati

  return 0;
}




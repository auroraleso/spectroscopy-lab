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

#ifndef LABNUC
#define LABNUC
struct bragg_signal {
  short int s[128];
};
#endif

int AnaBragg(const char *filename, int intto=128, float blfix=13, int nsig=0) {

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
int integerTo=128; //to change, integer to integerTo

  // altri parametri iniziali DA VERIFICARE ED EVENTUALMENTE MODIFICARE
  float thr_frac = 0.4; // soglia rispetto al vmax per il calcolo della larghezza
  int intfrom = 0;// regione di integrazione da 0=intfrom a intto
  if (intto>integerTo) intto=integerTo;
  int blfrom = 108, blto = 128; // regione per il calcolo della baseline


  float bl; // baseline evento x evento
  float integral; // integrale di carica
  float vmax; // massimo relativo alla bl
  float width; // larghezza temporale dei segnali


  char outfilename[200];
  strcpy(outfilename,"anabragg_");
  const char *cc=strrchr(filename,'/');
  if (cc) {cc++; strcat(outfilename,cc);}
  else strcat(outfilename,filename);

  TFile *fout=new TFile(outfilename,"RECREATE"); // output file

  TNtuple *nt=new TNtuple("nt","","ev:vmax:integral:width:baseline");

  int maxev=nev;
  if (nsig && nsig<nev) maxev=nsig;
  
  // LOOP SUGLI EVENTI
  for (int i=0; i<maxev; i++) 
  {
      // recupera l'evento
      br->GetEntry(i);
      // inizializza a zero le variabili utili, in genere necessarie per delle somme cicliche
      bl=0; 
      integral=0;
      vmax=0;				     
      width=0;
      double count=0;
      // calcolo integrali e vmax
      for (int j=intfrom; j<intto; j++) 
      {
        integral += (signal.s[j] - blfix);
        if ( (signal.s[j] - blfix) > vmax ) 
        {vmax = (signal.s[j] - blfix);
        count=j;
        };
      }
      //std::cout<<"max is at"<< count<<std::endl;
      integral += gRandom->Rndm(); //WTF

      // DA IMPLEMENTARE:
      
      // CALCOLO DELLA BASELINE
      // ...
         for (int r=blfrom; r<blto; r++) 
      bl += signal.s[r]; bl /= (blto-blfrom); //should be that: signal in position j, between max and min for 
      //baseline, and then renormalized

      // CALCOLO DELLA LARGHEZZA DEL SEGNALE A UNA CERTA PERCENTUALE DEL VMAX

      /* set where you want to calculate width, actaually there
      is a variable called "thr_frac" that should be used for this calc*/
      double perc=50.0;
      perc/=100;
      double percVmax=thr_frac*(vmax);//vmax+blfix-bl
      //std::cout<<percVmax<<std::endl;
      double counter1=0.,counter2=0.;
      for (int p=intfrom; p<intto;p++)
      {
          if(signal.s[p]<percVmax)
          {
            //std::cout<<"under"<<std::endl;
            counter1=p;
          }
          if(signal.s[p+1]>percVmax && counter1>0) 
          {
            //std::cout<<"counter1 set to "<<counter1<<std::endl;
            break;
          }

      }
    // std::cout<<"intto"<<intto<<std::endl;
      for (int p2=50; p2>count;p2--)
      {
       //std::cout<<"signal p2 "<<signal.s[p2]-bl<<std::endl;
          if(signal.s[p2]<percVmax)
          {
           // std::cout<<p2<<std::endl;
            counter2=p2;
          }
          if(signal.s[p2+1]>percVmax) break;

      }
      width=counter2-counter1;
      if(width>0)
      std::cout<<"width="<<width<<std::endl;
      else
      std::cout<<"error "<<width<<std::endl;
      nt->Fill(i,vmax,integral,width,bl);//bl and width should be checked since code was implemented by me, 
      //and I am not a good programmer :(
  }
  std::cout << maxev << " events analyzed..." << std::endl;

  fout->Write();
  fout->Close();

  fin->Close();

  new TFile(outfilename); // riapre il file dei risultati

  return 0;
}

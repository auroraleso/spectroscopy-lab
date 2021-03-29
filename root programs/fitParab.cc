
#include <iostream>

#include "TROOT.h"
#include "TMath.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TFitResult.h"
#include "TMatrixDSym.h"


 /*           ------- come usarmi  --------
            nel file metti Q, Vout, 0, errVout
            --------------------------------
            do errore alla fine perchè 
            sono un bimbo speciale,
            quindi se devi ricompilarmi devi dare un .exit(), 
            poi rientrare e compilare
 */



double_t fitfparab(double_t *x, double_t *par) 
{
   double_t fitval2 = par[0] + par[1]*x[0] + par[2]*x[0]*x[0];
  return fitval2;
}
double fitzero3(double *x, double *par) {
  double fitval = par[0] + par[1]*0;
   
  return fitval;
}

 

void par() {
  //crea un puntatore e nel costruttore gli passiamo direttamente il nome del file da cui prendere i dati
  TGraphErrors *g = new TGraphErrors("calibBragg600.txt");
    // associo un nome al seti di dati, do' un nome al puntatore del TGraphErrors
  g->SetName("fit");

  TCanvas * c1 = new TCanvas("c1", "fit calibrazione larghezza vs radq(E)",
                             27, 50, 1020, 760);
  g->SetMarkerStyle(8);
  g->SetMarkerSize(1);
  g->SetMarkerColor(1);
  //  "ap" significa che deve disegnare anche gli assi ( a sta per assi) e che deve disegnare i punti con un marker i cui parametri sono impostati precedentemente (p per i punti)
  g->Draw("AP");
  g->SetTitle("calib");
  g->GetXaxis()->SetTitle("Efficiency ");
  g->GetYaxis()->SetTitle(" E (keV) ");
  
  // definisco funzione del fit
  // la chiamo f1 e la associo alla funzione esponenziale che ho creato prima di nome fitf2. 
  //Devo dare i parametri xmin e xmax
  TF1 *f1 = new TF1("f1", fitfparab, 0, 10000, 3);
  f1->SetLineStyle(1);
    f1->SetLineColor(2);
    f1->SetLineWidth(1);
  // setta i valori iniziali dei parametri
  f1->SetParameters(0,0,0);
    // (nome parametro 0, nome parametro 1, nome parametro2)
  f1->SetParNames("par0", "par1", "par2" );

  
  g->Fit(f1,"RS");
  double p0= f1->GetParameter(0);
    double p1 = f1->GetParameter(1);
       double p2 = f1->GetParameter(2);
 // prendo l'errore del fit che mi hp0dato sul parametro
  double ep0= f1->GetParError(0);
    double ep1 = f1->GetParError(1);
     double ep2 = f1->GetParError(2);
  
  //stampo questi valori che ho recuperato
  std::cout << "\nParametri parabola "
            << p0<< " +- " << ep0<< " "
            << p1<< " +- " << ep1 << " "
            << p2<< " +- " << ep2
            << std::endl;

          
TMultiGraph* mg = new TMultiGraph();
    mg->Add(g);
    mg->Draw("ap");
    mg->SetTitle("Efficienza LaBr3");
    mg->GetYaxis()->SetTitle("Efficiency ");
    mg->GetXaxis()->SetTitle("E (keV)");
    f1->Draw("same");

  //c1->Print("fitsalitashaper.pdf");

  //residui del fit
  TCanvas* c2 = new TCanvas("c2", "residuisalita",
                             27, 50, 1020, 760);
  TGraphErrors* gr = (TGraphErrors*)g->Clone("gres");
  gr->SetTitle("residui");
  gr->GetXaxis()->SetTitle("f");
  gr->GetYaxis()->SetTitle("Residui");

  for (int i = 0; i < gr->GetN(); i++) {
      double x = gr->GetX()[i];
      double res = gr->GetY()[i] - f1->Eval(x);
      gr->SetPoint(i, x, res);
  }
 
 
  for (int i = 0; i < gr->GetN(); i++) {
        double x = gr->GetX()[i];
      double ex = gr->GetEX()[i];
      double ey = gr->GetEY()[i];
      double eres2 = ey * ey + ex * ex * f1->Derivative(x) * f1->Derivative(x);
      gr->SetPointError(i, 0, TMath::Sqrt(eres2)); // ora ex=0 perche' lo abbiamo riportato sulle y
  }


  
  TF1 *zero = new TF1("zero", fitzero3, 0, 10000, 2);
    zero->SetLineStyle(9);
    zero->SetLineColor(7);
   TMultiGraph* mg1 = new TMultiGraph();
    mg1->Add(gr);
    mg1->Draw("AP");
    mg1->SetTitle("Residui Efficienza LaBr3 ");
    mg1->GetXaxis()->SetTitle("E (keV) ");
    mg1->GetYaxis()->SetTitle("Residuals (kEv)");
    zero->Draw("same");
 
 // c2->Print("Residuiobliq.pdf");
    
    auto r = g->Fit(f1,"RS0"); // R = Region   S = StoreResults
    gStyle->SetOptFit();
    TMatrixDSym cov = r->GetCovarianceMatrix();  //  invoco la matrice di covarianza
    double chi2   = r->Chi2();
    std::cout << "\nCOVARIANCE MATRIX :\n ";
    cov.Print();

   
  return;
}

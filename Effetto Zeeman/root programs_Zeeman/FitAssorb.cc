// Macro di esempio che fitta delle misure di assorbimento di raggi gamma
// a differenti spessori di assorbitore, con una funzione esponenziale 
// decrescente piu' un fondo costante

// per le opzioni di fit guardare anche:
// 
// https://root.cern.ch/doc/master/classTH1.html#TH1:Fit
//

double fitf(double *x, double *par) {
  double fitval = par[0] + par[1]*TMath::Exp(-par[2]*x[0]);
  return fitval;
}

void FitAssorb() {

  // crea l'oggetto GRAFICO CON ERRORI leggendo da file di testo
  TGraphErrors *g = new TGraphErrors("assorb.dat");
  g->SetName("assorb");
  // formattazione del grafico con comandi testuali
  g->SetMarkerColor(4);
  g->SetMarkerStyle(20);
  g->SetTitle("Gamma absorption");
  g->GetXaxis()->SetTitle("Thickness [#mum]");
  g->GetXaxis()->SetTitleSize(0.04);
  g->GetXaxis()->CenterTitle();
  g->GetYaxis()->SetTitle("Counts");
  g->GetYaxis()->SetTitleSize(0.04);
  g->GetYaxis()->CenterTitle();

  TCanvas *c1 = new TCanvas("c1"); // apre la finestra grafica
  c1->SetLogy();

  g->Draw("ap"); // disegna il grafico a punti (p) in un nuovo frame (a)
  
  g->GetYaxis()->SetRangeUser(2000,50000); // setta il range dell'asse Y

  // due modi per definire la funzione di fit:
  //  TF1 *f1 = new TF1("f1","[0]+[1]*exp(-x*[2])",0,60); // formula definita nel costruttore 
  TF1 *f1 = new TF1("f1",fitf,0,60,3); // formula definita da funzione

  // setta i valori iniziali dei parametri
  f1->SetParameters(0,30000,0.1);
  f1->SetParNames("bkg","constant","absorption corfficient");

  TFitResultPtr r = g->Fit(f1,"RS"); // R = Region   S = StoreResults
  
  gStyle->SetOptFit(); // cliccare sulla pad dei risultati con l'editor e provare a cambiare qualcosa...
  // gStyle->SetoptFit(1111); // stampa tutto il necessario

  double mu = f1->GetParameter(2);
  double emu = f1->GetParError(2);
  
  std::cout << "\nGamma absorption coefficient mu " << mu << " +- " << emu << std::endl;

  // RECUPERA LA MATRICE DI COVARIANZA
  TMatrixDSym cov = r->GetCovarianceMatrix();  //  to access the covariance matrix
  double chi2   = r->Chi2(); // to retrieve the fit chi2

  std::cout << "\nCOVARIANCE MATRIX :\n ";
  cov.Print();

  std::cout << "chi2 : " << chi2 << std::endl;
  std::cout << "sigma_mu = " << TMath::Sqrt(cov(2,2)) << std::endl;

  c1->Print("FitAssorb.png"); // salva la canvas in formato grafico
  c1->Print("FitAssorb.root"); // salva la canvas in formato root: e' possibile modificare la figura in un secondo momento...


  // GRAFICO DEI RESIDUI
  TGraphErrors *gr = new TGraphErrors("assorb.dat"); // parte da una copia del grafico originale
  for (int i=0; i<g->GetN(); i++) {
    double res = g->GetY()[i] - f1->Eval(g->GetX()[i]); // residuo
    gr->SetPoint(i,g->GetX()[i],res);
    double eresy = g->GetEY()[i]; // contributo delle Yi
    double eresx = f1->Derivative(g->GetX()[i])*g->GetEX()[i]; // contrib. Xi (approx. 1 ordine)
    double eres = TMath::Sqrt(eresy*eresy+eresx*eresx);
    gr->SetPointError(i,0,eres);
  }
  gr->SetName("gr");
  // formattazione del grafico con comandi testuali
  gr->SetMarkerColor(4);
  gr->SetMarkerStyle(20);
  gr->SetTitle("Gamma absorption - residuals");
  gr->GetXaxis()->SetTitle("Thickness [#mum]");
  gr->GetXaxis()->SetTitleSize(0.04);
  gr->GetXaxis()->CenterTitle();
  gr->GetYaxis()->SetTitle("Counts");
  gr->GetYaxis()->SetTitleSize(0.04);
  gr->GetYaxis()->CenterTitle();

  TCanvas *c2 = new TCanvas("c2");
  c2->SetGrid();
  gr->Draw("ap");
  gr->GetYaxis()->SetRangeUser(-100,100); // setta il range dell'asse Y

  c2->Print("FitAssorb_res.png");
}

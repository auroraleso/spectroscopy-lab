// con ROOT 6 occorre caricare prima la macro ReadHistoFromTextFile.C con .L ...

void Prisma1(const char *hname, const char *hnamebkg) {

  // con ROOT 5 e' possibile caricare la macro esterna da dentro la macro:
  // gROOT->LoadMacro("ReadHistoFromTextFile.C");
  // gSystem->Load("ReadHistoFromTextFile_C.so"); // altra opzione per la precompilata

  // carica gli istogrammi, senza disegnarli
  TH1F *hsource = ReadHistoFromTextFile(hname,"hsource",0); 
  if (!hsource) return;
  TH1F *hbkg = ReadHistoFromTextFile(hnamebkg,"hbkg",0);
  if (!hbkg) return;
  hbkg->SetLineColor(2);

  TCanvas *c1=new TCanvas("c1"); // apre una prima canvas "c1"
  hsource->Draw();
  hbkg->Draw("same"); // disegna il bkg sovrapposto al primo spettro
  c1->Print("prisma1_1.root"); // salva la canvas su root file

  TH1F *hnet = (TH1F*) hsource->Clone("hnet"); // crea una copia di hsource
  hnet->SetLineColor(1);
  hnet->SetTitle("net spectrum");
  hnet->Sumw2(1); // setta il calcolo automatico degli errori statistici
  hnet->Add(hbkg,-1.); // sottrae il background

  TCanvas *c2=new TCanvas("c2");
  hnet->Draw(); 
  c2->Print("prisma1_2.root"); // salva la canvas su root file

}

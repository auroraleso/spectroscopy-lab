
using namespace std;

//C++ LIBRARIES
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
//ROOT LIBRARIES
#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMath.h"



void read_data(vector<double>&,vector<double>&, vector<double>&,vector<double>&, const string,  double &,  double &,  double &,  double &);

TGraph *make_plot(vector<double>&, vector<double>&);

void settings_plot(TGraph*, const double, const double, const double, const double);

void settings_global();




/*-------- MAIN -------*/

void plot()
{   
    /*--- COSTANTI ---*/
    const string FILE_NAME = "xdiff.txt";
     double XMIN = 100;
     double XMAX = 600;
     double YMIN = 100;
     double YMAX = 400;
     
    const int FREQ = 1/(5*10^-6); //Hertz
    const double PERIODO = pow(FREQ, -1); //Secondi
   //  double THRESHOLD = 80;

    /* --- ROOT OBJECTS ---*/
    TCanvas* c1 = nullptr;
    TGraph *plot = nullptr;
    TGraph *der = nullptr;

    /*--- DATA VECTORS ---*/
    vector<double> x, y,erry,errx;
   // vector<double> x_deriv, y_deriv;
    //vector<double> xma_results, yma_results,xmi_results, ymi_results;
    vector<double> n_points, sampling_rates;

    /*--- ARDUINO SAMPLING RATE ---*/
 
    /*--- CANVAS ---*/
    c1 = new TCanvas("mean vs E", "calibrazione HPGe", 1080, 720);
   // c1->Divide(0, 2);

    /*--- READING DATA FROM FILE ---*/
    read_data(x, y, errx, erry, FILE_NAME, YMAX, YMIN, XMAX, XMIN);
    cout<<"xmin "<<XMIN<<" xmax "<<XMAX<<" ymax "<<(YMAX-30)<<" ymin "<<YMIN+30<<endl;
    /*--- UPPER PLOT - WAVEFORM ---*/
    plot = make_plot(x, y);
    plot-> SetTitle("Plot peak spacing ; peak position (a.u.); peak spacing (a.u.)");
    c1->cd(1);
    plot->Draw("AP");

    settings_plot(plot, XMIN, XMAX, YMIN, YMAX);

    /*--- GLOBAL PLOT SETTINGS ---*/
    settings_global();

   return;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-------- FUNCTIONS -------*/

void read_data(vector<double>& x, vector<double>& y, vector<double>& errx, vector<double>& erry, const string FILE_NAME,  double &YMAX,  double &YMIN, double &XMAX,  double &XMIN) 
{

    ifstream f;
    f.open(FILE_NAME);
    double i = 0;
    int n=0;
    while(f >> i) 
    {

        x.push_back(i);   
        f >> i;
        //erry.push_back(i); 
        //f >> i;
        y.push_back(i);  
        //f >> i;
        //erry.push_back(i);
        n++;

    }
   
      
        YMAX=*max_element(y.begin(), y.end())+30;
        YMIN=*min_element(y.begin(), y.end())-30;
      
    return;
}

TGraph* make_plot(vector<double>& x, vector<double>& y) {
    
    TGraph* graph = new TGraph(x.size(), &x[0], &y[0]);

    return graph;
}

void settings_global() {

    TGaxis::SetMaxDigits(4);
    gStyle->SetStripDecimals(kFALSE);
    gStyle->SetImageScaling(3.);

    return;
}

void settings_plot(TGraph* graph, const double XMIN, const double XMAX, const double YMIN, const double YMAX) {

    graph-> SetLineColor(34);
    graph-> SetMarkerStyle(20);
    graph-> SetMarkerColor(1);
    graph-> SetMarkerSize(0.75);

    gPad->Modified();
    
    graph->GetXaxis()->SetLimits(3000, 8000);
    graph->SetMinimum(100);
    graph->SetMaximum(400);

    graph->GetXaxis()->SetTickLength(0.02);
    graph->GetYaxis()->SetTickLength(0.02);

    return;
}

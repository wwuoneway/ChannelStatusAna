{

  int runLow = 5000; // replace with new low run number
  int runHigh = 6000; // replace with new high run number
  int runRange = runHigh - runLow;
  int numRuns = 500; // count of valid runs in the analysis, from input files

  TFile f("dead_channel_hists_run1.root");
  TH1D *h_deadCh_runs = (TH1D*)f.Get("h_dead_channel_count_vs_runs");

  TGraph *gr_deadCh_runs = new TGraph();
  gr_deadCh_runs->SetTitle("Number of Dead Channels vs Runs");
  gr_deadCh_runs->GetXaxis()->SetTitle("Run number");
  gr_deadCh_runs->GetYaxis()->SetTitle("Number of dead channels");
  for (int i=0; i<runRange; i++){
    if (h_deadCh_runs->GetBinContent(i+1) <= 0) continue;
    gr_deadCh_runs->SetPoint(i,h_deadCh_runs->GetBinCenter(i+1), h_deadCh_runs->GetBinContent(i+1));
  }
  gr_deadCh_runs->GetXaxis()->SetRangeUser(runLow,runHigh);
  gr_deadCh_runs->GetYaxis()->SetRangeUser(0,1500);
  gr_deadCh_runs->SetMarkerStyle(20);
  gr_deadCh_runs->SetMarkerColor(2);
  TCanvas *c1 = new TCanvas("c1","c1",800,600);
  gr_deadCh_runs->Draw("ap");

  TH1D *h_noisyCh_runs = (TH1D*)f.Get("h_noisy_channel_count_vs_runs");

  TGraph *gr_noisyCh_runs = new TGraph();
  gr_noisyCh_runs->SetTitle("Number of Noisy Channels vs Runs");
  gr_noisyCh_runs->GetXaxis()->SetTitle("Run number");
  gr_noisyCh_runs->GetYaxis()->SetTitle("Number of noisy channels");
  for (int i=0; i<runRange; i++){
    if (h_noisyCh_runs->GetBinContent(i+1) <= 0) continue;
    gr_noisyCh_runs->SetPoint(i,h_noisyCh_runs->GetBinCenter(i+1), h_noisyCh_runs->GetBinContent(i+1));
  }                                                             
  gr_noisyCh_runs->GetXaxis()->SetRangeUser(runLow,runHigh);
  gr_noisyCh_runs->SetMarkerStyle(20);
  gr_noisyCh_runs->SetMarkerColor(4);
  TCanvas *c2 = new TCanvas("c2","c2",800,600);
  gr_noisyCh_runs->GetYaxis()->SetRangeUser(0, 120);
  gr_noisyCh_runs->Draw("ap");

  TH1D *h0 =(TH1D*)f.Get("h_dead_channel_count_plane0_vs_runs");
  TH1D *h1 =(TH1D*)f.Get("h_dead_channel_count_plane1_vs_runs");
  TH1D *h2 =(TH1D*)f.Get("h_dead_channel_count_plane2_vs_runs");
  
  TCanvas *c3 = new TCanvas("c3","c3", 800, 600);
  TMultiGraph *mg = new TMultiGraph();
  TGraph *gr0 = new TGraph();
  TGraph *gr1 = new TGraph();
  TGraph *gr2 = new TGraph();
  
  for (int i=0;i<runRange; i++) {
    if (h0->GetBinContent(i+1) <=0) continue;
    gr0->SetPoint(i, h0->GetBinCenter(i+1), h0->GetBinContent(i+1));
  }
  for (int i=0;i<runRange; i++) {
    if (h1->GetBinContent(i+1) <=0) continue;
    gr1->SetPoint(i, h1->GetBinCenter(i+1), h1->GetBinContent(i+1));
  }
  for (int i=0;i<runRange; i++) {
    if (h2->GetBinContent(i+1) <=0) continue;
    gr2->SetPoint(i, h2->GetBinCenter(i+1), h2->GetBinContent(i+1));
  }

  gr0->SetMarkerStyle(20);
  gr1->SetMarkerStyle(21);
  gr2->SetMarkerStyle(22);

  gr0->SetMarkerColor(3);
  gr1->SetMarkerColor(6);
  gr2->SetMarkerColor(9);

  
  mg->Add(gr0);
  mg->Add(gr1);
  mg->Add(gr2);
  mg->Draw("ap");
 
  gr0->GetXaxis()->SetRangeUser(runLow,runHigh);
  gr1->GetXaxis()->SetRangeUser(runLow,runHigh);
  gr2->GetXaxis()->SetRangeUser(runLow,runHigh);
  mg->GetXaxis()->SetRangeUser(runLow,runHigh);
  mg->GetYaxis()->SetRangeUser(0,600);
  mg->SetTitle("Number of Dead Channels on Different Wire Planes");
  mg->GetXaxis()->SetTitle("Run number");
  mg->GetYaxis()->SetTitle("Number of dead channels");
 
  TLegend *leg = new TLegend(0.7,0.75,0.9,0.9);
  leg->AddEntry(gr0,"Plane 0","p");
  leg->AddEntry(gr1,"Plane 1","p");
  leg->AddEntry(gr2,"Plane 2","p");
  leg->Draw();
 
  // dead freq
  TH1D *h_deadCh_freq = (TH1D*)f.Get("h_dead_channel_freq_vs_channel_number");
  int N_CHANNELS = 8256;

  auto c0 = new TCanvas("c0", "c0", 1200, 600);
  auto mg_freq = new TMultiGraph();
  mg_freq->SetTitle(Form("run1_%d_%d: dead channel frequency (normalized); channel number; freq (normalized)", runLow, runHigh));

  TGraph *gr_freq = new TGraph();
  gr_freq->SetMarkerStyle(21);
  gr_freq->SetMarkerColor(4);
  gr_freq->SetMarkerSize(0.4);

  for (int i = 0; i  < N_CHANNELS; ++i) {
    if (h_deadCh_freq->GetBinContent(i+1) < 0) {
      continue;
    }
    else {
      gr_freq->SetPoint(i, h_deadCh_freq->GetBinCenter(i+1), h_deadCh_freq->GetBinContent(i+1)/numRuns*1.0);
    }
  }

  mg_freq->Add(gr_freq);

  mg_freq->Draw("ap");
  mg_freq->GetHistogram()->GetYaxis()->SetRangeUser(0,1.05);
  mg_freq->GetHistogram()->GetXaxis()->SetRangeUser(-100, 8300);
  gPad->Modified();
  gPad->Update();

  TFile* f_output = new TFile("./plots/plot.root", "RECREATE");
  f_output->cd();
  c0->Write(Form("run1_%d_%d_deadChannel", runLow, runHigh));
  c0->SaveAs(Form("./plots/run1_%d_%d_deadChannel.pdf", runLow, runHigh));
  c1->Write(Form("run1_%d_%d_numDeadChannel_vs_run", runLow, runHigh));
  c1->SaveAs(Form("./plots/run1_%d_%d_numDeadChannel_vs_run.pdf", runLow, runHigh));
  c2->Write(Form("run1_%d_%d_numNoisyChannel_vs_run", runLow, runHigh));
  c2->SaveAs(Form("./plots/run1_%d_%d_numNoisyChannel_vs_run.pdf", runLow, runHigh));
  c3->Write(Form("run1_%d_%d_numDeadChannelEachPlane_vs_run", runLow, runHigh));
  c3->SaveAs(Form("./plots/run1_%d_%d_numDeadChannelEachPlane_vs_run.pdf", runLow, runHigh));
  f_output->Close();

}

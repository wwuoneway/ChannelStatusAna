#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <numeric> // std::accumulate
#include <sys/stat.h>
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"

using namespace std;

unsigned const int ASIC_SIZE = 16;

/////////////////////////////////////////////////////////////////////////
// complie: g++ -o analyze_channelStatus analyze_channelStatus.C `root-config --cflags --libs`
//deadFactor is from 0 to 1 for each channel in each run.
// deadFactor = number of channel-dead events / number of total events  
// need to replace lowRun and highRun
///////////////////////////////////////////////////////////////////////////

// define a data type to sort the filename by run, subrun, 
struct filesPerRun {
  std::string run_number;
  std::string subRunNober;
  std::string file_close_time; // not necessary, but in case
};

// define a data type for ASIC
struct checkASIC {
  int asic_number;
  bool asic_dead_status[ASIC_SIZE];
};

int main(int argc, char* argv[]) {

  if (argc < 4 || argc > 5) {
    cout << "Error: Improper number of arguments. " << endl;
    cout << "Here is an example: ./analyze_channelStatus inputlist deadFactor output_tag output_rootfile_tag\n"
        << "\t ./analyze_channelStatus playlist.dat 1.0 run1"
        << "\n output_rootfile_tag can be omitted, i.e., ./analyze_channelStatus playlist.dat 1.0 run1"
        << endl;
    return -1;
  }

  // total channels: 2400 + 2400 +3456 = 8256
  unsigned const int N_CHANNELS = 8256;

  // input playlist
  std::string playlist_name(argv[1]);
  std::ifstream playlist(playlist_name.c_str());

  // dead factor to check if a channel is dead or not
  const double DEAD_FACTOR = (double)std::atof(argv[2]);
  if (DEAD_FACTOR > 1.0 || DEAD_FACTOR < 0.0) {
    cout << "Error: improper DEAD_FACTOR. DEAD_FACTOR is in range [0,1.0]" << endl;
    return -1;
  }

  // output 
  int lowRun = 5000; // replace with the new low run number
  int highRun = 6000; // replace with the new high run number
  int runRange = highRun - lowRun;
  std::string output_tag(argv[3]);
  std::string output_directory = "channel_status_files_"+output_tag+"/";
  std::string output_rootfile_tag;
  std::string output_root_file;
  if (argv[4]) {
    output_rootfile_tag = argv[4];
    output_root_file = "dead_channel_hists_"+output_tag+"_"+output_rootfile_tag+".root";
  }
  else {
    output_root_file = "dead_channel_hists_"+output_tag+".root";
  }
  cout << "....output_directory: " << output_directory << endl;
  cout << "....output_root_file: " << output_root_file << endl;
  // output_root_file
  TFile* f_output = new TFile(output_root_file.c_str(),"RECREATE");

  // histograms to plot dead channel freq (among all the runs) vs channel number
  TH1D* h_dead_channel_freq_vs_channel_number = new TH1D("h_dead_channel_freq_vs_channel_number", "h_dead_channel_freq_vs_channel_number (among all the runs); Channel number; Dead channel freq", N_CHANNELS+2, -1, N_CHANNELS);

  // histograms to plot "noisy" channel freq (among all the runs) vs channel number
  TH1D* h_noisy_channel_freq_vs_channel_number = new TH1D("h_noisy_channel_freq_vs_channel_number", "h_noisy_channel_freq_vs_channel_number (among all the runs); Channel number; Dead channel freq", N_CHANNELS+2, -1, N_CHANNELS);
  
  // histograms to plot dead asic count vs runs
  TH1D* h_dead_asic_count_vs_runs = new TH1D("h_dead_asic_count_vs_runs","h_dead_asic_count_vs_runs; Run number; Number of dead ASICs", runRange, lowRun, highRun);
  
  // histograms to plot dead channel count vs runs
  TH1D* h_dead_channel_count_vs_runs = new TH1D("h_dead_channel_count_vs_runs","h_dead_channel_count_vs_runs; Run number; Number of dead channels", runRange, lowRun, highRun);
  TH1D* h_dead_channel_count_plane0_vs_runs = new TH1D("h_dead_channel_count_plane0_vs_runs","h_dead_channel_count_plane0_vs_runs; Run number; Number of dead channels",runRange, lowRun, highRun);
  TH1D* h_dead_channel_count_plane1_vs_runs = new TH1D("h_dead_channel_count_plane1_vs_runs","h_dead_channel_count_plane1_vs_runs; Run number; Number of dead channels", runRange, lowRun, highRun);
  TH1D* h_dead_channel_count_plane2_vs_runs = new TH1D("h_dead_channel_count_plane2_vs_runs","h_dead_channel_count_plane2_vs_runs; Run number; Number of dead channels", runRange, lowRun, highRun);

  // histograms to plot noisy channel count vs runs
  TH1D* h_noisy_channel_count_vs_runs = new TH1D("h_noisy_channel_count_vs_runs","h_noisy_channel_count_vs_runs; Run number; Number of noisy channels", runRange, lowRun, highRun);
  // array to store bad channnel information
  vector< vector<bool> > bad_array;

  // dead asic numbers
  vector<int> num_dead_asic;

  // dead channel numbers
  int n_dead_channel = 0; // intialize to "0" for each run
  int n_dead_channel_plane0 = 0; // intialize to "0" for each run
  int n_dead_channel_plane1 = 0; // intialize to "0" for each run
  int n_dead_channel_plane2 = 0; // intialize to "0" for each run

  // "noisy" channel numbers
  int n_noisy_channel = 0; // intialize to "0" for each run

//  int totalRun = 0;
//  int totalSubRun = 0;
//  int totalEvent = 0;

  // loop over files in playlist
  std::string line;
  std::string current_run_number = "0";
  while ( getline(playlist, line) ) {
    
    // get the run number
    std::string local_file_name = line.substr(line.find_last_of("/")+1);
    //cout << "local_file_name: " << local_file_name << endl;
    std::string local_substring = local_file_name.substr(0, local_file_name.find_last_of("_"));
    local_substring = local_substring.substr(0, local_substring.find_last_of("_"));
    //cout << "local_substring: " << local_substring << endl;
    std::string run_number = local_substring.substr(local_substring.find_last_of("_")+1);
    cout << "run_number: " << run_number << endl;

    // if we've moved on to next run_number, process the previous one
    if ( current_run_number != "0" && run_number != current_run_number ) {
      cout << "Move on to another run...." << endl;

      // results
      mkdir(output_directory.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
      std::ofstream output_file(output_directory+"run_"+current_run_number+".dat");

      output_file << "# time " << current_run_number << endl;
      output_file << "# channel status" << endl;
      for (unsigned int c=0; c!=N_CHANNELS;++c){
        unsigned int bad_count = 0;
        for (unsigned int e=0; e!=bad_array.size();++e){
          if (bad_array[e][c]) bad_count++;
        }

        if ((double)bad_count/bad_array.size() >= DEAD_FACTOR) {
          output_file << c << " 1" << endl; // dead
          n_dead_channel++; // count how many dead channels in each run
          if (c < 2400) {
            n_dead_channel_plane0++;
          }
          else if (c < 2400 + 2400 ) {
            n_dead_channel_plane1++;
        }
        else {
            n_dead_channel_plane2++;
          }
          h_dead_channel_freq_vs_channel_number->Fill(c);
        }
        else if ((double)bad_count/bad_array.size() > 0.5) {
          output_file << c << " 3" << endl; // noisy
          n_noisy_channel++;
          h_noisy_channel_freq_vs_channel_number->Fill(c);
        }
        else {
          output_file << c << " 4" << endl; // good
        }

      } // end of: for (unsigned int c=0; c!=N_CHANNELS;++c)

      output_file.close();

      // fill dead ASIC histogram for "last" run
      h_dead_asic_count_vs_runs->Fill(std::stoi(current_run_number), (int)std::accumulate(num_dead_asic.begin(), num_dead_asic.end(), 0.0)/num_dead_asic.size());

      // fill deac channel histogram for "last" run
      h_dead_channel_count_vs_runs->Fill(std::stoi(current_run_number),n_dead_channel);
      h_dead_channel_count_plane0_vs_runs->Fill(std::stoi(current_run_number),n_dead_channel_plane0);
      h_dead_channel_count_plane1_vs_runs->Fill(std::stoi(current_run_number),n_dead_channel_plane1);
      h_dead_channel_count_plane2_vs_runs->Fill(std::stoi(current_run_number),n_dead_channel_plane2);

      // fill noisy channel histogram for "last" run
      h_noisy_channel_count_vs_runs->Fill(std::stoi(current_run_number),n_noisy_channel);

      // reset for new run
      current_run_number = run_number;
      num_dead_asic.clear();
      bad_array.clear();
      n_dead_channel = 0; // not really necessary
      n_dead_channel_plane0 = 0; // not really necessary
      n_dead_channel_plane1 = 0; // not really necessary
      n_dead_channel_plane2 = 0; // not really necessary
      n_noisy_channel = 0; // not really necessary

    } // end of: if ( current_run_number != "0" && run_number != current_run_number )
    else if ( current_run_number == "0") {
      current_run_number = run_number;
    }
    
    //cout << "bad_array.size(): " << bad_array.size() << endl;


    // get TTrees and set branch addresses
    TFile* f = new TFile(line.c_str(), "READ");
    TTree* t = (TTree*)f->Get("ChannelStatus/badChannelTree");
    if (!t) {
      cout <<"Error: cannot get the tree" << endl;
      return -1;
    }
    int runNo = -1;
    int subRunNo = -1;
    int eventNo = -1;
    vector<int>* badChannel = NULL;
    TBranch* branch_runNo;
    TBranch* branch_subRunNo;
    TBranch* branch_eventNo;
    t->SetBranchAddress("runNo", &runNo, &branch_runNo);
    t->SetBranchAddress("subRunNo", &subRunNo, &branch_subRunNo);
    t->SetBranchAddress("eventNo", &eventNo, &branch_eventNo);
    t->SetBranchAddress("badChannel", &badChannel);

    //cout << "badChannel->size(): " << badChannel->size() << endl;

    // get the channel info for this file in line, every entry is an event
    //cout << "t->GetEntries(): " << t->GetEntries() << endl;
    for (unsigned int i = 0; i != t->GetEntries(); ++i) {
      t->GetEntry(i);

//      cout << "runNo: " << runNo << endl;
//      cout << "subRunNo: " << subRunNo << endl;
//      cout << "eventNo: " << eventNo << endl;
      if (runNo != std::stoi(run_number)) {
        cout << "Error: check the run number." << endl;
        return -1;
      }

      // resize the bad_array for event in each subrun, for each run
      //cout << "bad_array.size(): " << bad_array.size() << endl;
      bad_array.resize(bad_array.size()+1);
      bad_array.back().resize(N_CHANNELS);
      std::fill(bad_array.back().begin(), bad_array.back().end(), false);
      
      // create an object to hold the current ASIC number and ASIC channel status
      checkASIC current_asic;
      current_asic.asic_number = -1;
      for (unsigned int k = 0; k != ASIC_SIZE; ++k) {
        current_asic.asic_dead_status[k] = false; // false: asic channel is functional
      }
      
      unsigned int n_dead_asic = 0; // intialize to "0" for each event

      //cout << "badChannel->size(): " << badChannel->size() << endl;
      for (unsigned int j=0; j < badChannel->size(); ++j) {
        // get the ASIC number and set current ASIC channel status
        int asic_num = (*badChannel)[j]/ASIC_SIZE;
        if (asic_num != current_asic.asic_number) {
          current_asic.asic_number = asic_num;
          for (unsigned int k=0; k!=ASIC_SIZE;++k){
            current_asic.asic_dead_status[k] = false;
          }
        }
        current_asic.asic_dead_status[(*badChannel)[j]%ASIC_SIZE] = true;
        
        // check if ASIC is dead
        if ((*badChannel)[j]%ASIC_SIZE == ASIC_SIZE -1) {
          bool dead_asic = current_asic.asic_dead_status[0];
          for (unsigned int k=0; k!=ASIC_SIZE; ++k){
            dead_asic = dead_asic && current_asic.asic_dead_status[k];
          }
          if (dead_asic) n_dead_asic++;
        }

        // get the dead channel information
        bad_array.back()[(*badChannel)[j]] = true;

      } // end of: for (unsigned int j=0; j < badChannel->size(); ++j) 
      
      num_dead_asic.push_back(n_dead_asic);
      //cout << "num_dead_asic.back(): " << num_dead_asic.back() << endl;
      // think about how to store into histograms

    } // end of: for (unsigned int i = 0; i != t->GetEntries(); ++i)
    
    f->Close();
  } // end of: while ( getline(playlist, line) )

  
  // if last event/file for the "last" run
  //cout << "\n current_run_number: " << current_run_number << endl;
  //cout << "num_dead_asic.size(): " << num_dead_asic.size() << endl;
  //cout << "average dead asic for a run: " << (int)std::accumulate(num_dead_asic.begin(), num_dead_asic.end(), 0.0)/num_dead_asic.size() << endl;
  //cout << "bad_array.size(): " << bad_array.size() << endl;
  
  // results
  mkdir(output_directory.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  std::ofstream output_file(output_directory+"run_"+current_run_number+".dat");

  output_file << "# time " << current_run_number << endl;
  output_file << "# channel status" << endl;
  for (unsigned int c=0; c!=N_CHANNELS;++c){
    unsigned int bad_count = 0;
    for (unsigned int e=0; e!=bad_array.size();++e){
      if (bad_array[e][c]) bad_count++;
    }
    
    if ((double)bad_count/bad_array.size() >= DEAD_FACTOR) {
      output_file << c << " 1" << endl; // dead
      n_dead_channel++; // count how many dead channels in each run
      if (c < 2400) {
        n_dead_channel_plane0++;
      }
      else if (c < 2400 + 2400 ) {
        n_dead_channel_plane1++;
      }
      else {
        n_dead_channel_plane2++;
      }
      h_dead_channel_freq_vs_channel_number->Fill(c);
    }
    else if ((double)bad_count/bad_array.size() > 0.5) {
      output_file << c << " 3" << endl; // noisy
      n_noisy_channel++;
      h_noisy_channel_freq_vs_channel_number->Fill(c);
    }
    else {
      output_file << c << " 4" << endl; // good
    }

  } // end of: for (unsigned int c=0; c!=N_CHANNELS;++c)

  output_file.close();

  // fill dead ASIC histogram for "last" run
  h_dead_asic_count_vs_runs->Fill(std::stoi(current_run_number), (int)std::accumulate(num_dead_asic.begin(), num_dead_asic.end(), 0.0)/num_dead_asic.size());
 
  // fill dead channel histogram for "last" run
  h_dead_channel_count_vs_runs->Fill(std::stoi(current_run_number),n_dead_channel);
  h_dead_channel_count_plane0_vs_runs->Fill(std::stoi(current_run_number),n_dead_channel_plane0);
  h_dead_channel_count_plane1_vs_runs->Fill(std::stoi(current_run_number),n_dead_channel_plane1);
  h_dead_channel_count_plane2_vs_runs->Fill(std::stoi(current_run_number),n_dead_channel_plane2);

  // fill noisy channel histogram for "last" run
  h_noisy_channel_count_vs_runs->Fill(std::stoi(current_run_number),n_noisy_channel);

  f_output->cd();
  
  // set bin error for all the bins, here 800 bins
  for(int i = 1; i<= h_dead_asic_count_vs_runs->GetNbinsX(); ++i) {
    h_dead_asic_count_vs_runs->SetBinError(i,0.0);
    h_dead_channel_count_vs_runs->SetBinError(i,0.0);
    h_dead_channel_count_plane0_vs_runs->SetBinError(i,0.0);
    h_dead_channel_count_plane1_vs_runs->SetBinError(i,0.0);
    h_dead_channel_count_plane2_vs_runs->SetBinError(i,0.0);
    h_noisy_channel_count_vs_runs->SetBinError(i,0.0);
  }
  h_dead_channel_freq_vs_channel_number->Write();
  h_noisy_channel_freq_vs_channel_number->Write();
  h_dead_asic_count_vs_runs->Write();
  h_dead_channel_count_vs_runs->Write();
  h_dead_channel_count_plane0_vs_runs->Write();
  h_dead_channel_count_plane1_vs_runs->Write();
  h_dead_channel_count_plane2_vs_runs->Write();
  h_noisy_channel_count_vs_runs->Write();
  
  f_output->Close();
  cout << "..... DONE ...." << endl;

  return 0;
}

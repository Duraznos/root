// @(#)root/roofitcore:$name:  $:$id$
// Authors: Wouter Verkerke  November 2007

// C/C++ headers
#include <list>
#include <iostream>
#include <iomanip>

// ROOT headers
#include "TWebFile.h"
#include "TSystem.h"
#include "TString.h"
#include "TROOT.h"
#include "TFile.h"
#include "TBenchmark.h"

// RooFit headers
#include "RooNumIntConfig.h"
#include "RooResolutionModel.h"
#include "RooAbsData.h"
#include "RooTrace.h"
#include "RooMath.h"
#include "RooUnitTest.h"

// Tests file
#include "stressHistFactory_tests.cxx"

using namespace std ;
using namespace RooFit ;


//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*//
//                                                                           //
// RooStats Unit Test S.T.R.E.S.S. Suite                                     //
// Authors: Ioan Gabriel Bucur, Lorenzo Moneta, Wouter Verkerke              //
//                                                                           //
//*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*//


//------------------------------------------------------------------------
void StatusPrint(const Int_t id, const TString &title, const Int_t status, const Int_t lineWidth)
{
   // Print test program number and its title
   TString header = TString::Format("Test %d : %s ", id, title.Data());
   cout << left << setw(lineWidth) << setfill('.') << header << " " << (status > 0 ? "OK" : (status < 0 ? "SKIPPED" : "FAILED")) << endl;
}

//______________________________________________________________________________
Int_t stressHistfactory(const char* refFile, Bool_t writeRef, Int_t verbose, Bool_t allTests, Bool_t oneTest, Int_t testNumber, Bool_t dryRun)
{
   // width of lines when printing test results
   const Int_t lineWidth = 120;

   // Save memory directory location
   RooUnitTest::setMemDir(gDirectory) ;

   TFile* fref = 0 ;
   if (!dryRun) {
      if (TString(refFile).Contains("http:")) {
         if (writeRef) {
            cout << "stressHistfactory ERROR: reference file must be local file in writing mode" << endl ;
            return kFALSE ;
         }
         fref = new TWebFile(refFile) ;
      } else {
         fref = new TFile(refFile, writeRef ? "RECREATE" : "") ;
      }
      if (fref->IsZombie()) {
         cout << "stressHistfactory ERROR: cannot open reference file " << refFile << endl ;
         return kFALSE ;
      }
   }

   if (dryRun) {
      // Preload singletons here so they don't show up in trace accounting
      RooNumIntConfig::defaultConfig() ;
      RooResolutionModel::identity() ;

      RooTrace::active(1) ;
   }

   // Add dedicated logging stream for errors that will remain active in silent mode
   RooMsgService::instance().addStream(RooFit::ERROR) ;


   cout << left << setw(lineWidth) << setfill('*') << "" << endl;
   cout << "*" << setw(lineWidth - 2) << setfill(' ') << " Histfactory S.T.R.E.S.S. suite " << "*" << endl;
   cout << setw(lineWidth) << setfill('*') << "" << endl;
   cout << setw(lineWidth) << setfill('*') << "" << endl;


   TStopwatch timer;
   timer.Start();

   list<RooUnitTest*> testList;
   testList.push_back(new PdfComparison(fref, writeRef, verbose));
   
   TString suiteType = TString::Format(" Starting S.T.R.E.S.S. %s",
                                       allTests ? "full suite" : (oneTest ? TString::Format("test %d", testNumber).Data() : "basic suite")
                                      );

   cout << "*" << setw(lineWidth - 3) << setfill(' ') << suiteType << " *" << endl;
   cout << setw(lineWidth) << setfill('*') << "" << endl;

   gBenchmark->Start("stressHistfactory");

   {
      Int_t i;
      list<RooUnitTest*>::iterator iter;

      if (oneTest && (testNumber <= 0 || (UInt_t) testNumber > testList.size())) {
         cout << "Tests are numbered from 1 to " << testList.size() << endl;
      } else {
         for (iter = testList.begin(), i = 1; iter != testList.end(); iter++, i++) {
            if (!oneTest || testNumber == i) {
               StatusPrint(i, (*iter)->GetName(), (*iter)->isTestAvailable() ? (*iter)->runTest() : -1, lineWidth);
            }
            delete *iter;
         }
      }
   }

   if (dryRun) {
      RooTrace::dump();
   }

   gBenchmark->Stop("stressHistfactory");


   //Print table with results
   Bool_t UNIX = strcmp(gSystem->GetName(), "Unix") == 0;
   cout << setw(lineWidth) << setfill('*') << "" << endl;
   if (UNIX) {
      TString sp = gSystem->GetFromPipe("uname -a");
      cout << "* SYS: " << sp << endl;
      if (strstr(gSystem->GetBuildNode(), "Linux")) {
         sp = gSystem->GetFromPipe("lsb_release -d -s");
         cout << "* SYS: " << sp << endl;
      }
      if (strstr(gSystem->GetBuildNode(), "Darwin")) {
         sp  = gSystem->GetFromPipe("sw_vers -productVersion");
         sp += " Mac OS X ";
         cout << "* SYS: " << sp << endl;
      }
   } else {
      const Char_t *os = gSystem->Getenv("OS");
      if (!os) cout << "*  SYS: Windows 95" << endl;
      else     cout << "*  SYS: " << os << " " << gSystem->Getenv("PROCESSOR_IDENTIFIER") << endl;
   }

   cout << setw(lineWidth) << setfill('*') << "" << endl;
   gBenchmark->Print("stressHistfactory");
#ifdef __CINT__
   Double_t reftime = 186.34; //pcbrun4 interpreted
#else
   Double_t reftime = 93.59; //pcbrun4 compiled
#endif
   const Double_t rootmarks = 860 * reftime / gBenchmark->GetCpuTime("stressHistfactory");

   cout << setw(lineWidth) << setfill('*') << "" << endl;
   cout << TString::Format("*  ROOTMARKS = %6.1f  *  Root %-8s %d/%d", rootmarks, gROOT->GetVersion(),
                           gROOT->GetVersionDate(), gROOT->GetVersionTime()) << endl;
   cout << setw(lineWidth) << setfill('*') << "" << endl;

   // NOTE: The function TStopwatch::CpuTime() calls Tstopwatch::Stop(), so you do not need to stop the timer separately.
   cout << "Time at the end of job = " << timer.CpuTime() << " seconds" << endl;

   if (fref) {
      fref->Close() ;
      delete fref ;
   }

   delete gBenchmark ;
   gBenchmark = 0 ;

   return 0;
}

//_____________________________batch only_____________________
#ifndef __CINT__

int main(int argc, const char *argv[])
{
   Bool_t doWrite     = kFALSE;
   Int_t  verbose     =      0;
   Bool_t allTests    = kFALSE;
   Bool_t oneTest     = kFALSE;
   Int_t testNumber   =      0;
   Bool_t dryRun      = kFALSE;

   string refFileName = "$ROOTSYS/test/stressHistfactory_ref.root" ;

   // Parse command line arguments
   for (Int_t i = 1 ;  i < argc ; i++) {
      string arg = argv[i] ;

      if (arg == "-f") {
         cout << "stressHistfactory: using reference file " << argv[i + 1] << endl ;
         refFileName = argv[++i] ;
      } else if (arg == "-w") {
         cout << "stressHistfactory: running in writing mode to update reference file" << endl ;
         doWrite = kTRUE ;
      } else if (arg == "-mc") {
         cout << "stressHistfactory: running in memcheck mode, no regression tests are performed" << endl;
         dryRun = kTRUE;
      } else if (arg == "-v") {
         cout << "stressHistfactory: running in verbose mode" << endl;
         verbose = 1;
      } else if (arg == "-vv") {
         cout << "stressHistfactory: running in very verbose mode" << endl;
         verbose = 2;
      } else if (arg == "-a") {
         cout << "stressHistfactory: deploying full suite of tests" << endl;
         allTests = kTRUE;
      } else if (arg == "-n") {
         cout << "stressHistfactory: running single test" << endl;
         oneTest = kTRUE;
         testNumber = atoi(argv[++i]);
      } else if (arg == "-d") {
         cout << "stressHistfactory: setting gDebug to " << argv[i + 1] << endl;
         gDebug = atoi(argv[++i]);
      } else if (arg == "-h") {
         cout << "usage: stressHistfactory [ options ] " << endl;
         cout << "" << endl;
         cout << "       -f <file> : use given reference file instead of default (" << refFileName << ")" << endl;
         cout << "       -w        : write reference file, instead of reading file and running comparison tests" << endl;
         cout << "       -n N      : only run test with sequential number N" << endl;
         cout << "       -a        : run full suite of tests (default is basic suite); this overrides the -n single test option" << endl;
         cout << "       -mc       : memory check mode, no regression test are performed. Set this flag when running with valgrind" << endl;
         cout << "       -vs       : use vector-based storage for all datasets (default is tree-based storage)" << endl;
         cout << "       -v/-vv    : set verbose mode (show result of each regression test) or very verbose mode (show all roofit output as well)" << endl;
         cout << "       -d N      : set ROOT gDebug flag to N" << endl ;
         cout << " " << endl ;
         return 0 ;
      }

   }

   // Disable caching of complex error function calculation, as we don't
   // want to write out the cache file as part of the validation procedure
   RooMath::cacheCERF(kFALSE) ;

   gBenchmark = new TBenchmark();
   stressHistfactory(refFileName.c_str(), doWrite, verbose, allTests, oneTest, testNumber, dryRun);
   return 0;
}

#endif
//Task for EEC in pp collisions
//Code inherited and modified from AliAnalysisTaskJetsEEC.cxx authored by Leticia Cunqueiro and Laura Havener
//Authors: Ananya Rai, Laura Havener
#include "AliAODMCHeader.h"
#include "AliAnalysisManager.h"
#include "AliAODInputHandler.h"
#include "AliInputEventHandler.h"
#include "AliEmcalJet.h"
#include "AliEmcalParticle.h"
#include "AliEmcalPythiaInfo.h"
#include "AliGenPythiaEventHeader.h"
#include "AliJetContainer.h"
#include "AliEmcalDownscaleFactorsOCDB.h"
#include "AliLog.h"
#include "AliMCEvent.h"
#include "AliParticleContainer.h"
#include "AliRhoParameter.h"
#include "AliVCluster.h"
#include "AliVTrack.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"
#include "TMatrixDSymEigen.h"
#include "TRandom3.h"
#include "TVector2.h"
#include "TVector3.h"
#include <AliAnalysisDataContainer.h>
#include <AliAnalysisDataSlot.h>
#include <TChain.h>
#include <TClonesArray.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <THnSparse.h>
#include <TKey.h>
#include <TList.h>
#include <TLorentzVector.h>
#include <TProfile.h>
#include <TSystem.h>
#include <TTree.h>
#include <TMath.h>
#include "AliAODEvent.h"
#include "AliAnalysisTaskJetsEEC.h"

using std::cout;
using std::endl;

ClassImp(AliAnalysisTaskJetsEEC)

//________________________________________________________________________
AliAnalysisTaskJetsEEC::AliAnalysisTaskJetsEEC(): AliAnalysisTaskEmcalJet("AliAnalysisTaskJetsEEC", kTRUE),
  fContainer(0), fMinFractionShared(0), fJetShapeType(kData),
  fJetShapeSub(kNoSub), fJetSelection(kInclusive), fPtThreshold(-9999.), fCentSelectOn(kTRUE), fCentMin(0), fCentMax(10),
  fOneConstSelectOn(kFALSE), fTrackCheckPlots(kFALSE), fCheckResolution(kFALSE),
  fMinPtConst(1), fHardCutoff(0), fDoTwoTrack(kFALSE), fCutDoubleCounts(kTRUE),
  fPowerAlgo(1), fPhiCutValue(0.02),
  fEtaCutValue(0.02), fDerivSubtrOrder(0),
  fStoreDetLevelJets(0), fStoreTrig(kFALSE), EEC_hist(0), jet_pt_hist(0), EEC_pt_hist(0), EEC_pt_hist_log(0), EEEC_hist(0), EEEC_pt_hist(0), EEEC_pt_hist_log(0)
{
  SetMakeGeneralHistograms(kTRUE);
  DefineOutput(1, TList::Class());
  DefineOutput(2, TTree::Class());
}


//________________________________________________________________________
AliAnalysisTaskJetsEEC::AliAnalysisTaskJetsEEC(const char *name): AliAnalysisTaskEmcalJet(name, kTRUE), fContainer(0),
    fMinFractionShared(0), fJetShapeType(kData), fJetShapeSub(kNoSub),
    fJetSelection(kInclusive), fPtThreshold(-9999.), fCentSelectOn(kTRUE), fCentMin(0), fCentMax(10),
    fOneConstSelectOn(kFALSE), fTrackCheckPlots(kFALSE), fCheckResolution(kFALSE),
    fMinPtConst(1), fHardCutoff(0), fDoTwoTrack(kFALSE), fCutDoubleCounts(kTRUE),
    fPowerAlgo(1), fPhiCutValue(0.02),
    fEtaCutValue(0.02), fDerivSubtrOrder(0),
    fStoreDetLevelJets(0), fStoreTrig(kFALSE), EEC_hist(0), jet_pt_hist(0), EEC_pt_hist(0), EEC_pt_hist_log(0), EEEC_hist(0), EEEC_pt_hist(0), EEEC_pt_hist_log(0)

{
  SetMakeGeneralHistograms(kTRUE);
  DefineOutput(1, TList::Class());
  DefineOutput(2, TTree::Class());
}


//________________________________________________________________________
AliAnalysisTaskJetsEEC::~AliAnalysisTaskJetsEEC() {
  // Destructor.
}


//________________________________________________________________________
void AliAnalysisTaskJetsEEC::UserCreateOutputObjects() {

// Create user output.
  AliAnalysisTaskEmcalJet::UserCreateOutputObjects();

  Bool_t oldStatus = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);
  TH1::AddDirectory(oldStatus);

  const char *nameoutput = GetOutputSlot(2)->GetContainer()->GetName(); //What does this line do? Can i comment it out

  EEC_hist = new TH1D("EEC_hist","EEC", 100, 0,1);//for now this is the binning
  fOutput->Add(EEC_hist);
  
  jet_pt_hist = new TH1D("jet_pt_hist", "Jet Pt", 70, 15,50);
//    jet_pt_hist = new TH1D("jet_pt_hist", "Jet Pt", 50, 0,25); //keep this same as 2D histogram
  fOutput->Add(jet_pt_hist);
    
  EEC_pt_hist = new TH2D("EEC_pt_hist", "EEC and jet_pt 2D", 100, 0,1,70,15,50);
  fOutput->Add(EEC_pt_hist);
    
  EEC_pt_hist_log = new TH2D("EEC_pt_hist_log", "EEC and jet_pt 2D", 100, -10,10,70,15,50);
  fOutput->Add(EEC_pt_hist_log);
  
 //EEEC histograms
  EEEC_hist = new TH1D("EEEC_hist","EEEC", 100, 0,1);//for now this is the binning
  fOutput->Add(EEEC_hist);
  
  EEEC_pt_hist = new TH2D("EEEC_pt_hist", "EEEC and jet_pt 2D", 100, 0,1,70,15,50);
  fOutput->Add(EEEC_pt_hist);
    
  EEEC_pt_hist_log = new TH2D("EEEC_pt_hist_log", "EEEC and jet_pt 2D", 100, -10,10,70,15,50);
  fOutput->Add(EEEC_pt_hist_log);



  PostData(1, fOutput);


}

//________________________________________________________________________
Bool_t AliAnalysisTaskJetsEEC::Run() {
  // Run analysis code here, if needed. It will be executed before
  // FillHistograms().
  return kTRUE;
}

//________________________________________________________________________
Bool_t AliAnalysisTaskJetsEEC::FillHistograms()
{
//fill histogram goes through event loops
  AliEmcalJet *jet1 = NULL;
  AliJetContainer *jetCont = GetJetContainer(0);
  // container zero is always the base containe: the data container, the
  // embedded subtracted in the case of embedding or the detector level in case
  // of pythia.
//Get jet container
    
  if (fCentSelectOn)
    if ((fCent > fCentMax) || (fCent < fCentMin))
      return 0;

  Float_t rhoVal = 0, rhoMassVal = 0.;
  if (jetCont)
  {
    jetCont->ResetCurrentID();
    if ((fJetShapeSub == kConstSub) || (fJetShapeSub == kDerivSub))
    {
      // rho
      AliRhoParameter *rhoParam = dynamic_cast<AliRhoParameter *>(
                                  InputEvent()->FindListObject("RhoSparseR020"));
      if (!rhoParam)
      {
        Printf("%s: Could not retrieve rho %s (some histograms will be filled "
               "with zero)!",
               GetName(), jetCont->GetRhoName().Data());
      }
      else
        rhoVal = rhoParam->GetVal();
      // rhom
      AliRhoParameter *rhomParam = dynamic_cast<AliRhoParameter *>(
                                   InputEvent()->FindListObject("RhoMassSparseR020"));
      if (!rhomParam)
      {
        Printf("%s: Could not retrieve rho_m %s (some histograms will be "
               "filled with zero)!",
               GetName(), jetCont->GetRhoMassName().Data());
      }
      else
        rhoMassVal = rhomParam->GetVal();
    }
    
//Keeping triggers for full jet analysis (if desired)
//     Int_t mytrig=-1;
//     Bool_t mytrigmb=kFALSE;
//     Bool_t mytrigej1=kFALSE;
//     Bool_t mytrigej2=kFALSE;
//     Double_t weightmb=-1;
//     Double_t weightej1=-1;
//     Double_t weightej2=-1;
//
//    if(fStoreTrig==kTRUE)
//    {
//    if(fInputHandler->IsEventSelected() & AliVEvent::kINT7 && fInputEvent->GetFiredTriggerClasses().Contains("INT7")) mytrigmb=kTRUE;
//    if(fInputHandler->IsEventSelected() & AliVEvent::kEMCEJE && fInputEvent->GetFiredTriggerClasses().Contains("EJ1")) mytrigej1=kTRUE;
//    if(fInputHandler->IsEventSelected() & AliVEvent::kEMCEJE && fInputEvent->GetFiredTriggerClasses().Contains("EJ2")) mytrigej2=kTRUE;
//    if(mytrigmb==kTRUE && mytrigej1==kTRUE && mytrigej2==kTRUE) mytrig=3;
//    if(mytrigmb==kTRUE && mytrigej1==kFALSE && mytrigej2==kFALSE) mytrig=0;
//    if(mytrigmb==kFALSE && mytrigej1==kTRUE && mytrigej2==kFALSE) mytrig=1;
//    if(mytrigmb==kFALSE && mytrigej1==kFALSE && mytrigej2==kTRUE) mytrig=2;
//    if(mytrigmb==kTRUE && mytrigej1==kTRUE && mytrigej2==kFALSE) mytrig=4;
//    if(mytrigmb==kTRUE && mytrigej1==kFALSE && mytrigej2==kTRUE) mytrig=5;
//    if(mytrigmb==kFALSE && mytrigej1==kTRUE && mytrigej2==kTRUE) mytrig=6;
//    if(mytrig==-1) return 0;
//    }
//
//  UInt_t newrun=InputEvent()->GetRunNumber();
//
//if(fStoreTrig==kTRUE)
//  {
//    RunChanged(newrun);
//      if(mytrig==3)
//    {
//      weightmb = 1./GetDownscaleWeight("INT7");
//        weightej1 = 1./GetDownscaleWeight("EJ1");
//      weightej2 = 1./GetDownscaleWeight("EJ2");
//    }
//      if(mytrig==0)
//    {
//   weightmb = 1./GetDownscaleWeight("INT7");
//    }
//      if(mytrig==1)
//      {
//          weightej1 = 1./GetDownscaleWeight("EJ1");
//      }
//      if(mytrig==2)
//      {
//          weightej2 = 1./GetDownscaleWeight("EJ2");
//      }
//      if(mytrig==4)
//      {
//      weightmb = 1./GetDownscaleWeight("INT7");
//      weightej1 = 1./GetDownscaleWeight("EJ1");
//      }
//      if(mytrig==5)
//      {
//    weightmb = 1./GetDownscaleWeight("INT7");
//    weightej2 = 1./GetDownscaleWeight("EJ2");
//      }
//
//      if(mytrig==6)
//      {
//      weightej1 = 1./GetDownscaleWeight("EJ1");
//      weightej2 = 1./GetDownscaleWeight("EJ2");
//      }
//  }

//Jet Loop
    while ((jet1 = jetCont->GetNextAcceptJet()))
    {
      if (!jet1)
        continue;
        if (jet1->Pt() < 15) //Cuts on jet_pt 
        {
            continue;
        }
        ComputeEEC(jet1, jetCont);//Computing the eec on the jet object

        
      AliEmcalJet *jet2 = 0x0;
      AliEmcalJet *jet3 = 0x0;

      AliEmcalJet *jetUS = NULL;
      Int_t ifound = 0, jfound = 0;
      Int_t ilab = -1, jlab = -1;

// All for MC. this is the mode to run over pythia to produce a det-part response. Here we have also added the constituent-subtraction case, but we don't use it normally in pp the matching is purely geometrical
      if (fJetShapeType == kPythiaDef)
      {AliJetContainer *jetContTrue = GetJetContainer(1);
        AliJetContainer *jetContUS = GetJetContainer(2);
        AliJetContainer *jetContPart = GetJetContainer(3);
        if (fJetShapeSub == kConstSub)
        {for (Int_t i = 0; i < jetContUS->GetNJets(); i++)
          {jetUS = jetContUS->GetJet(i);
            if (jetUS->GetLabel() == jet1->GetLabel())
            { ifound++;
              if (ifound == 1)
                ilab = i;}}
          if (ilab == -1)
            continue;
          jetUS = jetContUS->GetJet(ilab);
          jet2 = jetUS->ClosestJet();
          if (!jet2)
          {Printf("jet2 does not exist, returning");
            continue;}
          for (Int_t j = 0; j < jetContPart->GetNJets(); j++)
          {jet3 = jetContPart->GetJet(j);
            if (!jet3)
              continue;
            if (jet3->GetLabel() == jet2->GetLabel()){
              jfound++;
              if (jfound == 1)
                jlab = j;}          }
          if (jlab == -1)
            continue;
          jet3 = jetContPart->GetJet(jlab);
          if (!jet3)
          {Printf("jet3 does not exist, returning");
            continue;}}
        if (!(fJetShapeSub == kConstSub))
          jet3 = jet1->ClosestJet();
        if (!jet3)
        {//   Printf("jet3 does not exist, returning");
          continue;}}
      Double_t ptSubtracted = 0;
      if (fJetShapeSub == kConstSub || fJetShapeSub == kEventSub)
        ptSubtracted = jet1->Pt();
      else if (fJetShapeSub == kDerivSub)
      {
        ptSubtracted = jet1->Pt() - GetRhoVal(0) * jet1->Area();
      }
      else if (fJetShapeSub == kNoSub) ptSubtracted = jet1->Pt();
      if (ptSubtracted < fPtThreshold)
        continue;
      if ((fCentSelectOn == kFALSE) && (jet1->GetNumberOfTracks() <= 1))
        continue;


      Double_t ptMatch=0;
      Int_t kMatched = 0;

      if (fJetShapeType == kPythiaDef)
      {
        kMatched = 1;
        if (fJetShapeSub == kConstSub)
        kMatched = 3;
        ptMatch = jet3->Pt();
        cout<<"the matched jet "<<jet3->Pt()<<" "<<kMatched<<endl;
      }

    } //close while loop
  } //close the jet cont loop

  return kTRUE;
}



//________________________________________________________________________
int AliAnalysisTaskJetsEEC::GetConstituentID(int constituentIndex, const AliVParticle* part, AliEmcalJet * jet)
{
  // NOTE: Usually, we would use the global offset defined for the general subtracter extraction task. But we don't want to
  //       depend on that task, so we just define it here locally.
  int id = part->GetLabel() != -1 ? part->GetLabel() : (jet->TrackAt(constituentIndex) + 2000000);
  return id;
}


//_______________________________________________________________________
Double_t AliAnalysisTaskJetsEEC::GetDownscaleWeight(string trigString)
{
  Double_t weight = 1.;
  TString triggerclass;
  if(trigString == "INT7") triggerclass = "CINT7-B-NOPF-CENT";
  else if(trigString == "EJ1") triggerclass = "CEMC7EJ1-B-NOPF-CENTNOTRD";
  else if(trigString == "EJ2") triggerclass = "CEMC7EJ2-B-NOPF-CENT";
  if(triggerclass.Length()) weight = PWG::EMCAL::AliEmcalDownscaleFactorsOCDB::Instance()->GetDownscaleFactorForTriggerClass(triggerclass);
  return weight;
}

    
//EEC computation-------------------------------------------------------
void AliAnalysisTaskJetsEEC::ComputeEEC(AliEmcalJet *fJet, AliJetContainer *fJetCont)
    //(jet, jet container, vector of jets, vector of constituents within each jet?)
{
//General EEC computation: Need to loop over jets, then loop within a single jet to compute EECs.
//NOTE: Already in the jet loop defined in the Fill Histogram function. This puts you in the jet loop. The event loop stuff is being taken care of by AliAnalysis manager
//fjet is my single jet. AliEmCalJet is pointing to the fJet object, figure out what arguments you need for computing eec -- you need the jet or a vector of jets for each event

  std::vector<fastjet::PseudoJet> fConstituents; //Is a pseudojet object with constituents of the jet
  fConstituents.clear();
//This snippet of code is getting particles within a single jet (fjet) and turning them into pseudojet objects so that fastjet capabilities can be used
  fastjet::PseudoJet PseudoTracks; //Creating a pseudojet object called PseduoTracks
  unsigned int constituentIndex = 0;
//The line below gets constituent particles within fjet. C++ syntax[ for (auto elem : container)    // capture elements by value ]
  for (auto part: fJet->GetParticleConstituents())
  {
    PseudoTracks.reset(part.Px(), part.Py(), part.Pz(), part.E()); //part is the constituent at that point in the loop, part keeps getting redefined in each step.
    const AliVParticle* part2 = part.GetParticle(); //"hack", leave this in , to get the index of the jet from AliPhysics
    PseudoTracks.set_user_index(GetConstituentID(constituentIndex, part2, fJet)); //leave this in for the same reason as above
    fConstituents.push_back(PseudoTracks);
    constituentIndex++;
  }

//Initializing objects
    std::vector<Double_t> delta_Rvec;
    std::vector<Double_t> energy_pairs_vec; //the weighting vector with EE
    std::vector<Double_t> energy_pairs_tri; //the weighting vector with EEE
    std::vector<Double_t> R_dist;
    std::vector<Double_t> logR_dist;
    std::vector<Double_t> max_R_distvec;
    std::vector<Double_t> max_logR_distvec;

//Looping over the jet
    double jet_pt = fJet->Pt();
    jet_pt_hist->Fill(jet_pt); //filling histogram with momentum of jets
//    constit = fConstitutents.constituents(); //vector of single jet constituents. Using pseudojet jet.constituent
    for(int j=0; j<int(fConstituents.size()); j++)  //looping over constituents of the fConstituents object
        {
            for(int s=0; s<j ; s++)
            {
//For 3 point correlator
                for( int m=0; m<j; m++)
                   {
                   double eee_jsm = ((fConstituents[j].pt()*fConstituents[s].pt()*fConstituents[m].pt())/(pow(jet_pt,3)));
                   double deltaR_js = fConstituents[j].delta_R(fConstituents[s]);
                   double delta_logR_js = log(deltaR_js);

                   double deltaR_jm = fConstituents[j].delta_R(fConstituents[m]);
                   double delta_logR_jm = log(deltaR_jm);

                   double deltaR_sm = fConstituents[s].delta_R(fConstituents[m]);
                   double delta_logR_sm = log(deltaR_sm);

                energy_pairs_tri.push_back(eee_jsm);
//                jetE.push_back(jet_pt);

                R_dist.push_back(deltaR_js);
                R_dist.push_back(deltaR_jm);
                R_dist.push_back(deltaR_sm);

                logR_dist.push_back(delta_logR_js);
                logR_dist.push_back(delta_logR_jm);
                logR_dist.push_back(delta_logR_sm);

                int max_R = distance(R_dist.begin(), max_element(R_dist.begin(), R_dist.end()));//pick the longest side to compute the correlators with

                max_R_distvec.push_back(R_dist[max_R]);
                max_logR_distvec.push_back(logR_dist[max_R]);
                
                EEEC_hist->Fill(R_dist[max_R],eee_jsm);
                EEEC_pt_hist->Fill(R_dist[max_R],jet_pt,eee_jsm);
                EEEC_pt_hist_log->Fill(logR_dist[max_R],jet_pt,eee_jsm);
                
                R_dist.clear();
                logR_dist.clear();
                   }//close m loop
            
//For 2 point correlator

                double delta_R_js = fConstituents[j].delta_R(fConstituents[s]);
                double log_delta_R_js = log(delta_R_js);
                double ee_js = (fConstituents[j].pt()*fConstituents[s].pt())/(pow((jet_pt),2));

//Filling the vectors
                delta_Rvec.push_back(delta_R_js);
                energy_pairs_vec.push_back(ee_js);
                EEC_hist->Fill(delta_R_js,ee_js);
                EEC_pt_hist->Fill(delta_R_js,jet_pt,ee_js);
                EEC_pt_hist_log->Fill(log_delta_R_js,jet_pt,ee_js);
////                fTreeEEC->Fill();
                }//close s loop
             } //close j loop


//catch (fastjet::Error)
// {
//    AliError(" [w] FJ Exception caught.");
//    // return -1;
// } //end error message
  return;
}


//_________________________________________________________________
void AliAnalysisTaskJetsEEC::RunChanged(Int_t newrun)
{
  if(fStoreTrig)
  {
    auto downscalehandler = PWG::EMCAL::AliEmcalDownscaleFactorsOCDB::Instance();
    if(downscalehandler->GetCurrentRun() != newrun)
    {
      downscalehandler->SetRun(newrun);
    }
  }
}


//________________________________________________________________________
Bool_t AliAnalysisTaskJetsEEC::RetrieveEventObjects()
{
  //
  // retrieve event objects
  //
  if (!AliAnalysisTaskEmcalJet::RetrieveEventObjects())
    return kFALSE;
  return kTRUE;
}


//_______________________________________________________________________
void AliAnalysisTaskJetsEEC::Terminate(Option_t *)
{
  // Called once at the end of the analysis.
  // fTreeObservableTagging = dynamic_cast<TTree*>(GetOutputData(1));
  // if (!fTreeObservableTagging){
  //   Printf("ERROR: fTreeObservableTagging not available");
  //   return;
  // }
}

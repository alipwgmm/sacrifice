#include "UserHookUtils.hh"
#include "Sacrifice/UserHooksFactory.hh"
#include <stdexcept>
#include <iostream>

namespace Pythia8{
  class PoWHEGVetoedShower;
}

Sacrifice::UserHooksFactory::Creator<Pythia8::PoWHEGVetoedShower> powhegVetoedShowerCreator("PoWHEGVetoedShower");

namespace Pythia8{

  /**
   *  This UserHook should be used when showering QCD jets generated with PoWHEG-box.
   *  The following settings must be activated in Pythia:
   *
   *    SpaceShower:pTMaxMatch = 2
   *    TimeShower:pTMaxMatch  = 2
   *
   *  These set the limit for emission from both initial and final state radiation to the kinematic (beam) energy.
   *  The veto hook then vetos any emission that would be above the appropriate scale.
   *
   *  This Hook determines a new (lower) veto scale according to the following prescription: 
   *  Compute the pT of every PoWHEG leg relative to the beam, and the pT of every outgoing PoWHEG leg relative to all
   *  the other legs in the CMS frame.  Use the lowest of these values as the new veto scale
   *
   *  This hook is ONLY SUITABLE FOR QCD JET PRODUCTION!!
   */
  class PoWHEGVetoedShower : public UserHooks{

  public:

    PoWHEGVetoedShower(): m_powhegScale(0.){

      std::cout<<"*******************************************************************"<<std::endl;
      std::cout<<"*                                                                 *"<<std::endl;
      std::cout<<"*  Using new PoWHEG author scale definition for QCD production!   *"<<std::endl;
      std::cout<<"*                                                                 *"<<std::endl;
      std::cout<<"*******************************************************************"<<std::endl;
      
    }

    ~PoWHEGVetoedShower(){}

    /**
     *  doVetoMPIStep is called immediately after the MPI generation
     *  In this case it never actually vetoes the MPI, but since it is called
     *  before the ISR veto check this is a convenient place to find the PoWHEG
     *  scale from the LHEF event
     */
    bool doVetoMPIStep(int nMPI, const Event &evt){

      // Only do anything on the first call.
      if(nMPI > 1) return false;
      
      m_powhegScale = infoPtr->QRen();;
      
      // momentum components to boost to CMS frame
      double pxCMS = 0.;
      double pyCMS = 0.;
      double pzCMS = 0.;
      double eCMS  = 0.;
      
      vector<Particle> powhegLegs;
      
      // Find the entries corresponding to outgoing legs from PoWHEG
      // start the loop at 1, since entry 0 represents the event as a whole
      for(int ii=1; ii != evt.size(); ++ii){

        // status -21 is the incoming hard partons
        if(evt[ii].status() == -21){
          pxCMS += evt[ii].px();
          pyCMS += evt[ii].py();
          pzCMS += evt[ii].pz();   
          eCMS  += evt[ii].e();
        }
        
        // here Event::isFinal refers to whether the particle can still decay/radiate, or whether it is an internal leg
        if(evt[ii].isFinal()){
          powhegLegs.push_back(Particle(evt[ii]));
        }
      }
      
      // compare the pT of each leg to the powheg scale.
      // Set the scale to the lowest (or leave the scale unchanged if it is already lower)
      for(vector<Particle>::const_iterator leg=powhegLegs.begin();
          leg != powhegLegs.end(); ++leg){
        if(leg->pT() < m_powhegScale )m_powhegScale = leg->pT();
      }
      
      double norm = -1./eCMS;
      pxCMS *= norm;
      pyCMS *= norm;
      pzCMS *= norm;
      
      // boost all outgoing legs to the CMS frame
      for(vector<Particle>::iterator leg=powhegLegs.begin();
          leg != powhegLegs.end(); ++leg){
        leg->bst(pxCMS, pyCMS, pzCMS);
      }
      
      for(vector<Particle>::const_iterator leg=powhegLegs.begin();
          leg != powhegLegs.end(); ++leg){
        
        if(leg->pT() < m_powhegScale )m_powhegScale = leg->pT();
        
        // calculate the pT relative to each other leg 
        // if any such pT is lower than the current scale, reset the scale to that value
        for(vector<Particle>::const_iterator otherLeg = powhegLegs.begin();
            otherLeg != powhegLegs.end(); ++otherLeg){
          if(otherLeg == leg) continue;
          
          double pTLeg = Sacrifice::pTLeg(*leg, *otherLeg);

          if(pTLeg < m_powhegScale) m_powhegScale = pTLeg;
        }
      }

//      std::cout<<m_powhegScale<<"  "<<infoPtr->QRen()<<std::endl;

      
      return false;
    }

    /**
     *  This is called after the generation of each new ISR emission
     *  Can use it to test if the last generated emission is above the
     *  veto scale
     *
     */
    bool doVetoISREmission(int, const Event &evt, int iSys){

      // only veto emissions from the hard system
      if(iSys != 0) return false;
      
      size_t emission = Sacrifice::findLastISREmission(evt);

      // Veto emission above the veto scale
      if(evt[emission].pT() > m_powhegScale) return true;
      
      return false;
    }
    /**
     *  This is similar to the ISR veto
     *
     */
    bool doVetoFSREmission(int, const Event &evt, int iSys, bool){

      // only veto emissions from the hard system
      if(iSys != 0) return false;
      
      size_t emission = Sacrifice::findLastFSREmission(evt);

      // Veto if above the POWHEG scale
      if (evt[emission].pT() > m_powhegScale) return true;
      
      return false;
    }

    /// Switch on calling of doVetoMPIStep
    bool canVetoMPIStep(){return true;}
    /// Call doVetoMIStep once
    int numberVetoMPIStep(){return 1;}
    /// Switch on veto of ISR
    bool canVetoISREmission(){return true;}
    /// Switch off veto of FSR
    bool canVetoFSREmission(){return true;}

  private:

    double m_powhegScale;
    
  };
}


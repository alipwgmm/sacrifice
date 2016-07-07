#include "UserHookUtils.hh"
#include "Sacrifice/UserHooksFactory.hh"
#include "boost/lexical_cast.hpp"
#include <stdexcept>
#include <iostream>

namespace Pythia8{
  class PTRel_boostVetoedShower;
}

Sacrifice::UserHooksFactory::Creator<Pythia8::PTRel_boostVetoedShower> pTRel_boostVetoedShowerCreator("PTRel_boostVetoedShower");

namespace Pythia8{
  
 /**
  *  This user hook is derived from the main31 example distributed with Pythia 8 and
  *  is for use with QCD events generated with PoWHEG.  The following settings must be
  *  used in Pythia:
  *
  *    SpaceShower:pTMaxMatch = 2
  *    TimeShower:pTMaxMatch  = 2
  *
  *  These set the limit for emission from both initial and final state radiation to the kinematic (beam) energy.
  *  The veto hook then vetos any emission that would be above the appropriate scale.
  *
  */
 
  class PTRel_boostVetoedShower : public UserHooks{
    
  public:
    
    PTRel_boostVetoedShower(): m_nPoWHEGFinal(2), m_powhegScale(0.){
      
      std::cout<<"*******************************************************************************"<<std::endl;
      std::cout<<"*                                                                             *"<<std::endl;
      std::cout<<"*        Using PTRel_boost vetoed shower for PoWHEG QCD production!           *"<<std::endl;
      std::cout<<"*                                                                             *"<<std::endl;
      std::cout<<"*******************************************************************************"<<std::endl;
      
    }
    
    ~PTRel_boostVetoedShower(){}
    
    /**
     *  doVetoMPIStep is called immediately after the MPI generation
     *  In this case it never actually vetoes the MPI, but since it is called
     *  before the ISR veto check this is a convenient place to find the PoWHEG
     *  scale from the LHEF event
     */
    bool doVetoMPIStep(int nMPI, const Event &evt){
      
      // Only do anything on the first call.
      if(nMPI > 1) return false;
      
      m_powhegScale = infoPtr->QRen();

      size_t nPwgOutgoing = 0;
      
      for(int ii = evt.size()-1; ii > 0; --ii){
        if(! (evt[ii].isFinal())) break;
        ++nPwgOutgoing;
      }
      
      if(nPwgOutgoing == m_nPoWHEGFinal) return false;
      
      if(nPwgOutgoing != m_nPoWHEGFinal + 1){
        throw std::runtime_error("Wrong number of final state PoWHEG legs: " + boost::lexical_cast<string>(nPwgOutgoing));
      }
      
      // momentum components to boost to CMS frame
      m_pxCMS = 0.;
      m_pyCMS = 0.;
      m_pzCMS = 0.;
      double eCMS  = 0.;
      
      // The outgoing powheg legs
      vector<Particle> powhegLegs;
      
      // Find the entries corresponding to outgoing legs from PoWHEG
      // start the loop at 1, since entry 0 represents the event as a whole
      for(int ii=1; ii != evt.size(); ++ii){
        
        // status -21 is the incoming hard partons
        if(evt[ii].status() == -21){
          m_pxCMS += evt[ii].px();
          m_pyCMS += evt[ii].py();
          m_pzCMS += evt[ii].pz();   
          eCMS  += evt[ii].e();
        }
        
        // here Event::isFinal refers to whether the particle can still decay/radiate, or whether it is an internal leg
        if(evt[ii].isFinal()){
          powhegLegs.push_back(Particle(evt[ii]));
        }
      }
      
      // Start the search for the powheg scale above the collision energy, 
      // which is guaranteed to be above the veto scale.
      
      m_powhegScale = 1.1*infoPtr->s();
      
      // compare the pT of each leg to the powheg scale.
      // Set the scale to the lowest
      for(vector<Particle>::const_iterator leg=powhegLegs.begin();
          leg != powhegLegs.end(); ++leg){
        double pTTmp = leg->pT();
        if(pTTmp < m_powhegScale )m_powhegScale = pTTmp;
      }
      
      // normalise the boost vector to the CMS frame...
      double norm = -1./eCMS;
      m_pxCMS *= norm;
      m_pyCMS *= norm;
      m_pzCMS *= norm;
      
      // ...and boost all outgoing legs to that frame
      for(vector<Particle>::iterator leg=powhegLegs.begin();
          leg != powhegLegs.end(); ++leg){
        leg->bst(m_pxCMS, m_pyCMS, m_pzCMS);
      }
      
      for(vector<Particle>::const_iterator leg=powhegLegs.begin();
          leg != powhegLegs.end(); ++leg){
        // calculate the pT relative to each other leg 
        // if any such pT is lower than the current scale, reset the scale to that value
        for(vector<Particle>::const_iterator otherLeg = powhegLegs.begin();
            otherLeg != powhegLegs.end(); ++otherLeg){
          if(otherLeg == leg) continue;
          
//          double pTLeg = Sacrifice::pTLeg(*leg, *otherLeg);

          double pTLeg = Sacrifice::pTPowheg(*leg, *otherLeg);

          
          if(pTLeg < m_powhegScale) m_powhegScale = pTLeg;
        }
      }
      
      // barf if the scale was not found properly!
      if(m_powhegScale > infoPtr->s()){
        throw std::runtime_error("Veto scale could not be determined!");
      }
      
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
      
      if(evt[emission].pT() > m_powhegScale) return true;
      
//      size_t radiator = Sacrifice::findLastISRRadiator(evt);
      
      return false;
    }
    
    /**
     *  This is similar to the ISR veto
     *
     */
    bool doVetoFSREmission(int, const Event &evt, int iSys, bool){
      if(iSys != 0) return false;
      
      size_t emissionIndex = Sacrifice::findLastFSREmission(evt);
      size_t radiatorIndex = Sacrifice::findLastFSRRadiator(evt);
      
      Particle emission(evt[emissionIndex]);
      Particle radiator(evt[radiatorIndex]);
      
      int iInA = partonSystemsPtr->getInA(0);
      int iInB = partonSystemsPtr->getInB(0);
      double betaZ = - ( evt[iInA].pz() + evt[iInB].pz() ) /
      ( evt[iInA].e()  + evt[iInB].e()  );
      
//      emission.bst(m_pxCMS, m_pyCMS, m_pzCMS);
//      radiator.bst(m_pxCMS, m_pyCMS, m_pzCMS);

      emission.bst(0., 0., betaZ);
      radiator.bst(0., 0., betaZ);
      
//      double pTRel = Sacrifice::pTLeg(emission, radiator);
      double pTRel = Sacrifice::pTPowheg(emission, radiator);

      if(pTRel > m_powhegScale) return true;
      
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
    
    const size_t m_nPoWHEGFinal;
    
    double m_powhegScale;
    
    // defines the boost vector to the CMS frame
    double m_pxCMS;
    double m_pyCMS;
    double m_pzCMS;
    
  };
}


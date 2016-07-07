#pragma once

#include "MCUtils/MathUtils.h"
#include "MCUtils/Utils.h"
#include <sstream>
#include <iostream>
#include <cmath>

/// @file Physics vectors stuff
/// @author Andy Buckley <andy.buckley@cern.ch>

namespace MCUtils {


  /// @brief A robust 4-momentum class for on-shell vectors.
  ///
  /// P4 is a typical 4-momentum class, cf. HepLorentzVector or TLorentzVector
  /// with the exception that the data member storage is specifically based on
  /// (px, py, pz, m) rather than (px, py, pz, E). This means that there are (or
  /// at least should) never be numerical precision problems due to calculations
  /// like m^2 = E^2 - p^2 when p^2 >> m^2. In this form, m^2 is always
  /// well-defined and the "equivalent" calculation is the numerically safe E^2
  /// = p^2 + m^2.
  ///
  /// This design restricts usage to on-shell vectors... which as far as I'm
  /// aware is not really a restriction in practice, at last not for physical
  /// particles, but please let me know if it is! (Off-shell vectors require a
  /// 5th component so the spatial and time components can be inconsistent with
  /// the mass.)
  ///
  class P4 {
  private:

    /// @name Storage
    //@{
    double _x, _y, _z, _m;
    //@}


  public:

    /// @name Constructors etc.
    //@{

    /// Default constructor of a null vector
    P4()
      : _x(0), _y(0), _z(0), _m(0) {  }

    /// Constructor from Cartesian/Minkowski coordinates
    P4(double px, double py, double pz, double E) {
      setPE(px, py, pz, E);
    }

    /// Copy constructor
    P4(const P4& v)
      : _x(v._x), _y(v._y), _z(v._z), _m(v._m) {  }

    /// Copy assignment operator
    P4& operator = (const P4& v) {
      _x = v._x;
      _y = v._y;
      _z = v._z;
      _m = v._m;
      return *this;
    }

    /// Set the components to zero
    void clear() {
      _x = 0;
      _y = 0;
      _z = 0;
      _m = 0;
    }

    //@}


    /// @name Static methods for vector making
    //@{

    /// Make a vector from (px,py,pz,E) coordinates
    static P4 mkXYZE(double px, double py, double pz, double E) {
      return P4().setPE(px, py, pz, E);
    }

    /// Make a vector from (px,py,pz) coordinates and the mass
    static P4 mkXYZM(double px, double py, double pz, double mass) {
      return P4().setPM(px, py, pz, mass);
    }

    /// Make a vector from (eta,phi,energy) coordinates and the mass
    static P4 mkEtaPhiME(double eta, double phi, double mass, double E) {
      return P4().setEtaPhiME(eta, phi, mass, E);
    }

    /// Make a vector from (eta,phi,pT) coordinates and the mass
    static P4 mkEtaPhiMPt(double eta, double phi, double mass, double pt) {
      return P4().setEtaPhiMPt(eta, phi, mass, pt);
    }

    /// Make a vector from (y,phi,energy) coordinates and the mass
    static P4 mkRapPhiME(double y, double phi, double mass, double E) {
      return P4().setRapPhiME(y, phi, mass, E);
    }

    /// Make a vector from (y,phi,pT) coordinates and the mass
    static P4 mkRapPhiMPt(double y, double phi, double mass, double pt) {
      return P4().setRapPhiMPt(y, phi, mass, pt);
    }

    /// Make a vector from (theta,phi,energy) coordinates and the mass
    static P4 mkThetaPhiME(double theta, double phi, double mass, double E) {
      return P4().setThetaPhiME(theta, phi, mass, E);
    }

    /// Make a vector from (theta,phi,pT) coordinates and the mass
    static P4 mkThetaPhiMPt(double theta, double phi, double mass, double pt) {
      return P4().setThetaPhiMPt(theta, phi, mass, pt);
    }

    /// Make a vector from (pT,phi,energy) coordinates and the mass
    static P4 mkPtPhiME(double pt, double phi, double mass, double E) {
      return P4().setPtPhiME(pt, phi, mass, E);
    }

    //@}


    /// @name Coordinate setters
    //@{

    /// Set the px coordinate
    P4& setPx(double px) {
      _x = px;
      return *this;
    }

    /// Set the py coordinate
    P4& setPy(double py) {
      _y = py;
      return *this;
    }

    /// Set the pz coordinate
    P4& setPz(double pz) {
      _z = pz;
      return *this;
    }

    /// Set the mass
    P4& setM(double mass) {
      assert(mass >= 0);
      _m = mass;
      return *this;
    }

    /// Set the p coordinates and mass simultaneously
    P4& setPM(double px, double py, double pz, double mass) {
      assert(mass >= 0);
      setPx(px); setPy(py); setPz(pz);
      setM(mass);
      return *this;
    }
    /// Alias for setPM
    P4& setXYZM(double px, double py, double pz, double mass) {
      return setPM(px, py, pz, mass);
    }

    /// Set the p coordinates and energy simultaneously
    P4& setPE(double px, double py, double pz, double E) {
      assert(E >= 0);
      setPx(px); setPy(py); setPz(pz);
      const double mass = sqrt( sqr(E) - sqr(p()) );
      setM(mass);
      return *this;
    }
    /// Alias for setPE
    P4& setXYZE(double px, double py, double pz, double E) {
      return setPE(px, py, pz, E);
    }

    /// Set the vector state from (eta,phi,energy) coordinates and the mass
    ///
    /// eta = -ln(tan(theta/2))
    /// -> theta = 2 atan(exp(-eta))
    P4& setEtaPhiME(double eta, double phi, double mass, double E) {
      assert(mass >= 0);
      assert(E >= 0);
      const double theta = 2 * atan(exp(-eta));
      assert(theta >= 0 && theta <= M_PI);
      setThetaPhiME(theta, phi, mass, E);
      return *this;
    }

    /// Set the vector state from (eta,phi,pT) coordinates and the mass
    ///
    /// eta = -ln(tan(theta/2))
    /// -> theta = 2 atan(exp(-eta))
    P4& setEtaPhiMPt(double eta, double phi, double mass, double pt) {
      assert(mass >= 0);
      assert(pt >= 0);
      const double theta = 2 * atan(exp(-eta));
      assert(theta >= 0 && theta <= M_PI);
      const double p = pt / sin(theta);
      const double E = sqrt( sqr(p) + sqr(mass) );
      setThetaPhiME(theta, phi, mass, E);
      return *this;
    }

    /// Set the vector state from (y,phi,energy) coordinates and the mass
    ///
    /// y = 0.5 * ln((E+pz)/(E-pz))
    /// -> (E^2 - pz^2) exp(2y) = (E+pz)^2
    ///  & (E^2 - pz^2) exp(-2y) = (E-pz)^2
    /// -> E = sqrt(pt^2 + m^2) cosh(y)
    /// -> pz = sqrt(pt^2 + m^2) sinh(y)
    /// -> sqrt(pt^2 + m^2) = E / cosh(y)
    P4& setRapPhiME(double y, double phi, double mass, double E) {
      assert(mass >= 0);
      assert(E >= 0);
      const double sqrt_pt2_m2 = E / cosh(y);
      const double pt = sqrt( sqr(sqrt_pt2_m2) - sqr(mass) );
      assert(pt >= 0);
      const double pz = sqrt_pt2_m2 * sinh(y);
      const double px = pt * cos(phi);
      const double py = pt * sin(phi);
      setPE(px, py, pz, E);
      return *this;
    }

    /// Set the vector state from (y,phi,pT) coordinates and the mass
    ///
    /// y = 0.5 * ln((E+pz)/(E-pz))
    /// -> E = sqrt(pt^2 + m^2) cosh(y)  [see above]
    P4& setRapPhiMPt(double y, double phi, double mass, double pt) {
      assert(mass >= 0);
      assert(pt >= 0);
      const double E = sqrt( sqr(pt) + sqr(mass) ) * cosh(y);
      assert(E >= 0);
      setRapPhiME(y, phi, mass, E);
      return *this;
    }

    /// Set the vector state from (theta,phi,energy) coordinates and the mass
    ///
    /// p = sqrt(E^2 - mass^2)
    /// pz = p cos(theta)
    /// pt = p sin(theta)
    P4& setThetaPhiME(double theta, double phi, double mass, double E) {
      assert(theta >= 0 && theta <= M_PI);
      assert(mass >= 0);
      assert(E >= 0);
      const double p = sqrt( sqr(E) - sqr(mass) );
      const double pz = p * cos(theta);
      const double pt = p * sin(theta);
      assert(pt >= 0);
      const double px = pt * cos(phi);
      const double py = pt * sin(phi);
      setPE(px, py, pz, E);
      return *this;
    }

    /// Set the vector state from (theta,phi,pT) coordinates and the mass
    ///
    /// p = pt / sin(theta)
    /// pz = p cos(theta)
    /// E = sqrt(p^2 + mass^2)
    P4& setThetaPhiMPt(double theta, double phi, double mass, double pt) {
      assert(theta >= 0 && theta <= 2*M_PI);
      assert(pt >= 0);
      assert(mass >= 0);
      const double p = pt / sin(theta);
      const double px = pt * cos(phi);
      const double py = pt * sin(phi);
      const double pz = p * cos(theta);
      const double E = sqrt( sqr(p) + sqr(mass) );
      setPE(px, py, pz, E);
      return *this;
    }

    /// Set the vector state from (pT,phi,energy) coordinates and the mass
    ///
    /// pz = sqrt(E^2 - mass^2 - pt^2)
    P4& setPtPhiME(double pt, double phi, double mass, double E) {
      assert(pt >= 0);
      assert(mass >= 0);
      assert(E >= 0);
      const double px = pt * cos(phi);
      const double py = pt * sin(phi);
      const double pz = sqrt(sqr(E) - sqr(mass) - sqr(pt));
      setPE(px, py, pz, E);
      return *this;
    }

    //@}


    /// @name Coordinate getters
    //@{

    /// Get px^2
    double px2() const { return sqr(_x); }
    /// Get px
    double px() const { return _x; }
    /// Get py^2
    double py2() const { return sqr(_y); }
    /// Get py
    double py() const { return _y; }
    /// Get pz^2
    double pz2() const { return sqr(_z); }
    /// Get pz
    double pz() const { return _z; }
    /// Get m^2
    double m2() const { return sqr(_m); }
    /// Get m
    double m() const { return _m; }

    /// Get E^2
    double E2() const { return p2() + sqr(_m); }
    /// Get E
    double E() const { return sqrt(E2()); }
    /// Get the spatial 3-vector |p|^2
    double p2() const { return sqr(px()) + sqr(py()) + sqr(pz()); }
    /// Get the spatial 3-vector |p|
    double p() const { return sqrt(p2()); }
    /// Get the spatial 3-vector |px^2 + py^2|
    double rho2() const { return sqr(px()) + sqr(py()); }
    /// Get the spatial 3-vector sqrt|px^2 + py^2|
    double rho() const { return sqrt(rho2()); }
    /// Get the transverse momentum squared (same as rho2)
    double pT2() const { return rho2(); }
    /// Get the transverse momentum (same as rho)
    double pT() const { return rho(); }

    /// Get the spatial phi
    double phi() const { if (rho2() == 0) return 0; else return atan2(py(),px()); }
    /// Get the spatial theta
    double theta() const { if (p2() == 0) return 0; else if (pz() == 0) return M_PI; else return atan2(rho(),pz()); }
    /// Get the spatial vector pseudorapidity
    double eta() const { return -log(tan( 0.5 * theta() )); } //< Optimise with a trig reln on tan(x/2) to avoid tan(atan(..)/2)?
    /// Get the 4-momentum rapidity
    double rap() const { return 0.5 * (E() + pz()) / (E() - pz()); }

    //@}


    /// @name Calculations w.r.t. other P4 vectors
    //@{

    /// Lorentz dot product with the positive metric term on E
    double dot(const P4& v) const { return E()*v.E() - px()*v.px() - py()*v.py() - pz()*v.pz(); }
    /// Spatial angle to another P4 vector
    double angleTo(const P4& v) const { return acos(px()*v.px() + py()*v.py() + pz()*v.pz()) / ( p()*v.p() ); }
    /// Difference in phi between two vectors
    double deltaPhi(const P4& v) const { return deltaphi(phi(), v.phi()); }
    /// Difference in pseudorapidity between two vectors
    double deltaEta(const P4& v) const { return fabs(eta() - v.eta()); }
    /// Difference in rapidity between two vectors
    double deltaRap(const P4& v) const { return fabs(rap() - v.rap()); }
    /// Difference in pseudorapidity-based R^2 between two vectors
    double deltaR2_eta(const P4& v) const { return sqr(deltaEta(v)) + sqr(deltaPhi(v)); }
    /// Difference in pseudorapidity-based R between two vectors
    double deltaR_eta(const P4& v) const { return sqrt(deltaR2_eta(v)); }
    /// Difference in rapidity-based R^2 between two vectors
    double deltaR2_rap(const P4& v) const { return sqr(deltaRap(v)) + sqr(deltaPhi(v)); }
    /// Difference in rapidity-based R between two vectors
    double deltaR_rap(const P4& v) const { return sqrt(deltaR2_rap(v)); }

    //@}

    /// @name Self-modifying operators
    //@{
    P4  operator - () const { P4 rtn; return rtn.setPM(-_x, -_y, -_z, _m); } //< Not self-modifying...
    P4& operator += (const P4& v) { double e = E() + v.E(); _x += v.px(); _y += v.py(); _z += v.pz(); _m = sqrt( sqr(e) - p2() ); return *this; }
    P4& operator -= (const P4& v) { double e = E() - v.E(); _x -= v.px(); _y -= v.py(); _z -= v.pz(); _m = sqrt( sqr(e) - p2() ); return *this; }
    P4& operator *= (double a) { _x *= a; _y *= a; _z *= a; _m *= a; return *this; }
    P4& operator /= (double a) { _x /= a; _y /= a; _z /= a; _m /= a; return *this; }
    //@}

  };


  /// @name String representations
  //@{

  /// Make a string representation of the vector
  inline std::string to_str(const P4& p4) {
    std::stringstream ss;
    ss << "(" << p4.px() << ", " << p4.py() << ", " << p4.pz() << "; " << p4.E() << ")";
    return ss.str();
  }

  /// Write a string representation of the vector to the provided stream
  inline std::ostream& operator <<(std::ostream& ostr, const P4& p4) {
    ostr << to_str(p4);
    return ostr;
  }

  //@}


  /// Convenience "external" functions
  //@{

  /// Lorentz inner product between two vectors
  inline double dot(const P4& a, const P4& b) {
    return a.dot(b);
  }


  /// Angle between two vectors
  inline double angle(const P4& a, const P4& b) {
    return a.angleTo(b);
  }


  /// Difference in phi between two vectors
  inline double deltaPhi(const P4& a, const P4& b) {
    return a.deltaPhi(b);
  }


  /// Difference in pseudorapidity between two vectors
  inline double deltaEta(const P4& a, const P4& b) {
    return a.deltaEta(b);
  }


  /// Difference in rapidity between two vectors
  inline double deltaRap(const P4& a, const P4& b) {
    return a.deltaRap(b);
  }


  /// Difference in pseudorapidity-based R^2 between two vectors
  inline double deltaR2_eta(const P4& a, const P4& b) {
    return a.deltaR2_eta(b);
  }
  /// Difference in pseudorapidity-based R between two vectors
  inline double deltaR_eta(const P4& a, const P4& b) {
    return a.deltaR_eta(b);
  }


  /// Difference in rapidity-based R^2 between two vectors
  inline double deltaR2_rap(const P4& a, const P4& b) {
    return a.deltaR2_rap(b);
  }
  /// Difference in rapidity-based R between two vectors
  inline double deltaR_rap(const P4& a, const P4& b) {
    return a.deltaR_rap(b);
  }

  //@}


  /// @name Operators taking two vectors
  //@{
  inline P4 operator + (const P4& a, const P4& b) { P4 rtn = a; return rtn += b; }
  inline P4 operator - (const P4& a, const P4& b) { P4 rtn = a; return rtn -= b; }
  inline P4 operator * (const P4& a, double f) { P4 rtn = a; return rtn *= f; }
  inline P4 operator * (double f, const P4& a) { P4 rtn = a; return rtn *= f; }
  inline P4 operator / (const P4& a, double f) { P4 rtn = a; return rtn /= f; }
  //@}



  /// @name Converters between HepMC and P4 momentum types
  //@{

  /// Convert a HepMC FourVector to a P4
  inline P4 mk_p4(const HepMC::FourVector& p) {
    return P4(p.px(), p.py(), p.pz(), p.e());
  }

  /// Convert a HepMC GenParticle to a FastJet PseudoJet
  inline P4 mk_p4(const HepMC::GenParticle* gp) {
    return mk_p4(gp->momentum());
  }

  /// Convert a vector of HepMC GenParticles to a vector of FastJet PseudoJets
  inline std::vector<P4> mk_p4s(const std::vector<const HepMC::GenParticle*>& gps) {
    std::vector<P4> p4s;
    foreach (const HepMC::GenParticle* gp, gps) {
      p4s.push_back( mk_p4(gp) );
    }
    return p4s;
  }

  //@}


  /// @name HepMC vector utils
  //@{

  /// Compute 3-vector magnitude-squared from a HepMC FourVector
  inline double mag2(const HepMC::FourVector& v) {
    return mk_p4(v).p2();
  }

  /// Compute 3-vector magnitude from a HepMC FourVector
  inline double mag(const HepMC::FourVector& v) {
    return mk_p4(v).p();
  }

  /// Compute pseudorapidity from a HepMC FourVector
  inline double eta(const HepMC::FourVector& v) {
    return mk_p4(v).eta();
  }

  /// Compute rapidity from a HepMC FourVector
  inline double rap(const HepMC::FourVector& v) {
    return mk_p4(v).rap();
  }

  //@}


}

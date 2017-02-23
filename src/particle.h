/*=========================================================================
 * 
 *  Program:   TEUFEL - THz Emission from Undulators and Free-Electron Lasers
 * 
 *  Copyright (c) 2017 U. Lehnert
 * 
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * =========================================================================*/

#ifndef PARTICLE_H
#define PARTICLE_H

#include "externalfield.h"
#include "vector.h"
#include "undulator.h"
#include <tuple>
using namespace std;

// a class for charged particle trajectories

class ChargedParticle
{

  public:

    // default constructor
    // charge set to electron charge
    ChargedParticle();

    // copy constructor
    ChargedParticle(const ChargedParticle *Part);

    // destructor
    ~ChargedParticle();

    // return the number of points in the track
    int GetNP();

    // return a position from the trajectory
    double TrajTime(int step);
    Vector TrajPoint(int step);
    Vector TrajMomentum(int step);

    // track the particle through a given field
    // use the most simple Euler algorithm
    void TrackEuler
      (  int Nstep,            // number of timesteps
         double tstep,         // time step size
         Vector X0,            // initial position
         Vector P0,            // initial momentum
         Lattice *field);

    // track the particle through a given field
    // using a leap-frog algorithm
    void TrackLF
      (  int Nstep,            // number of timesteps
         double tstep,         // time step size
         Vector X0,            // initial position
         Vector P0,            // initial momentum
         Lattice *field);

    // track the particle through a given field
    // using the Vay algorithm
    void TrackVay
      (  int Nstep,            // number of timesteps
         double tstep,         // time step size
         Vector X0,            // initial position
         Vector P0,            // initial momentum
         Lattice *field);

    // translate a given particle trajectory
    void Translate(Vector R);

    // mirroring a given particle trajectory
    // on a plane y=MirrorY
    // (this also reverses the charge)
    void MirrorY(double MirrorY);

    // electric field and magnetic field radiated by the particle
    // at a given observation point at time t in lab frame
    // retardation is properly accounted for
    tuple<Vector,Vector> RetardedEField(double time, Vector ObservationPoint);

    //set the particle's charge in terms of Elementary Charge
    void setCharge(int charge);

    //return the integer value of the Charge
    int getCharge();

    //set the particle's mass in terms of electron's mass
    void setMass(int Mass);

    //return the integer value of the mass;
    int getMass();
 
    //set intial position of the particle -- X[0]
    void setInitialPosition(Vector InitialPosition);
 
    //set intial momentum of the particle  --P[0]
    void setInitialMomentum(Vector InitialMomentum);

    //get the initial position and the momentum

    Vector getInitialPosition();
    Vector getInitialMomentum();

    //set the various trajectory values while tracking via external routine
    void setTrajPoint(int stepnumber,Vector Position);
    void setTrajMomentum(int stepnumber,Vector Momentum);
    void setTrajAcceleration(int stepnumber,Vector Accel);
    void setTrajTime(int stepnumber, double time);
    //set the number of trajectory points the particle can have	
    void setNOP(int NOP);

 
    // electric field and magnetic field radiated by the particle
    // and seen by some other particle
    // time is the  present time value and Observation point
    // is the 'present' Vector Position  of the other paricle
    // retardation is properly accounted for
    // returns a Coloumb Field , if the particle is not yet seeing the retarded field
    // if particles are too close (less than ~5.10 microns), then a minimum distance (equal to debye length) is 
    //assumed. The default value is calculated for an electron beam with 
    // Energy=135MeV; Total Beam Charge=1nC; bunch radius=100 microns; bunch length=6ps
    //thermal temperature; kbTe=0.2eV; lambdaD=sqrt(epsilon*gamma*kbTe/(e*e*ne)) =5.1 microns
    // typical LCLS parameter
    tuple<Vector,Vector> InteractionField(int stepnumber,double time, Vector ObservationPoint);

    //set and return the debyeL value ~ minimum value between two particles

    void setdebyeL(double L);
    double getdebyeL();


  private:

    int NP =0;			// number of trajectory points
    int Charge=-1;			// charge in units of ElementaryCharge
    int Mass= 1;			// mass in unit of the electron rest mass
    double *Time;		// time in lab-frame [s]
    Vector *X;			// position in lab frame [m]
    Vector *P;                  // momentum in lab frame : c p = beta gamma mc²
                                // dimensionless in units of mc²
    Vector *A;                  // acceleration in lab frame a = d/dt(p*c)
				// in unit of 1/s (scaled by mc²)
    Vector X0;			//initial position
    Vector P0;			//initial momentum (gamma*beta)
    double debyeL =5.1e-6;
};

#endif

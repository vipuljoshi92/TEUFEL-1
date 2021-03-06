/*=========================================================================

  Program:   TEUFEL - THz Emission from Undulators and Free-Electron Lasers

  Module:    E cross B test case

  Copyright (c) 2017 U. Lehnert

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

=========================================================================*/

/*!
    \brief E Cross B test case

    @author Ulf Lehnert, Vipul Joshi
    @date 10.2.2017
    @file teufel.EcrossB.cpp
    
    This test case tracks a single electron in an perpendicular Electric and Magnetic field. 
    The fields will be such that the particle will move unaffected
    The change in the momentum and the particles trajectory will be compared to the actual case.

    The program computes the trajectory of the electron starting at the coordinate system origin
    with an arbitrary velocity but the magnitude of the velocity is choosen such that
    it is relativistic with gamma = 30.35823. The trajectory's displacment in x and y should be very close to zero in
    an arbitrary magnetic field but with the Electric Field = cross(beta*c,B).  After tracking the particle, it is checked that :
    @li the maximum displacement is correct
    @li the kinetic energy has not changed
    
    Using the Vay algorithm, the particle will be tracked for 10ns and changes in its path will be noticed.
    
    @return The number of errors encountered in the above list of checks is reported.
    
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "particle.h"
#include "externalfield.h"
#include "homogeneouselectricfield.h"
#include "homogeneousmagnet.h"
#include <iostream>
#include <fstream>
int NOTS = 3000;                // number of time steps

int main ()
{
    printf("\nTEUFEL - E Cross B\n");
    double Bx=0.12;
    double By=0.52;
    double Bz=0.45;
    Vector B=Vector(Bx,By,Bz);
    HomogeneousMagnet *mag = new HomogeneousMagnet(B);
    printf("B =  %9.6g T\n",B.norm());
    printf("Bx =  %9.6g T\t",B.x);
    printf("By =  %9.6g T\t",B.y);
    printf("Bz =  %9.6g T\n",B.z);
    double gamma = 30.35823388162631;
    double beta = sqrt(1.0-1.0/(gamma*gamma));
    double betagamma= sqrt(gamma*gamma-1.0);
    //create a random unit vector and generate particle's momentum
    Vector RandVector= Vector(2.0,3.0,5.0);
    RandVector=RandVector/RandVector.norm();
    printf("beta =  %12.9g\n",beta);
    printf("gamma =  %12.9g\n",gamma);
    printf("c*p =  %12.9g MeV\n",1e-6*mecsquared*betagamma);
    printf("beta*gamma= %12.9g\n",beta*gamma);
    Vector Vel=RandVector*beta*SpeedOfLight;
    printf("Vx =  %9.6g m/sec\t",Vel.x);
    printf("Vy =  %9.6g m/sec\t",Vel.y);
    printf("Vz =  %9.6g m/sec\n",Vel.z);
    

    //compute a crossed electric field
    Vector E=cross(B,Vel);
    HomogeneousElectricField *ef = new HomogeneousElectricField(E);
    printf("E =  %9.6g V/m\n",E.norm());
    printf("Ex =  %9.6g V/m\t",E.x);
    printf("Ey =  %9.6g V/m\t",E.y);
    printf("Ez =  %9.6g V/m\n",E.z);

    // a simple lattice with just the E and B fields
    Lattice *lattice = new Lattice;
    lattice->addElement(mag);
    lattice->addElement(ef);
    // one single electron
    ChargedParticle *electron = new ChargedParticle();

    // initial position at the origin
    Vector X0 = Vector(0.0, 0.0, 0.0);
    // initial momentum of the particle
    Vector P0 = RandVector*betagamma;
    
    // track the particle 
    double tau=10e-9;
    double deltaT = tau/NOTS;
    electron->TrackVay(NOTS, deltaT, X0, P0, lattice);

    // count the errors
    int errors = 0;
  
   //the last trajectory point should lie on a line which can be described as (0+Vel.x*tau,0+Vel.y*tau,0+Vel.z*tau)
    Vector ExpectedPosition=Vector(Vel.x*tau,Vel.y*tau,Vel.z*tau);
    printf("Expected Final Position x =  %9.6g m\t",ExpectedPosition.x);
    printf("y =  %9.6g m\t",ExpectedPosition.y);
    printf("z =  %9.6g m\n",ExpectedPosition.z);
    
    // time of the last timestep should be equal to tau
    double FinalTime = electron->TrajTime(NOTS);
    if (fabs(FinalTime-tau) > deltaT) {
	errors++;
	printf("error in final time = %12.9g s - \033[1;31m test failed!\033[0m\n", FinalTime-tau);
    } else {
	printf("final time = %12.9g s - \033[1;32m OK\033[0m\n", FinalTime);
    }

    
    Vector FinalPosition=electron->TrajPoint(NOTS);
    if (fabs(FinalPosition.x-ExpectedPosition.x) >2e-6 && fabs(FinalPosition.y-ExpectedPosition.y) >2e-6 && fabs(FinalPosition.z-ExpectedPosition.z) >2e-6) {
	errors++;
	printf("final x position = %12.9g m - \033[1;31m test failed!\033[0m\n", FinalPosition.x); 
	printf("final y position = %12.9g m - \033[1;31m test failed!\033[0m\n", FinalPosition.y);
	printf("final z position = %12.9g m - \033[1;31m test failed!\033[0m\n", FinalPosition.z);
    } else {
	printf("final x position = %12.9g m - \033[1;32m OK\033[0m\n",FinalPosition.x);
	printf("final y position = %12.9g m - \033[1;32m OK\033[0m\n",FinalPosition.y);
	printf("final z position = %12.9g m - \033[1;32m OK\033[0m\n",FinalPosition.z);
    }
    // look for the momentum changes
    Vector ExpectedMomentum=P0;
    Vector FinalMomentum=electron->TrajMomentum(NOTS);
    if (ExpectedMomentum.norm()-FinalMomentum.norm() > 1.5e-3) {
	errors++;
	printf("Final Momentum = %9.6f - \033[1;31m test failed!\033[0m\n", FinalMomentum.norm());
    } else {
	printf("Final Momentum = %9.6f - \033[1;32m OK\033[0m\n",FinalMomentum.norm());}

    //Track the electron in the same fields with the Euler routine
    ChargedParticle *electron1 = new ChargedParticle();
    electron1->TrackEuler(NOTS, deltaT, X0, P0, lattice);
    FinalPosition=electron1->TrajPoint(NOTS);
    FinalMomentum=electron1->TrajMomentum(NOTS);
    if (fabs(FinalPosition.x-ExpectedPosition.x) >2e-6 && fabs(FinalPosition.y-ExpectedPosition.y) >2e-6 && fabs(FinalPosition.z-ExpectedPosition.z) >2e-6 && FinalMomentum.norm()-ExpectedMomentum.norm() >1.5e-3) {
	printf(" Euler Algorithm Fails ! \033[1;31m test failed!\033[0m\n"); 
    } else {
	printf("Euler Algorithm Pass - \033[1;32m OK\033[0m\n");
	printf("final x position = %12.9g m - \033[1;32m OK\033[0m\n",FinalPosition.x);
	printf("final y position = %12.9g m - \033[1;32m OK\033[0m\n",FinalPosition.y);
	printf("final z position = %12.9g m - \033[1;32m OK\033[0m\n",FinalPosition.z);
    }
   
    //Track the electron in the same fields with the LeapFrog routine
    ChargedParticle *electron2 = new ChargedParticle();
    electron2->TrackLF(NOTS, deltaT, X0, P0, lattice);
    FinalPosition=electron2->TrajPoint(NOTS);
    FinalMomentum=electron2->TrajMomentum(NOTS);
    if (fabs(FinalPosition.x-ExpectedPosition.x) >2e-6 && fabs(FinalPosition.y-ExpectedPosition.y) >2e-6 && fabs(FinalPosition.z-ExpectedPosition.z) >2e-6 && FinalMomentum.norm()-ExpectedMomentum.norm() >1.5e-3) {
	printf(" Leap Frog Algorithm Fails ! \033[1;31m test failed!\033[0m\n"); 
    } else {
	printf("Leap Frog Algorithm Pass - \033[1;32m OK\033[0m\n");
	printf("final x position = %12.9g m - \033[1;32m OK\033[0m\n",FinalPosition.x);
	printf("final y position = %12.9g m - \033[1;32m OK\033[0m\n",FinalPosition.y);
	printf("final z position = %12.9g m - \033[1;32m OK\033[0m\n",FinalPosition.z);
	printf("final Momentum = %12.9g m - \033[1;32m OK\033[0m\n", FinalMomentum.norm());
    }
    

    return errors;
    
}

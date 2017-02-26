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

#include "bunch.h"
#include "particle.h"
#include "global.h"
#include <iostream>
#include <math.h>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <tuple>
#include <externalfield.h>



Bunch::Bunch()
{
	//Create a single charged particle with default charge, mass values.
	setNOP(2);
	b=new ChargedParticle[2];
	//Initial values has to be set at the initialization
	b[0].setInitialMomentum(Vector(0.0,0,0.0));
	b[0].setInitialPosition(Vector(0.0,0.00,0.0));
	b[0].setInitialTime(0.0);	
	b[0].setParticleID(0);
	b[0].setCharge(-1);
	b[1].setCharge(-1);
	b[0].setMass(1);
	b[1].setMass(1);
	b[1].setInitialMomentum(Vector(-0.00,-0.00,0.0));
	b[1].setInitialPosition(Vector(0.0,0.0,0.0));
	b[1].setInitialTime(0.0);	
	b[1].setParticleID(1);

}

Bunch::Bunch(const char *filename, int NP, int charge, int mass)
{
	//set the number of particles equal to NP
	setNOP(NP);
	b=new ChargedParticle[NP];
	Charge=charge;
	Mass=mass;
	//check if the file has correct number of rows and columns
	//Rows must be equal to number of particles
	//columns must be equal to 7 to completely describe phase space
	// (x,px,y,py,z,pz,time)
	LoadBeamProfile(filename, b);

};

void Bunch:: setNOP(int NP)
{
	NOP=NP;

};


int Bunch::getNOP()
{
	return NOP;
};


tuple<Vector,Vector> Bunch::MutualField(int stepnumber, int ParticleID, double t)
{
	Vector Ef = Vector(0.0,0.0,0.0);
	Vector Bf = Vector(0.0,0.0,0.0);
	int j=ParticleID;
	Vector Robs=b[j].TrajPoint(stepnumber);
	tuple<Vector,Vector> *F=new tuple<Vector,Vector>[NOP];
	for (int i=0; i<NOP;i++)
	{
		// for i == j; i.e. field due to particle i on its position is zero
		//implemented in the InteractionField routine of particles
		F[i]= b[i].InteractionField(j,stepnumber,t, Robs);
	}
	// tuple of electric and magnetic fields have been obtained
	//add all of them together
	
	for (int i=0; i<NOP; i++)
	{
		Ef+=get<0>(F[i]);
		Bf+=get<1>(F[i]);
	}
	
	//make tuple of total fields and return

	return make_tuple(Ef,Bf);
}


// Track the particles in an external field 
//field is generated by the lattice
//the interaction fields are added with the lattice fields

void Bunch::Track_Euler(int NOTS, double tstep, Lattice *field)
{

	//allocate memory to every particle
	//to store trajectory details
	InitializeTrajectory(NOTS);
	for (int i=0;i<NOTS;i++)
	{
		for(int k=0;k<NOP;k++)
		{
			Vector X0=b[k].TrajPoint(i);
			Vector P0=b[k].TrajMomentum(i);
			double t0=b[k].TrajTime(i);
			double qm=b[k].getCharge()*InvRestMass/b[k].getMass();
			Vector p=b[k].TrajMomentum(i);
			double betagamma2=p.abs2nd();
			double gamma=sqrt(betagamma2+1.0);
			Vector beta = p/gamma;
			tuple<Vector,Vector>MField=MutualField(i,k,t0);
    			Vector efield = field->EField(t0, X0) + get<0>(MField);
    			Vector bfield = field->BField(t0,X0)+ get<1>(MField);
    			Vector force = cross(beta, bfield) + efield/SpeedOfLight;
   			Vector dX_dt = beta * SpeedOfLight;
   			Vector dP_dt = force * qm;
    			// store acceleration
   			b[k].setTrajAcceleration(i,dP_dt);
   			// integrator step
			b[k].setTrajPoint(i+1,X0+dX_dt*tstep);
			b[k].setTrajTime(i+1,t0+tstep);
			b[k].setTrajMomentum(i+1,P0+dP_dt*tstep);
		}
	}

}

void Bunch::Track_LeapFrog(int NOTS, double tstep, Lattice *field)
{
	InitializeTrajectory(NOTS);
	for (int i=0;i<NOTS;i++)
	{
		for (int k=0;k<NOP;k++)
		{
			double qm=b[k].getCharge()*InvRestMass/Mass;
			
		}
	}



}

void Bunch::InitializeTrajectory(int NOTS)
{
	for (int i=0; i<NOP; i++)
	{
		b[i].setNP(NOTS);
	}

}

void Bunch::LoadBeamProfile(const char *filename, const ChargedParticle *part)
{

	//Generate a stream to the Beam Profile file
	ifstream BunchProfile;
	file=filename;
	//Check if file has correct number of rows and coumns
	//if ok, then stream the values.
	//else print error message and exit properly
	if (FileCheck(file,NOP)==1)
	{		
		ifstream BeamProfile(filename);
		printf("\033[7;31m Loading Beam Profile....\n\033[0m\n");
		for(int i=0;i<NOP;i++)
		{
			double t;			
			Vector position;
			Vector momentum;
			BeamProfile>>t;
			BeamProfile>>position.x;
			BeamProfile>>position.y;
			BeamProfile>>position.z;
			BeamProfile>>momentum.x;
			BeamProfile>>momentum.y;
			BeamProfile>>momentum.z;
			b[i].setInitialPosition(position);
			b[i].setInitialMomentum(momentum);
			b[i].setInitialTime(t);
			b[i].setCharge(Charge);
			b[i].setMass(Mass);
			b[i].setParticleID(i);
			
		}
		printf("\033[7;31m Beam Profile Loaded....\n\033[0m\n");
		BeamProfile.close();
	}
	else
	{
		printf("\033[7;31m Error Loading Beam Profile....\n\033[0m\n");
		printf("\033[7;31m Exiting Program\n\033[0m\n");
		exit (EXIT_FAILURE);
	}
}


int Bunch::FileCheck(const char *filename, int NP)
{
	NOP=NP;
	ifstream InFile(filename);
	ifstream InFile2(filename);
	int rows,columns,tabs;
	if (InFile.is_open() && InFile2.is_open())
	{
	  rows=count(istreambuf_iterator<char>(InFile),istreambuf_iterator<char>(),'\n');
	  tabs=count(istreambuf_iterator<char>(InFile2),istreambuf_iterator<char>(),'\t');
	  columns=tabs/rows+1;
	  if(rows!=NOP || columns!=7)
		{
			printf("\033[1;31m Incorrect Phase Space Distribution\n Number of Particles for which Info Provided: %d \n Number of phase space parameters provided: %d \n \033[0m\n", rows, columns);
			return 0;
		}
	  else {return 1;}
	  
	}
	else {
		return 0;	
		printf("Error opening the file: %p", filename);};
	InFile.close();
	InFile2.close();
}




  


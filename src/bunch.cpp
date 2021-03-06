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
#include "externalfield.h"
#include "omp.h"
#include "SDDS.h"
#include <string.h>
#include <utility>
Bunch::Bunch()
{
	
	NOP =0;
	Charge=0;
	Mass = 0;	
}



Bunch::Bunch(const char *filename, int NP, int charge, int mass)
{
	
	NOP = NP;
	Charge = charge*ElementaryCharge*NOP;
	Mass = mass*m_e*NOP;
	LoadBeamProfile(filename);
	for(int i=0;i<NOP;i++)
	{
		b.push_back(new ChargedParticle(charge,mass,InitialPosition[i],InitialMomentum[i],InitialTime[i]));
	}
	printf("\033[7;31m Particles Loaded In Bunch....\n\033[0m\n");
	

};


Bunch::Bunch(Bunch *bunch)
{
	NOP = bunch->NOP;
	NOTS = bunch->NOTS;
	Charge = bunch->Charge;
	Mass = bunch->Mass;
	for(int i=0;i<NOP;i++)
	{
		//ChargedParticle *copy_particle = new ChargedParticle();
		// *copy_particle = bunch->b.at(i);
		//memcpy(&copy_particle, &bunch->b[i], sizeof(bunch->b[i]));
		b.push_back(new ChargedParticle(bunch->b[i]));
		
	}
}


Bunch:: ~Bunch()
{
	if(!b.empty())
	{
		b.erase(b.begin(),b.begin()+NOP);
	}

}

void Bunch::AddParticles( ChargedParticle *part)
{
	NOP +=1;
	Charge = Charge+(part->getCharge())*ElementaryCharge;
	Mass = Mass + (part->getMass())*m_e;
	b.push_back(part);
	
	
}



void Bunch::JoinBunch(Bunch *bunch)
{
	
	Charge += bunch->Charge;
	Mass += bunch ->Mass;
	for(int i=0; i<=bunch->getNOP()-1;i++)
	{
		b.push_back(bunch->PointerToParticle(i));
	}
	NOP+=bunch->getNOP();

}

ChargedParticle* Bunch::PointerToParticle(int i)
{
	return b[i];
}
int Bunch::getNOP()
{
	return NOP;
};



int Bunch:: getNOTS()
{
	return NOTS;
}



double Bunch:: getTotalCharge()
{
	return Charge;
}



double Bunch:: getTotalMass()
{
	return Mass;
}


Vector Bunch:: getTrajPoint(int i, int j)
{
	if (i<NOP)
	{
		return b[i]->TrajPoint(j);
	}

	else
	{
		cout <<"Particle No. "<<i<<" not found in bunch"<<endl;
		cout <<"Total Number of Particles in Bunch: "<<NOP<<endl;
	}

}

Vector Bunch:: getTrajMomentum(int i, int j)
{
	if (i<NOP)
	{
		return b[i]->TrajMomentum(j);
	}

	else
	{
		cout <<"Particle No. "<<i<<" not found in bunch"<<endl;
		cout <<"Total Number of Particles in Bunch: "<<NOP<<endl;
	}

}

Vector Bunch:: getTrajAcceleration(int i, int j)
{
	if (i<NOP)
	{
		return b[i]->TrajAccel(j);
	}

	else
	{
		cout <<"Particle No. "<<i<<" not found in bunch"<<endl;
		cout <<"Total Number of Particles in Bunch: "<<NOP<<endl;
	}

}

double Bunch:: getTrajTime(int i, int j)
{
	if (i<NOP)
	{
		return b[i]->TrajTime(j);
	}

	else
	{
		cout <<"Particle No. "<<i<<" not found in bunch"<<endl;
		cout <<"Total Number of Particles in Bunch: "<<NOP<<endl;
	}

}


pair<Vector,Vector> Bunch::MutualField(int stepnumber, int ParticleID, double t)
{
	
	int j=ParticleID;
	Vector Robs=b[j]->TrajPoint(stepnumber);
	pair<Vector,Vector>FF[NOP];
	double Ex=0.0;
	double Ey=0.0;
	double Ez=0.0;
	double Bx=0.0;
	double By=0.0;
	double Bz=0.0;
	
#pragma omp parallel for shared(j,stepnumber,t,Robs) reduction(+:Ex,Ey,Ez,Bx,By,Bz)
	for (int i=0; i<NOP;i++)
	{
		FF[i]= b[i]->RetardedEField(t, Robs);
		Ex+=(FF[i].first).x;
		Ey+=(FF[i].first).y;
		Ez+=(FF[i].first).z;
		Bx+=(FF[i].second).x;
		By+=(FF[i].second).y;
		Bz+=(FF[i].second).z;
		
	}

	return make_pair(Vector(Ex,Ey,Ez),Vector(Bx,By,Bz));
}



pair<Vector,Vector> Bunch::RadiationField(Vector Robs, double t)
{
	Vector Ef = Vector(0.0,0.0,0.0);
	Vector Bf = Vector(0.0,0.0,0.0);
	pair<Vector,Vector>FF[NOP];
	double Ex=0.0;
	double Ey=0.0;
	double Ez=0.0;
	double Bx=0.0;
	double By=0.0;
	double Bz=0.0;
#pragma omp parallel for shared(t,Robs) reduction(+:Ex,Ey,Ez,Bx,By,Bz)
	for (int i=0; i<NOP;i++)
	{
		// for i == j; i.e. field due to particle i on its position is zero
		//implemented in the InteractionField routine of particles
		FF[i]= b[i]->RetardedEField(t, Robs);
		Ex+=(FF[i].first).x;
		Ey+=(FF[i].first).y;
		Ez+=(FF[i].first).z;
		Bx+=(FF[i].second).x;
		By+=(FF[i].second).y;
		Bz+=(FF[i].second).z;
	}

	return make_pair(Vector(Ex,Ey,Ez),Vector(Bx,By,Bz));
}


void Bunch::Track_Vay(int NT, double tstep, Lattice *field, int SpaceCharge)
{
	NOTS = NT;
	TIMESTEP=tstep;
	TotalTime = tstep*NOTS;
	double start=omp_get_wtime();
	pair<Vector,Vector> InteractionField[NOP];
#pragma omp parallel for 
	for (int i=0;i<NOP;i++)
	{
		
		b[i]->init(NOTS);
		b[i]->InitVay(tstep,field);
		InteractionField[i] = make_pair(Vector(0,0,0),Vector(0,0,0));
				
	}
	if(SpaceCharge==0)
	{
		for (int i=0;i<NOTS-1;i++)
		{
#pragma omp parallel for 
			for (int k=0;k<NOP;k++)
			{
				b[k]->StepVay();
			}
		}
	}
	else
	{		
		
		for (int i=0;i<NOTS-1;i++)
		{
						

#pragma omp parallel for 
			for (int k=0;k<NOP;k++)
			{	
				
				b[k]->StepVay((InteractionField[k].first),(InteractionField[k].second));
			}
		
#pragma omp parallel for  
			for (int l=0;l<NOP;l++)
			{
				
				double time = b[l]->TrajTime(0)+i*tstep;
				InteractionField[l]= MutualField(i, l, time );
				
			}

		}
		
				
	}

	double end=omp_get_wtime();
	cout<<"\033[1;31m Tracking Completed in: \033[0m"<<(end-start)<<"\033[1;31m seconds\033[0m\n"<<endl;

}

void Bunch::MirrorY(double Mirror)
{
	for(int i=0;i<NOP;i++)
	{	
		b[i]->MirrorY(Mirror);
	}
}

void Bunch::Translate(Vector R)
{
	for(int i=0;i<NOP;i++)
	{	
		b[i]->Translate(R);
		
	}
}
int Bunch::WriteSDDSTrajectory()
{

	cout << "Dumping each particle's Trajectory Data to file: trajectory.sdds"<<endl;
	SDDS_DATASET data;
	char buffer[100];
	int Initialize = SDDS_InitializeOutput(&data,SDDS_BINARY,1,NULL,NULL,"trajectory.sdds");
	double nots,nop;
	nots =(double)NOTS;
	nop =(double)NOP;
	cout <<NOP<<endl;
	if(Initialize!=1)
	{
			cout<<"Error Initializing Output\n";
			return 1;
	}
	else
	{
			cout<<"output initialized\n";
	}

	if(
			SDDS_DefineSimpleParameter(&data,"NumberOfParticles",NULL, SDDS_DOUBLE)!=1 || 
			SDDS_DefineSimpleParameter(&data,"TotalTime","s", SDDS_DOUBLE)!=1 || 
			SDDS_DefineSimpleParameter(&data,"BunchTotalCharge","C", SDDS_DOUBLE)!=1 ||
			SDDS_DefineSimpleParameter(&data,"BunchTotalMass","kg", SDDS_DOUBLE)!=1
	  )
	{
			cout<<"error in defining parameters\n";
			return 2;;
	}
	else
	{
			cout<<"parameters defined \n";
	}
	
	if(
			SDDS_DefineColumn(&data,"Number\0","No\0",NULL,"RowNumber\0",NULL, SDDS_DOUBLE,0) ==-1 ||
 			SDDS_DefineColumn(&data,"t\0","t\0","s\0","TimeInSeconds\0",NULL, SDDS_DOUBLE,0)   ==-1 || 
			SDDS_DefineColumn(&data,"x\0","x\0","m\0","DisplacementInX\0",NULL, SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"y\0","y\0","m\0","DisplacementInY\0",NULL, SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"z\0","z\0","m\0","DisplacementInZ\0",NULL, SDDS_DOUBLE,0) == -1 || 
			SDDS_DefineColumn(&data,"px\0","px\0",NULL,"Gammabetax\0",NULL, SDDS_DOUBLE,0)== -1 || 
			SDDS_DefineColumn(&data,"py\0","py\0",NULL,"Gammabetay\0",NULL,SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"pz\0","pz\0",NULL,"Gammabetaz\0",NULL,SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"Ax\0","ax\0","msec2\0","AccelerationInX\0",NULL,SDDS_DOUBLE,0)==-1 ||
			SDDS_DefineColumn(&data,"Ay\0","ay\0","msec2\0","AccelerationInY\0",NULL,SDDS_DOUBLE,0)==-1 ||
			SDDS_DefineColumn(&data,"Az\0","az\0","msec2\0","AccelerationInZ\0",NULL,SDDS_DOUBLE,0)==-1 ||
			SDDS_DefineColumn(&data,"gamma\0","gamma\0",NULL,"RelativisticFactor\0",NULL,SDDS_DOUBLE,0)==-1
	  )
	{
			cout<<"error in defining columns\n";
			return 3;
	}
	else
	{
			cout<<"Columns defined \n";	
	}

	if (SDDS_WriteLayout(&data)!=1)
	{
			cout<<"error in writing layout\n";
			return 4;
	}
	else{
			cout<<"layout written \n";
	}

	for (int32_t i = 0;i<(int32_t)nop;i++)
	{

		
		if (SDDS_StartPage(&data,(int32_t)nots)!=1)


			{
				cout<<"error in starting page\n";
				return 5;
			}


		if (
			SDDS_SetParameters(&data,SDDS_SET_BY_NAME|SDDS_PASS_BY_VALUE,
			"NumberOfParticles",nop,
			"TotalTime",TotalTime,
			"BunchTotalCharge",Charge,
			"BunchTotalMass",Mass,
			 NULL)!=1
		   )

			{
				cout<<"error in defining parameter\n";
				return 6;
			}
			
		for( int32_t k =0;k<(int32_t)nots;k++)
		{
			
			
			if(SDDS_SetRowValues(&data,SDDS_SET_BY_NAME|SDDS_PASS_BY_VALUE,k,
						"Number",(double)(k),
						"t",(double)(b[i]->TrajTime(k)),
						"x",(double)((b[i]->TrajPoint(k)).x),
						"y",(double)((b[i]->TrajPoint(k)).y),
						"z",(double)((b[i]->TrajPoint(k)).z),
						"px",(double)((b[i]->TrajMomentum(k)).x),
						"py",(double)((b[i]->TrajMomentum(k)).y),
						"pz",(double)((b[i]->TrajMomentum(k)).z),
						"Ax",(double)((b[i]->TrajAccel(k)).x)*SpeedOfLight,
						"Ay",(double)((b[i]->TrajAccel(k)).y)*SpeedOfLight,
						"Az",(double)((b[i]->TrajAccel(k)).z)*SpeedOfLight,
						"gamma",(double)(sqrt(1.0+b[i]->TrajMomentum(k).abs2nd())),
						NULL)!=1
			  )

			{
				cout<<"error in writing columns\n";
				return 7;
			}
	
		}

		if(SDDS_WritePage(&data)!=1)
	
			{
				cout<<"error in writing page\n";
				return 8;
			}

	}
	
	if (SDDS_Terminate(&data)!=1)
	{
		cout<<"error terminating data\n";
		return 9;
	}	

	return 0;
}

//write sdds file with time as a parameter. each page has x,y,z,px,py,pz at different time steps
int
 Bunch::WriteSDDSTime()
{

	cout<<"Dumping Data for each time step for every particle to file: time-trajectory.sdds"<<endl;
	SDDS_DATASET data;
	char buffer[100];
	int Initialize = SDDS_InitializeOutput(&data,SDDS_BINARY,1,NULL,NULL,"time-trajectory.sdds");
	double nots,nop;
	nots =(double)NOTS;
	nop =(double)NOP;
	if(Initialize!=1)
	{
		cout<<"Error Initializing Output\n";
		return 1;

	}
	else
	{
		fprintf(stdout,"output initialized\n");
	}

	if(
			SDDS_DefineSimpleParameter(&data,"NumberOfParticles",NULL, SDDS_DOUBLE)!=1 || 
			SDDS_DefineSimpleParameter(&data,"TotalTime","seconds", SDDS_DOUBLE)!=1 || 
			SDDS_DefineSimpleParameter(&data,"BunchTotalCharge","Coloumbs", SDDS_DOUBLE)!=1 ||
			SDDS_DefineSimpleParameter(&data,"BunchTotalMass","kg", SDDS_DOUBLE)!=1 || 
			SDDS_DefineSimpleParameter(&data,"Time","seconds", SDDS_DOUBLE)!=1
	  )
	{
		fprintf(stdout,"error in defining parameters\n");
		return 2;
		
	}

	else
	{
		fprintf(stdout,"parameters defined \n");
	}
	
	if(
			SDDS_DefineColumn(&data,"Number\0","No\0",NULL,"RowNumber\0",NULL, SDDS_DOUBLE,0) ==-1 ||
			SDDS_DefineColumn(&data,"x\0","x\0","m\0","DisplacementInX\0",NULL, SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"y\0","y\0","m\0","DisplacementInY\0",NULL, SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"z\0","z\0","m\0","DisplacementInZ\0",NULL, SDDS_DOUBLE,0) == -1 || 
			SDDS_DefineColumn(&data,"px\0","px\0",NULL,"Gammabetax\0",NULL, SDDS_DOUBLE,0)== -1 || 
			SDDS_DefineColumn(&data,"py\0","py\0",NULL,"Gammabetay\0",NULL,SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"pz\0","pz\0",NULL,"Gammabetaz\0",NULL,SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"Ax\0","ax\0","msec2\0","AccelerationInX\0",NULL,SDDS_DOUBLE,0)==-1 ||
			SDDS_DefineColumn(&data,"Ay\0","ay\0","msec2\0","AccelerationInY\0",NULL,SDDS_DOUBLE,0)==-1 ||
			SDDS_DefineColumn(&data,"Az\0","az\0","msec2\0","AccelerationInZ\0",NULL,SDDS_DOUBLE,0)==-1
	  )
	{
		fprintf(stdout,"error in defining columns\n");
		return 3;
	}

	else
	{
		fprintf(stdout,"Columns defined \n");	
	}

	if (SDDS_WriteLayout(&data)!=1)
	{
		fprintf(stdout,"error in writing layout\n");
		return 4;
	}

	else
	{
		fprintf(stdout,"layout written \n");
	}

	for (int32_t i = 0;i<(int32_t)nots;i++)
	{
		if (SDDS_StartPage(&data,(int32_t)nop)!=1)
			{
				fprintf(stdout,"error in starting page\n");
				return 5;
			}
		if (
			SDDS_SetParameters(&data,SDDS_SET_BY_NAME|SDDS_PASS_BY_VALUE,
			"NumberOfParticles",nop,
			"TotalTime",TotalTime,
			"BunchTotalCharge",Charge,
			"BunchTotalMass",Mass,
			"Time",(double)(b[0]->TrajTime(i)),
			 NULL)!=1)
			{
				fprintf(stdout,"error in defining parameter\n");
				return 6;
			}
			
		for(int32_t k =0;k<(int32_t)nop;k++)
		{
			
			
			if(SDDS_SetRowValues(&data,SDDS_SET_BY_NAME|SDDS_PASS_BY_VALUE,k,
						"Number",(double)(k),
						"x",(double)((b[k]->TrajPoint(i)).x),
						"y",(double)((b[k]->TrajPoint(i)).y),
						"z",(double)((b[k]->TrajPoint(i)).z),
						"px",(double)((b[k]->TrajMomentum(i)).x),
						"py",(double)((b[k]->TrajMomentum(i)).y),
						"pz",(double)((b[k]->TrajMomentum(i)).z),
						"Ax",(double)((b[k]->TrajAccel(i)).x)*SpeedOfLight,
						"Ay",(double)((b[k]->TrajAccel(i)).y)*SpeedOfLight,
						"Az",(double)((b[k]->TrajAccel(i)).z)*SpeedOfLight,
						NULL)!=1)
			{
				fprintf(stdout,"error in writing columns\n");
				return 7;
			}
	
		}
		if(SDDS_WritePage(&data)!=1)
			{
				fprintf(stdout,"error in writing page\n");
				return 8;
			}

	}
	
	if (SDDS_Terminate(&data)!=1)
	{
		fprintf(stdout,"error terminating data\n");
		return 9;
	}	
	
	return 0;
}

int Bunch::WriteSDDSRadiation(Vector Robs, double time_begin, double time_end, int NumberOfPoints )
{

	cout<<"Dumping Radiation Field data for the given time window to file : radiation.sdds"<<endl;
	double dt = (time_end-time_begin)/(double)NumberOfPoints;
	pair<Vector,Vector>Field[NumberOfPoints];
#pragma omp parallel for shared(time_begin, dt, Robs) 
	for (int i=0;i<NumberOfPoints;i++)
	{
		Field[i] = RadiationField(Robs,time_begin+i*dt);

	}
	SDDS_DATASET data;
	char buffer[100];
	int Initialize = SDDS_InitializeOutput(&data,SDDS_BINARY,1,NULL,NULL,"radiation.sdds");
	double nots,nop;
	nots =(double)NOTS;
	nop =(double)NOP;
	if(Initialize!=1)
	{
			cout<<"Error Initializing Output\n";
			return 1;
	}
	else
	{
			cout<<"output initialized\n";
	}

	if(
			SDDS_DefineSimpleParameter(&data,"NumberOfParticles",NULL, SDDS_DOUBLE)!=1 || 
			SDDS_DefineSimpleParameter(&data,"TimeWindow","s", SDDS_DOUBLE)!=1 || 
			SDDS_DefineSimpleParameter(&data,"BunchTotalCharge","C", SDDS_DOUBLE)!=1 ||
			SDDS_DefineSimpleParameter(&data,"BunchTotalMass","kg", SDDS_DOUBLE)!=1
	  )
	{
			cout<<"error in defining parameters\n";
			return 2;;
	}
	else
	{
			cout<<"parameters defined \n";
	}
	
	if(
			SDDS_DefineColumn(&data,"Number\0","No\0",NULL,"RowNumber\0",NULL, SDDS_DOUBLE,0) ==-1 ||
 			SDDS_DefineColumn(&data,"t\0","t\0","s\0","TimeInSeconds\0",NULL, SDDS_DOUBLE,0)   ==-1 || 
			SDDS_DefineColumn(&data,"Ex\0","Ex\0","V/m\0","ElectricFieldInX\0",NULL, SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"Ey\0","Ey\0","V/m\0","ElectricFieldInY\0",NULL, SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"Ez\0","Ez\0","V/m\0","ElectricFieldInZ\0",NULL, SDDS_DOUBLE,0) == -1 || 
			SDDS_DefineColumn(&data,"Bx\0","Bx\0","Tesla\0","MagneticFieldInX\0",NULL, SDDS_DOUBLE,0)== -1 || 
			SDDS_DefineColumn(&data,"By\0","By\0","Tesla\0","MagneticFieldInY\0",NULL,SDDS_DOUBLE,0) == -1 ||
			SDDS_DefineColumn(&data,"Bz\0","Bz\0","Tesla\0","MagneticFieldInZ\0",NULL,SDDS_DOUBLE,0)==-1   ||
			SDDS_DefineColumn(&data,"PoyntingVector\0","|S|\0","dp/da\0","MagnitudeOfPoyntingVector\0",NULL,SDDS_DOUBLE,0) == -1
	  )
	{
			cout<<"error in defining columns\n";
			return 3;
	}
	else
	{
			cout<<"Columns defined \n";	
	}

	if (SDDS_WriteLayout(&data)!=1)
	{
			cout<<"error in writing layout\n";
			return 4;
	}
	else{
			cout<<"layout written \n";
	}


	if (SDDS_StartPage(&data,(int32_t)NumberOfPoints)!=1)
	{
		cout<<"error in starting page\n";
		return 5;
	}


	for (int32_t i = 0;i<(int32_t)NumberOfPoints;i++)
	{

		Vector E = (Field[i].first);
		Vector B = (Field[i].second);
		Vector Poynting = cross(E,B);
		


		if (
			SDDS_SetParameters(&data,SDDS_SET_BY_NAME|SDDS_PASS_BY_VALUE,
			"NumberOfParticles",nop,
			"TimeWindow",time_end-time_begin,
			"BunchTotalCharge",Charge,
			"BunchTotalMass",Mass,
			 NULL)!=1
		   )

			{
				cout<<"error in defining parameter\n";
				return 6;
			}
			
		if(SDDS_SetRowValues(&data,SDDS_SET_BY_NAME|SDDS_PASS_BY_VALUE,i,
			"Number",(double)(i),
			"t",(double)(time_begin+i*dt),
			"Ex",(double)(E.x),
			"Ey",(double)(E.y),
			"Ez",(double)(E.z),
			"Bx",(double)(B.x),
			"By",(double)(B.y),
			"Bz",(double)(B.z),
			"PoyntingVector",(double)(Poynting.norm()),
			NULL)!=1
			)

			{
				cout<<"error in writing columns\n";
				return 7;
			}
	}

	if(SDDS_WritePage(&data)!=1)	
	{
		cout<<"error in writing page\n";
		return 8;
	}
	
	if (SDDS_Terminate(&data)!=1)
	{
		cout<<"error terminating data\n";
		return 9;
	}	

	return 0;
}
void Bunch::LoadBeamProfile(const char *filename)
{

	//Generate a stream to the Beam Profile file
	ifstream BunchProfile;
	file=filename;
	//Check if file has correct number of rows and coumns
	//if ok, then stream the values.
	//else print error message and exit properly
	if (FileCheck(file)==1)
	{		
		ifstream BeamProfile(filename);
		printf("\033[7;31m Loading Beam Profile....\n\033[0m\n");
		for(int i=0;i<NOP;i++)
		{
			double t;
			Vector T0;
			Vector X0;
			Vector P0;
			BeamProfile>>t;
			BeamProfile>>X0.x;
			BeamProfile>>X0.y;
			BeamProfile>>X0.z;
			BeamProfile>>P0.x;
			BeamProfile>>P0.y;
			BeamProfile>>P0.z;
			InitialTime.push_back(t);
			InitialPosition.push_back(X0);
			InitialMomentum.push_back(P0);
			
			
		}
		
		BeamProfile.close();
	}
	else
	{
		printf("\033[7;31m Error Loading Beam Profile....\n\033[0m\n");
		printf("\033[7;31m Exiting Program\n\033[0m\n");
		exit (EXIT_FAILURE);
	}
}


int Bunch::FileCheck(const char *filename)
{
	
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




  


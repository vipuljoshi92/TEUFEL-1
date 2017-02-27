#include "bunch.h"
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "omp.h"

using namespace std;
int main()
{
  ofstream Out;
  Out.open("test.txt");
  const char *filename= "BeamProfile.txt";
  //Bunch *BB= new Bunch();
  int NOP=300;
  int NOTS=3000;
  double dt=2.5e-12;
  Bunch *BB= new Bunch(filename, NOP, -1, 1);
  Undulator *undu =new Undulator(0.532,0.037,54);
  Lattice *field =new Lattice();
  field->addElement(undu);
  BB->Track_Euler(NOTS,dt,field);
  for(int k=0;k<10;k++)
  {

	for (int i=0;i<NOTS;i++)
  	{
		Vector XP=BB->b[k].TrajPoint(i);
		Out<<XP.x<<"\t"<<XP.y<<"\t"<<XP.z<<"\n";
  	}
  }
  Out.close();
  return 0;
}

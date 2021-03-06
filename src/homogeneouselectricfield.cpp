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

#include "homogeneouselectricfield.h"
#include "global.h"
#include <math.h>

HomogeneousElectricField::HomogeneousElectricField(Vector E)
{
  E0=E;
}

Vector HomogeneousElectricField::getE0()
{
  return E0;
}

Vector HomogeneousElectricField::ElementLocalEField(double t, Vector X)
{
  Vector p=E0;
  return (p);
}

Vector HomogeneousElectricField::ElementLocalBField(double t, Vector X)
{
  return Vector(0.0,0.0,0.0);
}



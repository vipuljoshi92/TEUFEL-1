#usr/bin/bash

#sddsplot -columnNames=z,'(x,y,px,py,pz,Ax,Ay,Az)' -separate -graphic=symbol,vary -legend trajectory.sdds
sddsenvelope trajectory.sdds traj.env -copy=z -mean=x,y,z,px,py,pz,Ax,Ay,Az,gamma -standarddeviation=x,y,z,px,py,pz,gamma
#sddsplot -columnNames=z,'(x,y)' -split=pages -separate -graphic=dot,vary -legend time-trajectory.sdds
sddsplot -columnNames=z,'(xMean,yMean,zMean,pxMean,pyMean,pzMean,gammaMean,AxMean,AyMean,AzMean,xStDev,yStDev,zStDev,pxStDev,pyStDev,pzStDev,gammaStDev)' -separate -graphic=line,vary -legend traj.env
sddsplot -columnNames=t,'(Ex,Ey,Ez,Bx,By,Bz)' -separate -graphic=line,vary -legend  radiation.sdds

import matplotlib.pyplot as plt
import numpy as np

time,x,y,z, px,py,pz=np.loadtxt("avg.txt",delimiter='\t',unpack='True')
time1,stdx,stdy,stdz, stdpx,stdpy,stdpz=np.loadtxt("stdx.txt",delimiter='\t',unpack='True')
avgz,bunching = np.loadtxt("Bfactor.txt",delimiter='\t',unpack='True')
plt.figure(0)
plt.subplot(221)
plt.plot(z,x,'.')
plt.xlabel("Displacement in z(m)")
plt.ylabel("Displacement in x(m)")
plt.grid()
plt.subplot(222)
plt.plot(z,y,'.')
plt.xlabel("Displacement in z(m)")
plt.ylabel("Displacement in y(m)")
plt.grid()
plt.subplot(223)
plt.plot(z,px,'.')
plt.xlabel("Displacement in z(m)")
plt.ylabel("Normalized Momentum in x")
plt.grid()
plt.subplot(220)
plt.plot(z,pz,'.')
plt.xlabel("Displacement in z(m)")
plt.ylabel("Normalized Momentum in z")
plt.grid()
plt.figure(1)
plt.subplot(221)
plt.plot(z,stdx,'.')
plt.xlabel("Displacement in z(m)")
plt.ylabel("stdx(m)")
plt.grid()
plt.subplot(222)
plt.plot(z,stdy,'.')
plt.xlabel("Displacement in z(m)")
plt.ylabel("stdy(m)")
plt.grid()
plt.subplot(223)
plt.plot(z,stdz,'.')
plt.xlabel("Displacement in z(m)")
plt.ylabel("stdz(m)")
plt.grid()
plt.subplot(220)
plt.plot(z,stdpz,'.')
plt.xlabel("Displacement in z(m)")
plt.ylabel("stdpz(m)")
plt.grid()

plt.figure(2)
plt.plot(z,bunching)
plt.xlabel("Displacement in z(m)")
plt.ylabel("Bunching Factor")
plt.grid()
plt.show()

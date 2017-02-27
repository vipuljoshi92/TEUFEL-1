import matplotlib.pyplot as plt
import numpy as np

x,y,z = np.loadtxt("test.txt",delimiter='\t',unpack='True')

plt.figure(1)
plt.scatter(z,x)
plt.grid()
plt.show()

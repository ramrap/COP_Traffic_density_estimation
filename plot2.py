from matplotlib import pyplot as plt
from matplotlib import style
import numpy as np

style.use('ggplot')
x,y,z = np.loadtxt('m2.csv',unpack=True,delimiter=',')

plt.subplot(1,2,1)
plt.plot(x,y)
plt.axis([0.1,1,240,290])
plt.title('Runtime')
plt.xlabel('Scaling')
plt.ylabel('Runtime(s)')


plt.subplot(1,2,2)
plt.plot(x,z)
plt.axis([0.1,1,0,0.007])
plt.title('RMS Error')
plt.xlabel('Scaling')
plt.ylabel('RMS Error')
plt.show()
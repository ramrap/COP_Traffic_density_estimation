from matplotlib import pyplot as plt
from matplotlib import style
import numpy as np

style.use('ggplot')
x,y,z = np.loadtxt('data.csv',unpack=True,delimiter=',')
a,b,c = np.loadtxt('a.csv',unpack=True,delimiter=',')
w=b-y
rms = np.sqrt(np.mean(np.square(w), axis = 0))
print(rms)
plt.plot(x,w)
plt.legend(['Queue Density'])
plt.axvline(x=1480, color = 'g', linestyle = 'dashed')
plt.axvline(x=2800, color = 'r', linestyle = 'dashed')
plt.axvline(x=4200, color = 'g', linestyle = 'dashed')
plt.axvline(x=5250, color = 'r', linestyle = 'dashed')
plt.title('Chart')
plt.xlabel('Frame number')
plt.ylabel('value')

plt.show()


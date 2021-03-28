from matplotlib import pyplot as plt
from matplotlib import style
import numpy as np

style.use('ggplot')
x,y,z = np.loadtxt('data.csv',unpack=True,delimiter=',')

plt.plot(x,y)
plt.plot(x,z)
plt.legend(['Queue Density','Dynamic Density'])
plt.axvline(x=1480, color = 'g', linestyle = 'dashed')
plt.axvline(x=2850, color = 'r', linestyle = 'dashed')
plt.axvline(x=4250, color = 'g', linestyle = 'dashed')
plt.axvline(x=5250, color = 'r', linestyle = 'dashed')
plt.title('Chart')
plt.xlabel('Frame number')
plt.ylabel('value')
plt.show()

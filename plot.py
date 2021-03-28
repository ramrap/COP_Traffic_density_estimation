from matplotlib import pyplot as plt
from matplotlib import style
import numpy as np

style.use('ggplot')
x,y,z = np.loadtxt('data.csv',unpack=True,delimiter=',')

plt.plot(x,y)
plt.plot(x,z)
plt.title('Chart')
plt.xlabel('Frame number')
plt.ylabel('value')
plt.show()
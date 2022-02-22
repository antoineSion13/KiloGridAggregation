import csv
import matplotlib
import matplotlib.pyplot as plt
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import scipy.linalg
import random as rd
import seaborn as sns
sns.set_theme()
import pandas as pd
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42
#2d histogram for non informed robots

dataNonInformed = [[] for i in range(3)]

for i in range(1,51):
	with open("Results/results_non_informed" + str(i) + ".csv") as csv_file:
		csv_reader = csv.reader(csv_file, delimiter=',')
		lineCount = 0
		for row in csv_reader:
			if lineCount == 36000*2-1:
				dataNonInformed[0].append(int(row[1])) #black site
				dataNonInformed[1].append(int(row[2])) #white site
				dataNonInformed[2].append(50 - int(row[1]) - int(row[2]))
			lineCount += 1

print(dataNonInformed)
dataNonInformed = pd.DataFrame(data = dataNonInformed, index = ["black", "white", "lost"]).transpose()
print(dataNonInformed)
fig = plt.figure(figsize = (10,8))
ax = sns.histplot(data=dataNonInformed, x="black", y="white", cbar = True, binwidth = 2, color = "gray")
ax.set_ylim(0,50)
ax.set_xlim(0,50)
# fig.suptitle("Histogram of the number of robots on each site after 300000 timesteps without informed robots with the new probability to leave (50 runs)")
plt.xlabel("Number of robots on the black site")
plt.ylabel("Number of robots on the white site")
plt.show()

print("Median of the number of lost robots on all the runs : " + str(dataNonInformed.loc[:,"lost"].median()))
print("Interquartile range of the number of lost robots on all the runs : " + str(dataNonInformed.loc[:,"lost"].quantile(q = 0.75) - dataNonInformed.loc[:,"lost"].quantile(q = 0.25)))

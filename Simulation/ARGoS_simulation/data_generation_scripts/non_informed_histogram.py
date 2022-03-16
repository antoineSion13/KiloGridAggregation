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

fileEntryPoint = "Results/StripesRigorousTest/05_225_2s_ni34_b8_w8/"
swarmSize = 50
dataNonInformed = [[] for i in range(3)]

for i in range(1,51):
	with open(fileEntryPoint + "results_non_informed" + str(i) + ".csv") as csv_file:
		csv_reader = csv.reader(csv_file, delimiter=',')
		lineCount = 0
		for row in csv_reader:
			if lineCount == 36000*2-1:
				dataNonInformed[0].append(int(row[1])) #black site
				dataNonInformed[1].append(int(row[2])) #white site
				dataNonInformed[2].append(swarmSize - int(row[1]) - int(row[2]))
			lineCount += 1

print(dataNonInformed)
dataNonInformed = pd.DataFrame(data = dataNonInformed, index = ["black", "white", "lost"]).transpose()
print(dataNonInformed)
fig = plt.figure(figsize = (10,8))
ax = sns.histplot(data=dataNonInformed, x="black", y="white", cbar = True, binwidth = 2, color = "gray", vmax = 10)
ax.set_ylim(0,swarmSize)
ax.set_xlim(0,swarmSize)
# fig.suptitle("Histogram of the number of robots on each site after 300000 timesteps without informed robots with the new probability to leave (50 runs)")
plt.xlabel("Number of robots on the black site")
plt.ylabel("Number of robots on the white site")
plt.show()

print("Median of the number of black robots on all the runs : " + str(dataNonInformed.loc[:,"black"].median()))
print("Median of the number of white robots on all the runs : " + str(dataNonInformed.loc[:,"white"].median()))
print("Median of the number of lost robots on all the runs : " + str(dataNonInformed.loc[:,"lost"].median()))
print("Interquartile range of the number of lost robots on all the runs : " + str(dataNonInformed.loc[:,"lost"].quantile(q = 0.75) - dataNonInformed.loc[:,"lost"].quantile(q = 0.25)))

dataNonInformedTime = [[[] for i in range(50)] for j in range(3)]

for i in range(1,51):
	with open(fileEntryPoint + "results_non_informed" + str(i) + ".csv") as csv_file:
		csv_reader = csv.reader(csv_file, delimiter=',')
		lineCount = 0
		for row in csv_reader:
			if lineCount%2 != 0:
				dataNonInformedTime[0][i-1].append(int(row[1])) #black site
				dataNonInformedTime[1][i-1].append(int(row[2])) #white site
				dataNonInformedTime[2][i-1].append(swarmSize - int(row[1]) - int(row[2]))
			lineCount += 1
print(np.shape((dataNonInformedTime)))

#plot a random run
fig, axs = plt.subplots()

# axs.plot(range(0,36000), dataNonInformedTimePercentile[0][0], '--', color = "black", label = "25%")
axs.plot(range(0,36000), dataNonInformedTime[0][0], color = "black", label = "median")
# axs.plot(range(0,36000), dataNonInformedTimePercentile[0][2], '--', color = "black", label = "75%")
# axs.plot(range(0,36000), dataNonInformedTimePercentile[1][0], '--', color = "white")
axs.plot(range(0,36000), dataNonInformedTime[1][0], color = "white")
# axs.plot(range(0,36000), dataNonInformedTimePercentile[1][2], '--', color = "white")

axs.set_xlim(0,36000)
axs.set_ylim(0,50)
axs.set_facecolor('grey')
axs.set_xlabel("timestep")
plt.show()

dataNonInformedTimePercentile = [[[] for i in range(36000)] for j in range(3)]

for i in range(3):
	transposed = np.transpose(dataNonInformedTime[i])
	print(np.shape(transposed))
	for j in range(36000):
		dataNonInformedTimePercentile[i][j] = np.median(transposed[j])
	# dataNonInformedTimePercentile[i] = np.transpose(dataNonInformedTimePercentile[i])

# print(dataNonInformedTimePercentile[0])


fig, axs = plt.subplots()

# axs.plot(range(0,36000), dataNonInformedTimePercentile[0][0], '--', color = "black", label = "25%")
axs.plot(range(0,36000), dataNonInformedTimePercentile[0], color = "black", label = "median")
# axs.plot(range(0,36000), dataNonInformedTimePercentile[0][2], '--', color = "black", label = "75%")
# axs.plot(range(0,36000), dataNonInformedTimePercentile[1][0], '--', color = "white")
axs.plot(range(0,36000), dataNonInformedTimePercentile[1], color = "white")
# axs.plot(range(0,36000), dataNonInformedTimePercentile[1][2], '--', color = "white")

axs.set_xlim(0,36000)
axs.set_ylim(0,50)
axs.set_facecolor('grey')
axs.set_xlabel("timestep")
plt.show()

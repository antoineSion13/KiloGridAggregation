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

fileEntryPoint = "Results/BiggerStepAndEntryStep/time_18000_ni_34_b_16_w_0_samp_1_alpha_0.5_beta_2.25_weight_1_INCREMENT_4/"
swarmSize = 50
experimentTime = 180000
dataNonInformed = [[] for i in range(3)]

for i in range(1,51):
	with open(fileEntryPoint + "results_non_informed" + str(i) + ".csv") as csv_file:
		csv_reader = csv.reader(csv_file, delimiter=',')
		lineCount = 0
		for row in csv_reader:
			if lineCount == experimentTime*2-1:
				dataNonInformed[0].append(int(row[1])) #black site
				dataNonInformed[1].append(int(row[2])) #white site
				dataNonInformed[2].append(swarmSize - int(row[1]) - int(row[2]))
			lineCount += 1

print(dataNonInformed)
dataNonInformed = pd.DataFrame(data = dataNonInformed, index = ["black", "white", "lost"]).transpose()
print(dataNonInformed)
# fig = plt.figure(figsize = (10,8))
# ax = sns.histplot(data=dataNonInformed, x="black", y="white", cbar = True, binwidth = 2, color = "gray", vmax = 10)
# ax.set_ylim(0,swarmSize)
# ax.set_xlim(0,swarmSize)
# # fig.suptitle("Histogram of the number of robots on each site after 300000 timesteps without informed robots with the new probability to leave (50 runs)")
# plt.xlabel("Number of robots on the black site")
# plt.ylabel("Number of robots on the white site")
# plt.show()

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
# for i in range(0,50):
# 	fig, axs = plt.subplots()
#
# 	# axs.plot(range(0,36000), dataNonInformedTimePercentile[0][0], '--', color = "black", label = "25%")
# 	axs.plot(range(0,experimentTime), dataNonInformedTime[0][i], color = "black", label = "median")
# 	# axs.plot(range(0,36000), dataNonInformedTimePercentile[0][2], '--', color = "black", label = "75%")
# 	# axs.plot(range(0,36000), dataNonInformedTimePercentile[1][0], '--', color = "white")
# 	axs.plot(range(0,experimentTime), dataNonInformedTime[1][i], color = "white")
# 	# axs.plot(range(0,36000), dataNonInformedTimePercentile[1][2], '--', color = "white")
#
# 	axs.set_xlim(0,experimentTime)
# 	axs.set_ylim(0,50)
# 	axs.set_facecolor('grey')
# 	axs.set_xlabel("timestep")
# 	plt.show()

dataNonInformedTimePercentile25 = [[[] for i in range(experimentTime)] for j in range(3)]
dataNonInformedTimePercentile50 = [[[] for i in range(experimentTime)] for j in range(3)]
dataNonInformedTimePercentile75 = [[[] for i in range(experimentTime)] for j in range(3)]

for i in range(3):
	transposed = np.transpose(dataNonInformedTime[i])
	print(np.shape(transposed))
	for j in range(experimentTime):
		dataNonInformedTimePercentile25[i][j] = np.percentile(transposed[j],25)
		dataNonInformedTimePercentile50[i][j] = np.percentile(transposed[j],50)
		dataNonInformedTimePercentile75[i][j] = np.percentile(transposed[j],75)
	# dataNonInformedTimePercentile[i] = np.transpose(dataNonInformedTimePercentile[i])

# print(dataNonInformedTimePercentile[0])

rangeXTicks = range(0,20,2)
rangeXTicksString = []

for i in rangeXTicks:
	if i == 0:
		rangeXTicksString.append((str(i)))
	else:
		rangeXTicksString.append((str(i)+'$e3$'))


fig, axs = plt.subplots()

plt.xticks(fontsize=40)
plt.yticks(fontsize=40)
axs.plot(range(0,experimentTime), dataNonInformedTimePercentile25[0], '--', linewidth = 0.5, color = "black", label = "25%")
axs.plot(range(0,experimentTime), dataNonInformedTimePercentile50[0], color = "black", label = "median")
axs.plot(range(0,experimentTime), dataNonInformedTimePercentile75[0], '--',linewidth = 0.5, color = "black", label = "75%")
axs.plot(range(0,experimentTime), dataNonInformedTimePercentile25[1], '--',linewidth = 0.5, color = "white", label = "25%")
axs.plot(range(0,experimentTime), dataNonInformedTimePercentile50[1], color = "white", label = "median")
axs.plot(range(0,experimentTime), dataNonInformedTimePercentile75[1], '--',linewidth = 0.5, color = "white", label = "75%")
axs.plot(range(0,experimentTime), dataNonInformedTimePercentile25[2], '--',linewidth = 0.5, color = "blue", label = "25%")
axs.plot(range(0,experimentTime), dataNonInformedTimePercentile50[2], color = "blue", label = "median")
axs.plot(range(0,experimentTime), dataNonInformedTimePercentile75[2], '--',linewidth = 0.5, color = "blue", label = "75%")

axs.set_xlim(0,int(experimentTime))
axs.set_xticklabels(rangeXTicksString)
axs.set_ylim(0,50)
axs.set_facecolor('grey')
axs.set_ylabel("N", rotation = 0, fontsize=50)
axs.yaxis.set_label_coords(-.075, .5)
axs.set_xlabel("Time [s]", fontsize=50)
plt.show()

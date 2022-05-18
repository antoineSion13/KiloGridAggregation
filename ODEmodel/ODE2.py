from scipy.integrate import odeint
from lmfit import minimize, Parameters, Parameter, report_fit
import csv
import matplotlib
import matplotlib.pyplot as plt
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import scipy.linalg
import random as rd
import pandas as pd
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42
#2d histogram for non informed robots

fileEntryPoint = "../Simulation/ARGoS_simulation/data_generation_scripts/Results/BiggerStepAndEntryStep/time_18000_ni_34_b_16_w_0_samp_2_alpha_0.5_beta_2.25_weight_1/"
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

# print(dataNonInformed)
dataNonInformed = pd.DataFrame(data = dataNonInformed, index = ["black", "white", "lost"]).transpose()
# print(dataNonInformed)
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

# rangeXTicks = range(0,20,2)
# rangeXTicksString = []
#
# for i in rangeXTicks:
# 	if i == 0:
# 		rangeXTicksString.append((str(i)))
# 	else:
# 		rangeXTicksString.append((str(i)+'$e3$'))


# fig, axs = plt.subplots()
#
# axs.grid(False)
# plt.xticks(fontsize=40)
# plt.yticks(fontsize=40)
# axs.plot(range(0,experimentTime), dataNonInformedTimePercentile25[0], '--', linewidth = 0.5, color = "black", label = "25%")
# axs.plot(range(0,experimentTime), dataNonInformedTimePercentile50[0], color = "black", label = "median")
# axs.plot(range(0,experimentTime), dataNonInformedTimePercentile75[0], '--',linewidth = 0.5, color = "black", label = "75%")
# axs.plot(range(0,experimentTime), dataNonInformedTimePercentile25[1], '--',linewidth = 0.5, color = "grey", label = "25%")
# axs.plot(range(0,experimentTime), dataNonInformedTimePercentile50[1], color = "grey", label = "median")
# axs.plot(range(0,experimentTime), dataNonInformedTimePercentile75[1], '--',linewidth = 0.5, color = "grey", label = "75%")
# # axs.plot(range(0,experimentTime), dataNonInformedTimePercentile25[2], '--',linewidth = 0.5, color = "blue", label = "25%")
# # axs.plot(range(0,experimentTime), dataNonInformedTimePercentile50[2], color = "blue", label = "median")
# # axs.plot(range(0,experimentTime), dataNonInformedTimePercentile75[2], '--',linewidth = 0.5, color = "blue", label = "75%")
#
# axs.set_xlim(0,int(experimentTime))
# axs.set_xticklabels(rangeXTicksString)
# axs.set_ylim(0,50)
# # axs.set_facecolor('white')
# axs.set_ylabel("N", rotation = 0, fontsize=50)
# axs.yaxis.set_label_coords(-.075, .5)
# axs.set_xlabel("Time [s]", fontsize=50)
# plt.show()

Xdata = np.array(dataNonInformedTimePercentile50[0])/50
Ydata = np.array(dataNonInformedTimePercentile50[1])/50

def model(x, t, paras):
    Xi, Yi, Xni, Yni = x

    try:
        ts = paras['ts'].value
        s = paras['s'].value
        S = paras['S'].value
        ent = paras['ent'].value
        beta = paras['beta'].value
        alpha = paras['alpha'].value
        iX = paras['iX'].value
        iY = paras['iY'].value
        pushed = paras['pushed'].value

    except KeyError:
        ts, s, S, ent, beta, alpha, iX, iY, pushed = paras

    rhoX = (alpha * np.exp(-beta*s*(Xi + Xni)*S))/ts
    rhoY = (alpha * np.exp(-beta*s*(Yi + Yni)*S))/ts
    dXidt = ent * (1 - Xni - Xi) * (iX - Xi)
    dYidt = ent * (1 - Yni - Yi) * (iY - Yi)
    dXnidt = -rhoX * Xni + ent * (1 - Xni - Xi) * ((1 - iX - iY) - Xni - Yni) - pushed * (Xni + Xi) * Xni
    dYnidt = -rhoY * Yni + ent * (1 - Yni - Yi) * ((1 - iX - iY) - Xni - Yni) - pushed * (Yni + Yi) * Yni
    return dXidt, dYidt, dXnidt, dYnidt

def g(t, x0, paras):
    """
    Solution to the ODE
    """
    x = odeint(model, x0, t, args=(paras,))
    return x

def residual(paras, t, data):

    """
    compute the residual between actual data and fitted data
    """

    x0 = 0,0,0,0
    model = g(t, x0, paras)

    x_model = model[:, 0] + model[:, 2]
    y_model = model[:, 1] + model[:, 3]
    return (x_model - data[0]).ravel() #+ (y_model - data[1]).ravel()

x0 = 0,0,0,0
t_measured = np.linspace(0, 18000, 180000)

plt.figure()
plt.scatter(t_measured, Xdata, marker='o', color='b', label='measured data for X')
plt.scatter(t_measured, Ydata, marker='o', color='g', label='measured data for Y')

# set parameters including bounds; you can also fix parameters (use vary=False)
ts = 2 #sampling time
s = 0.25 #portion of the site that a robot perceive
S = 50 #swarm size
ent = 0.001 #rate of entry
beta = 2.25
alpha = 0.5
iX = 0.3 #number of informed robots for black
iY = 0 #number of informed robots for black
pushed = 0.0002

params = Parameters()
params.add('ts', value=2, vary=False)
params.add('s', value=0.25, vary=False)
params.add('S', value=50, vary=False)
params.add('ent', value=0.0026, min=0, max = 0.5)
params.add('beta', value=2.25, vary=False)
params.add('alpha', value=0.5, vary=False)
params.add('iX', value=0.3, vary=False)
params.add('iY', value=0, vary=False)
params.add('pushed',value=0.0003, min=0, max = 0.5)

# fit model
totalData = [[],[]]
totalData[0] = Xdata
totalData[1] = Ydata
result = minimize(residual, params, args=(t_measured, totalData), method='leastsq')  # leastsq nelder
# check results of the fit
data_fitted = g(np.linspace(0, 18000, 180000), x0, result.params)

# plot fitted data
plt.plot(np.linspace(0, 18000, 180000), data_fitted[:, 0] + data_fitted[:, 2], '-', linewidth=2, color='red', label='fitted data')
plt.plot(np.linspace(0, 18000, 180000), data_fitted[:, 1] + data_fitted[:, 3], '-', linewidth=2, color='orange', label='fitted data')
plt.legend()
plt.xlim([0, max(t_measured)])
plt.ylim([0, 1.1])
# display fitted statistics
report_fit(result)

plt.show()



# sol = odeint(model, x0, t, args=(ts, s, S, ent, beta, alpha, iX, iY, pushed))
#
# plt.plot(t, sol[:, 0] + sol[:, 2], 'r', label='X(t)')
# plt.plot(t, sol[:, 1] + sol[:, 3], 'g', label='Y(t)')
# plt.legend(loc='best')
# plt.xlabel('t')
# ax = plt.gca()
# ax.set_ylim([0, 1])
# ax.set_xlim([0, 18000])
# plt.grid()
# plt.show()
#
# from scipy.optimize import fsolve
#
# eq = fsolve(model, x0, args=(0, ts, s, S, ent, beta, alpha, iX, iY, pushed))
# print("X at steady state : " + str(eq[0] + eq[2]))
# print("Y at steady state : " + str(eq[1] + eq[3]))

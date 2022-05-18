from scipy.integrate import odeint
import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import odeint
from lmfit import minimize, Parameters, Parameter, report_fit

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

x0 = 0,0,0,0
t_measured = np.linspace(0, 18000, 180000)

plt.figure()
params = Parameters()
params.add('ts', value=8, vary=False)
params.add('s', value=0.25, vary=False)
params.add('S', value=50, vary=False)
params.add('ent', value=0.0026, vary=False)
params.add('beta', value=2.25, vary=False)
params.add('alpha', value=0.5, vary=False)
params.add('iX', value=0.3, vary=False)
params.add('iY', value=0.0, vary=False)
params.add('pushed', value=0.0003, vary=False)

result = g(t_measured, x0, params)

plt.plot(np.linspace(0, 18000, 180000), result[:, 0] + result[:, 2], '-', linewidth=2, color='red', label='X')
plt.plot(np.linspace(0, 18000, 180000), result[:, 1] + result[:, 3], '-', linewidth=2, color='orange', label='Y')
plt.legend()
plt.xlim([0, max(t_measured)])
plt.ylim([0, 1])
plt.show()


# ts = 5 #sampling time
# s = 0.25 #portion of the site that a robot perceive
# S = 50 #swarm size
# ent = 0.001 #rate of entry
# beta = 2.25
# alpha = 0.5
# iX = 0.3 #number of informed robots for black
# iY = 0 #number of informed robots for black
# pushed = 0
#
# def model(x, t, *args):
#     X, Y = x
#     ts, s, S, ent, beta, alpha, iX, iY, pushed = args
#     rhoX = (alpha * np.exp(-beta*s*(X)*S))/ts + pushed
#     rhoY = (alpha * np.exp(-beta*s*(Y)*S))/ts + pushed
#     dXdt = -rhoX * X + ent * (1 - X - Y)
#     dYdt = -rhoY * Y + ent * (1 - X - Y)
#     return dXdt, dYdt
#
# t = np.linspace(0, 20000, 200)
# X0 = iX
# Y0 = iY
# x0 = X0, Y0
# sol = odeint(model, x0, t, args=(ts, s, S, ent, beta, alpha, iX, iY, pushed))
#
# plt.plot(t, sol[:, 0] , 'r', label='X(t)')
# plt.plot(t, sol[:, 1] , 'g', label='Y(t)')
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
# print("X at steady state : " + str(eq[0]))
# print("Y at steady state : " + str(eq[1]))

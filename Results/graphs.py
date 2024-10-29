# Code for generation of results graphs
import matplotlib.pyplot as plt

activation_times = [0, 1, 2, 3, 4, 5, 6,7,8,9,10,11,12,13,14,15]  

final_speeds = [
    0.123194014, 0.123194014, 0.126756805, 0.113236446, 0.095248831, 0.100499095, 0.100112425, 0.09991978, 0.102262132, 0.100693121, 0.083368018, 0.086522568, 0.102858305, 0.086200676, 0.087989283, 0.090161694
]  

errors = [
    0.012,
    0.003,
    0.02,
    0.02,
    0.03,
    0.02,
    0.01,
    0.01,
    0.003,
    0.012,
    0.021,
    0.0121,
    0.013,
    0.01,
    0.0123,
    0.01,
]  

plt.figure(figsize=(10, 6))
plt.errorbar(activation_times, final_speeds, yerr=errors, fmt='o', capsize=5, label='Final Speed')

plt.title('Effect of Electromagnet Activation Timing on Final Speed')
plt.xlabel('Activation Time (ms)')
plt.ylabel('Acceleration (m/s^2)')
plt.legend()
plt.grid(True)
plt.show()

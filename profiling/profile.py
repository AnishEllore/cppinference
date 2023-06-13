#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


def plot_cdf():
    # Calculate the CDF values
# cdf_values = np.arange(1, len(mem_usage) + 1) / len(mem_usage)
    cdf_values = 1. * np.arange(len(mem_usage)) / (len(mem_usage) - 1)
    cdf_values = cdf_values/cdf_values[-1]

    # Plot the CDF
    plt.plot(mem_usage, cdf_values)
    plt.ylabel('CDF')
    plt.xlabel('Memory Usage (RSS) Gb')
    plt.title('CDF of Memory Consumption')
    plt.grid(True)
    plt.savefig('memory_usage_cdf.png')
    plt.show()
    
def plot_min_median_max(data1, data2, data3):
    # Example data


    # Calculate the minimum, median, and maximum values for each dataset
    min_values = [min(data1), min(data2), min(data3)]
    median_values = [np.median(data1), np.median(data2), np.median(data3)]
    max_values = [max(data1), max(data2), max(data3)]
    
    val1 = [min(data1), np.median(data1), max(data1)]
    val2 = [min(data2), np.median(data2), max(data2)]
    val3 = [min(data3), np.median(data3), max(data3)]

    # Plot the bar plot
    bar_width = 0.2
    index = np.arange(len(min_values))

    plt.bar(index, val1, bar_width, label='data1')
    plt.bar(index + bar_width, val2, bar_width, label='data2')
    plt.bar(index + 2*bar_width, val3, bar_width, label='data3')

    plt.xlabel('Statistics')
    plt.ylabel('Values')
    plt.title('Bar Plot: Minimum, Median, Maximum')
    plt.xticks(index + bar_width, ('Minimum', 'Median', 'Maximum'))
    plt.legend()
    plt.savefig('min_median_max.png')
    plt.show()
    
    

# Read the memory usage data from the log file
df = pd.read_csv('memory_usage.log', skiprows=2, delim_whitespace=True)

# Calculate the cumulative distribution function (CDF)
mem_usage = df['RSS']
mem_usage = mem_usage.sort_values()
mem_usage = mem_usage/(1024*1024)
data1 = [2, 4, 5, 7, 8, 8, 9, 11, 12, 13]
data2 = [1, 3, 6, 8, 9, 10, 11, 12, 13, 14]
plot_cdf()
plot_min_median_max(data1, data2, mem_usage)


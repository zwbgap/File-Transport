import matplotlib.pyplot as plt

# 假设我们已经收集了以下数据
threads = [1, 2, 4, 8, 16]
bandwidth = [106.68, 122.61, 193.33, 125.57, 126.08]  # 这些是示例数据
# 1 106.68
# 2: (61.79+61.82) = 122.61
# 4: (45.66+48.99+44.88+53.80) = 193.33
# 8: (15.70+15.60+15.81+15.93+15.74+15.41+15.56+15.82) = 125.57
# 16:(7.89+7.84+7.86+7.81+7.84+7.88+7.94+7.86+7.84+7.85+7.83+7.85+7.98+8.04+7.93+7.84) = 126.08
plt.plot(threads, bandwidth, marker='o')
plt.xlabel('Number of Threads')
plt.ylabel('Total Bandwidth (MB/s)')
plt.title('Threads vs Bandwidth')
plt.grid(True)
plt.show()
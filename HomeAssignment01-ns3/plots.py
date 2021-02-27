import matplotlib.pyplot as plt

data_rate = [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52]
thpt = [0, 3.86549, 7.72853, 11.5867, 15.4473, 19.3055, 23.1662, 24.5734, 24.5681, 24.5758, 24.5758, 24.5581, 24.5684, 24.5586]

plt.figure()
plt.xlabel("CBR Data Rate (Mbps)")
plt.ylabel("Throughput (Mbps)")
plt.title("Single-flow CBR Throughput vs CBR Data Rate")
plt.plot(data_rate, thpt)
plt.savefig("single_flow_cbr.png")

window_size = [0, 1000, 1100, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000]
thpt = [0, 0.0237772, 12.3844, 12.3924, 12.6893, 12.4505, 12.5319, 12.5783, 12.5633, 12.531, 12.5588]

plt.figure()
plt.xlabel("Sender Window Size (bytes)")
plt.ylabel("Throughput (Mbps)")
plt.title("Single-flow FTP Throughput vs Sender Window Size")
plt.plot(window_size, thpt)
plt.savefig("single_flow_ftp.png")
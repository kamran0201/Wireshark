import os
import numpy as np
import matplotlib.pyplot as plt

delays = ["10ms", "50ms", "100ms"]
delays_in_millis = [10, 50, 100]
losses = ["0.1%", "0.5%", "1%"]
losses_without_ampersand = [0.1, 0.5, 1]
tcps = ["cubic", "reno"]
# Z for 90% = 1.645 / sqrt(n) = 1.645 / sqrt(20)
Z = 1.645 / (20 **  0.5)

for loss in losses:
	# Find file names in which data is stored
	file_names = [["results_delay_" + d + "_loss_" + loss + "_tcp_cubic.txt" for d in delays], ["results_delay_" + d + "_loss_" + loss + "_tcp_reno.txt" for d in delays]]
	results = np.zeros([2, 3, 2])
	for i in range(len(file_names)):
		tcp_type = file_names[i]
		for j in range(len(tcp_type)):
			file_name = tcp_type[j]
			sum_throughput = 0
			sum_square = 0
			with open(file_name) as infile:
				for line in infile:
					# Calculate throughput
					# data[0] = Bytes transferred. Multiply by 8 to get bits.
					# data[1] = time in microseconds. Divide by 1e6 to get time in s.
					data = line.strip().split(' ')
					bits = int(data[0]) * 8
					time_in_s = float(data[1]) / (10 ** 6)
					throughput = bits / time_in_s
					# Add x to get sum x
					# Add x^2 to get sum x^2
					sum_throughput += throughput
					sum_square += (throughput * throughput)
			# Mean = E(x) = sum / n
			mean = sum_throughput / 20
			# std = sqrt(E(x^2) - E(x)^2) = sqrt((sum x^2 / n) - mean^2)
			std = ((sum_square / 20) - (mean ** 2)) ** (0.5)
			results[i][j][0] = mean
			results[i][j][1] = std
	# Store throughputs in array
	throughputs_y = [results[0][count][0] / (10 ** 6) for count in range(len(delays))]
	M = max(throughputs_y)
	# Error bars at x +- Z . std
	error_bars_throughputs = [Z * results[0][count][1] / (10 ** 6) for count in range(len(delays))]
	# plot given data with legend
	plt.errorbar(delays_in_millis, throughputs_y, yerr = error_bars_throughputs, label = "Cubic")
	throughputs_y = [results[1][count][0] / (10 ** 6) for count in range(len(delays))]
	M = max( max(throughputs_y), M)
	error_bars_throughputs = [Z * results[1][count][1] / (10 ** 6) for count in range(len(delays))]
	plt.errorbar(delays_in_millis, throughputs_y, yerr = error_bars_throughputs, label = "Reno")
	plt.title("Throughput vs Delay for Loss = " + loss)
	plt.xlabel("Delay (in ms)")
	plt.ylim([0, 1.5 * M])
	plt.ylabel(r"Throughput (in $10^6$ bits/s)")
	plt.legend()
	name = "Throughput vs Delay for Loss = " + loss + ".png"
	# save figure
	plt.savefig(name)
	# show figure
	plt.show()
			
for delay in delays:
	# Find file names in which data is stored
	file_names = [["results_delay_" + delay + "_loss_" + loss + "_tcp_cubic.txt" for loss in losses], ["results_delay_" + delay + "_loss_" + loss + "_tcp_reno.txt" for loss in losses]]
	results = np.zeros([2, 3, 2])
	for i in range(len(file_names)):
		tcp_type = file_names[i]
		for j in range(len(tcp_type)):
			file_name = tcp_type[j]
			sum_throughput = 0
			sum_square = 0
			with open(file_name) as infile:
				for line in infile:
					# Calculate throughput
					# data[0] = Bytes transferred. Multiply by 8 to get bits.
					# data[1] = time in microseconds. Divide by 1e6 to get time in s.
					data = line.split(' ')
					bits = int(data[0]) * 8
					time_in_s = float(data[1]) / (10 ** 6)
					throughput = bits / time_in_s
					# Add x to get sum x
					# Add x^2 to get sum x^2
					sum_throughput += throughput
					sum_square += (throughput ** 2)
			# Mean = E(x) = sum / n
			mean = sum_throughput / 20
			# std = sqrt(E(x^2) - E(x)^2) = sqrt((sum x^2 / n) - mean^2)
			std = ((sum_square / 20) - (mean ** 2)) ** (0.5)
			results[i][j][0] = mean
			results[i][j][1] = std
		
	# Store throughputs in array
	throughputs_y = [results[0][count][0] / (10 ** 6) for count in range(len(losses))]
	M = max(throughputs_y)
	# Error bars at x +- Z . std
	error_bars_throughputs = [Z * results[0][count][1] / (10 ** 6) for count in range(len(losses))]
	# plot given data with legend
	plt.errorbar(losses_without_ampersand, throughputs_y, yerr = error_bars_throughputs, label = "Cubic")
	throughputs_y = [results[1][count][0] / (10 ** 6) for count in range(len(losses))]
	M = max(max(throughputs_y), M)
	error_bars_throughputs = [Z * results[1][count][1] / (10 ** 6) for count in range(len(losses))]
	plt.errorbar(losses_without_ampersand, throughputs_y, yerr = error_bars_throughputs, label = "Reno")
	plt.title("Throughput vs Loss for Delay = " + delay)
	plt.xlabel("Loss (in %)")
	plt.ylim([0, 1.5 * M])
	plt.ylabel(r"Throughput (in $10^6$ bits/s)")
	plt.legend()
	name = "Throughput vs Loss for Delay = " + delay + ".png"
	# save figure
	plt.savefig(name)
	# show figure
	plt.show()

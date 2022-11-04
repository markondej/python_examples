import math
import dft

signal = []
for i in range(0, 8):
    signal.append(math.sin(i * 0.5 * math.pi / 8.0))
    signal.append(0.0)
print(dft.fft(signal))

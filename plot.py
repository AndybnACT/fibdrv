import matplotlib.pyplot as plt

class data(object):
	"""fibonacci number processing time data"""

	def __init__(self, str):
		lines = str.split("\n")[:-1]
		self.k = []
		self.time = []
		self.error = []
		for l in lines:
			[title, info] = l.split(":")
			self.k.append(int(title.split(",")[0]))
			self.time.append(float(info.split(",")[0]))
			self.error.append(float(info.split(",")[1])**0.5)

with open("fib_r.dat") as f:
	str = f.read()
	regular = data(str)
	
with open("fib_d.dat") as f:
	str = f.read()
	doubling = data(str)

with open("fib_d_ll.dat") as f:
	str = f.read()
	doubling_ll = data(str)

with open("fib_r_ll.dat") as f:
	str = f.read()
	regular_ll = data(str)


fig, (ax, ax2) =  plt.subplots(nrows=2, sharex=True)
ax.set_title('Execution Time of F(k) Using Custom uint128_t')
ax.errorbar(regular.k, regular.time, yerr=regular.error, fmt="-")
ax.errorbar(doubling.k, doubling.time, yerr=doubling.error, fmt="-")
# ax.legend(("Regular", "Fast Doubling"))
ax.set_ylabel("nsec")

ax2.set_title('Execution Time of F(k) Using Builtin u64')
ax2.errorbar(regular_ll.k, regular_ll.time, yerr=regular_ll.error, fmt="-")
ax2.errorbar(doubling_ll.k, doubling_ll.time, yerr=doubling_ll.error, fmt="-")
ax2.legend(("Regular", "Fast Doubling"))
ax2.set_xlabel("k")
ax2.set_ylabel("nsec")
# ax2.set_yscale('log')

plt.show()
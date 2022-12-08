import matplotlib.pyplot as plt
x = [i + 1 for i in range(10)]
y = [4.812682, 4.845835, 4.842088, 2.066343, 1.621507, 1.937155, 1.946944, 1.452831, 1.244667, 1.236065]
plt.figure()
plt.plot(x, y)
plt.xlabel('Процессоры')
plt.ylabel('Время')

y = [y[0] / y[i] for i in range(10)]
plt.figure()
plt.plot(x, y)
plt.xlabel('Процессоры')
plt.ylabel('Ускорение')

y = [y[i] / x[i] for i in range(10)]
plt.figure()
plt.plot(x, y)
plt.xlabel('Процессоры')
plt.ylabel('Эффективность')
plt.show()

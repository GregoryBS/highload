import matplotlib.pyplot as plt
x = [i + 1 for i in range(10)]
y = [70.125638, 108.325212, 108.367683, 108.537704, 108.526555, 108.556134, 108.400593, 108.883118, 109.096955, 66.790136]
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

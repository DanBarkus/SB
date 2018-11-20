import matplotlib.pyplot as plt
import csv

fileNum = 16

def saveFile(i):
    x = []
    y = []
    test_file = 'test_data_%i.csv' % i
    csvfile = open(test_file , "r" ).readlines()[::2]
    plots = csv.reader(csvfile, delimiter=',')
    print(plots)
    for row in plots:
        x.append(float(row[0]))
        y.append(float(row[1]))

    plt.plot(x,y, label='pressure')
    plt.xlabel('x')
    plt.ylabel('y')
    plt.grid(True)
    plt.title(test_file)
    plt.legend()
    # plt.show()
    plt.savefig('./images/%i.png' % i)
    plt.clf()
    plt.cla()
    plt.close()


for i in range(14,fileNum + 1):
    saveFile(i)
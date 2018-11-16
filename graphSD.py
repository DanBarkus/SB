import matplotlib.pyplot as plt
import csv
import statistics
import os, os.path

# fileRoot = "test_data\\"
# filePrefix = "test_data_"

fileRoot = "test_data\\Home\\"
filePrefix = "DATA_"
imageDirectory = fileRoot + "\\images"

# files = path, dirs, files = next(os.walk(fileRoot))
# fileNum = len(files)

fileNum = 2

def saveFile(i):
    x = []
    y = []
    score = []
    test_file = fileRoot + filePrefix + str(i) + '.CSV'
    try:
        csvfile = open(test_file , "r" ).readlines()
        plots = csv.reader(csvfile, delimiter=',')
        if next(plots)[0] == "initReading":
            print("Has Heading")
            next(plots)
            next(plots)
        for row in plots:
            x.append(float(row[0]))
            y.append(float(row[1]))
            score.append(float(row[3]))
        plot, ax1 = plt.subplots()
        ax2 = ax1.twinx()
        ax1.plot(x,y, label='pressure')
        ax2.set_ylabel("score", color='r')
        ax2.plot(x,score, 'r')
        plt.xlabel('x')
        plt.grid(True)
        plt.title(test_file)
        plt.legend()
        plt.show()
        timeDiff = []
        for e, time in enumerate(x):
            try:
                diff = x[e+1] - time
                timeDiff.append(diff)
            except:
                continue
        # print("Times between Readings")
        # print("mean: %f" % statistics.mean(timeDiff))
        # print("median: %f" % statistics.median(timeDiff))
        # print("mode: %f" % statistics.mode(timeDiff))
        # print("min: %f" % min(timeDiff))
        # print("max: %f" % max(timeDiff))
        # plt.savefig('imageDirectory/%i.png' % i)
        # plt.clf()
        # plt.cla()
        # plt.close()
    except:
        print("Failed to find File: " + test_file)
        return


for i in range(1,fileNum + 1):
    saveFile(i)
import matplotlib.pyplot as plt
import csv
import statistics
import os, os.path

# fileRoot = "test_data\\"
# filePrefix = "test_data_"

fileRoot = "test_data\\TheOne\\"
filePrefix = "DATA_"
imageDirectory = fileRoot + "\\images"

if not os.path.exists(imageDirectory):
    os.makedirs(imageDirectory)

# files = path, dirs, files = next(os.walk(fileRoot))
# fileNum = len(files)

fileNum = 61

def saveFile(i):
    x = []
    y = []
    score = []
    rate = []
    timeDiff = []
    scoreDiff = []
    test_file = fileRoot + filePrefix + str(i) + '.CSV'
    try:
        csvfile = open(test_file , "r" ).readlines()
        plots = csv.reader(csvfile, delimiter=',')
        if next(plots)[0] == "initReading":
            print("Has Heading")
            next(plots)
            next(plots)
        for e, row in enumerate(plots):
            x.append(float(row[0]))
            y.append(abs(float(row[1])))
            score.append(float(row[3]))
            rate.append(float(row[2]))
            try:
                diff = x[e] - x[e-1]
                if diff == 0:
                    diff = 23
                timeDiff.append(diff)
            except:
                timeDiff.append(23)
                continue

            try:
                diff = score[e] - score[e-1]
                scoreDiff.append(diff)
            except:
                scoreDiff.append(0)
                continue
        plot, ax1 = plt.subplots()
        plot.set_size_inches(12,8)
        ax2 = ax1.twinx()
        ax3 = ax1.twinx()
        # ax4 = ax1.twinx()
        # ax5 = ax1.twinx()
        ax1.plot(x,y, label='pressure')
        ax2.set_ylabel("score", color='r')
        ax2.plot(x,score, 'r')
        ax3.set_ylabel("rate", color='g')
        ax3.plot(x,rate, 'g')
        # ax4.set_ylabel("timeDiff", color='m')
        # ax4.plot(x,timeDiff, 'm')
        # ax5.set_ylabel("scoreDiff", color='y')
        # ax5.plot(x,scoreDiff, 'y')
        plt.xlabel('Time')
        plt.grid(True)
        plt.title(test_file)
        # plt.legend()
        # plt.show()      
        print("Times between Readings")
        print("mean: %f" % statistics.mean(timeDiff))
        print("median: %f" % statistics.median(timeDiff))
        print("mode: %f" % statistics.mode(timeDiff))
        print("min: %f" % min(timeDiff))
        print("max: %f" % max(timeDiff))
        plt.savefig(imageDirectory + '\\%i.png' % i)
        plt.clf()
        plt.cla()
        plt.close()
    except Exception as e:
        print(e)
        print("Failed to find File: " + test_file)
        return


for i in range(0,fileNum + 1):
    saveFile(i)
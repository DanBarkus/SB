import csv
import statistics
import os
import os.path

# fileRoot = "test_data\\"
# filePrefix = "test_data_"

fileRoot = "test_data\\TheOne\\"
filePrefix = "DATA_"
imageDirectory = fileRoot + "\\images"

if not os.path.exists(imageDirectory):
    os.makedirs(imageDirectory)

files = path, dirs, files = next(os.walk(fileRoot))
fileNum = len(files)

# fileNum = 33

averageScore = []
averageRate = []
averageDuration = []


def analyzeFile(i):
    x = []
    pressure = []
    score = []
    rate = []
    timeDiff = []
    scoreDiff = []
    test_file = fileRoot + filePrefix + str(i) + '.CSV'
    try:
        csvfile = open(test_file, "r").readlines()
        plots = csv.reader(csvfile, delimiter=',')
        if next(plots)[0] == "initReading":
            print("Has Heading")
            next(plots)
            next(plots)
        for e, row in enumerate(plots):
            if (int(row[4]) == 1):
                x.append(float(row[0]))
                pressure.append(abs(float(row[1])))
                score.append(float(row[3]))
                rate.append(float(row[2]))
                try:
                    diff = x[-1] - x[-2]
                    if diff == 0:
                        diff = 23
                    timeDiff.append(diff)
                except:
                    timeDiff.append(23)
                    continue

                try:
                    diff = score[-1] - score[e-1]
                    scoreDiff.append(diff)
                except:
                    scoreDiff.append(0)
                    continue

        print("Max Score: %f" % score[-1])
        print("Max Rate: %f" % max(rate))
        print("Mode of Rate %f" % statistics.mode(rate))
        print("Hit Duration: %f" % sum(timeDiff)/1000)

        
        # print("Times between Readings")
        # print("mean: %f" % statistics.mean(timeDiff))
        # print("mode: %f" % statistics.mode(timeDiff))
        # print("min: %f" % min(timeDiff))
        # print("max: %f" % max(timeDiff))

        # averageRate.append(statistics.mean)
        # print("Times between Readings")
        # print("mean: %f" % statistics.mean(timeDiff))
        # print("median: %f" % statistics.median(timeDiff))
        # print("mode: %f" % statistics.mode(timeDiff))
        # print("min: %f" % min(timeDiff))
        # print("max: %f" % max(timeDiff))

    except Exception as e:
        print(e)
        print("Failed to find File: " + test_file)
        return


for i in range(0, fileNum + 1):
    analyzeFile(i)

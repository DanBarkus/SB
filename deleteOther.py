import os, os.path

path, dirs, files = next(os.walk("test_data"))
file_count = len(files)

for i in range(file_count):
    i+=1
    _file = open("test_data\\test_data_" + str(i) + "_2.csv", "r").readlines()[::2]
    _file2 = open("test_data\\test_data_" + str(i) + ".csv", "w")
    for i in _file:
        _file2.write(i)
    _file2.close()
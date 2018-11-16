_file = open("test_data_1.csv", "r")
text = _file.readlines()
for i, j in enumerate(text):
    if i % 2 == 0:
       del text[i]
       
_file.close()
_file = open("tets_data_12.csv", "w")
for i in text:
    _file.write(i)
    
_file.close()
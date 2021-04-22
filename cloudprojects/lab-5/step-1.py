# Import modules
import re
from pyspark import SparkContext, SparkConf

def cleanWords(words):
  clean1 = re.sub(r'(03:019:024)', '', words)
  clean2 = re.sub(r'([^A-Za-z0-9\s+])', '', clean1)
  splitwords  = clean2.split(' ')
  return [word.lower() for word in splitwords if word != '']

# Initialize Spark
master = "local"
appName = "lab5"
logLevel = "OFF"
conf = SparkConf().setMaster(master).setAppName(appName)
sc = SparkContext(conf = conf)
sc.setLogLevel(logLevel)

# Load data from files
words = sc.textFile('words.txt')
file1 = sc.textFile('file1.txt')
file2 = sc.textFile('file2.txt')
file3 = sc.textFile('file3.txt')

# Clean data
words = words.flatMap(cleanWords)
cleanFile1 = file1.flatMap(cleanWords)
cleanFile2 = file2.flatMap(cleanWords)
cleanFile3 = file3.flatMap(cleanWords)

# List the 5 first words (in ascending order) and from the words.txt which start with “b” and end with “t”.
print('\nFirst 5 Words:')
print(words.filter(lambda w: w[0] == 'b' and w[len(w)-1] == 't').take(5))

# List the 10 last longest words from the file words.txt.
print('\n10 Last Longest Words:')
print(words.map(lambda w: (len(w), w)).sortByKey(False).take(10))

# Calculate the number of lines and the number of distinct words from file1
print('\nLine Count:', file1.count())
print('\nDistinct Words:', cleanFile1.distinct().count())

# Find 3 common words in files1, file2 and file3 which their sum of frequencies (number of occurrences)
# within the three files is maximum compared to the other shared words. (e.g., assume that X is a set of
# words which are shared between all files, find common words like xi in X which maximize
# F(xi)= f1(count xi in file 1) +f2(count xi in file 2) +f3(count xi in file 3).

tuples1 = cleanFile1.map(lambda w: (w, 1)).reduceByKey(lambda x, y: x + y).sortBy(lambda x: x[1], ascending=False)
tuples2 = cleanFile2.map(lambda w: (w, 1)).reduceByKey(lambda x, y: x + y).sortBy(lambda x: x[1], ascending=False)
tuples3 = cleanFile3.map(lambda w: (w, 1)).reduceByKey(lambda x, y: x + y).sortBy(lambda x: x[1], ascending=False)
combined = tuples1.join(tuples2).join(tuples3)

print("\nCommon Words:")
print(combined.sortByKey(False).take(3))

# Group words for words.txt according to their first 4 characters and then output the number of members
# for the first 10 groups.

validWords = words.filter(lambda w: len(w) >= 4).groupBy(lambda x: x[:4]).map(lambda x: (x[0], len(x[1])))
print("\nWords Grouped By First Four Letters:")
print(validWords.take(10))

# Kill Spark
sc.stop()
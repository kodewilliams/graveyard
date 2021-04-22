# Import modules
import re
import pyspark
from stop_words import get_stop_words
from datetime import datetime

def cleanWords(words):
  stopWords = get_stop_words("english")
  clean1 = re.sub(r'(03:019:024)', '', words)
  clean2 = re.sub(r'([^A-Za-z0-9\s+])', '', clean1)
  splitwords  = clean2.split(' ')
  return [word.lower() for word in splitwords if word != '']

# Initialize Spark
master = "local"
appName = "lab5"
logLevel = "OFF"
conf = pyspark.SparkConf().setMaster(master).setAppName(appName)
sc = pyspark.SparkContext(conf = conf)
sc.setLogLevel(logLevel)

# Load data from file and clean
file1 = sc.textFile('file1.txt')
cleanFile1 = file1.flatMap(cleanWords)
stopWords = get_stop_words('en')
cleanFile1 = cleanFile1.filter(lambda w: w not in stopWords)

# Number of occurence (Mapreduce)
occurrences = sc.parallelize(sorted(cleanFile1.map(lambda w: (w, 1)).countByKey().items(), key=lambda x: x[1], reverse=True))
occurrences.persist(pyspark.StorageLevel.DISK_ONLY)
print('\nTop 10 Words in File 1:')
print(occurrences.take(10))

# Kill Spark
print('----- Program Ended ----')
print(datetime.now())
sc.stop()
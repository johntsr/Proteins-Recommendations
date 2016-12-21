OUT  	= medoids
CC		= g++
FLAGS   = -Wall -g -c -O3

DIR_SOURCE = ./Source/

DIR_CLUSTER 	= $(DIR_SOURCE)Clusters/
SOURCE_CLUSTER	= $(DIR_CLUSTER)*.cpp
HEADER_CLUSTER 	= $(DIR_CLUSTER)*.h

DIR_PROTEIN 	= $(DIR_SOURCE)Proteins/
SOURCE_PROTEIN	= $(DIR_CLUSTER)*.cpp
HEADER_PROTEIN 	= $(DIR_CLUSTER)*.h

DIR_DATA_STRUCT = $(DIR_SOURCE)DataStructures/
SOURCE_DATA 	=
HEADER_DATA 	= $(DIR_DATA_STRUCT)*.h

DIR_GENERAL 	= $(DIR_SOURCE)General/
SOURCE_GENERAL 	= $(DIR_GENERAL)*.cpp
HEADER_GENERAL 	= $(DIR_GENERAL)*.h

DIR_METRICS 	= $(DIR_SOURCE)Metrics/
SOURCE_METRICS 	= $(DIR_METRICS)*.cpp
HEADER_METRICS 	= $(DIR_METRICS)*.h

SOURCE			= $(SOURCE_CLUSTER) $(SOURCE_METRICS) $(SOURCE_GENERAL) $(DIR_SOURCE)Main.cpp
HEADER			= $(HEADER_CLUSTER) $(HEADER_METRICS) $(HEADER_GENERAL) $(HEADER_DATA)

DIR_BUILD = ./Build/
OBJS 		= $(DIR_BUILD)Main.o $(DIR_BUILD)Hamming.o $(DIR_BUILD)Euclidean.o $(DIR_BUILD)Vector.o $(DIR_BUILD)CosineSimilarity.o $(DIR_BUILD)HashFunction.o $(DIR_BUILD)Clusters.o $(DIR_BUILD)ProteinsManager.o $(DIR_BUILD)MetricSpace.o $(DIR_BUILD)Quantity.o $(DIR_BUILD)Initializer.o $(DIR_BUILD)ClusterStructures.o $(DIR_BUILD)Assigner.o $(DIR_BUILD)Updater.o


# -g option enables debugging mode
# -c flag generates object code for separate files

all: $(OUT)

$(OUT): $(OBJS) $(SOURCE) $(HEADER)
	$(CC) -g $(OBJS) -o $(OUT) -lcunit  -lgsl -lgslcblas

# create/compile the individual files >>separately<<

$(DIR_BUILD)ProteinsManager.o: $(DIR_PROTEIN)ProteinsManager.cpp $(HEADER)
	$(CC) $(FLAGS) $(DIR_PROTEIN)ProteinsManager.cpp -o $(DIR_BUILD)ProteinsManager.o

$(DIR_BUILD)Clusters.o: $(DIR_CLUSTER)Clusters.cpp $(HEADER)
	$(CC) $(FLAGS) $(DIR_CLUSTER)Clusters.cpp -o $(DIR_BUILD)Clusters.o

$(DIR_BUILD)ClusterStructures.o: $(DIR_CLUSTER)ClusterStructures.cpp $(HEADER)
	$(CC) $(FLAGS) $(DIR_CLUSTER)ClusterStructures.cpp -o $(DIR_BUILD)ClusterStructures.o

$(DIR_BUILD)Initializer.o: $(DIR_CLUSTER)Initializer.cpp $(HEADER)
	$(CC) $(FLAGS) $(DIR_CLUSTER)Initializer.cpp -o $(DIR_BUILD)Initializer.o

$(DIR_BUILD)Assigner.o: $(DIR_CLUSTER)Assigner.cpp $(HEADER)
	$(CC) $(FLAGS) $(DIR_CLUSTER)Assigner.cpp -o $(DIR_BUILD)Assigner.o

$(DIR_BUILD)Updater.o: $(DIR_CLUSTER)Updater.cpp $(HEADER)
	$(CC) $(FLAGS) $(DIR_CLUSTER)Updater.cpp -o $(DIR_BUILD)Updater.o


$(DIR_BUILD)Vector.o: $(DIR_GENERAL)Vector.cpp $(DIR_GENERAL)Vector.h $(DIR_GENERAL)Math.h $(HEADER_DATA)
	$(CC) $(FLAGS) $(DIR_GENERAL)Vector.cpp -o $(DIR_BUILD)Vector.o

$(DIR_BUILD)HashFunction.o: $(DIR_GENERAL)HashFunction.cpp $(DIR_GENERAL)HashFunction.h $(DIR_GENERAL)Point.h $(DIR_GENERAL)Math.h $(HEADER_DATA)
	$(CC) $(FLAGS) $(DIR_GENERAL)HashFunction.cpp -o $(DIR_BUILD)HashFunction.o

$(DIR_BUILD)Hamming.o: $(DIR_METRICS)Hamming.cpp $(DIR_METRICS)Hamming.h $(DIR_GENERAL)Point.h $(DIR_GENERAL)HashFunction.h $(HEADER_DATA)
	$(CC) $(FLAGS) $(DIR_METRICS)Hamming.cpp -o $(DIR_BUILD)Hamming.o

$(DIR_BUILD)CosineSimilarity.o: $(DIR_METRICS)CosineSimilarity.cpp $(DIR_METRICS)CosineSimilarity.h $(HEADER_GENERAL) $(HEADER_DATA)
	$(CC) $(FLAGS) $(DIR_METRICS)CosineSimilarity.cpp -o $(DIR_BUILD)CosineSimilarity.o

$(DIR_BUILD)Euclidean.o: $(DIR_METRICS)Euclidean.cpp $(DIR_METRICS)Euclidean.h $(HEADER_GENERAL) $(HEADER_DATA)
	$(CC) $(FLAGS) $(DIR_METRICS)Euclidean.cpp -o $(DIR_BUILD)Euclidean.o

$(DIR_BUILD)MetricSpace.o: $(DIR_METRICS)MetricSpace.cpp $(DIR_METRICS)MetricSpace.h $(HEADER_GENERAL) $(HEADER_DATA)
	$(CC) $(FLAGS) $(DIR_METRICS)MetricSpace.cpp -o $(DIR_BUILD)MetricSpace.o

$(DIR_BUILD)Quantity.o: $(DIR_GENERAL)Quantity.cpp $(DIR_GENERAL)Quantity.h
	$(CC) $(FLAGS) $(DIR_GENERAL)Quantity.cpp -o $(DIR_BUILD)Quantity.o


$(DIR_BUILD)Main.o: $(SOURCE) $(HEADER) mkdir
	$(CC) $(FLAGS) $(DIR_SOURCE)Main.cpp -o $(DIR_BUILD)Main.o

mkdir:
	mkdir -p $(DIR_BUILD)

# typical execution
runHamming:
	./$(OUT) -c cluster.conf -d DataSets/DataHamming.csv -o OUR_ResultsHamming.txt

runCosine:
	./$(OUT) -c cluster.conf -d DataSets/DataCosine.csv -o OUR_ResultsCosine.txt

runEuclidean:
	./$(OUT) -c cluster.conf -d DataSets/DataEuclidean.csv -o OUR_ResultsEuclidean.txt

runEuclideanCUnit:
	./$(OUT) -c cluster.conf -d DataSets/DataEuclidean.csv -o OUR_ResultsEuclidean.txt -cu

runMetricSpace:
	./$(OUT) -c cluster.conf -d DataSets/ProteinsSmall.csv  -o OUR_ResultsProteins.txt

debug:
	gdb --args ./$(OUT) -c cluster.conf -d DataSets/ProteinsSmall.csv -o OUR_ResultsProteins.txt

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all ./$(OUT) -c cluster.conf -d DataSets/ProteinsSmall.csv -o OUR_ResultsProteins.txt > log.txt 2>&1


# clean house
clean:
	rm -f $(OBJS)
	rm -r -f $(DIR_BUILD)
	rm -f $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)

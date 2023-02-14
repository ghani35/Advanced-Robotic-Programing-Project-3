mkdir bin
mkdir .data
mkdir Logs
sudo chmod -R a+rwx . # this to edite the permission of this folder and subfolders
echo "Compiling the Process A ...";
g++ src/processA.cpp ./src/Myutilities.cpp -lrt -lncurses -lbmp -lm -o bin/ProcessA
echo "Compiling the Process B ...";
g++ src/processB.cpp ./src/Myutilities.cpp -lrt -lncurses -lbmp -lm -o bin/ProcessB
echo "Compiling the Master Process ...";
g++ src/master.cpp ./src/Myutilities.cpp  -o bin/master
echo "Start Runing ...";
./bin/master

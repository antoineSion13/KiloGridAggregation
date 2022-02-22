rm -rf build
mkdir build
cd build
cmake ../ARGoS_simulation
make
cd ../ARGoS_simulation/data_generation_scripts
for i in {1..50}
do
	echo "Run $i"
	argos3 -c ../experiment/kilogrid_stub.argos
  mv Results/results_non_informed.csv Results/results_non_informed$i.csv
done

#!/bin/bash
run(){
	#first we set the desired parameters of the experiment (config files are transformed into usable files)
	experiment_file="ARGoS_simulation/experiment/kilogrid_stub.argos"
	experiment_file_mod="ARGoS_simulation/experiment/kilogrid_stub_mod.argos"
	echo "start"
	experiment_time="$1"
	non_informed_quantity="$2"
	informed_black_quantity="$3"
	informed_white_quantity="$4"

	cp $experiment_file $experiment_file_mod
	sed -i "s|__experiment_time__|$experiment_time|g" $experiment_file_mod
	sed -i "s|__non_informed_quantity__|$non_informed_quantity|g" $experiment_file_mod
	sed -i "s|__informed_black_quantity__|$informed_black_quantity|g" $experiment_file_mod
	sed -i "s|__informed_white_quantity__|$informed_white_quantity|g" $experiment_file_mod

	black_informed_file="ARGoS_simulation/behaviours/agent_stub_black_base.c"
	white_informed_file="ARGoS_simulation/behaviours/agent_stub_white_base.c"
	non_informed_file="ARGoS_simulation/behaviours/agent_stub_non_informed_base.c"
	black_informed_file_mod="ARGoS_simulation/behaviours/agent_stub_black.c"
	white_informed_file_mod="ARGoS_simulation/behaviours/agent_stub_white.c"
	non_informed_file_mod="ARGoS_simulation/behaviours/agent_stub_non_informed.c"

	sampling_time="$5"
	alpha_param="$6"
	beta_param="$7"
	informed_weight="$8"

	cp $black_informed_file $black_informed_file_mod
	cp $white_informed_file $white_informed_file_mod
	cp $non_informed_file $non_informed_file_mod

	sed -i "s|__sampling_time__|$sampling_time|g" $black_informed_file_mod
	sed -i "s|__sampling_time__|$sampling_time|g" $white_informed_file_mod
	sed -i "s|__sampling_time__|$sampling_time|g" $non_informed_file_mod
	sed -i "s|__alpha_param__|$alpha_param|g" $black_informed_file_mod
	sed -i "s|__alpha_param__|$alpha_param|g" $white_informed_file_mod
	sed -i "s|__alpha_param__|$alpha_param|g" $non_informed_file_mod
	sed -i "s|__beta_param__|$beta_param|g" $black_informed_file_mod
	sed -i "s|__beta_param__|$beta_param|g" $white_informed_file_mod
	sed -i "s|__beta_param__|$beta_param|g" $non_informed_file_mod
	sed -i "s|__informed_weight__|$informed_weight|g" $black_informed_file_mod
	sed -i "s|__informed_weight__|$informed_weight|g" $white_informed_file_mod
	sed -i "s|__informed_weight__|$informed_weight|g" $non_informed_file_mod

	results_folder=time_"$experiment_time"_ni_"$non_informed_quantity"_b_"$informed_black_quantity"_w_"$informed_white_quantity"_samp_"$sampling_time"_alpha_"$alpha_param"_beta_"$beta_param"_weight_"$informed_weight"
	#build the project
	rm -rf build
	mkdir build
	cd build
	cmake ../ARGoS_simulation
	make
	cd ../ARGoS_simulation/data_generation_scripts
	#run the simulations
	mkdir Results/"$results_folder"
	for i in {1..50}
	do
		echo "Run $i"
		argos3 -c ../experiment/kilogrid_stub_mod.argos
	  mv Results/results_non_informed.csv Results/"$results_folder"/results_non_informed$i.csv
	done
	cd ../..
}

#here run what you want !
run 18000 34 16 0 1 0.5 2.25 1
run 18000 34 8 8 1 0.5 2.25 1
run 18000 34 11 5 1 0.5 2.25 1



hp: 
	@echo "Compile hp_main...";
	gcc -I ./include/ -L ./lib/BF_lib/ -Wl,-rpath,./lib/BF_lib/ -g ./src/heap_file.c ./examples/main_exampleHP.c -o ./build/runner ./lib/BF_lib/BF_64.a -no-pie

ht:
	@echo "Compile ht_main..";
	gcc -I ./include/ -L ./lib/BF_lib/ -Wl,-rpath,./lib/BF_lib/ -g ./src/hashfile1.c ./examples/basicMain_HT.c  -o ./build/HT   ./lib/BF_lib/BF_64.a -no-pie


clean:	
	rm -rf ./src/*.o ./dataH* ./build/runner ./build/HT ./examples/*.o ./src/*.swp ./examples/.*swp



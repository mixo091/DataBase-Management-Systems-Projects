sht:
	@echo "Compile sht_main...";
	gcc -I ./include/ -L ./lib/BF_lib/ -Wl,-rpath,./lib/BF_lib/ -g ./src/hashfile1.c ./src/SecondaryHT.c ./examples/sht_main.c  -o ./build/SHT ./lib/BF_lib/BF_64.a -no-pie


ht:
	@echo "Compile ht_main..";
	gcc -I ./include/ -L ./lib/BF_lib/ -Wl,-rpath,./lib/BF_lib/ -g ./src/hashfile1.c ./src/SecondaryHT.c ./examples/sht_main.c  -o ./build/SHT ./lib/BF_lib/BF_64.a -no-pie


clean:	
	rm -rf ./src/*.o ./dataH* ./build/runner ./build/HT ./build/SHT ./examples/*.o ./src/*.swp ./examples/.*swp



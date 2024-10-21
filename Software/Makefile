FLAGS = -std=c++17 -Wall -Wfatal-errors -Wno-sign-compare -Wno-reorder-ctor -Wno-inconsistent-missing-override -g -O3
EASYLOCAL = ../../easylocal-3
ifeq ($(shell uname), Linux) #Linux OS
	COMPOPTS = -I$(EASYLOCAL)/include $(FLAGS)
	LINKOPTS = -lboost_program_options -pthread
else #Mac OS
	BOOST= /opt/homebrew/include
	BOOST_LIB=/opt/homebrew/Cellar/boost/1.83.0/lib
	COMPOPTS = -I$(EASYLOCAL)/include -I$(BOOST) $(FLAGS)
	LINKOPTS = -L$(BOOST_LIB) -lboost_program_options -pthread

endif

SOURCE_FILES = IPRNPA_Input.cc IPRNPA_Output.cc IPRNPA_Helpers.cc  IPRNPA_Main.cc
OBJECT_FILES = IPRNPA_Input.o IPRNPA_Output.o IPRNPA_Helpers.o IPRNPA_Main.o
HEADER_FILES = IPRNPA_Input.hh IPRNPA_Output.hh IPRNPA_Helpers.hh  

pna: $(OBJECT_FILES)
	g++ $(OBJECT_FILES) $(LINKOPTS) -o pna

IPRNPA_Input.o: IPRNPA_Input.cc IPRNPA_Input.hh
	g++ -c $(FLAGS) IPRNPA_Input.cc

IPRNPA_Output.o: IPRNPA_Output.cc IPRNPA_Input.hh IPRNPA_Output.hh
	g++ -c $(FLAGS) IPRNPA_Output.cc

IPRNPA_Helpers.o: IPRNPA_Helpers.cc IPRNPA_Helpers.hh IPRNPA_Input.hh IPRNPA_Output.hh
	g++ -c $(COMPOPTS) IPRNPA_Helpers.cc

IPRNPA_Main.o: IPRNPA_Main.cc IPRNPA_Helpers.hh IPRNPA_Input.hh IPRNPA_Output.hh
	g++ -c $(COMPOPTS) IPRNPA_Main.cc

clean:
	rm -f $(OBJECT_FILES) pna


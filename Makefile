# this runs first
all:

compile_options = -O2 -Wall

flags = `pkg-config opencv --cflags`
libs = -lboost_system -lboost_filesystem -lboost_program_options `pkg-config opencv --libs`

############# Declare program components ################

object_files = \
	clipper.o \
	analysis_tools.o \
	options.o \
	io.o \
	progress_bar.o

header_files = \
	clipper.h \
	analysis_tools.h \
	options.h \
	io.h \
	image_region_list.h \
	progress_bar.h

exec_files = \
	analysis

############# Build functions ###########################

# redefine implicit rules for building object files
%.o: %.cc Makefile
	g++ -c $(compile_options) $(flags) $< -o $@

# build the main program
analysis: analysis.cc $(object_files) $(header_files)
	g++ $(compile_options) $(libs) $(flags) $(object_files) -o $@ $<

############# Other Opperations ##########################
.PHONY: clean all

# remove object files
clean:
	rm $(object_files)

# remove all binaries
cclean:
	rm $(object_files) $(exec_files)

# all things that need to be built
all: $(object_files) $(exec_files)



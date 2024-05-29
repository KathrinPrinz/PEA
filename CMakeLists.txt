# CMakeList.txt: CMake-Projekt für "parallel_enumeration". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)
project ("PEA")
# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
set(SOURCE_FILES

	#### Header ####

	# Core
	"src/main.cpp"

	# Algorithmen
	"src/algorithm_parallel_three_stages.cpp"

	#Cplex
	"src/cplex_interface.cpp"
)
add_executable(PEA ${SOURCE_FILES})

######## OpenMP
	find_package(OpenMP REQUIRED)
	target_link_libraries(PEA  OpenMP::OpenMP_CXX)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET PEA PROPERTY CXX_STANDARD 20)
endif()
include_directories(include)

######## Cplex Dependency
find_path(CPLEX
		NAMES cplex
		PATHS "/opt/ibm/ILOG/CPLEX_Studio2211"
	)
	message(${CPLEX})
	find_path(CPLEX_INCLUDE_DIR ilcplex/cplex.h
		PATHS "${CPLEX}/cplex/include"
	)
	
	find_path(CONCERT_INCLUDE_DIR ilconcert/iloenv.h
		PATHS "${CPLEX}/concert/include"
	)
	find_library(ILOCPLEX_LIB libilocplex.a
	#find_library(ILOCPLEX_LIB ilocplex.lib
		PATHS "${CPLEX}/cplex/lib/x86-64_linux/static_pic"
			"${CPLEX}/cplex/lib/x64_windows_msvc14/stat_mda"
	)
	find_library(CONCERT_LIB libconcert.a
	#find_library(CONCERT_LIB concert.lib
		PATHS "${CPLEX}/concert/lib/x86-64_linux/static_pic"
			"${CPLEX}/concert/lib/x64_windows_msvc14/stat_mda"
	)
	find_library(CPLEX_LIB libcplex.a
	#find_library(CPLEX_LIB cplex2210.lib
		PATHS "${CPLEX}/cplex/lib/x86-64_linux/static_pic"
			"${CPLEX}/cplex/lib/x64_windows_msvc14/stat_mda"
	)
	include_directories(${CPLEX_INCLUDE_DIR})
	include_directories(${CONCERT_INCLUDE_DIR})
	target_link_libraries(PEA "${ILOCPLEX_LIB}")
	target_link_libraries(PEA "${CONCERT_LIB}")
	target_link_libraries(PEA "${CPLEX_LIB}")
	target_link_libraries(PEA pthread)
	target_link_libraries(PEA dl)

	

	


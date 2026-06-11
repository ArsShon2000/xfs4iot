
if(WIN32)
	cmake_minimum_required(VERSION 3.30)
elseif(UNIX)
	cmake_minimum_required(VERSION 3.28)
endif()

project(xfs4iot_cpp_sp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
# Boost
	find_package(Boost 1.74 REQUIRED COMPONENTS system thread asio beast filesystem)

# nlohmann::json
	find_package(nlohmann_json CONFIG REQUIRED)
	find_package(OpenSSL REQUIRED)
	find_package(spdlog REQUIRED)
	find_package(boost_url CONFIG REQUIRED)
    find_package(miniz CONFIG REQUIRED)
elseif(UNIX)
	find_package(Boost 1.74 REQUIRED COMPONENTS system thread asio beast filesystem)
	find_package(PkgConfig REQUIRED CONFIG)
	find_package(nlohmann_json CONFIG REQUIRED)
	find_package(OpenSSL REQUIRED)
	find_package(spdlog REQUIRED)
	find_package(boost_url CONFIG REQUIRED)
    find_package(miniz CONFIG REQUIRED)
endif()

# Настройка Doxygen для генерации документации ###########
find_package(Doxygen REQUIRED)
set(DOXYGEN_IN  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
add_custom_target(doc
	COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Генерация Doxygen-документации"
	VERBATIM)
##########################################################

add_executable(xfs4iot_cpp_sp
	app/XFS4IoT.cpp

	#Devices
	Devices/CashAcceptorSample/CashAcceptorSample.cpp
	
	Devices/CashAcceptorSample/Managers/EscrowManager/EscrowManager.cpp
	Devices/CashAcceptorSample/Managers/NotesInhibitManager/NotesInhibitManager.cpp
	Devices/CashAcceptorSample/Managers/PowerUpManager/PowerUpManager.cpp

	Devices/CashAcceptorSample/ExecuteFunctions/ExecuteCashInStart/ExecuteCashInStart.cpp
	Devices/CashAcceptorSample/ExecuteFunctions/ExecuteCashIn/ExecuteCashIn.cpp
	Devices/CashAcceptorSample/ExecuteFunctions/ExecuteCashInEnd/ExecuteCashInEnd.cpp


	Devices/HW/SearchDevices/SearchDevices.cpp
	Devices/HW/DevicePort/DevicePort.cpp
	Devices/HW/Dors_210BA.cpp
	Devices/HW/Identification.cpp
	
	# framework/server
	framework/server/ClientConnection.cpp
	framework/server/CommandDispatcher.cpp
	framework/server/CommandQueue.cpp
	framework/server/EndPoint.cpp
	framework/server/ServiceConfiguration.cpp
	framework/server/ServicePublisher.cpp
	framework/server/GetServiceHandler.cpp

	# framework/ServiceClasses
	framework/ServiceClasses/CashAcceptorServiceProvider/CashAcceptorServiceClass.cpp
	framework/ServiceClasses/CashAcceptorServiceProvider/CashAcceptorServiceProvider.cpp
	
	framework/ServiceClasses/CashManagementServiceProvider/Handlers/ResetHandler.cpp
	framework/ServiceClasses/CashManagementServiceProvider/Handlers/GetBankNoteTypesHandler.cpp
	
	framework/ServiceClasses/CashAcceptorServiceProvider/Handlers/ConfigureNoteTypesHandler.cpp
	framework/ServiceClasses/CashAcceptorServiceProvider/Handlers/CashInStartHandler.cpp
	framework/ServiceClasses/CashAcceptorServiceProvider/Handlers/CashInHandler.cpp
	framework/ServiceClasses/CashAcceptorServiceProvider/Handlers/CashInEndHandler.cpp
	framework/ServiceClasses/CashAcceptorServiceProvider/Handlers/GetCashInStatusHandler.cpp


	framework/ServiceClasses/CommonServiceProvider/Handlers/CapabilitiesHandler.cpp
	framework/ServiceClasses/CommonServiceProvider/Handlers/SetVersionsHandler.cpp
	framework/ServiceClasses/CommonServiceProvider/CommonServiceClass.cpp

	framework/ServiceClasses/CashManagementServiceProvider/CashManagementServiceClass.cpp
	# framework/ServiceClasses/CashManagementServiceProvider/Events/RetractCommandEvents.cpp

	framework/ServiceClasses/StorageServiceProvider/CashUnit.cpp
	framework/ServiceClasses/StorageServiceProvider/StorageServiceClass.cpp
	framework/ServiceClasses/StorageServiceProvider/Events/StorageErrorCommandEvent.cpp

	
	framework/ServiceClasses/INFO_MODULES/GetinfoBanknoteTypes/GetinfoBanknoteTypes.cpp
	framework/ServiceClasses/INFO_MODULES/GetInfoStatus/GetInfoStatus.cpp

	
	framework/Common/AsciiHexConversions/AsciiHexConversions.cpp
	# framework/core/CashManagement/Events/IncompleteRetractEvent.cpp
	# framework/core/CashManagement/Events/InfoAvailableEvent.cpp
	
	framework/core/Logger/Zipper/Zipper.cpp
	framework/core/SettingModule/SettingModule.cpp
	
	framework/core/Persistent/PersistentDatasHandler.cpp
)

target_include_directories(xfs4iot_cpp_sp PRIVATE
	${CMAKE_SOURCE_DIR}/framework
	${Boost_INCLUDE_DIRS}
	#${nlohmann_json_INCLUDE_DIRS}
)

if(WIN32)
	target_link_libraries(xfs4iot_cpp_sp PRIVATE
		Ws2_32
		Boost::system
		Boost::thread
		Boost::asio
		Boost::beast
		Boost::url
		Boost::filesystem
		nlohmann_json::nlohmann_json
		OpenSSL::SSL
		OpenSSL::Crypto
		spdlog::spdlog
		Setupapi
        miniz::miniz
	)
	target_compile_definitions(xfs4iot_cpp_sp PRIVATE
		_WIN32_WINNT=0x0601
		WIN32_LEAN_AND_MEAN
		_CRT_SECURE_NO_WARNINGS
		GLEW_STATIC)
		if(MSVC)
			target_compile_options(xfs4iot_cpp_sp PRIVATE /bigobj)
		endif()
		target_compile_options(xfs4iot_cpp_sp PRIVATE /W4 /EHsc /utf-8 /bigobj)
		target_compile_options(xfs4iot_cpp_sp PRIVATE $<$<CONFIG:Debug>:/Zi /Od /RTC1>)
		target_compile_options(xfs4iot_cpp_sp PRIVATE $<$<CONFIG:Release>:/O2 /DNDEBUG>)
elseif(UNIX)
	target_link_libraries(xfs4iot_cpp_sp PRIVATE
	Boost::system
	Boost::thread
	Boost::asio
	Boost::beast
	Boost::url
	Boost::filesystem
	nlohmann_json::nlohmann_json
	OpenSSL::SSL
	OpenSSL::Crypto
	spdlog::spdlog
    miniz::miniz
)
	target_compile_options(xfs4iot_cpp_sp PRIVATE -Wall -Wextra -pedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
	target_compile_options(xfs4iot_cpp_sp PRIVATE $<$<CONFIG:Debug>:-g -O0>)
	target_compile_options(xfs4iot_cpp_sp PRIVATE $<$<CONFIG:Release>:-O3 -DNDEBUG>)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Выходные папки для артефактов
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)




# cmake -S .. -B . -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="C:/Program Files/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
# cmake --build . --config Debug --target doc


cmake_minimum_required(VERSION 3.16)

set(WB_IDF_I2C_COMPONENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/managed_components/wb-idf-core/wb-idf-i2c")
if(NOT EXISTS "${WB_IDF_I2C_COMPONENT_DIR}")
	set(WB_IDF_I2C_COMPONENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../wb-idf-core/wb-idf-i2c")
endif()

set(EXTRA_COMPONENT_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR}/../../"
	"${WB_IDF_I2C_COMPONENT_DIR}"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(wb_ezo_basic_read)

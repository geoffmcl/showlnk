# showlnk - 20140129

cmake_minimum_required (VERSION 2.8)

project (showlnk)

# to distinguish between debug and release libraries
if (WIN32)
    set( CMAKE_DEBUG_POSTFIX "d" )
endif ()

if(CMAKE_COMPILER_IS_GNUCXX)
    set( WARNING_FLAGS -Wall )
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
   set( WARNING_FLAGS "-Wall -Wno-overloaded-virtual" )
endif() 

if(WIN32)
    if(MSVC)
        # turn off various warnings
        set(WARNING_FLAGS "${WARNING_FLAGS} /wd4996")
        # foreach(warning 4244 4251 4267 4275 4290 4786 4305)
        #     set(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
        # endforeach(warning)

        set( MSVC_FLAGS "-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS" )
        # if (${MSVC_VERSION} EQUAL 1600)
        #    set( MSVC_LD_FLAGS "/FORCE:MULTIPLE" )
        # endif (${MSVC_VERSION} EQUAL 1600)
    endif(MSVC)
    # set( NOMINMAX 1 )
endif(WIN32)

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}" )

set(dir src)
set(sl_SRCS
    ${dir}/showlnk.cxx 
    ${dir}/showlnkW.cxx 
    ${dir}/sprtf.cxx
    ${dir}/conversions.cxx
    ${dir}/showlnk2.cxx 
    ${dir}/sl_utils.cxx
    ${dir}/sl_load.cxx
    )
set(sl_HDRS
    ${dir}/sprtf.hxx
    ${dir}/conversions.hxx
    ${dir}/showlnk2.hxx 
    ${dir}/sl_utils.hxx
    ${dir}/sl_load.hxx
    )
add_executable(showlnk ${sl_SRCS} ${sl_HDRS})
if(add_LIBS)
    target_link_libraries( showlnk ${add_LIBS} )
endif ()

if(WIN32)
    set_target_properties( showlnk PROPERTIES DEBUG_POSTFIX d )
endif()

# deal with install 
install(TARGETS showlnk DESTINATION C:/MDOS )

# eof

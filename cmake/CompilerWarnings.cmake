
set(PROJECT_WARNINGS_CXX
  -Wall
  -Wextra # reasonable and standard
  -Wshadow # warn the user if a variable declaration shadows one from a parent context
  -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
  # catch hard to track down memory errors
  -Wold-style-cast # warn for c-style casts
  -Wcast-align # warn for potential performance problem casts
  -Wunused # warn on anything being unused
  -Woverloaded-virtual # warn if you overload (not override) a virtual function
  -Wpedantic # warn if non-standard C++ is used
  -Wconversion # warn on type conversions that may lose data
  -Wsign-conversion # warn on sign conversions
  -Wnull-dereference # warn if a null dereference is detected
  -Wdouble-promotion # warn if float is implicit promoted to double
  -Wformat=2 # warn on security issues around functions that format output (ie printf)
  -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation
  -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
  -Wduplicated-cond # warn if if / else chain has duplicated conditions
  -Wduplicated-branches # warn if if / else branches have duplicated code
  -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
  -Wuseless-cast # warn if you perform a cast to the same type
)

# set(CompilerWarnings -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(PROJECT_WARNINGS_CXX
    ${PROJECT_WARNINGS_CXX}
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-but-set-variable)
endif()

# Add C warnings
set(PROJECT_WARNINGS_C "${PROJECT_WARNINGS_CXX}")
list(
  REMOVE_ITEM
  PROJECT_WARNINGS_C
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Woverloaded-virtual
  -Wuseless-cast)


function(set_project_warnings project_name)
  target_compile_options(
    ${project_name}
    INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_WARNINGS_CXX}>
    $<$<COMPILE_LANGUAGE:C>:${PROJECT_WARNINGS_C}>
  )
endfunction()
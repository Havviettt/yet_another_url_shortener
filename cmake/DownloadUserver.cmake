include_guard(GLOBAL)

function(download_userver)
    set(OPTIONS)
    set(ONE_VALUE_ARGS TRY_DIR VERSION GIT_TAG)
    cmake_parse_arguments(ARG "${OPTIONS}" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if(ARG_TRY_DIR)
        get_filename_component(ARG_TRY_DIR "${ARG_TRY_DIR}" REALPATH)
        if(EXISTS "${ARG_TRY_DIR}")
            add_subdirectory("${ARG_TRY_DIR}" third_party/userver)
            return()
        endif()
    endif()

    include(get_cpm)
    set(CPM_USE_NAMED_CACHE_DIRECTORIES ON)
    CPMAddPackage(
        NAME userver
        GITHUB_REPOSITORY userver-framework/userver
        VERSION ${ARG_VERSION}
        GIT_TAG ${ARG_GIT_TAG}
        GIT_SHALLOW TRUE
        ${ARG_UNPARSED_ARGUMENTS}
    )
endfunction()

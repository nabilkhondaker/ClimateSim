function(enable_sanitizers target)
    if(NOT TARGET ${target})
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(${target} PRIVATE
            -fsanitize=address,undefined
        )
    else()
        message(WARNING "Sanitizers requested but compiler does not support them")
    endif()
endfunction()

add_library(uswebservices
    AuthService.c
	RacingService.c
    AuthService.h
    RacingService.h
)

target_link_libraries(uswebservices PUBLIC ushttp)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(authtest)
endif()

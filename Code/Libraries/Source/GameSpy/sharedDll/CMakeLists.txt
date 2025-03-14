add_library(UniSpySDK STATIC dllmain.c)

target_link_libraries(UniSpySDK PUBLIC
    usbrigades
    uscommon
    uschat
    uscdkey
    ushttp
    usgp
    usstats
    usgt2
    usnatneg
    uspeer
    uspinger
    uspt
    usqr2
    ussake
    ussc
    usserverbrowsing
    usvoice2
    uswebservices
    usd2g
)

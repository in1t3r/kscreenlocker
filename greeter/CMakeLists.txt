remove_definitions(-DTRANSLATION_DOMAIN=\"kscreenlocker\")
add_definitions(-DTRANSLATION_DOMAIN=\"kscreenlocker_greet\")

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ../kcheckpass
    ${CMAKE_CURRENT_BINARY_DIR}/../
)

set(kscreenlocker_greet_SRCS
   authenticator.cpp
   greeterapp.cpp
   main.cpp
   noaccessnetworkaccessmanagerfactory.cpp
   lnf_integration.cpp
   wallpaper_integration.cpp
   kwinglplatform.cpp
)

if(HAVE_SECCOMP)
    set(kscreenlocker_greet_SRCS ${kscreenlocker_greet_SRCS} seccomp_filter.cpp)
endif()

qt5_add_resources(kscreenlocker_greet_SRCS fallbacktheme.qrc)
kconfig_add_kcfg_files(kscreenlocker_greet_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../kcfg/kscreensaversettings.kcfgc)

ecm_add_wayland_client_protocol(kscreenlocker_greet_SRCS
    PROTOCOL ../protocols/ksld.xml
    BASENAME ksld
)

add_executable(kscreenlocker_greet ${kscreenlocker_greet_SRCS})

target_link_libraries(kscreenlocker_greet
                        KF5::Package
                        KF5::Crash
                        KF5::I18n
                        KF5::ConfigGui
                        KF5::Declarative
                        KF5::QuickAddons
                        KF5::WindowSystem
                        Qt5::Quick
                        Qt5::Qml
                        Qt5::X11Extras
                        ${X11_LIBRARIES}
                        KF5::WaylandClient
                        Wayland::Client
                        )
if(HAVE_SECCOMP)
    target_link_libraries(kscreenlocker_greet Qt5::DBus Seccomp::Seccomp)
endif()

install(TARGETS kscreenlocker_greet DESTINATION ${KDE_INSTALL_LIBEXECDIR})

install(DIRECTORY themes/org.kde.passworddialog DESTINATION ${KDE_INSTALL_DATADIR}/ksmserver/screenlocker)

add_subdirectory(autotests)

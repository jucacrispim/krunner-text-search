cmake_minimum_required(VERSION 2.8.12)
set(QT_MIN_VERSION "5.6.1")
set(KF5_MIN_VERSION "5.26.0")
project (balootextsearch)

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_balootextsearchrunner\")


find_package(ECM 5.12.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH
     ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_MODULE_PATH}
)

find_package (Qt5 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS Widgets Core Quick QuickWidgets)
find_package (KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n Service Runner TextWidgets Baloo KIO)


include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

set(krunner_balootextsearchrunner_SRCS
  balootextsearchrunner.cpp
)

add_library(krunner_balootextsearchrunner MODULE balootextsearchrunner.cpp)


target_link_libraries(krunner_balootextsearchrunner
  KF5::Runner
  KF5::KIOWidgets
  KF5::Service
  KF5::I18n
  KF5::Baloo
  KF5::Plasma
  )

install(TARGETS krunner_balootextsearchrunner DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-runner-balootextsearch.desktop DESTINATION ${SERVICES_INSTALL_DIR})

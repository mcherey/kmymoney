include(ECMInstallIcons)

file(GLOB_RECURSE KMYMONEY_HICOLOR_ICONS "hicolor/*")
ecm_install_icons(
  ICONS ${KMYMONEY_HICOLOR_ICONS}
  DESTINATION ${ICON_INSTALL_DIR} # or stay to ${DATA_INSTALL_DIR}/kmymoney/icons ?
  THEME "hicolor"
)

file(GLOB_RECURSE KMYMONEY_OXYGEN_ICONS "oxygen/*")
ecm_install_icons(
  ICONS ${KMYMONEY_OXYGEN_ICONS}
  DESTINATION ${ICON_INSTALL_DIR}
  THEME "oxygen"
)

file(GLOB_RECURSE KMYMONEY_TANGO_ICONS "Tango/*")
ecm_install_icons(
  ICONS ${KMYMONEY_TANGO_ICONS}
  DESTINATION ${ICON_INSTALL_DIR}
  THEME "Tango"
)

file(GLOB_RECURSE KMYMONEY_ICONS "kmymoney/*.png")
ecm_install_icons(
  ICONS ${KMYMONEY_ICONS}
  DESTINATION ${ICON_INSTALL_DIR}
)

set (icons_SOURCES
  icons.cpp
)

add_library(kmm_icons SHARED ${icons_SOURCES})
generate_export_header(kmm_icons)
target_link_libraries(kmm_icons PUBLIC Qt5::Core)

install(TARGETS kmm_icons
  ${INSTALL_TARGETS_DEFAULT_ARGS} )

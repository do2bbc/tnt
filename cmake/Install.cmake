install(TARGETS tnt tntc RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}")
install(TARGETS tnt_logcall tnt_info RUNTIME DESTINATION "${tnt_bin_dir}")
install(PROGRAMS "$<TARGET_FILE:tnt_help>" DESTINATION "${tnt_bin_dir}" RENAME help)
install(FILES src/runprgs/.help src/runprgs/.info src/runprgs/.logcall
  DESTINATION "${tnt_bin_dir}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/src/runprgs/tntrun.h"
  DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
foreach(lang de en fr)
  install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/conf/tnt_setup.${lang}"
    DESTINATION "${CMAKE_INSTALL_FULL_SBINDIR}")
endforeach()
set(examples
  autostrt.tnt boxender.tnt ctext.tnt ctext.tnt.fr extrem.tnt f6fbb.box fkeys.tnt
  names.tnt netpass.tnt news.tnt norem.tnt notown.tnt pw.tnt qtext.tnt qtext.tnt.fr
  routes.tnt sys.tnt telltexte.tnt termcap.tnt tnt.dwn tnt.up tntrem.inf)
foreach(example IN LISTS examples)
  install(FILES "examples/${example}" DESTINATION "${tnt_conf_dir}" RENAME "${example}.ex")
endforeach()
foreach(example tnt.ini tntc.ini sounds.tnt)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/examples/${example}"
    DESTINATION "${tnt_conf_dir}" RENAME "${example}.ex")
endforeach()
foreach(lang de en fr)
  install(FILES "examples/tnt.hlp.${lang}" "examples/tntrem.hlp.${lang}"
    DESTINATION "${tnt_doc_dir}")
  foreach(number RANGE 1 6)
    install(FILES "doc/Readme${number}.${lang}" DESTINATION "${tnt_doc_dir}")
  endforeach()
  install(FILES "doc/tnt.doc.${lang}" DESTINATION "${tnt_doc_dir}")
endforeach()
install(FILES doc/ax25krnl.doc doc/iface.doc doc/tntdoc.dvi doc/tntdoc.html
  DESTINATION "${tnt_doc_dir}")
install(FILES doc/tntdoc.info doc/tntdoc.info-1 doc/tntdoc.info-2
  doc/tntdoc.info-3 doc/tntdoc.info-4 DESTINATION "${CMAKE_INSTALL_FULL_INFODIR}")
install(FILES examples/sounds/busy.au examples/sounds/connect.au examples/sounds/disc.au
  examples/sounds/failure.au examples/sounds/reset.au examples/sounds/ring.au
  DESTINATION "${tnt_sound_dir}")
install(PROGRAMS examples/sounds/play DESTINATION "${tnt_sound_dir}")
foreach(directory 7plus abin bcast bcast/newmail bcast/save ctext down macro
    newmail remote tntusers up yapp)
  install(DIRECTORY DESTINATION "${tnt_work_dir}/${directory}")
endforeach()
install(CODE "
  foreach(link \"${tnt_bin_dir}/info\" \"${tnt_work_dir}/bin\")
    if(link STREQUAL \"${tnt_bin_dir}/info\")
      set(target tnt-info)
    else()
      set(target \"${tnt_bin_dir}\")
    endif()
    if(NOT EXISTS \"\$ENV{DESTDIR}\${link}\" AND NOT IS_SYMLINK \"\$ENV{DESTDIR}\${link}\")
      execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink
        \"\${target}\" \"\$ENV{DESTDIR}\${link}\" RESULT_VARIABLE result)
      if(NOT result EQUAL 0)
        message(FATAL_ERROR \"Cannot create installation symlink \${link}\")
      endif()
    endif()
    if(IS_SYMLINK \"\$ENV{DESTDIR}\${link}\")
      file(READ_SYMLINK \"\$ENV{DESTDIR}\${link}\" installed_target)
      if(installed_target STREQUAL target)
        list(APPEND CMAKE_INSTALL_MANIFEST_FILES \"\${link}\")
      endif()
    endif()
  endforeach()
")

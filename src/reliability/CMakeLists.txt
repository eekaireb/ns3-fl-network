set(name reliability)

set(source_files
    model/power-model.cc
    model/util-power-model.cc
    model/app-power-model.cc
    model/temperature-model.cc
    model/temperature-simple-model.cc      
    model/performance-model.cc
    model/performance-simple-model.cc
    model/reliability-model.cc
    model/reliability-tddb-model.cc
    helper/reliability-helper.cc
)

set(header_files
    model/power-model.h
    model/util-power-model.h
    model/app-power-model.h
    model/temperature-model.h
    model/temperature-simple-model.h
    model/performance-model.h
    model/performance-simple-model.h
    model/reliability-model.h
    model/reliability-tddb-model.h
    helper/reliability-helper.h
)

set(libraries_to_link ${libcore} )


build_lib("${name}" "${source_files}" "${header_files}" "${libraries_to_link}" "")

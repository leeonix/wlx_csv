
solution 'wlx_csv'
    configurations { 'Debug', 'Release' }
    language 'C'
    defines { 'WIN32', '_WINDOWS' }
    flags { 'StaticRuntime' }

    location 'build'
    objdir 'obj'

    configuration 'vs*'
        defines { '_CRT_SECURE_NO_WARNINGS' }

    configuration 'Debug'
        flags { 'Symbols' }
        defines { '_DEBUG' }

    configuration 'Release'
        flags { 'Optimize' }
        defines { 'NDEBUG' }

project 'csv'
    kind 'StaticLib'
    targetdir 'lib'

    files {
        'src/*.h',
        'src/*.c',
    }

    prebuildcommands
    {
        '@echo on',
        "ragel -T1 ../csv/csv.rl",
    }

    configuration 'Debug'
        targetsuffix '_d'

project 'wlx_csv'
    kind 'SharedLib'
    targetdir 'bin'

    includedirs {
        'csv',
    }
    links {
        'csv',
    }

    files {
        'src/*.h',
        'src/*.c',
        'src/*.def',
    }

    targetname 'wlx_csv'
    targetextension '.wlx'

    configuration 'Debug'
        debugdir '$(TargetDir)'


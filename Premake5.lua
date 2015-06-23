
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

project 'wlx_csv'
    kind 'SharedLib'
--    targetdir 'bin'
    targetdir '$(COMMANDER_PATH)/Plugins/wlx/csv'
    
    includedirs {
        'csv',
    }

    files {
        'csv/csv.h',
        'csv/csv.c',
        'src/*.h',
        'src/*.c',
        'src/*.def',
    }

    prebuildcommands
    {
        '@echo on',
        "ragel -T1 ../csv/csv.rl",
    }

    targetname 'wlx_csv'
    targetextension '.wlx'

    configuration 'Debug'
        debugdir '$(TargetDir)'
        debugcommand '$(COMMANDER_PATH)/TOTALCMD.EXE'

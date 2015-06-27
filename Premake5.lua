
solution 'wlx_csv'
    configurations { 'Debug', 'Release' }
    language 'C'
    defines { 'WIN32', '_WINDOWS' }
    flags { 'StaticRuntime' }

    location 'build'
    objdir 'obj'

    filter { 'configurations:vs*' }
        defines { '_CRT_SECURE_NO_WARNINGS' }

    filter { 'configurations:Debug' }
        defines { '_DEBUG' }
        flags { 'Symbols' }
        optimize 'Debug'

    filter { 'configurations:Release' }
        defines { 'NDEBUG' }
        optimize 'Full'

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


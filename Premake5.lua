
externalrule 'ragel'
    location 'ragel'
    filename 'ragel'
    fileextension '.rl'
    propertydefinition {
        name = 'OutputFileName',
    }
    propertydefinition {
        name = 'Outputs',
    }
    propertydefinition {
        name = 'CodeStyle',
    }

solution 'wlx_csv'
    configurations { 'Debug', 'Release' }
    language 'C'
    defines { 'WIN32', '_WIN32', 'WINDOWS', '_WINDOWS' }
    flags { 'StaticRuntime' }
    characterset ("MBCS")

    location 'build'
    objdir 'obj'

    rules { 'ragel' }
    ragelVars {
        OutputFileName = '../csv/%(Filename).c',
        Outputs        = '%(OutputFileName)',
        CodeStyle      = '1',
    }

    filter { 'action:vs*' }
        defines { '_CRT_SECURE_NO_WARNINGS' }

    filter { 'configurations:Debug' }
        defines { '_DEBUG' }
        optimize 'Debug'
        symbols 'On'
        symbolspath '$(OutDir)$(TargetName).pdb'

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
        'csv/csv.rl',
        'src/*.h',
        'src/*.c',
        'src/*.def',
    }

    targetname 'wlx_csv'
    targetextension '.wlx'

    configuration 'Debug'
        debugdir '$(TargetDir)'
        debugcommand '$(COMMANDER_PATH)/TOTALCMD.EXE'


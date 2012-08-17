#!/usr/bin/env python
import os
import shutil
from waflib import Logs
from waflib.extras import autowaf as autowaf

# Variables for 'waf dist'
APPNAME = 'fabla.lv2'
VERSION = '1.0.0'

# Mandatory variables
top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    
    autowaf.set_options(opt)

def configure(conf):
    conf.load('compiler_c')
    conf.load('compiler_cxx')
    
    autowaf.configure(conf)
    autowaf.display_header('Fabla Configuration')

    if conf.env['MSVC_COMPILER']:
        conf.env.append_unique('CFLAGS', ['-TP', '-MD'])
    else:
        conf.env.append_unique('CFLAGS', '-std=c99')

    if not autowaf.is_child():
        autowaf.check_pkg(conf, 'lv2core', uselib_store='LV2CORE')
    
    
    # faust include and fast math 
    conf.env.CFLAGS = ['-g', '-pg','-I/usr/lib/faust/','-I/usr/lib/faust/gui','-Wall','-fPIC','-O3','-ffast-math'] # '-g', '-pg', 
    conf.env.CXXFLAGS = ['-g', '-pg','-I/usr/lib/faust/','-I/usr/lib/faust/gui','-Wall','-fPIC','-O3','-ffast-math']
    
    
    # Check for gtkmm to build UI
    autowaf.check_pkg(conf, 'gtkmm-2.4', uselib_store='GTKMM')
    autowaf.check_pkg(conf, 'sndfile', uselib_store='SNDFILE')
    
    # Set env['pluginlib_PATTERN']
    pat = conf.env['cshlib_PATTERN']
    if pat.startswith('lib'):
        pat = pat[3:]
    conf.env['pluginlib_PATTERN'] = pat
    conf.env['pluginlib_EXT'] = pat[pat.rfind('.'):]

    autowaf.display_msg(conf, "LV2 bundle directory", conf.env['LV2DIR'])
    print('')

def build(bld):
    bundle = APPNAME

    # Build manifest.ttl by substitution (for portable lib extension)
    bld(features     = 'subst',
        source       = 'manifest.ttl.in',
        target       = '%s/%s' % (bundle, 'manifest.ttl'),
        install_path = '${LV2DIR}/%s' % bundle,
        LIB_EXT      = bld.env['pluginlib_EXT'])

    # Copy other data files to build bundle (build/fabla.lv2)
    for i in [ 'fabla.ttl' ]:
        bld(features     = 'subst',
            source       = i,
            target       = '%s/%s' % (bundle, i),
            install_path = '${LV2DIR}/%s' % bundle,
            LIB_EXT      = bld.env['pluginlib_EXT'])

    # Create a build environment that builds module-style library names
    # e.g. refractor.so instead of refractor.so
    # Note for C++ you must set cxxshlib_PATTERN instead
    penv                   = bld.env.derive()
    penv['cshlib_PATTERN'] = bld.env['pluginlib_PATTERN']
    penv['cxxshlib_PATTERN'] = bld.env['pluginlib_PATTERN']

    includes = None
    if autowaf.is_child:
        includes = '../..'

    # Build plugin library
    obj = bld(features     = 'cxx cshlib',
              env          = penv,
              source       = 'fabla.cxx',
              name         = 'fabla',
              target       = '%s/fabla' % bundle,
              install_path = '${LV2DIR}/%s' % bundle,
              uselib       = 'LV2CORE SNDFILE',
              includes     = includes)
    
    # Build plugin UI library
    obj = bld(features     = 'cxx cxxshlib',
              env          = penv,
              source       = 'canvas.cxx fabla_ui.cxx',
              name         = 'fabla_ui',
              target       = '%s/fabla_gui' % bundle,
              install_path = '${LV2DIR}/%s' % bundle,
              uselib       = 'LV2CORE GTKMM',
              includes     = includes)
    
    # install files
    bld.install_files('${PREFIX}/lib/lv2/fabla.lv2/', 'fablapresets.ttl')

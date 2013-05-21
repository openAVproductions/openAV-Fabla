#! /usr/bin/env python

VERSION= '1.0.0'
APPNAME= 'fabla.lv2'

top = '.'
out = '.build'

def options(opt):
  opt.load('compiler_c')
  opt.load('compiler_cxx')

def configure(conf):
  conf.load('compiler_c')
  conf.load('compiler_cxx')
  
  conf.env.CFLAGS = ['-I/usr/lib/faust/','-I/usr/lib/faust/gui','-Wall','-fPIC','-O3','-ffast-math']
  conf.env.CXXFLAGS = ['-I/usr/lib/faust/','-I/usr/lib/faust/gui','-Wall','-fPIC','-O3','-ffast-math']
    
  conf.env.append_value('CXXFLAGS', ['-g', '-Wall']) #'-O2'
  
  conf.check_cfg(package='gtkmm-2.4',args='--cflags --libs',uselib_store='GTKMM')
  conf.check_cfg(package='sndfile',at_least_version='1.0',args='--cflags --libs',uselib_store='SNDFILE')
  
  pat = conf.env['cshlib_PATTERN']
  if pat.startswith('lib'):
      pat = pat[3:]
  conf.env['pluginlib_PATTERN'] = pat
  conf.env['pluginlib_EXT'] = pat[pat.rfind('.'):]

def build(bld):
  
  bundle = APPNAME

  # Build manifest.ttl by substitution (for portable lib extension)
  bld(features     = 'subst',
      source       = 'manifest.ttl.in',
      target       = '%s/%s' % (bundle, 'manifest.ttl'),
      install_path = '${PREFIX}/lib/lv2/fabla.lv2',
      LIB_EXT      = bld.env['pluginlib_EXT'])
  
  # Copy other data files to build bundle (build/fabla.lv2)
  for i in [ 'fabla.ttl' ]:
      bld(features     = 'subst',
          source       = i,
          target       = '%s/%s' % (bundle, i),
          install_path = '${PREFIX}/lib/lv2/fabla.lv2',
          LIB_EXT      = bld.env['pluginlib_EXT'])

  # Create a build environment that builds module-style library names
  # e.g. refractor.so instead of refractor.so
  # Note for C++ you must set cxxshlib_PATTERN instead
  penv                   = bld.env.derive()
  penv['cshlib_PATTERN'] = bld.env['pluginlib_PATTERN']
  penv['cxxshlib_PATTERN'] = bld.env['pluginlib_PATTERN']
  
  includes = ""
  
  # Build plugin library
  obj = bld(features     = 'cxx cshlib',
            env          = penv,
            source       = 'fabla.cxx',
            name         = 'fabla',
            target       = '%s/fabla' % bundle,
            install_path = '${PREFIX}/lib/lv2/fabla.lv2',
            uselib       = 'LV2CORE SNDFILE',
            includes     = includes)
  
  # Build plugin UI library
  obj = bld(features     = 'cxx cxxshlib',
            env          = penv,
            source       = 'canvas.cxx fabla_ui.cxx',
            name         = 'fabla_ui',
            target       = '%s/fabla_gui' % bundle,
            install_path = '${PREFIX}/lib/lv2/fabla.lv2',
            uselib       = 'LV2CORE GTKMM',
            includes     = includes)
  
  # install files
  bld.install_files('${PREFIX}/lib/lv2/fabla.lv2/', 'planning/padload.png')
  bld.install_files('${PREFIX}/lib/lv2/fabla.lv2/', 'planning/padempty.png')
  bld.install_files('${PREFIX}/lib/lv2/fabla.lv2/', 'planning/padplay.png')
  bld.install_files('${PREFIX}/lib/lv2/fabla.lv2/', 'planning/padselect.png')
  bld.install_files('${PREFIX}/lib/lv2/fabla.lv2/', 'planning/header.png')


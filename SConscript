# -*- C++ -*-
# $Header$
# Authors: M.E. Monzani <monzani@slac.stanford.edu>
# Version: TestReport-12-00-00

Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
#libEnv = baseEnv.Clone()   #  doesn't build a lib


progEnv.Tool('addLibrary', library = baseEnv['rootLibs'])
progEnv.Tool('commonRootDataLib')
progEnv.Tool('digiRootDataLib')
progEnv.Tool('xmlBaseLib')
progEnv.Tool('tipLib')

RunVerify = progEnv.Program('RunVerify', ['src/MainVerify.cxx', 'src/RunVerify.cxx', 'src/AcdXmlUtil.cxx', 'src/DomElement.cxx'])
ft2Verify = progEnv.Program('ft2Verify', ['src/MainFt2Verify.cxx', 'src/ft2Verify.cxx', 'src/AcdXmlUtil.cxx', 'src/DomElement.cxx'])
ft1Verify = progEnv.Program('ft1Verify', ['src/MainFt1Verify.cxx', 'src/ft1Verify.cxx', 'src/AcdXmlUtil.cxx', 'src/DomElement.cxx'])
meritVerify = progEnv.Program('meritVerify', ['src/MainMeritVerify.cxx', 'src/meritVerify.cxx', 'src/AcdXmlUtil.cxx', 'src/DomElement.cxx'])

progEnv.Tool('registerTargets', package = 'TestReport',
             binaryCxts = [[RunVerify, progEnv],[ft2Verify, progEnv],[ft1Verify, progEnv],[meritVerify, progEnv]])



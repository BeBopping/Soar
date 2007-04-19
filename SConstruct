#!/usr/bin/python
import os
import sys
import SoarSCons

if os.name != "posix" and os.name != "nt":
	print "Unsupported platform:", os.name
	Exit(1)
	
# Optimization on the mac (posix) crashes things
if sys.platform == 'darwin':
	optimizationDefault = 'no'
else:
	optimizationDefault = 'full'

# Don't build python on windows (doesn't work yet)
# Don't build debug on windows (can't find the debug dll)
if os.name == 'nt':
	pythonDefault = 'no'
	debugDefault = 'no'
else:
	pythonDefault = 'yes'
	debugDefault = 'yes'

opts = Options()
opts.AddOptions(
	BoolOption('java', 'Build the Soar Java interface', 'yes'), 
	BoolOption('python', 'Build the Soar Python interface', pythonDefault), 
	BoolOption('static', 'Use static linking when possible', 'no'), 
	BoolOption('debug', 'Build with debugging symbols', debugDefault),
	BoolOption('warnings', 'Build with warnings', 'yes'),
	BoolOption('csharp', 'Build the Soar CSharp interface', 'no'), 
	BoolOption('tcl', 'Build the Soar Tcl interface', 'no'), 
	EnumOption('optimization', 'Build with optimization (May cause run-time errors!)', optimizationDefault, ['no','partial','full'], {}, 1),
	BoolOption('swt', 'Build Java SWT projects', 'yes'), 
)

env = Environment(options = opts)
Help(opts.GenerateHelpText(env))

# Create configure context to configure the environment
custom_tests = {
	'CheckVisibilityFlag' : SoarSCons.CheckVisibilityFlag,
}
conf = Configure(env, custom_tests = custom_tests)
conf.env.Append(CPPFLAGS = ' -DSCONS')
conf.env.Append(CPPPATH = ['#Core/shared'])
conf.env.Append(ENV = {'PATH' : os.environ['PATH']})

# configure java
if conf.env['java']:
	if not SoarSCons.ConfigureJNI(conf.env):
		print "Could not configure Java. If you know where java is on your system,"
		print "set environment variable JAVA_HOME to point to the directory containing"
		print "the Java include, bin, and lib directories."
		print "You may disable java, see help (scons -h)"
		print "Java Native Interface is required... Exiting"
		Exit(1)
	if env['swt'] and not SoarSCons.CheckForSWTJar(conf.env):
		print "Could not find swt.jar. You can obtain the jar here:"
		print "\thttp://winter.eecs.umich.edu/jars"
		print "Place swt.jar in SoarLibrary/bin"
		print "You may disable swt, see help (scons -h)"
		print "swt.jar required... Exiting"
		Exit(1)

# check SWIG version if necessary
if conf.env['java'] or conf.env['python'] or conf.env['csharp'] or conf.env['tcl']:
	if not SoarSCons.CheckSWIG(conf.env):
		explainSWIG = ""
		if conf.env['java']:
			explainSWIG += "java=1, "
		if conf.env['python']:
			explainSWIG += "python=1, "
		if conf.env['csharp']:
			explainSWIG += "csharp=1, "
		if conf.env['tcl']:
			explainSWIG += "tcl=1, "

		print "SWIG is required because", explainSWIG[:-2]
		Exit(1)
	
if os.name == 'posix':
	if sys.platform == "darwin":
		# From scons.org/wiki/MacOSX
		#conf.env['INSTALL'] = SoarSCons.osx_copy
		conf.env['SHLINKFLAGS'] = '$LINKFLAGS -dynamic '
		conf.env['SHLIBSUFFIX'] = '.dylib'
	
	# check if the compiler supports -fvisibility=hidden (GCC >= 4)
	if conf.CheckVisibilityFlag():
		conf.env.Append(CPPFLAGS = ' -fvisibility=hidden')
	
	# configure misc command line options
	if conf.env['debug']:
		conf.env.Append(CPPFLAGS = ' -g')
	if conf.env['warnings']:
		conf.env.Append(CPPFLAGS = ' -Wall')
	if conf.env['optimization'] == 'partial':
		conf.env.Append(CPPFLAGS = ' -O2')
	if conf.env['optimization'] == 'full':
		conf.env.Append(CPPFLAGS = ' -O3')
	
	# check for required libraries
	if not conf.CheckLib('dl'):
		Exit(1)
		
	if not conf.CheckLib('m'):
		Exit(1)
	
	if not conf.CheckLib('pthread'):
		Exit(1)
		
elif os.name == 'nt':
	conf.env.Append(CPPFLAGS = ' /D WIN32 /D _WINDOWS /D KERNELSML_EXPORTS /D "_CRT_SECURE_NO_DEPRECATE" /D "_VC80_UPGRADE=0x0710" /D "_MBCS" /D "_USRDLL" /W3 /Gy /c /EHsc /GF /errorReport:prompt')
	if conf.env['debug']:
		conf.env.Append(CPPFLAGS = ' /D _DEBUG /Od /D "DEBUG" /Gm /RTC1 /MDd /GS- /ZI /TP')
	else:
		conf.env.Append(CPPFLAGS = ' /O2 /Ob2 /Oy /GL /D "NDEBUG" /FD /MD /Zi')
else:
	print "Unknown os.name... Exiting"
	Exit(1)

env = conf.Finish()
Export('env')

# Core
SConscript('#Core/SoarKernel/SConscript')
SConscript('#Core/gSKI/SConscript')
SConscript('#Core/ConnectionSML/SConscript')
SConscript('#Core/ElementXML/SConscript')
SConscript('#Core/CLI/SConscript')
SConscript('#Core/ClientSML/SConscript')
SConscript('#Core/KernelSML/SConscript')

if env['java']:
	SConscript('#Core/ClientSMLSWIG/Java/SConscript')
	SConscript('#Tools/TestJavaSML/SConscript')
	
	# FIXME: VisualSoar's build command line is too long on windows
	if os.name != 'nt':
		SConscript('#Tools/VisualSoar/SConscript')
		
	if env['swt']:
		SConscript('#Tools/SoarJavaDebugger/SConscript')
		SConscript('#Environments/Soar2D/SConscript')
		SConscript('#Environments/JavaMissionaries/SConscript')
		SConscript('#Environments/JavaTOH/SConscript')

if env['python']:
	SConscript('#Core/ClientSMLSWIG/Python/SConscript')

SConscript('#Tools/TestCLI/SConscript')
SConscript('#Tools/FilterC/SConscript')
SConscript('#Tools/QuickLink/SConscript')
SConscript('#Tools/SoarTextIO/SConscript')
SConscript('#Tools/TOHSML/SConscript')
SConscript('#Tools/TestClientSML/SConscript')
SConscript('#Tools/TestConnectionSML/SConscript')
SConscript('#Tools/TestMultiAgent/SConscript')
SConscript('#Tools/TestSMLEvents/SConscript')
SConscript('#Tools/TestSMLPerformance/SConscript')
SConscript('#Tools/TestSoarPerformance/SConscript')


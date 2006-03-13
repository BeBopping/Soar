#!/usr/bin/perl
#
# New installer generation script.
#
# Assumptions:
# Run in Cygwin, svn in path

# Overview:
# Builds everything in release mode in the current tree.
# Exports everything in -source (up one dir and using distdir variable) for Source component
# Removes files from -source that are not to be included in the release
# Moves files from -source to -core for Core component
# Copies binaries from working tree to -core for Core component

use strict;
use File::Find::Rule;
use File::Copy::Recursive qw/rcopy rmove/;
use File::Copy;
use File::Path;
use Sys::Pushd;

##################
# Variables

# SVN url
my $soarurl = "https://winter.eecs.umich.edu/svn/soar/trunk/SoarSuite";

# Name and version
my $nameandversion = "Soar Suite 8.6.2-r3";

# File globs to completely remove from the tree (not distributed at all)
my @remove = qw/.cvsignore *.so *.so.2 *.jnilib java_swt make-mac-app.sh *.plist *.doc *.ppt *.pl *.am *.ac *.m4/;

# Globs to copy from working copy to Core component
# WORKING --copy-to-> CORE
my @copyglobs = qw(*.dll *.exe *.jar *.lib Tcl_sml_ClientInterface);

# Globs to MOVE from Source component to Core component
# SOURCE --move-to-> CORE
my @moveglobs = qw//;

# Nullsoft installer script input file
my $nsiinput = "8.6.2.nsi.in";
##################

my $namedashes = $nameandversion;
$namedashes =~ tr/ /-/;
my $core = "../$namedashes-core";
my $source = "../$namedashes-source";

# Parse command line options
my $build = 1;
my $checkout = 1;
my $nsionly = 0;
my @files;

foreach (@ARGV) {
	if ($_ eq "-nobuild") {
		$build = 0;
	}
	if ($_ eq "-nocheckout") {
		$checkout = 0;
	}
	if ($_ eq "-nsionly") {
		$nsionly = 1;
	}
}

if ($nsionly) {
	&nsi_step;
	exit(0);
}

if ($build == 1) {
	&build_step;
}

if ($checkout == 1) {
	&checkout_step;
}

&copy_step;
&move_step;
&nsi_step;
exit(0);

################################
# subroutines

sub build_step {
	print "Step 1: Rebuild everything...\n";
	system "rebuild-all.bat";
}

sub checkout_step { 
	print "Step 2: Check out source from SVN...\n";
	rmtree($source);
	system "svn export $soarurl $source";
	
	print "Step 3: Remove globs from source that are not to be distributed with the release...\n";
	@files = (File::Find::Rule->directory()->name(@remove)->in($source), File::Find::Rule->file()->name(@remove)->in($source));

	foreach (@files) {
		print "Removing from source: $_\n";
		unlink $_ or die "Unable to remove $_: $!";
	}
}

sub copy_step {
	print "Step 4: Copy globs from working tree to core...\n";
	rmtree($core, 1);
	@files = (File::Find::Rule->directory()->name(@copyglobs)->in("."), File::Find::Rule->file()->name(@copyglobs)->in("."));
	foreach(@files) {
		# This creates destination if it doesn't exist.
		print "Copying to core: $_\n";
		rcopy($_, "$core/$_");
	}
}

sub move_step {
	print "Step 5: Move globs from source tree to core...\n";
	@files = (File::Find::Rule->directory()->name(@moveglobs)->in($source), File::Find::Rule->file()->name(@moveglobs)->in($source));
	foreach(@files) {
		# This creates destination if it doesn't exist.
		print "Moving from source to core: $_\n";
		rcopy($_, "$core/$_");
	}
}

sub nsi_step {
	open(NSIINPUT, $nsiinput) or die "Couldn't open nsi input file: $!";

	while(<NSIINPUT>) {
		if (/(.*)nameandversion(.*)/) {
			print $1 . $nameandversion . $2 . "\n";
		} else if (/(.*)installdir(.*)/) {
			print $1 . "Soar\\" . $namedashes . $2 . "\n";
		} else if (/(.*)outfile(.*)/) {
			print $1 . $namedashes . ".exe" . $2 . "\n";
		} else if (/(.*)corefiles(.*)/) {
			# SetOutPath "$INSTDIR\"
			# File "SourceFile"
			# and so on
			my $ignore = new Sys::Pushd $core;
			my $windowscore = $core;
			$windowscore =~ tr/\//\\/;
			foreach (File::Find::Rule->directory->in(".")) {
				my $windowsdir = $_;
				$windowsdir =~ tr/\//\\/;
				
				print "\n\tSetOutPath \"\$INSTDIR\\$windowsdir\"\n";
				
				foreach (File::Find::Rule->file->in("$core/$_")) {
					my $windowsfile = $_;
					$windowsfile =~ tr/\//\\/;
					print "\tFile \"$windowsfile\"" . "\n";
				}
			}
			exit(0);
		} else {
			print $_ . "\n";		
		}
	}
}


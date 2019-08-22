# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.minifb.Debug:
/Users/litianyu/Documents/Projects/MyTracer/lib/minifb/Debug/libminifb.a:
	/bin/rm -f /Users/litianyu/Documents/Projects/MyTracer/lib/minifb/Debug/libminifb.a


PostBuild.minifb.Release:
/Users/litianyu/Documents/Projects/MyTracer/lib/minifb/Release/libminifb.a:
	/bin/rm -f /Users/litianyu/Documents/Projects/MyTracer/lib/minifb/Release/libminifb.a


PostBuild.minifb.MinSizeRel:
/Users/litianyu/Documents/Projects/MyTracer/lib/minifb/MinSizeRel/libminifb.a:
	/bin/rm -f /Users/litianyu/Documents/Projects/MyTracer/lib/minifb/MinSizeRel/libminifb.a


PostBuild.minifb.RelWithDebInfo:
/Users/litianyu/Documents/Projects/MyTracer/lib/minifb/RelWithDebInfo/libminifb.a:
	/bin/rm -f /Users/litianyu/Documents/Projects/MyTracer/lib/minifb/RelWithDebInfo/libminifb.a




# For each target create a dummy ruleso the target does not have to exist

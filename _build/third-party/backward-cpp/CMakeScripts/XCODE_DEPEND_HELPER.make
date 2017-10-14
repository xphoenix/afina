# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.backward.Debug:
/Users/romandegtyarev/start_here_now/technosphere/cplus/afina/_build/third-party/backward-cpp/Debug/libbackward.a:
	/bin/rm -f /Users/romandegtyarev/start_here_now/technosphere/cplus/afina/_build/third-party/backward-cpp/Debug/libbackward.a


PostBuild.backward_object.Debug:
PostBuild.backward.Release:
/Users/romandegtyarev/start_here_now/technosphere/cplus/afina/_build/third-party/backward-cpp/Release/libbackward.a:
	/bin/rm -f /Users/romandegtyarev/start_here_now/technosphere/cplus/afina/_build/third-party/backward-cpp/Release/libbackward.a


PostBuild.backward_object.Release:
PostBuild.backward.MinSizeRel:
/Users/romandegtyarev/start_here_now/technosphere/cplus/afina/_build/third-party/backward-cpp/MinSizeRel/libbackward.a:
	/bin/rm -f /Users/romandegtyarev/start_here_now/technosphere/cplus/afina/_build/third-party/backward-cpp/MinSizeRel/libbackward.a


PostBuild.backward_object.MinSizeRel:
PostBuild.backward.RelWithDebInfo:
/Users/romandegtyarev/start_here_now/technosphere/cplus/afina/_build/third-party/backward-cpp/RelWithDebInfo/libbackward.a:
	/bin/rm -f /Users/romandegtyarev/start_here_now/technosphere/cplus/afina/_build/third-party/backward-cpp/RelWithDebInfo/libbackward.a


PostBuild.backward_object.RelWithDebInfo:


# For each target create a dummy ruleso the target does not have to exist

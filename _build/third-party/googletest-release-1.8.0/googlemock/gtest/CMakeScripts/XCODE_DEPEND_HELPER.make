# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.gtest.Debug:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a


PostBuild.gtest_main.Debug:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest_main.a


PostBuild.gtest.Release:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a


PostBuild.gtest_main.Release:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest_main.a


PostBuild.gtest.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a


PostBuild.gtest_main.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest_main.a


PostBuild.gtest.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a


PostBuild.gtest_main.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest_main.a




# For each target create a dummy ruleso the target does not have to exist

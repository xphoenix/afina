# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.gmock.Debug:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Debug/libgmock.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Debug/libgmock.a


PostBuild.gmock_main.Debug:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Debug/libgmock_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Debug/libgmock_main.a


PostBuild.gtest.Debug:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a


PostBuild.gtest_main.Debug:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest_main.a


PostBuild.uv.Debug:
/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Debug/libuv.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Debug/libuv.a


PostBuild.backward.Debug:
/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/Debug/libbackward.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/Debug/libbackward.a


PostBuild.backward_object.Debug:
PostBuild.afina.Debug:
PostBuild.Network.Debug: /Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina
PostBuild.Storage.Debug: /Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina
PostBuild.cxxopts.Debug: /Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina
PostBuild.uv.Debug: /Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina
PostBuild.Protocol.Debug: /Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina
PostBuild.Execute.Debug: /Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina
PostBuild.Storage.Debug: /Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina
PostBuild.backward_object.Debug: /Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina
/Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina:\
	/Users/romandegtyarev/Desktop/afina/_build/src/network/Debug/libNetwork.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Debug/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Debug/libuv.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Debug/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/Debug/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Debug/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Debug/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/Debug/afina


PostBuild.Allocator.Debug:
/Users/romandegtyarev/Desktop/afina/_build/src/allocator/Debug/libAllocator.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/allocator/Debug/libAllocator.a


PostBuild.Execute.Debug:
/Users/romandegtyarev/Desktop/afina/_build/src/execute/Debug/libExecute.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/execute/Debug/libExecute.a


PostBuild.Protocol.Debug:
/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Debug/libProtocol.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/protocol/Debug/libProtocol.a


PostBuild.Network.Debug:
/Users/romandegtyarev/Desktop/afina/_build/src/network/Debug/libNetwork.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/network/Debug/libNetwork.a


PostBuild.Storage.Debug:
/Users/romandegtyarev/Desktop/afina/_build/src/storage/Debug/libStorage.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/storage/Debug/libStorage.a


PostBuild.runAllocatorTests.Debug:
PostBuild.Allocator.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Debug/runAllocatorTests
PostBuild.gtest.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Debug/runAllocatorTests
PostBuild.gtest_main.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Debug/runAllocatorTests
PostBuild.gtest.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Debug/runAllocatorTests
PostBuild.backward_object.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Debug/runAllocatorTests
/Users/romandegtyarev/Desktop/afina/_build/test/allocator/Debug/runAllocatorTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/allocator/Debug/libAllocator.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Debug/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Debug/runAllocatorTests


PostBuild.runExecuteTests.Debug:
PostBuild.Execute.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Debug/runExecuteTests
PostBuild.gtest.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Debug/runExecuteTests
PostBuild.gmock.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Debug/runExecuteTests
PostBuild.gmock_main.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Debug/runExecuteTests
PostBuild.Storage.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Debug/runExecuteTests
PostBuild.backward_object.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Debug/runExecuteTests
/Users/romandegtyarev/Desktop/afina/_build/test/execute/Debug/runExecuteTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/Debug/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Debug/libgmock.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Debug/libgmock_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Debug/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Debug/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/execute/Debug/runExecuteTests


PostBuild.runProtocolTests.Debug:
PostBuild.Protocol.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Debug/runProtocolTests
PostBuild.gtest.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Debug/runProtocolTests
PostBuild.gtest_main.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Debug/runProtocolTests
PostBuild.Execute.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Debug/runProtocolTests
PostBuild.Storage.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Debug/runProtocolTests
PostBuild.gtest.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Debug/runProtocolTests
PostBuild.backward_object.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Debug/runProtocolTests
/Users/romandegtyarev/Desktop/afina/_build/test/protocol/Debug/runProtocolTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Debug/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/Debug/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Debug/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Debug/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Debug/runProtocolTests


PostBuild.runNetworkTests.Debug:
PostBuild.Network.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests
PostBuild.gtest.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests
PostBuild.gtest_main.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests
PostBuild.uv.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests
PostBuild.Protocol.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests
PostBuild.Execute.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests
PostBuild.Storage.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests
PostBuild.gtest.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests
PostBuild.backward_object.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests
/Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/network/Debug/libNetwork.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Debug/libuv.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Debug/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/Debug/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Debug/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Debug/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/network/Debug/runNetworkTests


PostBuild.runStorageTests.Debug:
PostBuild.Storage.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Debug/runStorageTests
PostBuild.gtest.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Debug/runStorageTests
PostBuild.gtest_main.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Debug/runStorageTests
PostBuild.gtest.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Debug/runStorageTests
PostBuild.backward_object.Debug: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Debug/runStorageTests
/Users/romandegtyarev/Desktop/afina/_build/test/storage/Debug/runStorageTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Debug/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Debug/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/storage/Debug/runStorageTests


PostBuild.gmock.Release:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Release/libgmock.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Release/libgmock.a


PostBuild.gmock_main.Release:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Release/libgmock_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Release/libgmock_main.a


PostBuild.gtest.Release:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a


PostBuild.gtest_main.Release:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest_main.a


PostBuild.uv.Release:
/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Release/libuv.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Release/libuv.a


PostBuild.backward.Release:
/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/Release/libbackward.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/Release/libbackward.a


PostBuild.backward_object.Release:
PostBuild.afina.Release:
PostBuild.Network.Release: /Users/romandegtyarev/Desktop/afina/_build/src/Release/afina
PostBuild.Storage.Release: /Users/romandegtyarev/Desktop/afina/_build/src/Release/afina
PostBuild.cxxopts.Release: /Users/romandegtyarev/Desktop/afina/_build/src/Release/afina
PostBuild.uv.Release: /Users/romandegtyarev/Desktop/afina/_build/src/Release/afina
PostBuild.Protocol.Release: /Users/romandegtyarev/Desktop/afina/_build/src/Release/afina
PostBuild.Execute.Release: /Users/romandegtyarev/Desktop/afina/_build/src/Release/afina
PostBuild.Storage.Release: /Users/romandegtyarev/Desktop/afina/_build/src/Release/afina
PostBuild.backward_object.Release: /Users/romandegtyarev/Desktop/afina/_build/src/Release/afina
/Users/romandegtyarev/Desktop/afina/_build/src/Release/afina:\
	/Users/romandegtyarev/Desktop/afina/_build/src/network/Release/libNetwork.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Release/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Release/libuv.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Release/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/Release/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Release/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Release/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/Release/afina


PostBuild.Allocator.Release:
/Users/romandegtyarev/Desktop/afina/_build/src/allocator/Release/libAllocator.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/allocator/Release/libAllocator.a


PostBuild.Execute.Release:
/Users/romandegtyarev/Desktop/afina/_build/src/execute/Release/libExecute.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/execute/Release/libExecute.a


PostBuild.Protocol.Release:
/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Release/libProtocol.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/protocol/Release/libProtocol.a


PostBuild.Network.Release:
/Users/romandegtyarev/Desktop/afina/_build/src/network/Release/libNetwork.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/network/Release/libNetwork.a


PostBuild.Storage.Release:
/Users/romandegtyarev/Desktop/afina/_build/src/storage/Release/libStorage.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/storage/Release/libStorage.a


PostBuild.runAllocatorTests.Release:
PostBuild.Allocator.Release: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Release/runAllocatorTests
PostBuild.gtest.Release: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Release/runAllocatorTests
PostBuild.gtest_main.Release: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Release/runAllocatorTests
PostBuild.gtest.Release: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Release/runAllocatorTests
PostBuild.backward_object.Release: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Release/runAllocatorTests
/Users/romandegtyarev/Desktop/afina/_build/test/allocator/Release/runAllocatorTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/allocator/Release/libAllocator.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Release/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/allocator/Release/runAllocatorTests


PostBuild.runExecuteTests.Release:
PostBuild.Execute.Release: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Release/runExecuteTests
PostBuild.gtest.Release: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Release/runExecuteTests
PostBuild.gmock.Release: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Release/runExecuteTests
PostBuild.gmock_main.Release: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Release/runExecuteTests
PostBuild.Storage.Release: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Release/runExecuteTests
PostBuild.backward_object.Release: /Users/romandegtyarev/Desktop/afina/_build/test/execute/Release/runExecuteTests
/Users/romandegtyarev/Desktop/afina/_build/test/execute/Release/runExecuteTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/Release/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Release/libgmock.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Release/libgmock_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Release/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Release/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/execute/Release/runExecuteTests


PostBuild.runProtocolTests.Release:
PostBuild.Protocol.Release: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Release/runProtocolTests
PostBuild.gtest.Release: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Release/runProtocolTests
PostBuild.gtest_main.Release: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Release/runProtocolTests
PostBuild.Execute.Release: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Release/runProtocolTests
PostBuild.Storage.Release: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Release/runProtocolTests
PostBuild.gtest.Release: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Release/runProtocolTests
PostBuild.backward_object.Release: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Release/runProtocolTests
/Users/romandegtyarev/Desktop/afina/_build/test/protocol/Release/runProtocolTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Release/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/Release/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Release/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Release/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/protocol/Release/runProtocolTests


PostBuild.runNetworkTests.Release:
PostBuild.Network.Release: /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests
PostBuild.gtest.Release: /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests
PostBuild.gtest_main.Release: /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests
PostBuild.uv.Release: /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests
PostBuild.Protocol.Release: /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests
PostBuild.Execute.Release: /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests
PostBuild.Storage.Release: /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests
PostBuild.gtest.Release: /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests
PostBuild.backward_object.Release: /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests
/Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/network/Release/libNetwork.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Release/libuv.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Release/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/Release/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Release/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Release/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/network/Release/runNetworkTests


PostBuild.runStorageTests.Release:
PostBuild.Storage.Release: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Release/runStorageTests
PostBuild.gtest.Release: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Release/runStorageTests
PostBuild.gtest_main.Release: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Release/runStorageTests
PostBuild.gtest.Release: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Release/runStorageTests
PostBuild.backward_object.Release: /Users/romandegtyarev/Desktop/afina/_build/test/storage/Release/runStorageTests
/Users/romandegtyarev/Desktop/afina/_build/test/storage/Release/runStorageTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/Release/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Release/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/storage/Release/runStorageTests


PostBuild.gmock.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/MinSizeRel/libgmock.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/MinSizeRel/libgmock.a


PostBuild.gmock_main.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/MinSizeRel/libgmock_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/MinSizeRel/libgmock_main.a


PostBuild.gtest.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a


PostBuild.gtest_main.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest_main.a


PostBuild.uv.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/MinSizeRel/libuv.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/MinSizeRel/libuv.a


PostBuild.backward.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/MinSizeRel/libbackward.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/MinSizeRel/libbackward.a


PostBuild.backward_object.MinSizeRel:
PostBuild.afina.MinSizeRel:
PostBuild.Network.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina
PostBuild.Storage.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina
PostBuild.cxxopts.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina
PostBuild.uv.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina
PostBuild.Protocol.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina
PostBuild.Execute.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina
PostBuild.Storage.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina
PostBuild.backward_object.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina
/Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina:\
	/Users/romandegtyarev/Desktop/afina/_build/src/network/MinSizeRel/libNetwork.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/MinSizeRel/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/MinSizeRel/libuv.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/MinSizeRel/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/MinSizeRel/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/MinSizeRel/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/MinSizeRel/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/MinSizeRel/afina


PostBuild.Allocator.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/src/allocator/MinSizeRel/libAllocator.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/allocator/MinSizeRel/libAllocator.a


PostBuild.Execute.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/src/execute/MinSizeRel/libExecute.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/execute/MinSizeRel/libExecute.a


PostBuild.Protocol.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/src/protocol/MinSizeRel/libProtocol.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/protocol/MinSizeRel/libProtocol.a


PostBuild.Network.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/src/network/MinSizeRel/libNetwork.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/network/MinSizeRel/libNetwork.a


PostBuild.Storage.MinSizeRel:
/Users/romandegtyarev/Desktop/afina/_build/src/storage/MinSizeRel/libStorage.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/storage/MinSizeRel/libStorage.a


PostBuild.runAllocatorTests.MinSizeRel:
PostBuild.Allocator.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/MinSizeRel/runAllocatorTests
PostBuild.gtest.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/MinSizeRel/runAllocatorTests
PostBuild.gtest_main.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/MinSizeRel/runAllocatorTests
PostBuild.gtest.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/MinSizeRel/runAllocatorTests
PostBuild.backward_object.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/MinSizeRel/runAllocatorTests
/Users/romandegtyarev/Desktop/afina/_build/test/allocator/MinSizeRel/runAllocatorTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/allocator/MinSizeRel/libAllocator.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/MinSizeRel/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/allocator/MinSizeRel/runAllocatorTests


PostBuild.runExecuteTests.MinSizeRel:
PostBuild.Execute.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/execute/MinSizeRel/runExecuteTests
PostBuild.gtest.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/execute/MinSizeRel/runExecuteTests
PostBuild.gmock.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/execute/MinSizeRel/runExecuteTests
PostBuild.gmock_main.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/execute/MinSizeRel/runExecuteTests
PostBuild.Storage.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/execute/MinSizeRel/runExecuteTests
PostBuild.backward_object.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/execute/MinSizeRel/runExecuteTests
/Users/romandegtyarev/Desktop/afina/_build/test/execute/MinSizeRel/runExecuteTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/MinSizeRel/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/MinSizeRel/libgmock.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/MinSizeRel/libgmock_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/MinSizeRel/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/MinSizeRel/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/execute/MinSizeRel/runExecuteTests


PostBuild.runProtocolTests.MinSizeRel:
PostBuild.Protocol.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/MinSizeRel/runProtocolTests
PostBuild.gtest.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/MinSizeRel/runProtocolTests
PostBuild.gtest_main.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/MinSizeRel/runProtocolTests
PostBuild.Execute.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/MinSizeRel/runProtocolTests
PostBuild.Storage.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/MinSizeRel/runProtocolTests
PostBuild.gtest.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/MinSizeRel/runProtocolTests
PostBuild.backward_object.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/MinSizeRel/runProtocolTests
/Users/romandegtyarev/Desktop/afina/_build/test/protocol/MinSizeRel/runProtocolTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/MinSizeRel/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/MinSizeRel/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/MinSizeRel/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/MinSizeRel/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/protocol/MinSizeRel/runProtocolTests


PostBuild.runNetworkTests.MinSizeRel:
PostBuild.Network.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests
PostBuild.gtest.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests
PostBuild.gtest_main.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests
PostBuild.uv.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests
PostBuild.Protocol.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests
PostBuild.Execute.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests
PostBuild.Storage.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests
PostBuild.gtest.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests
PostBuild.backward_object.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests
/Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/network/MinSizeRel/libNetwork.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/MinSizeRel/libuv.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/MinSizeRel/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/MinSizeRel/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/MinSizeRel/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/MinSizeRel/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/network/MinSizeRel/runNetworkTests


PostBuild.runStorageTests.MinSizeRel:
PostBuild.Storage.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/storage/MinSizeRel/runStorageTests
PostBuild.gtest.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/storage/MinSizeRel/runStorageTests
PostBuild.gtest_main.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/storage/MinSizeRel/runStorageTests
PostBuild.gtest.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/storage/MinSizeRel/runStorageTests
PostBuild.backward_object.MinSizeRel: /Users/romandegtyarev/Desktop/afina/_build/test/storage/MinSizeRel/runStorageTests
/Users/romandegtyarev/Desktop/afina/_build/test/storage/MinSizeRel/runStorageTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/MinSizeRel/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/MinSizeRel/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/storage/MinSizeRel/runStorageTests


PostBuild.gmock.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/RelWithDebInfo/libgmock.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/RelWithDebInfo/libgmock.a


PostBuild.gmock_main.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/RelWithDebInfo/libgmock_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/RelWithDebInfo/libgmock_main.a


PostBuild.gtest.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a


PostBuild.gtest_main.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest_main.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest_main.a


PostBuild.uv.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/RelWithDebInfo/libuv.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/RelWithDebInfo/libuv.a


PostBuild.backward.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/RelWithDebInfo/libbackward.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/RelWithDebInfo/libbackward.a


PostBuild.backward_object.RelWithDebInfo:
PostBuild.afina.RelWithDebInfo:
PostBuild.Network.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina
PostBuild.Storage.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina
PostBuild.cxxopts.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina
PostBuild.uv.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina
PostBuild.Protocol.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina
PostBuild.Execute.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina
PostBuild.Storage.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina
PostBuild.backward_object.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina
/Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina:\
	/Users/romandegtyarev/Desktop/afina/_build/src/network/RelWithDebInfo/libNetwork.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/RelWithDebInfo/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/RelWithDebInfo/libuv.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/RelWithDebInfo/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/RelWithDebInfo/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/RelWithDebInfo/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/RelWithDebInfo/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/RelWithDebInfo/afina


PostBuild.Allocator.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/src/allocator/RelWithDebInfo/libAllocator.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/allocator/RelWithDebInfo/libAllocator.a


PostBuild.Execute.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/src/execute/RelWithDebInfo/libExecute.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/execute/RelWithDebInfo/libExecute.a


PostBuild.Protocol.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/src/protocol/RelWithDebInfo/libProtocol.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/protocol/RelWithDebInfo/libProtocol.a


PostBuild.Network.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/src/network/RelWithDebInfo/libNetwork.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/network/RelWithDebInfo/libNetwork.a


PostBuild.Storage.RelWithDebInfo:
/Users/romandegtyarev/Desktop/afina/_build/src/storage/RelWithDebInfo/libStorage.a:
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/src/storage/RelWithDebInfo/libStorage.a


PostBuild.runAllocatorTests.RelWithDebInfo:
PostBuild.Allocator.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/RelWithDebInfo/runAllocatorTests
PostBuild.gtest.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/RelWithDebInfo/runAllocatorTests
PostBuild.gtest_main.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/RelWithDebInfo/runAllocatorTests
PostBuild.gtest.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/RelWithDebInfo/runAllocatorTests
PostBuild.backward_object.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/allocator/RelWithDebInfo/runAllocatorTests
/Users/romandegtyarev/Desktop/afina/_build/test/allocator/RelWithDebInfo/runAllocatorTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/allocator/RelWithDebInfo/libAllocator.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/RelWithDebInfo/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/allocator/RelWithDebInfo/runAllocatorTests


PostBuild.runExecuteTests.RelWithDebInfo:
PostBuild.Execute.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/execute/RelWithDebInfo/runExecuteTests
PostBuild.gtest.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/execute/RelWithDebInfo/runExecuteTests
PostBuild.gmock.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/execute/RelWithDebInfo/runExecuteTests
PostBuild.gmock_main.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/execute/RelWithDebInfo/runExecuteTests
PostBuild.Storage.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/execute/RelWithDebInfo/runExecuteTests
PostBuild.backward_object.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/execute/RelWithDebInfo/runExecuteTests
/Users/romandegtyarev/Desktop/afina/_build/test/execute/RelWithDebInfo/runExecuteTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/RelWithDebInfo/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/RelWithDebInfo/libgmock.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/RelWithDebInfo/libgmock_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/RelWithDebInfo/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/RelWithDebInfo/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/execute/RelWithDebInfo/runExecuteTests


PostBuild.runProtocolTests.RelWithDebInfo:
PostBuild.Protocol.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/RelWithDebInfo/runProtocolTests
PostBuild.gtest.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/RelWithDebInfo/runProtocolTests
PostBuild.gtest_main.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/RelWithDebInfo/runProtocolTests
PostBuild.Execute.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/RelWithDebInfo/runProtocolTests
PostBuild.Storage.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/RelWithDebInfo/runProtocolTests
PostBuild.gtest.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/RelWithDebInfo/runProtocolTests
PostBuild.backward_object.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/protocol/RelWithDebInfo/runProtocolTests
/Users/romandegtyarev/Desktop/afina/_build/test/protocol/RelWithDebInfo/runProtocolTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/RelWithDebInfo/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/RelWithDebInfo/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/RelWithDebInfo/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/RelWithDebInfo/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/protocol/RelWithDebInfo/runProtocolTests


PostBuild.runNetworkTests.RelWithDebInfo:
PostBuild.Network.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests
PostBuild.gtest.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests
PostBuild.gtest_main.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests
PostBuild.uv.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests
PostBuild.Protocol.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests
PostBuild.Execute.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests
PostBuild.Storage.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests
PostBuild.gtest.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests
PostBuild.backward_object.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests
/Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/network/RelWithDebInfo/libNetwork.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/RelWithDebInfo/libuv.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/protocol/RelWithDebInfo/libProtocol.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/execute/RelWithDebInfo/libExecute.a\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/RelWithDebInfo/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/RelWithDebInfo/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/network/RelWithDebInfo/runNetworkTests


PostBuild.runStorageTests.RelWithDebInfo:
PostBuild.Storage.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/storage/RelWithDebInfo/runStorageTests
PostBuild.gtest.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/storage/RelWithDebInfo/runStorageTests
PostBuild.gtest_main.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/storage/RelWithDebInfo/runStorageTests
PostBuild.gtest.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/storage/RelWithDebInfo/runStorageTests
PostBuild.backward_object.RelWithDebInfo: /Users/romandegtyarev/Desktop/afina/_build/test/storage/RelWithDebInfo/runStorageTests
/Users/romandegtyarev/Desktop/afina/_build/test/storage/RelWithDebInfo/runStorageTests:\
	/Users/romandegtyarev/Desktop/afina/_build/src/storage/RelWithDebInfo/libStorage.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest_main.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a\
	/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/RelWithDebInfo/backward_object.build/Objects-normal/libbackward_object.a
	/bin/rm -f /Users/romandegtyarev/Desktop/afina/_build/test/storage/RelWithDebInfo/runStorageTests




# For each target create a dummy ruleso the target does not have to exist
/Users/romandegtyarev/Desktop/afina/_build/src/allocator/Debug/libAllocator.a:
/Users/romandegtyarev/Desktop/afina/_build/src/allocator/MinSizeRel/libAllocator.a:
/Users/romandegtyarev/Desktop/afina/_build/src/allocator/RelWithDebInfo/libAllocator.a:
/Users/romandegtyarev/Desktop/afina/_build/src/allocator/Release/libAllocator.a:
/Users/romandegtyarev/Desktop/afina/_build/src/execute/Debug/libExecute.a:
/Users/romandegtyarev/Desktop/afina/_build/src/execute/MinSizeRel/libExecute.a:
/Users/romandegtyarev/Desktop/afina/_build/src/execute/RelWithDebInfo/libExecute.a:
/Users/romandegtyarev/Desktop/afina/_build/src/execute/Release/libExecute.a:
/Users/romandegtyarev/Desktop/afina/_build/src/network/Debug/libNetwork.a:
/Users/romandegtyarev/Desktop/afina/_build/src/network/MinSizeRel/libNetwork.a:
/Users/romandegtyarev/Desktop/afina/_build/src/network/RelWithDebInfo/libNetwork.a:
/Users/romandegtyarev/Desktop/afina/_build/src/network/Release/libNetwork.a:
/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Debug/libProtocol.a:
/Users/romandegtyarev/Desktop/afina/_build/src/protocol/MinSizeRel/libProtocol.a:
/Users/romandegtyarev/Desktop/afina/_build/src/protocol/RelWithDebInfo/libProtocol.a:
/Users/romandegtyarev/Desktop/afina/_build/src/protocol/Release/libProtocol.a:
/Users/romandegtyarev/Desktop/afina/_build/src/storage/Debug/libStorage.a:
/Users/romandegtyarev/Desktop/afina/_build/src/storage/MinSizeRel/libStorage.a:
/Users/romandegtyarev/Desktop/afina/_build/src/storage/RelWithDebInfo/libStorage.a:
/Users/romandegtyarev/Desktop/afina/_build/src/storage/Release/libStorage.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Debug/backward_object.build/Objects-normal/libbackward_object.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/MinSizeRel/backward_object.build/Objects-normal/libbackward_object.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/RelWithDebInfo/backward_object.build/Objects-normal/libbackward_object.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/backward-cpp/afina.build/Release/backward_object.build/Objects-normal/libbackward_object.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Debug/libgmock.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Debug/libgmock_main.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/MinSizeRel/libgmock.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/MinSizeRel/libgmock_main.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/RelWithDebInfo/libgmock.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/RelWithDebInfo/libgmock_main.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Release/libgmock.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/Release/libgmock_main.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Debug/libgtest_main.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/MinSizeRel/libgtest_main.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/RelWithDebInfo/libgtest_main.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/googletest-release-1.8.0/googlemock/gtest/Release/libgtest_main.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Debug/libuv.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/MinSizeRel/libuv.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/RelWithDebInfo/libuv.a:
/Users/romandegtyarev/Desktop/afina/_build/third-party/libuv-1.11.0/Release/libuv.a:

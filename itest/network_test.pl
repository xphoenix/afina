#!/usr/bin/perl
use 5.016;
use warnings;
use threads;
use Test::More tests => 85;
use IO::Socket::INET;
use Getopt::Long;

# horrible gut digging to persuade `explain` to print source code
no warnings 'once';
$Data::Dumper::Deparse = 1;

my $server = $ENV{AFINA_SERVER} // "127.0.0.1";
my $port = $ENV{AFINA_PORT} // "8080";
my $silent = 0;

my ($rfifo, $wfifo) = @ENV{"AFINA_RFIFO", "AFINA_WFIFO"};

GetOptions(
	"address=s" => \$server,
	"port=i" => \$port,
	"silent" => \$silent,
	"rfifo=s" => \$rfifo,
	"wfifo=s" => \$wfifo,
) or die "Usage: $0 [-a server] [-p port]\n";

sub make_afina_socket {
	if (defined $rfifo and defined $wfifo) {
		open my $wf, ">", $rfifo; # afina reads from there; we write
		open my $rf, "<", $wfifo; # afina writes there; we read
		return ($wf, $rf);
	} else {
		my $socket = IO::Socket::INET::->new(
			PeerAddr => "$server:$port",
			Proto => "tcp"
		);
		return ($socket, $socket);
	}
}

sub afina_request { # 5 tests
	my ($request) = @_;
	my ($wf, $rf) = make_afina_socket;
	ok($wf, "Connected to Afina for writing");
	ok($rf, "Connected to Afina for reading");
	ok(
		ref $request ? $request->($wf)
			: print($wf $request),
		"Sent request"
	);
	$silent or note ref $request ? explain $request : $request =~ s/^/-> /mrg;
	if ($wf->isa("IO::Socket::INET")) {
		ok(shutdown($wf, SHUT_WR()), "Closed writing end of connection");
	} else {
		ok(close($wf), "Closed writing end of connection");
	}
	my $received;
	if ($rf->isa("IO::Socket::INET")) {
		$received .= $_ while (<$rf>); # networked Afina must properly close socket when client is done
	} else {
		sysread($rf, $received, 65536); # do a single read, don't expect Afina to close fd
	}
	$silent or note $received =~ s/^/<- /mrg;
	$received;
}

sub afina_test { # 4 tests
	my ($request, $response, $desc) = @_;
	ref $response ? like(afina_request($request), $response, $desc)
		: is(afina_request($request), $response, $desc);
}

afina_test("set foo 0 0 6\r\nfoobar\r\n", "STORED\r\n", "Set command");
afina_test("get foo\r\n", "VALUE foo 0 6\r\nfoobar\r\nEND\r\n", "Get the value we just set");
afina_test("get foo\r\nget foo\r\n", "VALUE foo 0 6\r\nfoobar\r\nEND\r\nVALUE foo 0 6\r\nfoobar\r\nEND\r\n", "Multiple commands");

afina_test(
	"set foo 0 0 3\r\nwtf\r\n"
	."set bar 0 0 3\r\nzzz\r\n"
	."get foo bar\r\n",
	"STORED\r\n"
	."STORED\r\n"
	."VALUE foo 0 3\r\nwtf\r\n"
	."VALUE bar 0 3\r\nzzz\r\n"
	."END\r\n",
	"Multiple commands with body"
);

afina_test(
	"add test 0 0 6\r\nfoobar\r\n",
	"STORED\r\n",
	"Add non-existent key"
);

afina_test(
	"add test 0 0 6\r\nfoobar\r\n",
	"NOT_STORED\r\n",
	"Don't add existent key"
);

afina_test(
	"append test_ 0 0 3\r\nwtf\r\n",
	"NOT_STORED\r\n",
	"Don't append non-existent key"
);

afina_test(
	"append test 0 0 3\r\nwtf\r\n",
	"STORED\r\n",
	"Append an existent key"
);

afina_test(
	"get test\r\n",
	"VALUE test 0 9\r\nfoobarwtf\r\nEND\r\n",
	"Verify the append"
);

TODO: {
	local $TODO = "Replace command isn't yet implemented in the parser";

	afina_test(
		"replace test_ 0 0 3\r\nwtf\r\n",
		"NOT_STORED\r\n",
		"Don't replace non-existent key"
	);

	afina_test(
		"replace test 0 0 3\r\nzzz\r\n",
		"STORED\r\n",
		"Replace an existent key"
	);

	afina_test(
		"get test\r\n",
		"VALUE test 0 3\r\nzzz\r\nEND\r\n",
		"Verify replace"
	);
}

TODO: {
	local $TODO = "Delete command isn't yet implemented";

	afina_test(
		"delete test\r\n",
		"DELETED\r\n",
		"Delete a key"
	);
}

afina_test(
	"blablabla 0 0 0\r\n",
	qr/ERROR/,
	"Must report unknown command error to user"
);

afina_test(
	"get var\r\r",
	qr/ERROR/,
	"Must report desync errors to user"
);

afina_test(
	sub {
		my $socket = shift;
		# make sure everything we write ends up before afina's eyes as we want it
		$socket->autoflush(1);
		print $socket "set foo 0 ";
		sleep 1;
		print $socket "0 3\r";
		sleep 1;
		print $socket "\nwtf\r\n";
	},
	"STORED\r\n",
	"Must correctly handle partial writes"
);

afina_test(
	"get foo\r\n",
	"VALUE foo 0 3\r\nwtf\r\nEND\r\n",
	"Correct result of partially written command"
);

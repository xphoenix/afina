use warnings;
use strict;
use feature qw(say);
use DDP;
use Fcntl;

use IO::Socket::INET;

sub logg {
    my @msg = @_;
    say "[$$] ".join(" ", @msg);
}

logg "Connect to server with port: 8080";
logg "Storage must be 1024 size!";

my $socket = get_socket_to_server(8080);
unless ($socket) {
    logg "can not create socket";
    exit;
}
$socket->autoflush(1);

logg "Simple set/get test";

my $key = "key";
my $value = "value";

my $data = set($socket, $key, $value);
if ($data eq "STORED") {
    logg "stored ok";
} else {
    logg "stored err";
    exit;
}

$data = get($socket, $key);
if ($data eq "VALUE $key 0 ".length($value)."\r\n$value\r\nEND") {
    logg "get ok";
} else {
    logg "get not ok", quote_symbols($data);
    exit;
}

logg "get bad key";
$data = get($socket, "no_key");
if ($data eq "END") {
    logg "bad key ok";
} else {
    logg "bad ket not ok";
    exit;
}

close($socket);
# Help subs

sub set {
    my ($socket, $key, $value) = @_;
    my $msg = _set($key, $value);

    if (!socket_write($socket, $msg)) {
        logg "can not write to socket";
        exit;
    }

    return recv_data($socket);
}

sub get {
    my ($socket, $key) = @_;
    my $msg = _get($key);
    if (!socket_write($socket, $msg)) {
        logg "can not write to socket";
        exit;
    }

    return recv_data($socket);
}

sub _get {
    my ($key) = @_;
    return "get $key\r\n";
}

sub _set {
    my ($key, $value) = @_;
    return "set $key 0 0 ".length($value)."\r\n$value\r\n";
}

sub recv_data {
    my ($socket) = @_;
    my $data;
    recv($socket, $data, 2000, 0);
    return $data;
}

sub quote_symbols {
    my ($msg) = @_;
    $msg =~ s/\n/\\n/g;
    $msg =~ s/\r/\\r/g;
    return $msg;
}

sub socket_write {
	my ($socket, $msg) = @_;
    my $sended = 0;
    my $buff;
    my $ret = recv($socket, $buff, 1, MSG_PEEK | MSG_DONTWAIT);;
    if (defined $ret) {
        return 0;
    }
    logg "start send: \"".quote_symbols($msg)."\", size=".length($msg);
	$sended = send($socket, $msg, MSG_DONTWAIT | O_NONBLOCK);
    logg "send($sended)";
    if ($sended) { 
        return 1;
    } else {
        return 0;
    }
}

sub get_socket_to_server {
	my ($port) = @_;
	my $socket = IO::Socket::INET->new(
		PeerAddr => 'localhost',
		PeerPort => $port,
		Proto => "tcp",
		Type => SOCK_STREAM,
	) or undef;

	return $socket;
}

1;

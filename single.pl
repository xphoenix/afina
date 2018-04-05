use warnings;
use strict;
use feature qw(say);
use DDP;

use IO::Socket::INET;

my $socket = get_socket_to_server(8080);
$socket->autoflush(1);
my $msg = "key";
socket_write($socket, "set $msg 0 0 6\r\nnewval\r\n");

my $data;
recv($socket, $data, 2000, 0);
say "data = $data";
$data = "";
say "wait type";
$data = <>;
close($socket);
say "close socket";


# Help subs

sub socket_write {
	my ($socket, $msg) = @_;
	send($socket, $msg, 0);
}

sub get_socket_to_server {
	my ($port) = @_;
	my $socket = IO::Socket::INET->new(
		PeerAddr => 'localhost',
		PeerPort => $port,
		Proto => "tcp",
		Type => SOCK_STREAM,
	) or die "Can`t connect to localhost $/";

	return $socket;
}

1;

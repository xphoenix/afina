package Server;

use strict;
use warnings;
use feature qw(say);

use Fcntl;
use IO::Socket::INET;
use Mouse;

use FindBin;
use lib "$FindBin::Bin";
use Utils qw(quote_symbols);

has 'port' => (
    is  => 'rw',
    isa => 'Int',
);

has 'socket' => (
    is  => 'rw',
    isa => 'IO::Socket::INET',
    lazy_build => 1,
);

sub _build_socket {
    my $self = shift;

	my $socket = IO::Socket::INET->new(
		PeerAddr => 'localhost',
		PeerPort => $self->port,
		Proto => "tcp",
		Type => SOCK_STREAM,
	) or undef;

    if ($socket) {
        $socket->autoflush(1);
    }

	return $socket;
}

sub replace {
    my ($self, $key, $value) = @_;
    my $msg = _replace($key, $value);

    return $self->apply_msg($msg);
}

sub _replace {
    my ($key, $value) = @_;
    return "replace $key 0 0 ".length($value)."\r\n$value\r\n";
}

sub append {
    my ($self, $key, $value) = @_;
    my $msg = _append($key, $value);

    if (!$self->socket_write($msg)) {
        say "can not write to socket";
        return;
    }

    return $self->recv_data;
}

sub _append {
    my ($key, $value) = @_;
    return "append $key 0 0 ".length($value)."\r\n$value\r\n";
}

sub add {
    my ($self, $key, $value) = @_;
    my $msg = _add($key, $value);

    if (!$self->socket_write($msg)) {
        say "can not write to socket";
        return;
    }

    return $self->recv_data;
}

sub _add {
    my ($key, $value) = @_;
    return "add $key 0 0 ".length($value)."\r\n$value\r\n";
}

sub set {
    my ($self, $key, $value) = @_;
    my $msg = _set($key, $value);

    if (!$self->socket_write($msg)) {
        say "can not write to socket";
        return;
    }

    return $self->recv_data;
}

sub _set {
    my ($key, $value) = @_;
    return "set $key 0 0 ".length($value)."\r\n$value\r\n";
}

sub get {
    my ($self, @keys) = @_;
    my $msg = _get(@keys);
    if (!$self->socket_write($msg)) {
        say "can not write to socket";
        return;
    }

    return $self->recv_data;
}

sub _get {
    my (@keys) = @_;
    return "get ".join(" ", @keys)."\r\n";
}

sub recv_data {
    my ($self) = @_;
    my $data;
    recv($self->socket, $data, 2000, 0);
    say "recv: " . quote_symbols($data);
    return $data;
}

sub socket_write {
	my ($self, $msg) = @_;
    my $sended = 0;
    my $buff;
    my $ret = recv($self->socket, $buff, 1, MSG_PEEK | MSG_DONTWAIT);;
    if (defined $ret) {
        return 0;
    }
    say "start send: \"".quote_symbols($msg)."\", size=".length($msg);
	$sended = send($self->socket, $msg, MSG_DONTWAIT | O_NONBLOCK);
    if ($sended) { 
        return 1;
    } else {
        return 0;
    }
}

sub close {
    my ($self) = @_;
    close($self->socket);
}

sub apply_msg {
    my ($self, $msg) = @_;

    if (!$self->socket_write($msg)) {
        say "can not write to socket";
        return;
    }

    return $self->recv_data;
}

1;

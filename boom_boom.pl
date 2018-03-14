use warnings;
use strict;
use feature qw(say);
use DDP;

use IO::Socket::INET;

my $fork_cnt = 0;
say "Write forks num";
$fork_cnt = <>;
$fork_cnt = int($fork_cnt);

my $worker_num = 0;
my @pids = ();
my $logg_prefix = "worker";

sub logg {
    my @msg = @_;
    say "$logg_prefix [$worker_num, $$]".join(" ", @msg);
}

for my $i (1 .. $fork_cnt) {
    $worker_num++;
    if (my $pid = fork) {
        push @pids, $pid;
        next;
    }
    my $pre = $$;
    my $suf = 0;
    while (1) {
        logg "create socket";
        my $socket = get_socket_to_server(8080);
        unless ($socket) {
            logg "can not create socket";
            next;
        }
        $socket->autoflush(1);
        for (1..3) {
            my $msg = $pre."foo".$suf;
            $suf++;
            my $full_msg = "set $msg 0 0 6\r\nnewval\r\n";
            logg "start send($msg), size=".length($full_msg);
            socket_write($socket, $full_msg);

            my $data;
            recv($socket, $data, 2000, 0);
            logg "data = $data";
            $data = "";
        }
        close($socket);
    }
    exit;
}

say "wait until type";
sleep 120;
for my $pid (@pids) {
    kill 'INT', $pid;
}



# Help subs

sub socket_write {
	my ($socket, $msg) = @_;
    my $sended = 0;
	$sended = send($socket, $msg, 0);
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

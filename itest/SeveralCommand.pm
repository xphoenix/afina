package SeveralCommand;

use strict;
use warnings;
use feature qw(say);

use FindBin;
use lib "$FindBin::Bin";

use Server;
use Utils qw(quote_symbols);

sub main {
    my $self = shift;

    my $server = Server->new(port => 8080);
    return 0 unless $server;

    my @keys = ("several_key_1", "several_key_2");
    my @values = ("value1", "value2");

    my $msg = "";
    for my $i (0 .. @keys - 1) {
        my ($key, $value) = ($keys[$i], $values[$i]);
        $msg .= Server::_set($key, $value);
    }

    if (!$server->socket_write($msg)) {
        say "can not write to socket";
        return;
    }
    
    my $data = $server->recv_data;
    say quote_symbols($data);
}

1;

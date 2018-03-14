package GetSetTest;

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

    my $key = "key";
    my $value = "value";

    my $data = $server->set($key, $value);
    if ($data ne "STORED") {
        say "can not stored";
        return 0;
    }

    $data = $server->get($key);
    $server->close;
    if ($data eq "VALUE $key 0 ".length($value)."\r\n$value\r\nEND") {
        return 1;
    } else {
        say "get not ok", quote_symbols($data);
        return 0;
    }

}

1;


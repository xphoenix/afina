package GetBadKey;

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

    my $data = $server->get("no_key");
    $server->close;
    if ($data eq "END") {
        return 1;
    } else {
        say "get bad key error", quote_symbols($data);
        return 0;
    }

}

1;

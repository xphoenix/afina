package AddLogic;

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

    my $data;
    my $rand_value = rand;
    $data = $server->set("$rand_value.exists_key", "some_value");
    if ($data ne "STORED") {
        say "can not stored" . quote_symbols($data);
        return 0;
    }
    
    $data = $server->add("$rand_value.exists_key", "new_value");
    if ($data ne "NOT_STORED") {
        say "error while add" . quote_symbols($data);
        return 0;
    }

    $data = $server->add("$rand_value.exists_key_no_such", "some_value");
    $server->close;
    if ($data ne "STORED") {
        say "error while add" . quote_symbols($data);
        return 0;
    }

    return 1;
}

1;

package AppendLogic;

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
    my $key = "test_add_key";
    my $value = "some_value";
    $data = $server->set($key, $value);
    if ($data ne "STORED") {
        say "can not stored" . quote_symbols($data);
        return 0;
    }

    $data = $server->get($key);
    if ($data ne "VALUE $key 0 ".length($value)."\r\n$value\r\nEND") {
        say "get not ok", quote_symbols($data);
        return 0;
    }

    my $additive_value = "add_value";
    $data = $server->append($key, $additive_value);
    if ($data ne "STORED") {
        say "can not stored for append: " . quote_symbols($data);
        return 0;
    }

    $data = $server->get($key);
    if ($data ne "VALUE $key 0 ".length($value.$additive_value)."\r\n".$value.$additive_value."\r\nEND") {
        say "get not ok", quote_symbols($data);
        $server->close;
        return 0;
    }

    $data = $server->append("no_key_append", "value");
    $server->close;
    if ($data ne "NOT_STORED") {
        return 0;
    }

    return 1;
}

1;

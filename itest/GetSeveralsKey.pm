package GetSeveralsKey;

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

    my @keys = qw(key1 key2 key3 key4 key5 key6 key7);
    my @values = qw(value1 value2 value3 value4 value5 value6 value7);

    my $data;
    for my $i (0.. @keys - 1) {
        my $key = $keys[$i];
        my $value = $values[$i];
        $data = $server->set($key, $value);
        if ($data ne "STORED\r\n") {
            say "can not stored";
            return 0;
        }
    }

    $data = $server->get(@keys);
    my @res = split(/\r\n/, $data);
    for (my $i = 0; $i < 2 * @keys; $i += 2) {
        my $str = $res[$i].$res[$i + 1];
        my $key = $keys[$i / 2];
        my $value = $values[$i / 2];
        
        if ($res[$i] ne "VALUE $key 0 ".length($value)) {
            say "error while get, data: ", quote_symbols($str);
            $server->close;
            return 0;
        }

        if ($res[$i + 1] ne $value) {
            say "error while get, data: ", quote_symbols($str);
            $server->close;
            return 0;
        }
    }
    
    $server->close;
    return 1;
}

1;

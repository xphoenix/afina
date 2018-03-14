package Utils;

use strict;
use warnings;
use feature qw(say);

use Exporter 'import';

our @EXPORT = qw(quote_symbols);

sub quote_symbols {
    my ($msg) = @_;
    $msg =~ s/\n/\\n/g;
    $msg =~ s/\r/\\r/g;
    return $msg;
}


use strict;
use warnings;

use feature qw(say);

use Term::ANSIColor;

my @tests = qw(
    GetSetTest
    GetBadKey
    GetSeveralsKey
    AddLogic
    AppendLogic
);

my $test_index = 1;
my $ok_test_cnt = 0;
my $bad_test_cnt = 0;
for my $test_name (@tests) {
    print color('yellow');
    say "[ TEST =================== # $test_index START $test_name]";
    print color('reset');
    eval { require "$test_name.pm" } or die $@;
    if ($test_name->main()) {
        print color('green');
        say "[ TEST =================== # $test_index OK ]";
        print color('reset');
        $ok_test_cnt++;
    } else {
        print color('red');
        say "[ TEST =================== # $test_index ERROR ]";
        print color('reset');
        $bad_test_cnt++;
    }
    $test_index++;
    say("");
}

say "RESULT: $ok_test_cnt / ".@tests;
if ($bad_test_cnt == 0) {
    print color('green');
    say "PASSED";
    print color('reset');
} else {
    print color('red');
    say "NOT PASSED";
    print color('reset');
}



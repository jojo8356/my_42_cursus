#!/usr/bin/perl
use strict;
use warnings;
my $name = "unknown";
foreach my $pair (split(/&/, $ENV{'QUERY_STRING'} || '')) {
    my ($k, $v) = split(/=/, $pair, 2);
    if ($k eq 'name') { $name = $v; }
}
my $body = "Hello, $name! (via Perl CGI)";
print "Status: 200 OK\n";
print "Content-Type: text/plain\n";
print "Content-Length: " . length($body) . "\n";
print "\n";
print $body;

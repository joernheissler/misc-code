#!/usr/bin/perl

use strict;

my $in = <STDIN>;
chomp $in;

my %chars;
if($in) {


my @valid = (33..47,58..64,91..96,123..126);
my %val = ();
foreach(@valid) { $val{$_} = 1; }

for(my $x = 32; $x < 127; $x++) {
	$chars{$x} = [];
	foreach my $i(@valid) {
		foreach my $j(@valid) {
			my $a = $i ^ $j ^ $x;
			next unless $val{$a};
			push(@{$chars{$x}},chr($i).chr($j).chr($x ^ $i ^ $j));
		}
	}
}

my $arg = '';
for(my $z = 0; $z < 13; $z++) {
	my $m = chr($valid[int(rand @valid)]);
	if($m eq '\'' || $m eq '\\') {
		$m = '\\'.$m;
	}
	$arg .= $m;
}

my $bla = '';
for(my $z = 0; $z < 7; $z++) {
	my $m = chr($valid[int(rand @valid)]);
	if($m eq '\'' || $m eq '\\') {
		$m = '\\'.$m;
	}
	$bla .= $m;
}

my $out = "'".$arg."'!~".enc('(?{'.'print \''.$in.'\',$/})').'#'.$bla;

print "$out\n";
}

sub enc {
	my $in = shift;
	my($foo,$bar,$baz) = ('','','');
	foreach(split(//,$in)) {
		my $a = $chars{ord($_)};
		my $c = @{$a};
		my $d;
		my($d0,$d1,$d2);	
		do {
			$d = @{$a}[int(rand $c)];
			$d0 = substr($d, 0, 1);
			$d1 = substr($d, 1, 1);
			$d2 = substr($d, 2, 1);
		} while(($d0 eq '\'' || $d0 eq '\\' || $d1 eq '\'' || $d1 eq '\\' || $d2 eq '\'' || $d2 eq '\\') && (rand(3) >= 1));
		
		$foo .= (($d0 eq '\\') ? '\\\\' : ($d0 eq '\'') ? '\\\'' : $d0);
		$bar .= (($d1 eq '\\') ? '\\\\' : ($d1 eq '\'') ? '\\\'' : $d1);
		$baz .= (($d2 eq '\\') ? '\\\\' : ($d2 eq '\'') ? '\\\'' : $d2);
	}
	return "('$foo'^'$bar'^'$baz')";
}

